﻿/* This file has UTF8 BOM, so to contain UTF8 chars in .cpp source code,
and at the same time, the BOM makes MSVC compiler happy. */
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <tchar.h>
#include <locale.h>
#include <windows.h>

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
			wprintf(L"wprintf() one WCHAR [%s] => %s\n", 
				HexdumpW(pszw, hexbuf, ARRAYSIZE(hexbuf)), 
				pszw);
		}
		else
		{
			wprintf(L"wprintf() %d WCHARs [%s] => %s\n", wlen,
				HexdumpW(pszw, hexbuf, ARRAYSIZE(hexbuf)), 
				pszw);
		}
	}
	
	my_tprintf(_T("\n"));
}

BOOL mySetConsoleOutputCP(UINT codepage)
{
	BOOL succ = SetConsoleOutputCP(codepage);
	if(succ)
	{
		UINT cp2 = GetConsoleOutputCP();
		if(cp2==codepage)
		{
			return TRUE;
		}
		else
		{
			my_tprintf(_T("[Unexpect] SetConsoleOutputCP(%d) success but no effect. GetConsoleOutputCP() returns %d\n"), codepage, cp2);
			return FALSE;
		}
	}
	else
	{
		DWORD winerr = GetLastError();
		my_tprintf(_T("[Unexpect] SetConsoleOutputCP(%d) fail, winerr=%d\n"), codepage, winerr);
		return FALSE;
	}
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
		my_tprintf(_T("[Unexpect] WriteConsoleW(handle=0x%p) fail with winerr=%d\n"), (void*)hcOut, winerr);
	}
	if(succ && slen!=written)
	{
		my_tprintf(_T("[Unexpect] WriteConsoleW(handle=0x%p) written(%d) less than slen(%d)\n"), (void*)hcOut, written, slen);
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
			my_tprintf(_T("[Unexpect] WriteConsoleA(handle=0x%p) fail with winerr=%d\n"), (void*)hcOut, winerr);
		}
		if(succ && slen!=written)
		{
			my_tprintf(_T("[Unexpect] WriteConsoleA(handle=0x%p) written(%d) less than slen(%d)\n"), (void*)hcOut, written, slen);
		}
	}
	else
	{
		succ = WriteFile(hcOut, psza, slen, &written, NULL);
		if(!succ)
		{
			DWORD winerr = GetLastError();
			my_tprintf(_T("[Unexpect] WriteFile(handle=0x%p) fail with winerr=%d\n"), (void*)hcOut, winerr);
		}
		if(succ && slen!=written)
		{
			my_tprintf(_T("[Unexpect] WriteFile(handle=0x%p) written(%d) less than slen(%d)\n"), (void*)hcOut, written, slen);
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
		if(!mySetConsoleOutputCP(codepage)) {
			printf("Will see erroneous glyph: ");
		}
		myWriteAnsiBytes(hcOut, is_console, psza);

		myWriteAnsiBytes(hcOut, is_console, "\n");

		// Do a pause, bcz switching console-output-codepage later 
		// may(?) cause already displayed text glyph to be ruined.
		Sleep(500);
	}

	mySetConsoleOutputCP(orig_codepage);
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

void check_debugbreak(const TCHAR *prefix)
{
	// MEMO:
	//	set consolecp.exe_BREAK=1

	TCHAR szEnvvar[40] = {}, szVal[10] = {};
	_sntprintf_s(szEnvvar, ARRAYSIZE(szEnvvar), _T("%s_BREAK"), prefix);
	GetEnvironmentVariable(szEnvvar, szVal, ARRAYSIZE(szVal));

	if(szVal[0]==_T('1'))
		DebugBreak();
}

int _tmain(int argc, TCHAR *argv[])
{
	setlocale(LC_ALL, "");

	TCHAR *pfn = GetFilenamePart(argv[0]);
	// -- For MSVC, argv[0] always contains the full pathname.

	check_debugbreak(pfn);

	my_tprintf(_T("%s compiled at %s with _MSC_VER=%d\n"), pfn, _T(__DATE__), _MSC_VER);

	my_tprintf(_T("This Windows OS version: %s\n"), app_GetWindowsVersionStr3());

	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

	HANDLE fhIn = CreateFile_stdio(_T("CONIN$"));
	HANDLE fhOut = CreateFile_stdio(_T("CONOUT$"));
	HANDLE fhErr = CreateFile_stdio(_T("CONERR$"));

//	DWORD xret1=0, xret2=0;
//	BOOL b1 = WriteFile(hOut, L"MMNN", 8, &xret1, NULL);
//	BOOL b2 = WriteFile(fhOut, L"mmnn", 8, &xret2, NULL);

	UINT orig_ocp = GetConsoleOutputCP();
	my_tprintf(_T("Initial GetConsoleCP()       = %d\n"), GetConsoleCP());
	my_tprintf(_T("Initial GetConsoleOutputCP() = %d\n"), orig_ocp);

	my_tprintf(_T("\n"));

	wprintf_Samples();
	// -- Note: GetConsoleOutputCP() may have been changed inside wprintf_Samples().

	SetConsoleOutputCP(orig_ocp);

	WriteConsoleW_Samples(hOut);

	WriteAnsiBytes_Samples(hOut, true); // WriteConsoleA

	WriteAnsiBytes_Samples(hOut, false); // WriteFile

	mySetConsoleOutputCP(orig_ocp);
	return 0;
}