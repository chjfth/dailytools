#!/usr/bin/env python3
#coding: utf-8

# Guided by AI
import os, sys
import http.server
import socketserver

prgname = os.path.basename(sys.argv[0])

class CustomHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def end_headers(self):
        # 允许跨域请求（可选，根据需要启用）
        self.send_header('Access-Control-Allow-Origin', '*')
        http.server.SimpleHTTPRequestHandler.end_headers(self)

def start_http_server(listen_addr, port):
	# 创建一个 HTTP 服务器，使用上面定义的请求处理器
	with socketserver.TCPServer((listen_addr, port), CustomHTTPRequestHandler) as httpd:
	    print(f"Serving on {listen_addr}:{port} ...")
	    # 运行服务器，直到用户中断
	    try:
	        httpd.serve_forever()
	    except KeyboardInterrupt:
	        print("Server stopped by user.")


if __name__ == "__main__":
	nargs = len(sys.argv)
	if nargs!=3:
		print(f"HTTP serving current dir. Usage:")
		print(f"    {prgname} localhost 8010")
		print(f"    {prgname} 0.0.0.0   8010")
		sys.exit(1)
	
	start_http_server(sys.argv[1], int(sys.argv[2]))
