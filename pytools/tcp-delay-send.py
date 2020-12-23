import os, sys
import argparse
import socketserver

args = None

class MyTCPHandler(socketserver.BaseRequestHandler):
    """
    The request handler class for our server.

    It is instantiated once per connection to the server, and must
    override the handle() method to implement communication to the
    client.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        print("My __init__() called")

    def handle(self):
        # self.request is the TCP socket connected to the client
        self.data = self.request.recv(1024).strip()
        print("{} wrote:".format(self.client_address[0]))
        print(self.data)
        # just send back the same data, but upper-cased
        self.request.sendall(self.data.upper())
    

def my_parse_args():

    ap =argparse.ArgumentParser(
        description='This is a TCP server that sends response to client with delays.'
    )

    ap.add_argument('-p', '--port', type=int, required=True,
        help='TCP listen port.'
    )

    ap.add_argument('-f', '--sendfile', type=str,
        help='The content of this file will be sent to clients. '
            'If not provided, an piece of internal content will be used.'
    )

    ap.add_argument('delayspec', type=str, nargs='*',
        help='Assign one or more delay parameters.\n'
            'For example: "1000,500ms" means sending 1000 bytes then delay 500 milliseconds. '
            'You can pass in multiple delay-params, separated by a space on the command line, '
            'so that chunks of bytes will be sent sequentially until all file is sent.'
    )

    args = ap.parse_args()
    return args

def main():
    global args
    args = my_parse_args()

    # Create the server, binding to localhost on port 9999
    with socketserver.ThreadingTCPServer(("0.0.0.0", args.port), MyTCPHandler) as server:
        # Activate the server; this will keep running until you
        # interrupt the program with Ctrl-C
        server.serve_forever()

if __name__=="__main__":
	ret = main()
	exit(ret)

