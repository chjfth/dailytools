# Guided by AI
import os, sys
import socket

prgname = os.path.basename(sys.argv[0])

def start_echo_server(host, port):
    # Create a TCP/IP socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    
    # Bind the socket to the address and port
    server_address = (host, port)
    server_socket.bind(server_address)
    
    # Listen for incoming connections
    server_socket.listen(5)
    print(f"TCP-echo server listening on {host}:{port}")
    
    while True:
        # Wait for a connection
        client_socket, client_address = server_socket.accept()
        try:
            print(f"Connection from {client_address}")
            
            while True:
                # Receive the data in small chunks
                data = client_socket.recv(1024)
                if data:
                    # Echo the received data back to the client
                    print(f"Received data: {data.decode('utf-8')}")
                    client_socket.sendall(data)
                else:
                    # No more data from the client
                    break
        finally:
            # Clean up the connection
            client_socket.close()
            print(f"Connection closed from {client_address}")

if __name__ == "__main__":

	nargs = len(sys.argv)
	if nargs!=3:
		print(f"TCP echo server. Usage:")
		print(f"    {prgname} localhost 1234")
		print(f"    {prgname} 0.0.0.0  12345")
		sys.exit(1)
	
	start_echo_server(sys.argv[1], int(sys.argv[2]))
