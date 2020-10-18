#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include <windows.h>

#include <process.h>
#include <commctrl.h>
#include <tchar.h>

#include <stdio.h>

#include "commdefs.h"

#include "fd_select.h"

int g_nodata_timeout_sec = 0; // default: timeout infinitely
int g_cur_connections = 0;
int g_isRecvToStderr = NO;
int g_isLogToFile = NO;

static char *
IPv4Addr2Str(const sockaddr_in &addr, char str[24])
{
	sprintf(str, "%s:%d", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
	return str;
}

static char * 
GetPeerNameStr(SOCKET sock, char str[24])
{
	str[0] = '\0';

	sockaddr_in addr_peer;
	int len = sizeof(addr_peer);
	if( getpeername(sock, (sockaddr*)&addr_peer, &len) == NOERROR_0 )
	{
		IPv4Addr2Str(addr_peer, str);
	}
	return str;
}

static char * 
GetTimeStr(char *buf, int bufsize)
{
	buf[bufsize-1] = '\0'; // Do this since _snprintf does not append '\0' is buffer would overflow.
	SYSTEMTIME st;
	GetLocalTime(&st);
	_snprintf(buf, bufsize-1, "%02d-%02d %02d:%02d:%02d.%03d",
		st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, 
		st.wMilliseconds);
	return buf;
}

static
FILE *
CreateFileByTime(const SYSTEMTIME &st)
{
	char szFileName[64]; // record the accepting time.
	_snprintf(szFileName, sizeof szFileName, "cli%04d%02d%02d-%02d%02d%02d.%03d",
		st.wYear, st.wMonth, st.wDay, 
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	FILE *fp = fopen(szFileName, "wb+");
	return fp;
}

struct SEsParam
{
	SOCKET sock_conn;
	Ulong thread_id;
	Ulong recv_bytes;
	Ulong msec_start;
};

unsigned int __stdcall _EchoServer(void *pParam)
{
	SEsParam *pEsp = (SEsParam*)pParam;
	SOCKET sock = pEsp->sock_conn;

	SYSTEMTIME st;
	GetLocalTime(&st);

	FILE *fp = NULL;

	int nRd = 0;
	char rbuf[2048];
	char tbuf[24];
	char timebuf[40];

	for(;;)
	{
		int nRd = select_for_read(sock, 
			g_nodata_timeout_sec ? g_nodata_timeout_sec*1000 : -1);
			// Use no timeout value(wait infinitely) if g_nodata_timeout_sec==0.
		if(nRd==0)
		{
			g_cur_connections--;
			printf("[%d]{%s} %s no-data timeout: %d seconds. Close it.", 
				g_cur_connections, GetTimeStr(timebuf, sizeof(timebuf)),
				GetPeerNameStr(sock, tbuf), g_nodata_timeout_sec);
			break;
		}

		nRd = recv(sock, rbuf, sizeof(rbuf)-1, 0);
			// -1: Leave one byte for adding null-terminator 
		if(nRd<=0)
		{
			g_cur_connections--;

			printf("[%d]{%s} %s closed(%d).", g_cur_connections, 
				GetTimeStr(timebuf, sizeof(timebuf)),
				GetPeerNameStr(sock, tbuf), nRd);
			break;
		}

		pEsp->recv_bytes += nRd;

		// Write the data to STDERR
		if(g_isRecvToStderr)
		{
			rbuf[nRd] = '\0';
			fwrite(rbuf, 1, nRd, stderr);
		}

		// Write the data to file
		if(g_isLogToFile)
		{
			if(!fp)
				fp = CreateFileByTime(st);
			if(fp)
			{
				int nWr = fwrite(rbuf, 1, nRd, fp);
				if(nWr!=nRd)
					printf("*Write file error!\n");
			}// if(fp)
		}

		// echo the data:
		send(sock, rbuf, nRd, 0);

	}// for(;;)
	
	Ulong msec_elapsed = GetTickCount() - pEsp->msec_start;

	printf(" recv-total: %d ", pEsp->recv_bytes);
	if(msec_elapsed>0)
	{
		printf("(%d.%03dKB/s)", pEsp->recv_bytes/msec_elapsed, 
			(pEsp->recv_bytes*1000)/msec_elapsed%1000);
	}
	else 
		printf("in 0 millisecond");

	printf(".\n");

	closesocket(sock);
	
	if(fp)
		fclose(fp);

	delete pEsp;
	return 0;
}

void sockserv(SOCKET sock_listen, Ushort listen_port)
{
	int re = 0;
	char tbuf[128];
	char timebuf[40];

	sockaddr_in addr_listen = {0};
	addr_listen.sin_family = AF_INET;
	addr_listen.sin_port = htons(listen_port);

	re = bind(sock_listen, (sockaddr*)&addr_listen, sizeof(addr_listen));
	if(re!=0)
	{
		printf("bind() to port %d fail!", listen_port);
		return;
	}

	re = listen(sock_listen, 5);
	if(re!=0)
	{
		printf("listen() fail! port = %d.", listen_port);
		return;
	}

	printf("listening on port %d.\n", listen_port);

	for(;;)
	{
		sockaddr_in addr_peer = {0};
		int peer_len = sizeof(addr_peer);

		// Wait accept() as well as keyboard hit
		
		for(;;)
		{
			int key;
			int n = select_for_read(sock_listen, 1000);

			// Check key-press
			if( (key=kbhit()) != 0 )
			{
				key = getch();
				if(key==27)
					goto END;
			}

			if(n>0)
				break; // to accept
			else if(n<0) // error occurs
			{
				printf("select() error!");
				goto END;
			}
			else
				continue; // continue to wait
		}

		SOCKET sock_conn = accept(sock_listen, (sockaddr*)&addr_peer, &peer_len);
		
		g_cur_connections++;
		printf("[%d]{%s} %s accepted(SOCKET=0x%X)\n", g_cur_connections,
			GetTimeStr(timebuf, sizeof(timebuf)),
			IPv4Addr2Str(addr_peer, tbuf), sock_conn);

		// Create a thread to serve the client.

		SEsParam *pEsp = new SEsParam;
		assert(pEsp);

		pEsp->sock_conn = sock_conn;
		pEsp->recv_bytes = 0;
		pEsp->msec_start = GetTickCount();

		HANDLE hThread = (HANDLE)
			_beginthreadex(NULL, 8192, _EchoServer, pEsp, 0, (unsigned int*)&pEsp->thread_id);
		if(hThread==NULL)
		{
			printf("Unexpected: Thread creation fail!\n");
			break;
		}

		CloseHandle(hThread);

	}
END:
	return ;
}

void sockserv_out(Ushort listen_port)
{
	SOCKET sock_listen = socket(AF_INET, SOCK_STREAM, 0);
	assert(sock_listen!=INVALID_SOCKET);

	sockserv(sock_listen, listen_port);

	closesocket(sock_listen);
}

int main(int argc, char *argv[])
{
	printf("Program compile date: %s %s\n", __DATE__, __TIME__);

	if(argc<2)
	{
		printf("Usage: tcpechoserv1 <port> [no-data-timeout(second)] [log] [recv-to-stderr]\n");
		printf("  [no-data-timeout]: If not 0, disconnect client after that idle period.\n");
		printf("  [log]: If present and not 0, every connection results in a log file.\n");
		printf("  [recv-to-stderr]: If 1, Dump received bytes to stderr.\n");
		printf("\n");
		printf("Example:\n");
		printf("  tcpechoserv1 777\n");
		printf("  tcpechoserv1 777 8\n");
		printf("  tcpechoserv1 777 0 1\n");
		printf("  tcpechoserv1 777 0 0 1\n");
		return 1;
	}

	Ushort port = atoi(argv[1]);
	if(port==0)
	{
		printf("port number invalid: %d.\n", port);
		return 1;
	}

	if(argc>=3)
	{
		g_nodata_timeout_sec = atoi(argv[2]);
		if(g_nodata_timeout_sec<0)
		{
			printf("no-data-timeout invalid: %d.\n", g_nodata_timeout_sec);
			return 1;
		}
	}

	if(argc>=4)
	{
		if(atoi(argv[3])>0)
			g_isLogToFile = YES;
	}

	if(argc>=5)
	{
		if(argv[4])
			g_isRecvToStderr = 1;
	}

	int re;
	WSADATA wsa_data;
	re = WSAStartup(0x101, &wsa_data);
	if(re!=0)
	{
		printf("WSAStarup()=%d.\n", re);
		return 1;
	}

	printf("Configuration: listening-port: %d.\n", port); 
	
	if(g_nodata_timeout_sec)	 
		printf("  no-data timeout: %d seconds.\n", g_nodata_timeout_sec);

	if(g_isLogToFile)
		printf("  Will write client data to file.\n");

	printf("(Press ESC to quit.)\n");

	sockserv_out(port);

	return 0;
}
