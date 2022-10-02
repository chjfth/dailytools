/* This file has UTF8 BOM, so to contain UTF8 chars in .cpp source code,
and at the same time, the BOM makes MSVC compiler happy. */

#include "utils.h"

const TCHAR *g_szversion = _T("1.4.0");

int g_start_codepage = 0;

int g_chcp_sleep_msec = 100; // the default value
#define CHCP_DO_PAUSE (-1)
#define CHCP_NO_CHCP (-2)

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
	{ L"\x7535",       65001, "\xE7\x94\xB5" }, // 电 in UTF8 sequence
	{ L"\xD83C\xDF4E", 65001, "\xF0\x9F\x8D\x8E" }, // RED APPLE: F0 9F 8D 8E (U+1F34E)

	// 936=GBK, 950=Big5, 949=Korean, 65001=UTF-8
};

void printf_Samples()
{
	my_tprintf(_T("==== printf()\n"));

	for(int i=0; i<ARRAYSIZE(ar_samps); i++)
	{
		const char* psza = ar_samps[i].psza;

		if (!psza)
			continue;

		int alen = (int)strlen(psza);
		char hexbuf[16];
		printf("printf() chars [%s] => ",
			HexdumpA(psza, alen, hexbuf, ARRAYSIZE(hexbuf))
			);

		printf("%s", psza);
		printf("\n");
	}

	my_tprintf(_T("\n"));
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
				HexdumpW(pszw, wlen, hexbuf, ARRAYSIZE(hexbuf))
				);
		}
		else
		{
			wprintf(L"wprintf() %d WCHARs [%s] => ", wlen,
				HexdumpW(pszw, wlen, hexbuf, ARRAYSIZE(hexbuf))
				);
		}
		
		wprintf(L"%s", pszw);
		wprintf(L"\n");
	}
	
	my_tprintf(_T("\n"));
}

void sleep_before_change_console_codepage(UINT icp, UINT ocp)
{
	// Do a sleep, to make human user aware that switching console-output-codepage 
	// may cause already displayed text glyph to be temporarily ruined.
	// That is, the whole CMD window is redrawn with a different font.
	// 
	if(g_chcp_sleep_msec>0) 
	{
		Sleep(g_chcp_sleep_msec);
	}
	else if(g_chcp_sleep_msec==CHCP_DO_PAUSE)
	{
		my_tprintf(_T("<<Will change console-codepage to(%u,%u). Press a key to continue.>>\n"), icp, ocp);
		_getch();
	}
}

BOOL mySetConsoleOutputCP2(UINT codepage, bool is_print_sample=false)
{
	if(is_print_sample && g_chcp_sleep_msec==CHCP_NO_CHCP)
	{
		return TRUE;
	}
	
	if(is_print_sample)
	{
		sleep_before_change_console_codepage(codepage, codepage);
	}

	bool is_err = false;

	// Win10.21H2: In order to get the *same* effect as `chcp <codepage>` command,
	// we need to set console-input-codepage(SetConsoleCP) as well.
	// Setting output-codepage alone seems to cause weird effect.

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

	return is_err ? FALSE : TRUE;
}

void myWriteConsoleW(HANDLE hcOut, const WCHAR *pwchars, int wchars_to_write=-1)
{
	BOOL succ = FALSE;
	DWORD written = 0;
	
	if(wchars_to_write==-1)
		wchars_to_write = (int)wcslen(pwchars);

	succ = WriteConsoleW(hcOut, pwchars, wchars_to_write, &written, NULL);
	if(!succ)
	{
		DWORD winerr = GetLastError();
		my_tprintf(_T("[Unexpect] WriteConsoleW(handle=0x%p) fail with winerr=%d.\n"), (void*)hcOut, winerr);
	}
	if(succ && wchars_to_write!=written)
	{
		my_tprintf(_T("[Unexpect] WriteConsoleW(handle=0x%p) written(%d) less than wchars_to_write(%d).\n"),
			(void*)hcOut, written, wchars_to_write);
	}
}

void myWriteAnsiBytes(HANDLE hcOut, bool is_console, const char *pbytes, int nbytes_to_write=-1)
{
	BOOL succ = FALSE;
	DWORD written = 0;

	if(nbytes_to_write==-1)
		nbytes_to_write = (int)strlen(pbytes);

	if(is_console)
	{
		succ = WriteConsoleA(hcOut, pbytes, nbytes_to_write, &written, NULL);
		if(!succ)
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] WriteConsoleA(handle=0x%p) fail with winerr=%d.\n"), (void*)hcOut, winerr);
		}
		if(succ && nbytes_to_write!=written)
		{
			my_tprintf(_T("[Unexpect] WriteConsoleA(handle=0x%p) written(%d) less than nbytes_to_write(%d).\n"), 
				(void*)hcOut, written, nbytes_to_write);
		}
	}
	else
	{
		succ = WriteFile(hcOut, pbytes, nbytes_to_write, &written, NULL);
		if(!succ)
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] WriteFile(handle=0x%p) fail with winerr=%d.\n"), (void*)hcOut, winerr);
		}
		if(succ && nbytes_to_write!=written)
		{
			my_tprintf(_T("[Unexpect] WriteFile(handle=0x%p) written(%d) less than slen(%d).\n"),
				(void*)hcOut, written, nbytes_to_write);
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
				L"Write one WCHAR [%s] => ", 
				HexdumpW(pszw, wlen, hexbuf, ARRAYSIZE(hexbuf)));
		}
		else
		{
			_snwprintf_s(wbuf, ARRAYSIZE(wbuf),
				L"Write %d WCHARs [%s] => ", wlen,
				HexdumpW(pszw, wlen, hexbuf, ARRAYSIZE(hexbuf)));
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

		if(!mySetConsoleOutputCP2(codepage, true)) {
			my_tprintf(_T("Will see erroneous glyph: "));
		}

		char hintbuf[80], hexbuf[16];
		int bytes = (int)strlen(psza);
		HexdumpA(psza, bytes, hexbuf, ARRAYSIZE(hexbuf));

		_snprintf_s(hintbuf, ARRAYSIZE(hintbuf), 
			"SetConsoleOutputCP()=%d and write %d bytes [%s]: ",
			codepage, (int)strlen(psza), hexbuf);

		DWORD written = 0;
		myWriteAnsiBytes(hcOut, is_console, hintbuf);

		// Now write the meat (set "correct" console-codepage first)
		//
		myWriteAnsiBytes(hcOut, is_console, psza);

		myWriteAnsiBytes(hcOut, is_console, "\n");
	}

	mySetConsoleOutputCP2(orig_codepage, true);
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

static bool is_hextoken(const TCHAR *psz)
{
	// psz needs to be sth like "41", "0041", "96FB" etc
	
	int slen = (int)_tcslen(psz);
	if (slen != 2 && slen != 4) 
		return false;
	
	for(int i=0; i<slen; i++)
	{
		if (!ishexdigit(psz[i]))
		{
			return false;
		}
	}
	return true;
}

void print_help()
{
	const TCHAR* helptext =
_T("  \n")
_T("Usage:  \n")
_T("  ConscpLab [OPTIONS] [hexdump...]\n")
_T("  \n")
_T("OPTIONS:\n")
_T("  crtlocale:<crtloc>   On startup, call setlocale(LC_CTYPE, \"<crtloc>\");\n")
_T("  consolecp:<codepg>   On startup, call call SetConsoleOutputCP(codepg); and\n")
_T("                       SetConsoleCP(codepg);. Old conscp is restored on quit.\n")
_T("  setmode:utf8         Call _setmode(_fileno(stdout), _O_U8TEXT);\n")
_T("  setmode:utf16        Call _setmode(_fileno(stdout), _O_U16TEXT);\n")
_T("  \n")
_T("      * If above setmode is used, I call it TPMODE=utf in general. \n")
_T("        In TPMODE=utf, MSVCRT allows wprintf only, printf asserts error.\n")
_T("        The utf8 vs utf16: when redirect stdout to file, text encoding diffs.\n")
_T("      * If no setmode is used, I call it TPMODE=ansi.\n")
_T("        In TPMODE=ansi, MSVCRT allows printf and (limited) wprintf.\n")
_T("  \n")
_T("  debug:break          When seeing this, DebugBreak(); is called.\n")
_T("  debug:pause          When seeing this, program pause for a keypress,\n")
_T("                       so you have a chance to attach a debugger.\n")
_T("  chcpsleep:pause      This will pause before changing console-codepage.\n")
_T("  chcpsleep:<millisec> This will sleep for millisec before change conscp.\n")
_T("  chcpsleep:nochcp     This will not call SetConsoleOutputCP().\n")
_T("                       Note: chcpsleep is only used for stock samples.\n")
_T("  \n")
_T("[hexdump...]\n")
_T("  If empty, this program runs with stock sample text.\n")
_T("  \n")
_T("  When seeing a 2-digit-hex or 4-digit-hex option, all remaining parameters\n")
_T("  are considered user hexdump data. each parameter is a TCHAR.\n")
_T("  \n")
_T("  * If first-seen is 2-digit-hex, then printf will dump those bytes.\n")
_T("  * If first-seen is 4-digit-hex, then wprintf will dump those WCHARs.\n")
_T("  \n")
_T("Example:\n")
_T("  ConscpLab crtlocale:zh-CN consolecp:936\n")
_T("  ConscpLab crtlocale:.65001 consolecp:65001\n")
_T("    // Note: 65001(UTF8 codepage value) needs Win10.1803+\n")
_T("  ConscpLab 41 42 43\n")
_T("    // printf(\"%s\", \"ABC\");\n")
_T("  ConscpLab 0041 42 43\n")
_T("    // wprintf(L\"%s\", L\"ABC\");\n")
_T("  ConscpLab crtlocale:zh-CN consolecp:936 B5 E7 C4 D4\n")
_T("    // Will printf two Chinese characters \"电脑\"\n")
_T("  ConscpLab setmode:utf8 consolecp:936 7535 8111\n")
_T("    // Will wprintf two Chinese characters \"电脑\"\n")
_T("  ConscpLab crtlocale:.65001 consolecp:65001 F0 9F 8D 8E\n")
_T("    // Will printf a red apple (U+1F34E). Need to run inside Win10Terminal.\n")
_T("  \n")
;
	UINT oldcp = GetConsoleOutputCP();

	// try to ensure '电脑' printf success.
	_setmode(_fileno(stdout), _O_U8TEXT);
	SetConsoleOutputCP(936);
	
	wprintf(L"%s", helptext);

	SetConsoleOutputCP(oldcp); // restore old conscp
}

int apply_startup_user_params(TCHAR *argv[])
{
	const TCHAR *exename = app_GetFilenamePart(argv[0]);
	argv++;
	
	const TCHAR szCRTLOCALE[] = _T("crtlocale:");
	const int   nzCRTLOCALE = ARRAYSIZE(szCRTLOCALE) - 1;
	const TCHAR szLOCALE[] = _T("locale:"); // old name
	const int   nzLOCALE = ARRAYSIZE(szLOCALE) - 1; // old name

	const TCHAR szCONSOLECP[] = _T("consolecp:");
	const int   nzCONSOLECP = ARRAYSIZE(szCONSOLECP) - 1;
	const TCHAR szCODEPAGE[] = _T("codepage:"); // old name
	const int   nzCODEPAGE   = ARRAYSIZE(szCODEPAGE)-1; // old name
	
	const TCHAR szSETMODE[]  = _T("setmode:");
	const int   nzSETMODE    = ARRAYSIZE(szSETMODE)-1;
	
	const TCHAR szDEBUG[]    = _T("debug:");
	const int   nzDEBUG      = ARRAYSIZE(szDEBUG)-1;
	
	const TCHAR szCHCPSLEEP[]= _T("chcpsleep:");
	const int   nzCHCPSLEEP  = ARRAYSIZE(szCHCPSLEEP)-1;

	const TCHAR *psz_start_locale = _T("");
	int start_codepage = 0;
	const TCHAR *psz_tpmode = _T("");
	const TCHAR *psz_chcpsleep = _T("");
	bool need_debug = false;

	int params = 0;
	for(; *argv!=NULL; argv++, params++)
	{
		if(_tcsnicmp(*argv, szCRTLOCALE, nzCRTLOCALE)==0)
		{
			psz_start_locale = (*argv)+nzCRTLOCALE;
		}
		else if (_tcsnicmp(*argv, szLOCALE, nzLOCALE) == 0) // old name
		{
			psz_start_locale = (*argv) + nzLOCALE;
		}
		else if (_tcsnicmp(*argv, szCONSOLECP, nzCONSOLECP) == 0)
		{
			start_codepage = _ttoi((*argv) + nzCONSOLECP);
		}
		else if(_tcsnicmp(*argv, szCODEPAGE, nzCODEPAGE)==0) // old name
		{
			start_codepage = _ttoi((*argv)+nzCODEPAGE);
		}
		else if(_tcsnicmp(*argv, szSETMODE, nzSETMODE)==0)
		{
			psz_tpmode = (*argv)+nzSETMODE;
		}
		else if(_tcsnicmp(*argv, szCHCPSLEEP, nzCHCPSLEEP)==0)
		{
			psz_chcpsleep = (*argv)+nzCHCPSLEEP;
		}
		else if(_tcsicmp(*argv, _T("nobuf"))==0)
		{
			my_tprintf(_T("Startup: setvbuf(stdout, NULL, _IONBF, 0);\n"));
			int err = setvbuf(stdout, NULL, _IONBF, 0);
			if(err!=0)
			{
				my_tprintf(_T("[Unexpect] setvbuf() fail, errno=%d\n"), (int)errno);
			}
		}
		else if(_tcsnicmp(*argv, szDEBUG, nzDEBUG)==0)
		{
			need_debug = true;

			const TCHAR *psz_debug = (*argv)+nzDEBUG;
			if(_tcsicmp(psz_debug, _T("break"))==0)
			{
				my_tprintf(_T("Startup: Now calling DebugBreak(), so you have a chance to attach Visual C++ debugger for this very consolecp.exe instance.\n"));
				DebugBreak();
			}
			else if(_tcsicmp(psz_debug, _T("pause"))==0)
			{
				my_tprintf(_T("Startup: [PAUSE] Press a key to continue, or attach a debugger.\n"));
				_getch();
			}
		}
		else
		{
			if(is_hextoken(*argv))
			{
				// OK remaining are all user hex
				break;
			}
			else
			{
				my_tprintf(_T("[ERROR] Unrecognized parameter: %s\n"), *argv);
				my_tprintf(_T("Run with \"%s -?\" to show parameter help.\n"), exename);
				exit(1);
			}
		}
	}

	if(psz_chcpsleep[0])
	{
		if (_tcsicmp(psz_chcpsleep, _T("pause")) == 0)
		{
			g_chcp_sleep_msec = CHCP_DO_PAUSE;
		}
		else if (_tcsicmp(psz_chcpsleep, _T("nochcp")) == 0)
		{
			g_chcp_sleep_msec = CHCP_NO_CHCP;
		}
		else if(_istdigit(psz_chcpsleep[0]))
		{
			g_chcp_sleep_msec = _tcstoul(psz_chcpsleep, NULL, 0);
		}
		else
		{
			my_tprintf(_T("[ERROR] Wrong input value for chcpsleep parameter.\n"));
			exit(1);
		}
	}

	my_tprintf(_T("Startup: setlocale(LC_CTYPE, \"%s\")\n"), psz_start_locale);
	const TCHAR *ret_locale = _tsetlocale(LC_CTYPE, psz_start_locale);
	if(ret_locale)
	{
		my_tprintf(_T("> setlocale() success, returns: %s\n"), ret_locale);
	}
	else
	{
		my_tprintf(_T("> setlocale() fail, errno=%d.\n"), errno);
		exit(1);
	}

	if(start_codepage>0)
	{
		g_start_codepage = start_codepage;

		my_tprintf(_T("Startup: Set Console-codepage to %d\n"), start_codepage);
		BOOL succ = mySetConsoleOutputCP2(start_codepage);
		if (!succ)
			exit(1);
	}

	if(psz_tpmode[0])
	{
		int newmode = 0;
		const TCHAR *modedesc = NULL;

		if(_tcsicmp(psz_tpmode, _T("utf16"))==0) {
			newmode = _O_U16TEXT;
			modedesc = _T("_O_U16TEXT");
		}
		else if(_tcsicmp(psz_tpmode, _T("utf8"))==0) {
			newmode = _O_U8TEXT;
			modedesc = _T("_O_U8TEXT");
		}
		else
		{
			my_tprintf(_T("[ERROR] Unrecognized setmode parameter: %s\n"), psz_tpmode);
			exit(1);
		}

		if(newmode!=0)
		{
			my_tprintf(_T("Startup: _setmode(stdout, %s) for wprintf.\n"), modedesc);
			_setmode(_fileno(stdout), newmode);
		}
	}

	return params;
}

void print_user_dump(TCHAR *argv[])
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
	// Each param corresponds to one byte or one WCHAR that will feed to output.
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

			if(!is_hextoken(argv[i]))
			{
				my_tprintf(_T("[ERROR] The parameter \"%s\" is not a valid hex-token.\n"), argv[i]);
				exit(1);
			}
			
			ansibuf[i] = (unsigned char)_tcstoul(argv[i], NULL, 16);
		}

		int nbytes_to_write = i;
		my_tprintf(_T("Will dump %d bytes to \"screen\", hex below:\n"), nbytes_to_write);

		printf("%s\n", HexdumpA(ansibuf, nbytes_to_write, hexbuf, ARRAYSIZE(hexbuf)));
		printf("\n");
		fflush(stdout);

		my_tprintf(_T("==== Using printf():\n"));
		printf("%s\n", ansibuf);
		printf("\n");
		fflush(stdout);

		my_tprintf(_T("==== Using _write(), to _fileno(stdout):\n"));
		int fh1 = _fileno(stdout);
		if(fh1!=1) {
			my_tprintf(_T("[Strange] _fileno(stdout) is NOT 1.\n"));
		}
		int nwritten = _write(fh1, ansibuf, nbytes_to_write); 
		printf("\n");
		if(nwritten!=nbytes_to_write) {
			my_tprintf(_T("[Unexpect] _write() returns %d, less than nbytes_to_write(%d).\n"), nwritten, nbytes_to_write);
		}
		printf("\n");
		fflush(stdout);

		my_tprintf(_T("==== Using WriteFile(), to STD_OUTPUT_HANDLE:\n"));
		myWriteAnsiBytes(hcOut, false, ansibuf, nbytes_to_write);
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

			if (!is_hextoken(argv[i]))
			{
				my_tprintf(_T("[ERROR] The parameter \"%s\" is not a valid hex-token.\n"), argv[i]);
				exit(1);
			}

			wcbuf[i] = (WCHAR)_tcstoul(argv[i], NULL, 16);
		}

		int ncell = i;
		my_tprintf(_T("Will dump %d WCHARs to \"screen\", hex below:\n"), ncell);

		wprintf(L"%s\n", HexdumpW(wcbuf, ncell, hexbuf, ARRAYSIZE(hexbuf)));
		wprintf(L"\n");
		fflush(stdout);

		my_tprintf(_T("Using wprintf():\n"));
		wprintf(L"%s\n", wcbuf);
		wprintf(L"\n");
		fflush(stdout);

		my_tprintf(_T("==== Using _write(), to _fileno(stdout):\n"));
		int fh1 = _fileno(stdout);
		if(fh1!=1) {
			my_tprintf(_T("[Strange] _fileno(stdout) is NOT 1.\n"));
		}
		int nbytes_to_write = ncell*sizeof(WCHAR);
		int nwritten = _write(fh1, wcbuf, nbytes_to_write); 
		wprintf(L"\n");
		if(nwritten!=nbytes_to_write) {
			my_tprintf(_T("[Unexpect] _write() returns %d, less than nbytes_to_write(%d).\n"), nwritten, nbytes_to_write);
		}
		wprintf(L"\n");

		my_tprintf(_T("==== Using WriteConsoleW() to STD_OUTPUT_HANDLE:\n"));
		myWriteConsoleW(hcOut, wcbuf, ncell);
		my_tprintf(_T("\n"));
	}
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

	DWORD ftIn = GetFileType(hIn);   // If console,  return FILE_TYPE_CHAR(2)
	DWORD ftOut = GetFileType(hOut); // If pipe, return FILE_TYPE_PIPE(3)
	DWORD ftErr = GetFileType(hErr); // If diskfile, return FILE_TYPE_DISK(1)
	// just test <<<

	UINT icp1 = GetConsoleCP();
	UINT ocp1 = GetConsoleOutputCP();

	printf_Samples();
	
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
		my_tprintf(_T("[PANIC!!!] GetConsoleCP() value changed during wprintf().\n"));
	}
	if(ocp2!=ocp1)
	{
		my_tprintf(_T("[PANIC!!!] GetConsoleOutputCP() value changed during wprint().\n"));
	}

	WriteConsoleW_Samples(hOut);

	WriteAnsiBytes_Samples(hOut, true); // WriteConsoleA

	WriteAnsiBytes_Samples(hOut, false); // WriteFile
}

int _tmain(int argc, TCHAR *argv[])
{
	const TCHAR *pfn = app_GetFilenamePart(argv[0]);
	// -- For MSVC, argv[0] always contains the full pathname.

	if(argc>1 && _tcscmp(_T("-?"), argv[1])==0)
	{
		print_help();
		exit(0);
	}

	my_tprintf(_T("%s compiled at %s with _MSC_VER=%d (v%s)\n"), pfn, _T(__DATE__), _MSC_VER, g_szversion);
	my_tprintf(_T("This Windows OS version: %s\n"), app_GetWindowsVersionStr3());
	my_tprintf(_T("\n"));

	if(_MSC_VER<1900)
	{
		my_tprintf(_T("[Warning] This program is compiled with a fairly old Visual C++ version\n"));
		my_tprintf(_T("(before VC2015), so it does not support \".UTF8\" in CRT-locale.\n"));
	}

	TCHAR *orig_lcctype = _tsetlocale(LC_CTYPE, NULL);
	_locale_t orig_locale = _get_current_locale();

	UINT orig_icp = GetConsoleCP();
	UINT orig_ocp = GetConsoleOutputCP();
	my_tprintf(_T("Initial GetConsoleCP()       = %d\n"), orig_icp);
	my_tprintf(_T("Initial GetConsoleOutputCP() = %d\n"), orig_ocp);

	int params_done = apply_startup_user_params(argv);
	// -- deal with "locale:.950" and "codepage:950" params

	my_tprintf(_T("\n"));

	if(*(argv+1+params_done) == NULL)
	{
		if (g_chcp_sleep_msec == CHCP_DO_PAUSE)
			my_tprintf(_T("[Stock samples] Will pause before each SetConsoleOutputCP().\n"));
		else if (g_chcp_sleep_msec == CHCP_NO_CHCP)
			my_tprintf(_T("[Stock samples] Will not call SetConsoleOutputCP().\n"));
		else if (g_chcp_sleep_msec>0)
			my_tprintf(_T("[Stock samples] Will sleep %d millisec before each SetConsoleOutputCP().\n"), g_chcp_sleep_msec);

		print_stock_samples();
	}
	else
	{
		// Dump user given char-stream or WCHAR-stream.
		print_user_dump(argv+1+params_done);
	}

	if(g_start_codepage)
	{
		sleep_before_change_console_codepage(orig_icp, orig_ocp);
	}

	// Restore original in/out-codepage.
	SetConsoleCP(orig_icp);
	SetConsoleOutputCP(orig_ocp);

	return 0;
}
