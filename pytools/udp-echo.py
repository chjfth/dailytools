# Guided by AI
import os, sys
import socket

prgname = os.path.basename(sys.argv[0])

def start_udp_echo_server(ip, port):
	# Create a datagram socket
	sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

	# Bind the socket to the port
	server_address = (ip, port)
	sock.bind(server_address)

	while True:
		print(f"\nWaiting to receive UDP message on port {port}")
		data, address = sock.recvfrom(4096)

		print(f"Received {len(data)} bytes from {address}")
		print(data)

		if data:
			sent = sock.sendto(data, address)
			print(f"Sent {sent} bytes back to {address}")

if __name__ == "__main__":
	
	nargs = len(sys.argv)
	if nargs!=3:
		print(f"UDP echo server. Usage:")
		print(f"    {prgname} localhost 1234")
		print(f"    {prgname} 0.0.0.0  12345")
		sys.exit(1)
	
	start_udp_echo_server(sys.argv[1], int(sys.argv[2]))

