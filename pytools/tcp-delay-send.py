import os, sys, time
import io
import argparse
import socket, socketserver
from datetime import datetime,tzinfo,timedelta,timezone

verstr = 'v1.0'

args = None

socketserver.ThreadingMixIn.daemon_threads = True

g_default_servport = 8000
gen_http_response_filename = "WHR.txt"
gen_http_response_filename_chunked = "WHRC.txt"

hms_pattern = b"{hh:mm:ss.000}"

default_tcp_response = b"""\
HTTP/1.0 200 OK
Server: My simple server
Content-type: text/plain
Content-Length: 29

%s
Hello, world!"""%(hms_pattern)

default_tcp_response_bytes = default_tcp_response.replace(b'\n', b'\r\n')

hexdmp = lambda bytes : ' '.join('{:02X}'.format(x) for x in bytes)

class MyTCPHandler(socketserver.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """

    def handle(self):
        # self.request is the TCP socket connected to the client

        # Chj: This is called from BaseRequestHandler.__init__(),
        # at the moment TCP connection is *accepted* .

        sendfile = args.sendfile
        specs = args.delayspec
        msec_wait_first_byte = args.msec_wait_first_byte

        self.idxchunk = 0
        self.prefix = "[{}:{}]".format(self.client_address[0], self.client_address[1])
        self.print_one_chunk(None, "Client connected.")

        fh = None
        if sendfile:
            fh = open(sendfile, 'rb')
        else:
            fh = io.BytesIO(default_tcp_response_bytes)

        sec_start = time.monotonic()

        with fh:
            try: # around tcp connection
                if msec_wait_first_byte>0:
                    try:
                        self.request.settimeout(msec_wait_first_byte/1000)
                        rbytes = self.request.recv(4096)
                        sec_elapsed = time.monotonic() - sec_start
                        if len(rbytes)>0:
                            self.print_one_chunk(None, 'Seen first-byte from client in %gs (%d bytes)'%(
                                sec_elapsed, len(rbytes)
                            ))
                        else:
                            self.print_one_chunk(None, 'Client is closing TCP connection.')
                    except socket.timeout:
                        self.print_one_chunk(None,
                            'Timeout! No bytes received from client in %dms.'%(msec_wait_first_byte))
                    finally:
                        self.request.settimeout(None)

                is_final_sent = False
                for spec in specs:
                    is_final_sent = self.send_chunk(fh, spec)
                    if is_final_sent:
                        break

                if not is_final_sent:
                    bytes_to_send = fh.read()
                    self.print_and_send_one_chunk(bytes_to_send, 0, True)

                if args.is_clean_tcp:
                    self.discard_incoming()

            except ConnectionError:
                self.print_one_chunk(None, "TCP connection lost.")

    def discard_incoming(self):
        self.request.settimeout(0) # non-blocking mode
        try:
            while True:
                rbytes = self.request.recv(4096)
                nbytes = len(rbytes)
                if nbytes>0:
                    info = "Discarded incoming bytes %d"%(nbytes)
                    self.print_one_chunk(None, info)
                else:
                    assert nbytes==0
                    info = "The peer has closed TCP; we are in CLOSE_WAIT(half-close). No more to discard."
                    self.print_one_chunk(None, info)
                    break

        except BlockingIOError: # when no byte in TCP-read-buffer
            pass

    def send_chunk(self, fh, spec):
        # Return(bool): is final chunk sent

        try:
            sbyte, sdelay = spec.split(',')
        except ValueError:
            # in case the spec does not contain a comma, or contains more than one comma
            print('Warning: Bad-format spec "%s" ignored.'%(spec))
            return False

        if sbyte.endswith('k'):
            nbyte = int(sbyte[0:-1]) * 1000
        else:
            nbyte = int(sbyte)

        if sdelay.endswith('ms'):
            delay_sec = int(sdelay[0:-2])/1000
        elif sdelay.endswith('s'):
            delay_sec = int(sdelay[0:-1])
        else:
            delay_sec = int(sdelay)

        if nbyte==0:
            # just pure sleep
            self.print_one_chunk(None, "delay %gs"%(delay_sec))
            time.sleep(delay_sec)
            return False
        else:
            bytes_to_send = fh.read(nbyte)
            return self.print_and_send_one_chunk(bytes_to_send, delay_sec)

    def print_and_send_one_chunk(self, bytes_to_send, delay_sec, is_final=False):
        # Note: bytes_to_send can be empty
        # Return(bool): is final chunk sent
        nbyte_to_send = len(bytes_to_send)
        if nbyte_to_send > 0:
            if not is_final:
                text = "send %db, delay %gs"%(nbyte_to_send, delay_sec)
            else: # final chunk
                text = "send remaining %db" % (nbyte_to_send)

            tsprefix, bytes_to_send = self.translate_bytes2send(bytes_to_send)

            self.print_one_chunk(tsprefix, text, bytes_to_send)
            self.sendall_in_parts(bytes_to_send)

            if delay_sec>0:
                time.sleep(delay_sec)

        if nbyte_to_send==0 or is_final:
            self.print_one_chunk(None, "all bytes sent, closing TCP")
            return True
        else:
            return False

    def sendall_in_parts(self, bytes_to_send):
        nbytes = len(bytes_to_send)
        assert nbytes>0

        split_size = args.splitsize
        if split_size==0:
            nparts = 1
        else:
            nparts = (nbytes-1)//split_size +1
            nbytes_sent = 0

        if nparts==1:
            self.request.sendall(bytes_to_send)

        else:
            for i in range(nparts):
                send_size = min(split_size, nbytes-nbytes_sent)
                self.print_one_chunk(None, '  sending part.%d, %d bytes'%(i, send_size))
                self.request.sendall(bytes_to_send[nbytes_sent:nbytes_sent+send_size])
                nbytes_sent += send_size

    @staticmethod
    def dtnow_prefix():
        dtnow = datetime.now()
        tsprefix = dtnow.strftime('%H:%M:%S.') + dtnow.strftime('%f')[0:3] # 08:30:00.123
        return tsprefix

    def translate_bytes2send(self, bytes_to_send):
        tsprefix_bare = __class__.dtnow_prefix()
        tsnow_bare = bytes(tsprefix_bare, 'ascii')

        if args.inject_timestamp:
            tsnow_with_brackets = b'{'+tsnow_bare+b'}'
            assert (len(hms_pattern) == len(tsnow_with_brackets))
            bytes_to_send = bytes_to_send.replace(hms_pattern, tsnow_with_brackets)

        return ('['+tsprefix_bare+']', bytes_to_send)

    s_last_print_sec = 0

    def print_one_chunk(self, tsprefix, info, bytes_to_send=None):

        # print time-gap line.
        # If this print is N seconds away from previous print, a line of N dots will be printed(max 10 dots).
        #
        now_sec = time.monotonic()
        gap_seconds = int(now_sec - __class__.s_last_print_sec)
        if gap_seconds>0 and __class__.s_last_print_sec!=0:
            print('.'*min(10, gap_seconds))
        __class__.s_last_print_sec = now_sec

        if not tsprefix:
            tsprefix = '['+__class__.dtnow_prefix()+']'

        self.idxchunk += 1
        print("%s%s#%d %s"%(tsprefix, self.prefix, self.idxchunk, info))

        # verbose byte-dumping below
        nbyte2dump = args.dump_bytes
        if nbyte2dump>0 and bytes_to_send:
            # If all bytes are printable ascii, we print ascii,
            # otherwise, we do hex dump.
            dumpbytes = bytes_to_send[0:nbyte2dump]
            str_to_print = ''
            try:
                str_to_print = dumpbytes.decode('ascii')
                if not __class__.can_print(str_to_print):
                    str_to_print = "(HEX) " + hexdmp(dumpbytes)
            except ValueError as e: # including UnicodeDecodeError
                str_to_print = "(HEX) "+ hexdmp(dumpbytes)
            print("    "+str_to_print)

    @staticmethod
    def can_print(s):
        return all([c.isprintable() or c in "\r\n" for c in s])

def ensure_non_negative_int(argval):
    i = int(argval)
    if i < 0:
        raise argparse.ArgumentTypeError('Negative values are not allowed')
    return i

def ensure_integer_ms(argval):
    if argval.endswith('ms'):
        return int(argval[:-2])
    else:
        raise argparse.ArgumentTypeError(
            'Argument value("%s") is in wrong format, should be sth like 1000ms.'%(argval)
        )

def yield_http_response(http_content_length, is_chunked):
    if not is_chunked:
        httpheaders0 = b"""\
HTTP/1.0 200 OK
Server: My simple server
Content-Type: text/plain
Content-Length: %d

"""%(http_content_length)
    else:
        httpheaders0 = b"""\
HTTP/1.1 200 OK
Server: My simple server
Content-type: text/plain
Transfer-Encoding: Chunked

"""
    httpheaders = httpheaders0.replace(b'\n', b'\r\n')
#   header_length = len(httpheaders)

    yield httpheaders

    # Now generating http body, each line 100 bytes.
    bytes_per_line = 100
    dot_pattern = '........'
    assert len(dot_pattern)==8
    totlines = (http_content_length-1)//bytes_per_line + 1

    for iline in range(totlines):

        # construct a line
        linetext = '#'  # append later
        for cval in range(ord('A'), ord('A')+12):
            cfill = '%d%s'%(iline, chr(cval))
            clen = len(cfill)
            snippet = cfill + dot_pattern[clen:]
            linetext += snippet
        linetext += '#\r\n'

        if iline < totlines-1:
            seg_payload = linetext.encode('ascii')
            if not is_chunked:
                yield seg_payload
            else:
                yield b'%x\r\n%s\r\n'%(len(linetext), seg_payload)
        else:
            remain = http_content_length % bytes_per_line
            if remain==0:
                remain = bytes_per_line

            seg_payload = linetext[:remain].encode('ascii')
            if not is_chunked:
                yield seg_payload
            else:
                yield b'%x\r\n%s\r\n'%(remain, seg_payload)

    if is_chunked:
        # Add last-chunk
        yield b'0\r\n\r\n'


def generate_http_response_file(content_length, is_chunked):
    filename = gen_http_response_filename if not is_chunked else gen_http_response_filename_chunked
    with open(filename, 'wb') as fh:
        for btext in yield_http_response(content_length, is_chunked):
            fh.write(btext)
    print("File generated: %s"%(filename))


def my_parse_args():

    ap =argparse.ArgumentParser(
        description='[%s] This is a TCP server that sends response to client with delays.'%(verstr)
    )

    ap.add_argument('-p', dest='port', type=int, default=8000,
        help='TCP listen port. Default is %d'%(g_default_servport)
    )

    ap.add_argument('-w', dest='msec_wait_first_byte', type=ensure_integer_ms, metavar='ms', default=0,
        help='Tells how many millisec to wait for first received byte before sending out contents. '
            'If the TCP client is an HTTP client, I suggest "-w 1000ms", so to avoid sending out '
            'HTTP respone prematurely before HTTP request is issued by client. '
            'Default is 0ms, no waiting.'
    )

    ap.add_argument('-f', dest='sendfile', type=str,
        help='The content of this file will be sent to clients. '
            'If not provided, an piece of internal content will be used.'
    )

    ap.add_argument('-s', dest='splitsize', type=int, default=0,
        help='Split a big-size TCP send API call into smaller sendsize. '
            'This tells each small sendsize, in bytes.'
    )

    ap.add_argument('-t', dest='inject_timestamp', action='store_true',
        help='Check for "{}" in each sent chunk, if present, replace it with current time. '
            'With this time substitution feature, client will receive different bytes each time.'.format(
            hms_pattern.decode('ascii'))
    )

    ap.add_argument('-d', dest='dump_bytes', type=int, default=0,
        help='For each send, print count of DUMP_BYTES to console, so that user can verify '
            'whether each chunk position is correct. Default to 0, no dump print.'
    )

    ap.add_argument('--clean-tcp', dest='is_clean_tcp', action='store_true',
        help='Before closing TCP, I will clear TCP read buffer(read and discard all bytes). '
            'If there are unread bytes, our TCP stack will set RST to client, which is probably not desired.'
    )

    ap.add_argument('--WHR', type=ensure_non_negative_int, metavar='bodybytes', default=0,
        help='Write a sample Http Response to disk file. The arg-value assigns its HTTP Content-Length. '
            'This file is named %s.'%(gen_http_response_filename)
    )
    ap.add_argument('--WHRC', type=ensure_non_negative_int, metavar='bodybytes', default=0,
        help='Similar to --WHRC, but in `Transfer-Encoding: Chunked` form. '
            'The file is named %s.'%(gen_http_response_filename_chunked)
    )
    nheader = default_tcp_response_bytes.find(b'\r\n\r\n')
    assert(nheader>0)
    nheader += 4
    ap.add_argument('delayspec', type=str, nargs='*',
        help='Assign one or more delay parameters.\n'
            'For example: "{nheader},500ms" means sending {nheader} bytes then delay 500 milliseconds(as a chunk). '
            'You can pass in multiple delay-params, separated by a space on the command line, '
            'so that chunks of bytes will be sent sequentially. If there are remaining bytes, '
            'those will be sent as the final chunk.'.format(nheader=nheader)
    )

    if len(sys.argv)==1:
        # When no argument given, just print help and exit, so that user knows how to use it.
        sys.argv.append('-h')

    try:
        args = ap.parse_args()
    except SystemExit as e:
        # This may due to user requesting -h/--help to print help message, or,
        # argparse encounters a parsing error(wrong parameter format etc).
        if '-h' in sys.argv or '--help' in sys.argv:
            # For the -h/--help case,
            # we print addition example usages here. We print it ourselves so that \n can be preserved.
            example = """
Usage examples:
    tcp-delay-send.py -p 8000
    tcp-delay-send.py 0,100ms 91,900ms 16,1s
    tcp-delay-send.py 0,100ms 91,900ms 8,0s 8,1s 1111,5s
    
    tcp-delay-send.py --WHR=50000 --WHRC=50000
    tcp-delay-send.py 10k,2s 10k,2s  -f WHR.txt -w 1000ms --clean-tcp 
"""
            print(example, end='')
        raise

    is_exit_now = False

    if args.WHR>0:
        generate_http_response_file(args.WHR, False)
        is_exit_now = True

    if args.WHRC>0:
        generate_http_response_file(args.WHRC, True)
        is_exit_now = True

    if is_exit_now:
        exit(0)

    if args.sendfile and not os.path.isfile(args.sendfile):
        print('ERROR: The file does not exist: %s'%(args.sendfile))
        exit(4)

    return args

def main():
    global args
    args = my_parse_args()

    server = socketserver.ThreadingTCPServer(("0.0.0.0", args.port),
            MyTCPHandler,
            bind_and_activate=False)
    with server:

        try:
            server.server_bind()
        except OSError as e:
            print("Fail to bind to TCP listen port %d ."%(args.port))
            print("  %s"%(e))
            print("Suggestion: Using -p <port> to select a different listen port.")
            return 4

        server.server_activate()

        print("Delay-send TCP server started on port %d ..."%(args.port))

        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()

    return 0

def sanity_check():
	global default_tcp_response
	if b'\r' in default_tcp_response:
		raise ValueError(r'PANIC! There should not be 0x0D(\r) byte in default_tcp_response string.')
	
if __name__=="__main__":
	sanity_check()
	ret = main()
	exit(ret)

