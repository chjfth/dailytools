import sys, socket

if len(sys.argv)<3:
	print("Usage: tcpcli.py <remote-host> <port> [text-to-send]")
	sys.exit(1)

dst_ip = sys.argv[1]
dst_port = int(sys.argv[2])
sendtext = "TextABC" if len(sys.argv)<4 else sys.argv[3]

print("Connecting...")

s1 = socket.socket(socket.AF_INET)
s1.connect((dst_ip, dst_port)) # dst_ip can be a hostname

print("server-side", s1.getpeername())
print("client-side", s1.getsockname())

s1.send(bytes(sendtext, 'ascii'))
input("Press Enter to close connection.")

print("Recving...")
rbytes = s1.recv(10)

print("Received bytes: %d"%(len(rbytes)))
print(rbytes)

