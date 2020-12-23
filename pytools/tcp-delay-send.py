import os, sys, time
import io
import argparse
import socketserver

args = None

socketserver.ThreadingMixIn.daemon_threads = True

default_tcp_response = b"""\
HTTP/1.0 200 OK
Server: My simple server
Content-type: text/plain
Content-Length: 13

Hello, world!"""

default_tcp_response_bytes = default_tcp_response.replace(b'\n', b'\r\n')

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

#       self.data = self.request.recv(1024).strip()
        prefix = "[{}:{}]".format(self.client_address[0], self.client_address[1])
        print(prefix+"Client connected.")

        fh = None
        if sendfile:
            fh = open(sendfile, 'rb')
        else:
            fh = io.BytesIO(default_tcp_response_bytes)

        with fh:
            for spec in specs:
                is_continue = self.send_chunk(fh, prefix, spec)
                if not is_continue:
                    break

            bytes = fh.read()
            nbyte_to_send = len(bytes)
            if nbyte_to_send>0:
                print("%s[send final %d bytes]"%(prefix, nbyte_to_send))
                self.request.sendall(bytes)

    def send_chunk(self, fh, prefix, spec):
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
            print("%s[delay %gs]" % (prefix, delay_sec))
            time.sleep(delay_sec)
            return True
        else:
            bytes = fh.read(nbyte)
            nbyte_to_send = len(bytes)

            if nbyte_to_send>0:
                print("%s[send %db, delay %gs]"%(prefix, nbyte_to_send, delay_sec))
                self.request.sendall(bytes)

                if delay_sec > 0:
                    time.sleep(delay_sec)
                return True

            else:
                print("%s[all bytes sent]"%(prefix))
                return False

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

    args = ap.parse_args()
    return args

def main():
    global args
    args = my_parse_args()

    with socketserver.ThreadingTCPServer(("0.0.0.0", args.port), MyTCPHandler) as server:
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
TODO: For default_tcp_response, Linux source will give only \n as separator, right? If so, make it all \r\n .

TODO: With delay spec `0,100ms 86,2s`, why Chrome 87 shows TTFB is 2.1s? I think it should be 0.1s .
"""