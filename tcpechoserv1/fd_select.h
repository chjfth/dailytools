#ifndef __fd_select_h__20040804
#define __fd_select_h__20040804

// ! for Win32's select(), the meaning of its first parameter is different from
//that of UNIX, therefore, I'm afraid it can only be used on Win32.
// Since we only wait for one socket, maybe I can use `sock' for select's 1st param

/* select_for_read:
	Return: 
		>0 if socket becomes readable,
		0 if timeout,
		<0 if error occurs.
*/
template<class socket_t>
int select_for_read(socket_t sock, int timeout)
{
	// timeout: in milliseconds

	fd_set fd_read;
	FD_ZERO(&fd_read);
	FD_SET(sock, &fd_read);

	timeval tv_read = { 0, 0 };
	
	if(timeout>0)
	{
		tv_read.tv_sec = timeout/1000; 
		tv_read.tv_usec = timeout%1000 * 1000;
	}
	
	int nAvai = select(0, &fd_read, NULL, NULL, timeout>=0 ? &tv_read : NULL);
	
	return nAvai;
}


template<class socket_t>
int select_for_write(socket_t sock, int timeout)
{
	fd_set fd_write;
	FD_ZERO(&fd_write);
	FD_SET(sock, &fd_write);
	
	timeval tv_write = { 0, 0 };

	if(timeout>0)
	{
		tv_write.tv_sec = timeout/1000;
		tv_write.tv_usec = timeout%1000 * 1000;
	}

	int nAvai = select(0, NULL, &fd_write, NULL, timeout>=0 ? &tv_write : NULL);

	return nAvai;
}


#endif
