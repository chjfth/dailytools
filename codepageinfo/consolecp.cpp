/* This file has UTF8 BOM, so to contain UTF8 chars in .cpp source code,
and at the same time, the BOM makes MSVC compiler happy. */
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <tchar.h>
#include <locale.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

const TCHAR *g_szversion = _T("1.0");

int g_chcp_sleep_sec = 0;

TCHAR *GetFilenamePart(TCHAR *pPath)
{
	TCHAR *p = _tcsrchr(pPath, _T('\\'));
	return p ? p+1 : pPath;
}

struct SampleStr_st
{
	const WCHAR *pszw;
	int codepage;
	const char *psza;
};

SampleStr_st ar_samps[] =
{
	{ L"\x7535", 936, "\xB5\xE7" }, // 电 E7 94 B5 
	{ L"\x96FB", 936, "\xEB\x8A" }, // 電 E9 9B BB
	{ L"\x96FB", 950, "\xB9\x71" }, // 電 E9 9B BB
	{ L"\xD55C", 949, "\xC7\xD1" }, // 한 ED 95 9C
	{ L"\xD83C\xDF4E", 65001, "\xF0\x9F\x8D\x8E" }, // RED APPLE: F0 9F 8D 8E (U+1F34E)

	// 936=GBK, 950=Big5, 949=Korean, 65001=UTF-8
};

void my_tprintf(const TCHAR *szfmt, ...)
{
	va_list args;
	va_start(args, szfmt);

	TCHAR buf[200] = {};
	_vsntprintf_s(buf, ARRAYSIZE(buf), szfmt, args);

	va_end(args);

	_tprintf(_T("%s"), buf);
	fflush(stdout); // important
}

WCHAR *HexdumpW(const WCHAR *pszw, WCHAR *hexbuf, int bufchars)
{
	int wlen = (int)wcslen(pszw);
	for(int i=0; i<wlen; i++)
	{
		_snwprintf_s(hexbuf+i*5, bufchars-i*5, _TRUNCATE,
			L"%04X ", (unsigned short)pszw[i]);
	}
	
	wlen = (int)wcslen(hexbuf);
	if(wlen>0 && hexbuf[wlen-1]==L' ')
		hexbuf[wlen-1] = L'\0';

	return hexbuf;
}

char *HexdumpA(const char *psza, char *hexbuf, int bufchars)
{
	int alen = (int)strlen(psza);
	for(int i=0; i<alen; i++)
	{
		_snprintf_s(hexbuf+i*3, bufchars-i*3, _TRUNCATE, 
			"%02X ", (unsigned char)psza[i]);
	}

	alen = (int)strlen(hexbuf);
	if(alen>0 && hexbuf[alen-1]==' ')
		hexbuf[alen-1] = L'\0';

	return hexbuf;
}

void wprintf_Samples()
{
	// NOTE: C-locale affects this, not system-codepage.

	my_tprintf(_T("==== wprintf()\n"));

	for(int i=0; i<ARRAYSIZE(ar_samps); i++)
	{
		const WCHAR *pszw = ar_samps[i].pszw;

		if(!pszw)
			continue;

		int wlen = (int)wcslen(pszw);
		WCHAR hexbuf[16];
		if(wlen==1)
		{
			wprintf(L"wprintf() one WCHAR [%s] => ", 
				HexdumpW(pszw, hexbuf, ARRAYSIZE(hexbuf)), 
				pszw);
		}
		else
		{
			wprintf(L"wprintf() %d WCHARs [%s] => ", wlen,
				HexdumpW(pszw, hexbuf, ARRAYSIZE(hexbuf)), 
				pszw);
		}
		
		wprintf(L"%s", pszw);
		wprintf(L"\n");
	}
	
	my_tprintf(_T("\n"));
}

BOOL mySetConsoleOutputCP2(UINT codepage)
{
	bool is_err = false;

	do
	{
		BOOL succ = SetConsoleOutputCP(codepage);
		if(succ)
		{
			UINT cp2 = GetConsoleOutputCP();
			if(cp2!=codepage)
			{
				my_tprintf(_T("[Unexpect] SetConsoleOutputCP(%d) success but no effect. GetConsoleOutputCP() returns %d.\n"), codepage, cp2);
				is_err = true;
			}
			break;
		}
		else
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] SetConsoleOutputCP(%d) fail, winerr=%d.\n"), codepage, winerr);
			is_err = true;
			break;
		}
	}while(0);

	// Win10.21H2: In order to get the *same* effect as `chcp <codepage>` command,
	// we need to set console-input-codepage(SetConsoleCP) as well.

	do
	{
		BOOL succ = SetConsoleCP(codepage);
		if(succ)
		{
			UINT cp2 = GetConsoleCP();
			if(cp2!=codepage)
			{
				my_tprintf(_T("[Unexpect] SetConsoleCP(%d) success but no effect. GetConsoleCP() returns %d.\n"), codepage, cp2);
				is_err = true;
			}
			break;
		}
		else
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] SetConsoleCP(%d) fail, winerr=%d.\n"), codepage, winerr);
			is_err = true;
			break;
		}
	}while(0);

	return is_err ? FALSE : TRUE;
}

void myWriteConsoleW(HANDLE hcOut, const WCHAR *pszw)
{
	BOOL succ = FALSE;
	DWORD written = 0;
	int slen = (int)wcslen(pszw);
	succ = WriteConsoleW(hcOut, pszw, slen, &written, NULL);
	if(!succ)
	{
		DWORD winerr = GetLastError();
		my_tprintf(_T("[Unexpect] WriteConsoleW(handle=0x%p) fail with winerr=%d.\n"), (void*)hcOut, winerr);
	}
	if(succ && slen!=written)
	{
		my_tprintf(_T("[Unexpect] WriteConsoleW(handle=0x%p) written(%d) less than slen(%d).\n"), (void*)hcOut, written, slen);
	}
}

void myWriteAnsiBytes(HANDLE hcOut, bool is_console, const char *psza)
{
	BOOL succ = FALSE;
	DWORD written = 0;
	int slen = (int)strlen(psza);
	if(is_console)
	{
		succ = WriteConsoleA(hcOut, psza, slen, &written, NULL);
		if(!succ)
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] WriteConsoleA(handle=0x%p) fail with winerr=%d.\n"), (void*)hcOut, winerr);
		}
		if(succ && slen!=written)
		{
			my_tprintf(_T("[Unexpect] WriteConsoleA(handle=0x%p) written(%d) less than slen(%d).\n"), (void*)hcOut, written, slen);
		}
	}
	else
	{
		succ = WriteFile(hcOut, psza, slen, &written, NULL);
		if(!succ)
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] WriteFile(handle=0x%p) fail with winerr=%d.\n"), (void*)hcOut, winerr);
		}
		if(succ && slen!=written)
		{
			my_tprintf(_T("[Unexpect] WriteFile(handle=0x%p) written(%d) less than slen(%d).\n"), (void*)hcOut, written, slen);
		}
	}
}

void WriteConsoleW_Samples(HANDLE hcOut)
{
	my_tprintf(_T("==== WriteConsoleW()\n"));

	for(int i=0; i<ARRAYSIZE(ar_samps); i++)
	{
		const WCHAR *pszw = ar_samps[i].pszw;

		if(!pszw)
			continue;

		int wlen = (int)wcslen(pszw);
		WCHAR wbuf[80], hexbuf[16];
		if(wlen==1)
		{
			_snwprintf_s(wbuf, ARRAYSIZE(wbuf), 
				L"Write one WCHAR [%s] => ", HexdumpW(pszw, hexbuf, ARRAYSIZE(hexbuf)));
		}
		else
		{
			_snwprintf_s(wbuf, ARRAYSIZE(wbuf), 
				L"Write %d WCHARs [%s] => ", wlen,
				HexdumpW(pszw, hexbuf, ARRAYSIZE(hexbuf)));
		}
		myWriteConsoleW(hcOut, wbuf);

		myWriteConsoleW(hcOut, pszw); // write the meat

		myWriteConsoleW(hcOut, L"\n");
	}

	my_tprintf(_T("\n"));
}

void WriteAnsiBytes_Samples(HANDLE hcOut, bool is_console)
{
	if(is_console)
		my_tprintf(_T("==== WriteConsoleA()\n"));
	else
		my_tprintf(_T("==== WriteFile() to STD_OUTPUT_HANDLE, ANSI bytes\n"));

	UINT orig_codepage = GetConsoleOutputCP();

	for(int i=0; i<ARRAYSIZE(ar_samps); i++)
	{
		int codepage = ar_samps[i].codepage;
		const char *psza = ar_samps[i].psza;

		char hintbuf[80], hexbuf[16];
		HexdumpA(psza, hexbuf, ARRAYSIZE(hexbuf));

		_snprintf_s(hintbuf, ARRAYSIZE(hintbuf), 
			"SetConsoleOutputCP()=%d and write %d bytes [%s]: ",
			codepage, strlen(psza), hexbuf);

		DWORD written = 0;
		myWriteAnsiBytes(hcOut, is_console, hintbuf);

		// Now write the meat (set "correct" console-codepage first)
		//
		if(!mySetConsoleOutputCP2(codepage)) {
			my_tprintf(_T("Will see erroneous glyph: "));
		}
		myWriteAnsiBytes(hcOut, is_console, psza);

		myWriteAnsiBytes(hcOut, is_console, "\n");

		// Do a pause, bcz switching console-output-codepage later 
		// may(?) cause already displayed text glyph to be ruined.
		if(g_chcp_sleep_sec==0) 
			Sleep(100);
		else
			Sleep(g_chcp_sleep_sec*1000);
	}

	mySetConsoleOutputCP2(orig_codepage);
	my_tprintf(_T("\n"));
}


HANDLE CreateFile_stdio(const TCHAR *szfn)
{
	HANDLE fh = CreateFile(szfn,
		GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_READ|FILE_SHARE_WRITE, // shareMode
		NULL, // no security attribute
		OPEN_EXISTING, // dwCreationDisposition
		0, // FILE_FLAG_DELETE_ON_CLOSE,
		NULL);
	return fh;
}

const TCHAR *app_GetWindowsVersionStr3()
{
	typedef DWORD PROC_RtlGetVersion(OSVERSIONINFOEX*);
	typedef BOOL PROC_GetVersionEx(OSVERSIONINFOEX*);

	static TCHAR s_verstr[40];
	OSVERSIONINFOEX exver = { sizeof(OSVERSIONINFOEX) };

	PROC_RtlGetVersion *RtlGetVersion = (PROC_RtlGetVersion*)
		GetProcAddress(GetModuleHandle(_T("ntdll")), "RtlGetVersion");

	PROC_GetVersionEx *dllGetVersionEx = (PROC_GetVersionEx*)
		GetProcAddress(GetModuleHandle(_T("kernel32")), "GetVersionEx");

	if(RtlGetVersion)
		RtlGetVersion(&exver);

	if (exver.dwMajorVersion == 0)
	{
		// RtlGetVersion() fail, fall back to traditional GetVersionEx()
		BOOL succ = dllGetVersionEx && dllGetVersionEx(&exver);
		if (!succ)
			exver.dwMajorVersion = 0;
	}

	if (exver.dwMajorVersion > 0)
	{
		_sntprintf_s(s_verstr, ARRAYSIZE(s_verstr), _T("%d.%d.%d"),
			exver.dwMajorVersion, exver.dwMinorVersion, exver.dwBuildNumber);
	}
	else
	{
		_sntprintf_s(s_verstr, ARRAYSIZE(s_verstr), _T("%s"),
			_T("Fail to get Windows OS version after trying NTDLL!RtlGetVersion() and GetVersionEx()!"));
	}

	return s_verstr;
}

int apply_startup_user_params(TCHAR *argv[])
{
	// On input, argv should points to first param, not to the exe name/path.
	// I recognize three instructions, can assign both in separate params:
	//
	// First,
	// "locale:zh_CN.936" will call setlocale(LC_ALL, "zh_CN.936");
	// "locale:.UTF-8" will call setlocale(LC_ALL, ".UTF-8"); // this is support since Win10.1803 with new UCRT.
	// If no such instruction, setlocale(LC_ALL, ""); is called.
	//
	// Second,
	// "codepage:936" will call SetConsoleOutputCP(936);
	// "codepage:65001" will call SetConsoleOutputCP(65001);
	// If no such instruction, SetConsoleOutputCP() is not called at startup.
	//
	// Third,
	// "setmode:utf16" 
	// "setmode:utf8"
	// 
	// "setmode:utf16" enables wprint()'s internal behavior of 
	// passing WCHARs to WriteConsoleW() directly, not doing a stupid 
	// Unicode -> ANSI -> Unicode round trip.

	const TCHAR szLOCALE[]   = _T("locale:");
	const int   nzLOCALE     = ARRAYSIZE(szLOCALE)-1;
	const TCHAR szCODEPAGE[] = _T("codepage:");
	const int   nzCODEPAGE   = ARRAYSIZE(szCODEPAGE)-1;
	const TCHAR szSETMODE[]  = _T("setmode:");
	const int   nzSETMODE    = ARRAYSIZE(szSETMODE)-1;

	const TCHAR *psz_start_locale = _T("");
	int start_codepage = 0;
	const TCHAR *psz_fdmode = _T("");

	int params = 0;
	for(; *argv!=NULL; argv++, params++)
	{
		if(_tcsnicmp(*argv, szLOCALE, nzLOCALE)==0)
		{
			psz_start_locale = (*argv)+nzLOCALE;
		}
		else if(_tcsnicmp(*argv, szCODEPAGE, nzCODEPAGE)==0)
		{
			start_codepage = _ttoi((*argv)+nzCODEPAGE);
		}
		else if(_tcsnicmp(*argv, szSETMODE, nzSETMODE)==0)
		{
			psz_fdmode = (*argv)+nzSETMODE;
		}
		else
			break;
	}

	my_tprintf(_T("Startup: setlocale(LC_ALL, \"%s\") .\n"), psz_start_locale);
	const TCHAR *ret_locale = _tsetlocale(LC_ALL, psz_start_locale);
	if(ret_locale)
	{
		my_tprintf(_T("> setlocale() success, returns: %s\n"), ret_locale);
	}
	else
	{
		my_tprintf(_T("> setlocale() fail, errno=%d.\n"), errno);
	}

	if(start_codepage>0)
	{
		my_tprintf(_T("Startup: Set Console-codepage to %d .\n"), start_codepage);
		mySetConsoleOutputCP2(start_codepage);
	}

	if(psz_fdmode[0])
	{
		int newmode = 0;
		const TCHAR *modedesc = NULL;

		if(_tcsicmp(psz_fdmode, _T("utf16"))==0) {
			newmode = _O_U16TEXT;
			modedesc = _T("_O_U16TEXT");
		}
		else if(_tcsicmp(psz_fdmode, _T("utf8"))==0) {
			newmode = _O_U8TEXT;
			modedesc = _T("_O_U8TEXT");
		}

		if(newmode!=0)
		{
			my_tprintf(_T("Startup: _setmode(stdout, %s) for wprintf.\n"), modedesc);
			_setmode(_fileno(stdout), newmode);
		}
	}

	return params;
}

void print_user_chars(TCHAR *argv[])
{
	// Each argv[] "points to" remaining command-line params like:
	//
	//	41 41 B5 E7 42 42
	//	41 41 E7 94 B5 42 42
	//	43 43 F0 9F 8D 8E 44 44
	//
	//	0041 41 7535 42 42
	//	0043 43 D83C DF4E 44 44
	//
	// each param corresponds to one byte or one WCHAR that will feed to output.
	// * If first param is two-chars(like 41), then all params are considered byte stream,
	//   and will feed to printf() and WriteFile().
	// * If first param is four-chars(like 0041), then all params are considered WCHAR stream,
	//   and will feed to wprintf() and WriteConsoleW(). Note there is NO WriteFileW().

	HANDLE hcOut = GetStdHandle(STD_OUTPUT_HANDLE);

	const int MAXCELLS = 80;
	int i;

	if(argv[0] && _tcslen(argv[0])<=2)
	{
		// user gives a byte stream
		char ansibuf[MAXCELLS+1] = {};
		char hexbuf[MAXCELLS*3+1] = {};

		for(i=0; i<MAXCELLS; i++)
		{
			if(argv[i]==NULL)
				break;

			ansibuf[i] = (unsigned char)_tcstoul(argv[i], NULL, 16);
		}

		int nbytes = i;
		my_tprintf(_T("Will dump %d bytes to \"screen\", hex below:\n"), nbytes);

		printf("%s\n", HexdumpA(ansibuf, hexbuf, ARRAYSIZE(hexbuf)));
		printf("\n");
		fflush(stdout);

		my_tprintf(_T("==== Using printf():\n"));
		printf("%s\n", ansibuf);
		printf("\n");
		fflush(stdout);

		my_tprintf(_T("==== Using WriteFile() to STD_OUTPUT_HANDLE:\n"));
		myWriteAnsiBytes(hcOut, false, ansibuf);
		my_tprintf(_T("\n"));
	}
	else
	{
		// user gives a WCHAR stream
		WCHAR wcbuf[MAXCELLS+1] = {};
		WCHAR hexbuf[MAXCELLS*5+1] = {};

		for(i=0; i<MAXCELLS; i++)
		{
			if(argv[i]==NULL)
				break;

			wcbuf[i] = (WCHAR)_tcstoul(argv[i], NULL, 16);
		}

		int ncell = i;
		my_tprintf(_T("Will dump %d WCHARs to \"screen\", hex below:\n"), ncell);

		wprintf(L"%s\n", HexdumpW(wcbuf, hexbuf, ARRAYSIZE(hexbuf)));
		wprintf(L"\n");
		fflush(stdout);

		my_tprintf(_T("Using wprintf():\n"));
		wprintf(L"%s\n", wcbuf);
		wprintf(L"\n");
		fflush(stdout);

		my_tprintf(_T("==== Using WriteConsoleW() to STD_OUTPUT_HANDLE:\n"));
		myWriteConsoleW(hcOut, wcbuf);
		my_tprintf(_T("\n"));
	}
}

void check_debugbreak(const TCHAR *prefix)
{
	// MEMO:
	//	`set consolecp.exe_BREAK=1` to enable DebugBreak();

	TCHAR szEnvvar[40] = {}, szVal[10] = {};
	_sntprintf_s(szEnvvar, ARRAYSIZE(szEnvvar), _T("%s_BREAK"), prefix);
	GetEnvironmentVariable(szEnvvar, szVal, ARRAYSIZE(szVal));

	if(szVal[0]==_T('1'))
		DebugBreak();
}

void print_stock_samples()
{
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

	// just test >>>
	HANDLE fhIn = CreateFile_stdio(_T("CONIN$"));
	HANDLE fhOut = CreateFile_stdio(_T("CONOUT$"));
	HANDLE fhErr = CreateFile_stdio(_T("CONERR$"));
	// just test <<<

	UINT icp1 = GetConsoleCP();
	UINT ocp1 = GetConsoleOutputCP();

	wprintf_Samples();
	// -- Note: GetConsoleOutputCP() may have been changed inside wprintf_Samples().
	//
	// Check whether wprintf() internally changes console-codepage.
	//
	UINT icp2 = GetConsoleCP();
	UINT ocp2 = GetConsoleOutputCP();
	//
	if(icp2!=icp1)
	{
		my_tprintf(_T("[PANIC!!!] GetConsoleCP() value changed during wprint().\n"));
	}
	if(ocp2!=ocp1)
	{
		my_tprintf(_T("[PANIC!!!] GetConsoleOutputCP() value changed during wprint().\n"));
	}

	WriteConsoleW_Samples(hOut);

	WriteAnsiBytes_Samples(hOut, true); // WriteConsoleA

	WriteAnsiBytes_Samples(hOut, false); // WriteFile
}

void temp_test()
{
#if 0
	_setmode(_fileno(stdout), _O_U16TEXT);
	printf("Crash the CRT!\n");
#endif
}

int _tmain(int argc, TCHAR *argv[])
{
	TCHAR *pfn = GetFilenamePart(argv[0]);
	// -- For MSVC, argv[0] always contains the full pathname.

	check_debugbreak(pfn);

	temp_test();

	my_tprintf(_T("%s compiled at %s with _MSC_VER=%d (v%s)\n"), pfn, _T(__DATE__), _MSC_VER, g_szversion);
	my_tprintf(_T("This Windows OS version: %s\n"), app_GetWindowsVersionStr3());
	my_tprintf(_T("\n"));

	UINT orig_icp = GetConsoleCP();
	UINT orig_ocp = GetConsoleOutputCP();
	my_tprintf(_T("Initial GetConsoleCP()       = %d\n"), orig_icp);
	my_tprintf(_T("Initial GetConsoleOutputCP() = %d\n"), orig_ocp);

	int params_done = apply_startup_user_params(argv+1);
	// -- deal with "locale:.950" and "codepage:950" params

	my_tprintf(_T("\n"));

	if(*(argv+1+params_done) == NULL)
	{
		print_stock_samples();
	}
	else
	{
		print_user_chars(argv+1+params_done);
	}

	// Restore original in/out-codepage.
	SetConsoleCP(orig_icp);
	SetConsoleOutputCP(orig_ocp);

	return 0;
}
