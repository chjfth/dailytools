#include "utils.h"


const TCHAR *app_GetFilenamePart(const TCHAR *pPath)
{
	const TCHAR *p = _tcsrchr(pPath, _T('\\'));
	return p ? p+1 : pPath;
}

int my_getch_noblock(unsigned char default_key)
{
	if(_isatty(_fileno(stdout)))
	{
		int key = _getch();
		return key;
	}
	else 
		return default_key;		
}

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

void app_print_version(const TCHAR *argv0, const TCHAR *verstr)
{
	const TCHAR *pfn = app_GetFilenamePart(argv0);
	// -- For MSVC, argv[0] always contains the full pathname.

	my_tprintf(_T("%s compiled at %s with _MSC_VER=%d (v%s)\n"), pfn, _T(__DATE__), _MSC_VER, verstr);
	my_tprintf(_T("This Windows OS version: %s\n"), app_GetWindowsVersionStr3());
	my_tprintf(_T("\n"));

}

const TCHAR *StrLCID(LCID lcid)
{
	static TCHAR s_szLCID[20];
	_sntprintf_s(s_szLCID, ARRAYSIZE(s_szLCID), _T("0x%04X.%04X"), lcid>>16, lcid&0xffff);
	return s_szLCID;
}


const TCHAR *app_GetWindowsVersionStr3()
{
	typedef DWORD __stdcall PROC_RtlGetVersion(OSVERSIONINFOEX*);
	typedef BOOL __stdcall PROC_GetVersionEx(OSVERSIONINFOEX*);

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

