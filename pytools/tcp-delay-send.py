import os, sys, time
import io
import argparse
import socketserver
from datetime import datetime,tzinfo,timedelta,timezone

args = None

socketserver.ThreadingMixIn.daemon_threads = True

hms_pattern = b"[hh:mm:ss.000]"

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
        # at moment TCP connection is *accepted* .

        sendfile = args.sendfile
        specs = args.delayspecs

        self.idxchunk = 0
        self.prefix = "[{}:{}]".format(self.client_address[0], self.client_address[1])
        self.print_one_chunk(None, "Client connected.")

        fh = None
        if sendfile:
            fh = open(sendfile, 'rb')
        else:
            fh = io.BytesIO(default_tcp_response_bytes)

        with fh:
            for spec in specs:
                is_continue = self.send_chunk(fh, spec)
                if not is_continue:
                    break

            bytes_to_send = fh.read()
            nbyte_to_send = len(bytes_to_send)
            if nbyte_to_send>0:
                tsprefix, bytes_to_send = self.translate_bytes2send(bytes_to_send)
                self.print_one_chunk(tsprefix,
                                     "send final %d bytes"%(nbyte_to_send),
                                     bytes_to_send)
                self.request.sendall(bytes_to_send)

    @staticmethod
    def dtnow_prefix():
        dtnow = datetime.now()
        tsprefix = dtnow.strftime('%H:%M:%S.') + dtnow.strftime('%f')[0:3] # 08:30:00.123
        tsprefix = '['+tsprefix+']'
        return tsprefix

    def translate_bytes2send(self, bytes_to_send):
        tsprefix = __class__.dtnow_prefix()
        tsnow = bytes(tsprefix, 'ascii')

        if args.inject_timestamp:
            assert (len(hms_pattern) == len(tsnow))
            bytes_to_send = bytes_to_send.replace(hms_pattern, tsnow)

        return (tsprefix, bytes_to_send)

    def send_chunk(self, fh, spec):
        sbyte, sdelay = spec.split(',')

        if sbyte.endswith('kb'):
            nbyte = int(sbyte[0:-2])
        elif sbyte.endswith('b'):
            nbyte = int(sbyte[0:-1])
        else:
            nbyte = int(sbyte)

        if sdelay.endswith('ms'):
            delay_sec = int(sdelay[0:-2])/1000
        elif sdelay.endswith('s'):
            delay_sec = int(sdelay[0:-1])
        else:
            delay_sec = int(sdelay)

        if nbyte==0:
            self.print_one_chunk(None, "delay %gs"%(delay_sec))
            time.sleep(delay_sec)
            return True
        else:
            bytes_to_send = fh.read(nbyte)
            nbyte_to_send = len(bytes_to_send)

            if nbyte_to_send>0:
                tsprefix, bytes_to_send = self.translate_bytes2send(bytes_to_send)
                self.print_one_chunk(tsprefix,
                                     "send %db, delay %gs"%(nbyte_to_send, delay_sec),
                                     bytes_to_send)
                self.request.sendall(bytes_to_send)

                if delay_sec > 0:
                    time.sleep(delay_sec)
                return True

            else:
                self.print_one_chunk(None, "all bytes sent")
                return False

    def print_one_chunk(self, tsprefix, info, bytes_to_send=None):
        if not tsprefix:
            tsprefix = __class__.dtnow_prefix()

        self.idxchunk += 1
        print("%s%s#%d %s"%(tsprefix, self.prefix, self.idxchunk, info))

        nbyte2dump = args.dump_bytes
        if nbyte2dump>0 and bytes_to_send:
            # If all bytes are printable ascii, we print ascii,
            # otherwise, we do hex dump.
            dumpbytes = bytes_to_send[0:nbyte2dump]
            str_to_print = ''
            try:
                str_to_print = dumpbytes.decode('ascii')
                if not str_to_print.isprintable():
                    str_to_print = "(HEX) " + hexdmp(dumpbytes)
            except ValueError as e: # including UnicodeDecodeError
                str_to_print = "(HEX) "+ hexdmp(dumpbytes)
            print("    "+str_to_print)

def my_parse_args():

    ap =argparse.ArgumentParser(
        description='This is a TCP server that sends response to client with delays.'
    )

    ap.add_argument('-p', dest='port', type=int, required=True,
        help='TCP listen port.'
    )

    ap.add_argument('-f', dest='sendfile', type=str,
        help='The content of this file will be sent to clients. '
            'If not provided, an piece of internal content will be used.'
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

    nheader = default_tcp_response_bytes.find(b'\r\n\r\n')
    assert(nheader>0)
    nheader += 4
    ap.add_argument('delayspecs', type=str, nargs='*',
        help='Assign one or more delay parameters.\n'
            'For example: "{nheader},500ms" means sending {nheader} bytes then delay 500 milliseconds(as a chunk). '
            'You can pass in multiple delay-params, separated by a space on the command line, '
            'so that chunks of bytes will be sent sequentially. If there are remaining bytes, '
            'those will be sent as the final chunk.'.format(nheader=nheader)
    )

    try:
        args = ap.parse_args()
    except SystemExit as e:
        # User has requested -h/--help to print help message.
        # We print addition example usages here. We print it ourselves so that \n can be preserved.
        example = """
Usage examples:

    tcp-delay-send.py -p 8800 -t 0,100ms 91,900ms 16,1s
"""
        print(example, end='')
        raise
    return args

def main():
    global args
    args = my_parse_args()

    with socketserver.ThreadingTCPServer(("0.0.0.0", args.port), MyTCPHandler) as server:

        print("Delay-send TCP server started on port %d..."%(args.port))

        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()

def sanity_check():
	global default_tcp_response
	if b'\r' in default_tcp_response:
		raise ValueError(r'PANIC! There should not be 0x0D(\r) byte in default_tcp_response string.')
	
if __name__=="__main__":
	sanity_check()
	ret = main()
	exit(ret)

"""
python tcp-delay-send.py -p 8800 -t 0,100ms 91,1000ms 16,1000ms
"""
