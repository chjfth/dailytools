# Provided by AI
import sys
import socket

if len(sys.argv)==1:
	listen_port = 7777 # default listen port
else:
	listen_port = int(sys.argv[1])

def start_echo_server(host='0.0.0.0', port=listen_port):
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
    start_echo_server()
