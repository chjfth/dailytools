/*
	Should be 64-bit capable.
*/

#include <ps_headers.h>
#include <shared.h>

int64 
CalNewSize(int64 OldSize, const TCHAR szUserHint[])
{
	enum { SizeSet=0, SizeInc=1, SizeDec=-1 };
	int proc = SizeSet;
	if(szUserHint[0]==_T('+'))
		proc = SizeInc;
	else if(szUserHint[0]==_T('-'))
		proc = SizeDec;

	int64 iUserInput = ps_str2i64(szUserHint);

	int64 NewSize;
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

	FHANDLE fh = ps_openfile(szfn);
	if (!fh)
	{
		_tprintf(_T("Open file error! %s\n"), OsErrStr());
		exit(EXIT_FILE_OPEN_ERROR);
	}

	int64 OldSize = ps_get_filesize(fh);
	
	_tprintf(_T("Current file size: ") _T(FMT_i64d) _T("\n"), OldSize);
	_tprintf(_T("   Input new size: "));
	
	ps_fgets_stdin(buf, ARRAYSIZE(buf));

	int ulen = (int)_tcslen(buf);
	if(buf[ulen-1]=='\r' || buf[ulen-1]=='\n')
		ulen--;
	if(buf[ulen-1]=='\r' || buf[ulen-1]=='\n')
		ulen--;
	buf[ulen] = '\0';

	int64 NewSize = CalNewSize(OldSize, buf);

	_tprintf(_T("  New size set to: ") _T(FMT_i64d) _T(", right(y / n) ? "), NewSize);
	ps_fgets_stdin(buf, ARRAYSIZE(buf));
	if(buf[0]!='y' && buf[1]!='Y')
	{
		_tprintf(_T("Nothing Done.\n"));
		exit(EXIT_SUCCESS);
	}	

	ps_set_filesize(fh, NewSize);

	_tprintf(_T("Set file size done.\n"));

	ps_closefile(fh);
}

void 
DoNonInteractive(const TCHAR szfn[], const TCHAR szUserHint[])
{
	HANDLE fh = ps_openfile(szfn);
	if (!fh)
	{
		_tprintf(_T("Open file error! %s\n"), OsErrStr());
		exit(EXIT_FILE_OPEN_ERROR);
	}

	int64 OldSize = ps_get_filesize(fh);
	
	int64 NewSize = CalNewSize(OldSize, szUserHint);

	ps_set_filesize(fh, NewSize);

	ps_closefile(fh);
}

void 
PrintUsageAndQuit()
{
	printf("Program compile date: %s, %s\n", __DATE__, __TIME__);

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
