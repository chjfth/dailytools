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

		if(key==3 || key==27) // Ctrl+C or ESC
		{
			my_tprintf(_T("User Quit.\n"));
			exit(1);
		}

		return key;
	}
	else 
		return default_key;		
}

void my_tprintf(const TCHAR *szfmt, ...)
{
	va_list args;
	va_start(args, szfmt);

	TCHAR buf[2000] = {};
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

const TCHAR *HexstrLCID(LCID lcid, bool detect_unspecified)
{
	static TCHAR s_szLCID[20];

	if(detect_unspecified && Is_LCID_unspecified(lcid))
	{
		_sntprintf_s(s_szLCID, _TRUNCATE, _T("unspecified"));
	}
	else
	{
		_sntprintf_s(s_szLCID, _TRUNCATE, _T("0x%04X.%04X"), HIWORD(lcid), LOWORD(lcid));
	}

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

DEFINE_DLPTR_WINAPI("kernel32.dll", LCIDToLocaleName)

const TCHAR *Desctext_from_LANGID(LANGID lcid, DepictLang_et dlang)
{
	// need_native_name==false, means desc-text in English, so it's always printf-safe

	static TCHAR szDesc[LOCALE_NAME_MAX_LENGTH*2] = {};

	szDesc[0] = 0;

	if(dlptr_LCIDToLocaleName)
	{
		TCHAR langtag[LOCALE_NAME_MAX_LENGTH+1] = {};
		dlptr_LCIDToLocaleName(lcid, langtag, ARRAYSIZE(langtag), LOCALE_ALLOW_NEUTRAL_NAMES);
		
		_sntprintf_s(szDesc, ARRAYSIZE(szDesc), _T("[%s] "), langtag);
	}

	LCTYPE lcLang = LOCALE_SENGLISHLANGUAGENAME;
	LCTYPE lcRegn = LOCALE_SENGLISHCOUNTRYNAME;
	if(dlang==DepictLang_localized)
		lcLang = LOCALE_SLOCALIZEDLANGUAGENAME, lcRegn = LOCALE_SLOCALIZEDCOUNTRYNAME;
	else if(dlang==DepictLang_native)
		lcLang = LOCALE_SNATIVELANGNAME, lcRegn = LOCALE_SNATIVECOUNTRYNAME;

	TCHAR sztmpLang[40] = _T("[unknown]");
	// -- WinXP will get [unknown] on LOCALE_SLOCALIZEDLANGUAGENAME

	GetLocaleInfo(lcid, lcLang, sztmpLang, ARRAYSIZE(sztmpLang)-1);
	_tcscat_s(szDesc, sztmpLang); 

	_tcscat_s(szDesc, _T(" @ "));

	TCHAR sztmpCountry[40] = _T("[unknown]");
	
	GetLocaleInfo(lcid, lcRegn, sztmpCountry, ARRAYSIZE(sztmpCountry)-1); 
	_tcscat_s(szDesc, sztmpCountry);

	return szDesc;
}

const TCHAR * app_WinErrStr(DWORD winerr)
{
	static TCHAR s_retbuf[400] = {};
	TCHAR szWinErr[200] = {};

	s_retbuf[0] = 0;
	
	if (winerr == (DWORD)-1)
		winerr = GetLastError();

	DWORD retchars = FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, 
		winerr,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), // LANGID
		szWinErr, ARRAYSIZE(szWinErr),
		NULL); 

	if(retchars>0)
	{
		_sntprintf_s(s_retbuf, _TRUNCATE, _T("WinErr=%d, %s"), winerr, szWinErr);
	}
	else
	{
		_sntprintf_s(s_retbuf, _TRUNCATE, 
			_T("WinErr=%d (FormatMessage does not known this error-code)"), 
			winerr);
	}
	
	return s_retbuf;
}

WCHAR *HexdumpW(const WCHAR *pszw, int count, WCHAR *hexbuf, int bufchars)
{
	if (count < 0)
		count = (int)wcslen(pszw);
	
	for (int i = 0; i < count; i++)
	{
		_snwprintf_s(hexbuf + i * 5, bufchars - i * 5, _TRUNCATE,
			L"%04X ", (unsigned short)pszw[i]);
	}

	int wlen = (int)wcslen(hexbuf);
	if (wlen > 0 && hexbuf[wlen - 1] == L' ')
		hexbuf[wlen - 1] = L'\0';

	return hexbuf;
}

char *HexdumpA(const char *pbytes, int count, char *hexbuf, int bufchars)
{
	if (count < 0)
		count = (int)strlen(pbytes);
	
	for (int i = 0; i < count; i++)
	{
		_snprintf_s(hexbuf + i * 3, bufchars - i * 3, _TRUNCATE,
			"%02X ", (unsigned char)pbytes[i]);
	}

	int alen = (int)strlen(hexbuf);
	if (alen > 0 && hexbuf[alen - 1] == ' ')
		hexbuf[alen - 1] = L'\0';

	return hexbuf;
}

bool ishexdigit(TCHAR c)
{
	if((c>='0' && c<='9') || (c>='a' && c<='f') || (c>='A' && c<='F'))
		return true;
	else
		return false;
}

int qsort_CompareString(void* context, const void* item1, const void* item2)
{
	const TCHAR* text1 = *(const TCHAR**)item1;
	const TCHAR* text2 = *(const TCHAR**)item2;

	int cmpret = CompareString(LOCALE_USER_DEFAULT, 0, 
		text1, (int)wcslen(text1), text2, (int)wcslen(text2));
	return cmpret - 2;
}

bool Is_LCID_unspecified(LCID lcid)
{
	if (lcid==LOCALE_CUSTOM_UNSPECIFIED || lcid==LOCALE_CUSTOM_USER_DEFAULT)
		return true;
	else
		return false;
}
