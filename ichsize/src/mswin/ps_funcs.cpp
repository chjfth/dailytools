#include "shared.h"

const TCHAR *
ps_get_os_errstring(int oserr, TCHAR *buf, int bufchars)
{
	TCHAR *lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		oserr,
		0, // LANGID
		(TCHAR*)&lpMsgBuf,
		0,
		NULL
	);

	buf[0] = 0;
	_sntprintf_s(buf, bufchars-1, _TRUNCATE, TEXT("[%d] %s"), oserr, lpMsgBuf);

	LocalFree(lpMsgBuf);

	return buf;
}

FHANDLE 
ps_openfile(const TCHAR *szfilename)
{
	HANDLE h = CreateFile(szfilename, GENERIC_WRITE,
		0, NULL,
		OPEN_EXISTING,
		0, NULL);
	if (h == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}
	else
	{
		return h;
	}
}

int64 
ps_get_filesize(FHANDLE fh)
{
	LARGE_INTEGER li;
	BOOL b = GetFileSizeEx(fh, &li);
	if (!b)
	{
		_tprintf(_T("Get file size error! %s\n"), OsErrStr());
		exit(EXIT_GET_SIZE_ERROR);
	}

	return (int64)li.QuadPart;
}

RE_CODE
ps_set_filesize(HANDLE fh, int64 NewSize)
{
	BOOL b;
	LARGE_INTEGER ili = {}, oli = {};
	ili.QuadPart = NewSize;

	b = SetFilePointerEx(fh, ili, &oli, FILE_BEGIN);
	assert(ili.QuadPart == oli.QuadPart);
	b = SetEndOfFile(fh);
	if (!b)
	{
		_tprintf(_T("Set file size error! %s\n"), OsErrStr());
		exit(EXIT_SET_SIZE_ERROR);
	}

	return NOERROR_0;
}
