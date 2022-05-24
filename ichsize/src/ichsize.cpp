/*
	Should be 64-bit capable.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include <tchar.h>
#include <locale.h>

//#include <commdefs.h>
typedef int RE_CODE;
#define NOERROR_0 0
#define GetEleQuan(array) ( sizeof(array)/sizeof(array[0]) )
#define GetEleQuan_i(array) ((int)GetEleQuan(array))

// Exit value defines:
#define EXIT_SUCCESS 0
#define EXIT_INVALID_OPTION 1
#define EXIT_FILE_OPEN_ERROR 2
#define EXIT_GET_SIZE_ERROR 3
#define EXIT_SET_SIZE_ERROR 4

TCHAR *
hpGetWinErrStr(DWORD winerr)
{
	TCHAR *lpMsgBuf;
	FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		winerr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(TCHAR*)&lpMsgBuf,
		0,
		NULL 
	);
	return lpMsgBuf;
}

void 
hpFreeWinErrStr(TCHAR *szErr)
{
	LocalFree(szErr);
}

HANDLE 
hpOpenFile(const TCHAR szfn[]) // hp: helper
{
	HANDLE h = CreateFile(szfn, GENERIC_WRITE, 
		0, NULL,
		OPEN_EXISTING,
		0, NULL);
	if(h==INVALID_HANDLE_VALUE)
	{
		DWORD werr = GetLastError();
		_tprintf(_T("Open file error! WinErr(%d): %s\n"),
			werr, hpGetWinErrStr(werr));
		exit(EXIT_FILE_OPEN_ERROR);
	}

	return h;
}

__int64 
hpGetFileSize(HANDLE h)
{
	LARGE_INTEGER li;
	BOOL b = GetFileSizeEx(h, &li);
	if(!b)
	{
		DWORD werr = GetLastError();
		_tprintf(_T("Get file size error! WinErr(%d): %s\n"),
			werr, hpGetWinErrStr(werr));
		exit(EXIT_GET_SIZE_ERROR);
	}
	
	return li.QuadPart;
}

RE_CODE 
hpSetFileSize(HANDLE h, __int64 NewSize)
{
	BOOL b;
	LARGE_INTEGER ili, oli;
	ili.QuadPart = NewSize;

	b = SetFilePointerEx(h, ili, &oli, FILE_BEGIN);
	assert(ili.QuadPart==oli.QuadPart);
	b = SetEndOfFile(h);
	if(!b)
	{
		DWORD werr = GetLastError();
		_tprintf(_T("Set file size error! WinErr(%d): %s\n"),
			werr, hpGetWinErrStr(werr));
		exit(EXIT_SET_SIZE_ERROR);
	}
	
	return NOERROR_0;
}

__int64 
CalNewSize(__int64 OldSize, const TCHAR szUserHint[])
{
	enum { SizeSet=0, SizeInc=1, SizeDec=-1 };
	int proc = SizeSet;
	if(szUserHint[0]==_T('+'))
		proc = SizeInc;
	else if(szUserHint[0]==_T('-'))
		proc = SizeDec;

	__int64 iUserInput = _ttoi64(szUserHint);

	__int64 NewSize;
	if(proc==SizeSet)
		NewSize = iUserInput;
	else
		NewSize = OldSize + iUserInput;

	return NewSize;
}

void 
DoInteractive(const TCHAR szfn[])
{
	TCHAR buf[256];

	HANDLE h = hpOpenFile(szfn);
	__int64 OldSize = hpGetFileSize(h);
	
	_tprintf(_T("Current file size: %I64d\n"), OldSize);
	_tprintf(_T("   Input new size: "));
	
	_fgetts(buf, GetEleQuan_i(buf), stdin);

	int ulen = _tcslen(buf);
	if(buf[ulen-1]=='\r' || buf[ulen-1]=='\n')
		ulen--;
	if(buf[ulen-1]=='\r' || buf[ulen-1]=='\n')
		ulen--;
	buf[ulen] = '\0';

	__int64 NewSize = CalNewSize(OldSize, buf);

	_tprintf(_T("  New size set to: %I64d, right(y/n)?"), NewSize);
	_fgetts(buf, GetEleQuan_i(buf), stdin);
	if(buf[0]!='y' && buf[1]!='Y')
	{
		_tprintf(_T("Nothing Done.\n"));
		exit(EXIT_SUCCESS);
	}	

	hpSetFileSize(h, NewSize);

	_tprintf(_T("Set file size done.\n"));

	CloseHandle(h);
}

void 
DoNonInteractive(const TCHAR szfn[], const TCHAR szUserHint[])
{
	HANDLE h = hpOpenFile(szfn);
	__int64 OldSize = hpGetFileSize(h);
	
	__int64 NewSize = CalNewSize(OldSize, szUserHint);

	hpSetFileSize(h, NewSize);

	CloseHandle(h);
}

void 
PrintUsageAndQuit()
{
	_tprintf(_T("Usage:\n"));
	_tprintf(_T("Interactive use:\n"));
	_tprintf(_T("    ichsize -i <filepath>\n"));
	_tprintf(_T("Non-interactive use:\n"));
	_tprintf(_T("    ichsize -n <filepath> <newsize or +/-size>\n"));
	_tprintf(_T("    Example:\n"));
	_tprintf(_T("        Change file size to 456001536: ichsize -n c:/a.iso 456001536\n"));
	_tprintf(_T("        Increase 2048 bytes: ichsize -n c:\\a.iso +2048\n"));
	_tprintf(_T("        Decrease 2048 bytes: ichsize -n c:\\a.iso -2048\n"));
	exit(EXIT_INVALID_OPTION);
}

int _tmain(int argc, TCHAR *argv[])
{
	setlocale(LC_ALL, "");

	if(argc<2)
		PrintUsageAndQuit();

	int is_iuse = _tcscmp(argv[1], _T("-i"))==0;
	int is_niuse = _tcscmp(argv[1], _T("-n"))==0;
	if(is_iuse)
	{
		if(argc!=3)
			PrintUsageAndQuit();

		DoInteractive(argv[2]);
	}
	else if(is_niuse)
	{
		if(argc!=4)
			PrintUsageAndQuit();

		DoNonInteractive(argv[2], argv[3]);
	}
	else
	{
		_tprintf(_T("Invalid option. Neither -i or -n is assigned.\n"));
		PrintUsageAndQuit();
	}
	
	return EXIT_SUCCESS;
}
