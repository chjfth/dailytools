#include "shared.h"

const TCHAR *
ps_get_os_errstring(int oserr, TCHAR *buf, int bufchars)
{
	buf[0] = 0;
	snprintf(buf, bufchars-1, "[errno=%d] %s", oserr, strerror(oserr));

	return buf;
}

const TCHAR *OsErrStr()
{
	static TCHAR buf[400];
	return ps_get_os_errstring(errno, buf, ARRAYSIZE(buf));
}

FHANDLE 
ps_openfile(const TCHAR *szfilename)
{
	int h = open64(szfilename, O_RDWR);
	// -- Yes, you'd better use open64() instead of open().
	// On VMware ESX 4.1U1, with open(), you will not be able to change
	// file size beyond 2GB, and you get errno 22(EINVAL).

	if (h < 0)
		return 0;
	else
		return h;
}

void ps_closefile(FHANDLE fh)
{
	close(fh);
}

int64 
ps_get_filesize(FHANDLE fh)
{
	int64 size = lseek64(fh, 0, SEEK_END);

	if (size == -1)
	{
		_tprintf(_T("Get file size error! %s\n"), OsErrStr());
		exit(EXIT_GET_SIZE_ERROR);
	}

	return size;
}

RE_CODE
ps_set_filesize(HANDLE fh, int64 NewSize)
{
	int err = ftruncate64(fh, NewSize);

	if (err)
	{
		_tprintf(_T("Set file size(%lld) error! %s\n"), NewSize, OsErrStr());
		exit(EXIT_SET_SIZE_ERROR);
	}

	return NOERROR_0;
}

int64 
ps_str2i64(const TCHAR *s)
{
	return strtoll(s, NULL, 0);
}

TCHAR *ps_fgets_stdin(TCHAR *buf, int bufchars)
{
	fgets(buf, bufchars, stdin);
	return buf;
}

