/*
This program shows various layers of locales on Windows system.
This should help user discriminate the abstract and ubiquitous word "locale".
*/

#include "utils.h"
#include <muiload.h>

const TCHAR *g_szversion = _T("1.0.5");

LCID g_set_thread_lcid = 0; // If not 0, will call SetThreadLocale() with this value.
const TCHAR *g_set_crtlocale = NULL;

////////

void print_api_notavai(const TCHAR *apiname)
{
	my_tprintf(_T("WinAPI %s() not available on this OS.\n"), apiname);
}

const TCHAR *get_ll2info(LCID lcid, LCTYPE ll2type)
{
	static TCHAR s_info[200];
	GetLocaleInfo(lcid, ll2type, s_info, ARRAYSIZE(s_info)-1);
	return s_info;
}

// Dynamic loading of some Vista+ WinAPI. 
DEFINE_DLPTR_WINAPI("kernel32.dll", GetSystemDefaultLocaleName)
DEFINE_DLPTR_WINAPI("kernel32.dll", GetUserDefaultLocaleName)
DEFINE_DLPTR_WINAPI("kernel32.dll", LCIDToLocaleName)
DEFINE_DLPTR_WINAPI("kernel32.dll", LocaleNameToLCID)

void verify_locname_lcid_match(const TCHAR *locname, LCID lcid)
{
	// Locale-name is like: en-US
	// LCID is like: 0x00000409

	if(!dlptr_LocaleNameToLCID)
		return;

	LCID lcid2 = dlptr_LocaleNameToLCID(locname, LOCALE_ALLOW_NEUTRAL_NAMES); //  LOCALE_ALLOW_NEUTRAL_NAMES since Win7
	// -- TODO: Check for retval LOCALE_CUSTOM_DEFAULT and LOCALE_CUSTOM_UNSPECIFIED
	if(lcid!=lcid2)
	{
		my_tprintf(_T("  [unexpect] LocaleNameToLCID(\"%s\") returns %s (not match!)\n"), 
			locname, StrLCID(lcid2));
	}
	// 

	TCHAR locname2[LOCALE_NAME_MAX_LENGTH+1] = {};
	dlptr_LCIDToLocaleName(lcid, locname2, LOCALE_NAME_MAX_LENGTH, LOCALE_ALLOW_NEUTRAL_NAMES);
	//
	if(_tcscmp(locname, locname2)!=0)
	{
		my_tprintf(_T("  [unexpect] LCIDToLocaleName(0x%08u) returns: %s (not match!)\n"),
			lcid, locname2);
	}
}

void LL2_print_ansicodepage_and_oemcodepage(LCID lcid)
{
	my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s\n"), 
		get_ll2info(lcid, LOCALE_IDEFAULTANSICODEPAGE));

	my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s\n"), 
		get_ll2info(lcid, LOCALE_IDEFAULTCODEPAGE));
}

void LL2_print_LCID_Desctext(LCID lcid)
{
	my_tprintf(_T("  : LCID English desc: %s\n"), Desctext_from_LCID(lcid, false));
	my_tprintf(_T("  : LCID Native  desc: %s\n"), Desctext_from_LCID(lcid, true));
}

void do_work()
{
	LCID lcid = 0; 
	TCHAR locname[LOCALE_NAME_MAX_LENGTH+1] = {};
	LANGID langid = 0;
		
	/// System-level ///

	lcid = GetSystemDefaultLCID();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetSystemDefaultLCID()  => %s (LangID=%u, decimal)\n"), 
		StrLCID(lcid), langid);
	LL2_print_LCID_Desctext(langid);

	if(dlptr_GetSystemDefaultLocaleName)
	{
		locname[0] = 0;
		dlptr_GetSystemDefaultLocaleName(locname, LOCALE_NAME_MAX_LENGTH);
		my_tprintf(_T("GetSystemDefaultLocaleName() =>  %s\n"), locname);

		verify_locname_lcid_match(locname, lcid);
	}

	LL2_print_ansicodepage_and_oemcodepage(lcid);

	langid = GetSystemDefaultUILanguage();
	my_tprintf(_T("GetSystemDefaultUILanguage() => 0x%04X\n"), langid);
	LL2_print_LCID_Desctext(langid);

	newline();

	/// User-level ///

	lcid = GetUserDefaultLCID();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetUserDefaultLCID()    => %s (LangID=%u, decimal)\n"), 
		StrLCID(lcid), langid);
	LL2_print_LCID_Desctext(langid);

	if(dlptr_GetUserDefaultLocaleName)
	{
		locname[0] = 0;
		dlptr_GetUserDefaultLocaleName(locname, LOCALE_NAME_MAX_LENGTH);
		my_tprintf(_T("GetUserDefaultLocaleName()   =>  %s\n"), locname);

		verify_locname_lcid_match(locname, lcid);
	}

	LL2_print_ansicodepage_and_oemcodepage(lcid);

	langid = GetUserDefaultUILanguage();
	my_tprintf(_T("GetUserDefaultUILanguage()   => 0x%04X\n"), langid);
	LL2_print_LCID_Desctext(langid);

	newline();

	/// Thread locale /// 

	lcid = GetThreadLocale();
	my_tprintf(_T("GetThreadLocale()   => %s\n"), StrLCID(lcid));
	LL2_print_ansicodepage_and_oemcodepage(lcid);

	newline();

	/// Show console-codepage ///

	UINT orig_icp = GetConsoleCP();
	UINT orig_ocp = GetConsoleOutputCP();
	my_tprintf(_T("Current GetConsoleCP()       = %d\n"), orig_icp);
	my_tprintf(_T("Current GetConsoleOutputCP() = %d\n"), orig_ocp);

	newline();

	/// Check what CRT locale() tells us.

	const TCHAR *crtlocstr = _tsetlocale(LC_ALL, NULL); // query current
	my_tprintf(_T("setlocale(LC_ALL) query returns: \n  %s\n"), crtlocstr);

}

int apply_startup_user_params(TCHAR *argv[])
{
	const TCHAR szThreadLcid[]   = _T("threadlcid:");
	const int   nzThreadLcid     = ARRAYSIZE(szThreadLcid)-1;
	// -- example: to call SetThreadLocale(0x411); jp-JP , use:
	//		threadlcid:0x0411
	// or
	//		threadlcid:1041

	const TCHAR szCrtLocale[]   = _T("crtlocale:");
	const int   nzCrtLocale     = ARRAYSIZE(szCrtLocale)-1;

	const TCHAR *psz_threadlcid = NULL;

	int params = 0;
	for(; *argv!=NULL; argv++, params++)
	{
		if(_tcsnicmp(*argv, szThreadLcid, nzThreadLcid)==0)
		{
			psz_threadlcid = (*argv)+nzThreadLcid;
		}
		else if(_tcsnicmp(*argv, szCrtLocale, nzCrtLocale)==0) 
		{
			g_set_crtlocale = (*argv)+nzCrtLocale;
		}
		else
			break;
	}

	if(psz_threadlcid)
	{
		g_set_thread_lcid = _tcstoul(psz_threadlcid, NULL, 0);
	}

	return params;
}

TCHAR * join_msz_strings(const TCHAR *msz, int totchars, TCHAR outbuf[], int bufchars)
{
	outbuf[0] = 0;
	for(; ;)
	{
		int onelen = _tcslen(msz);
		if(*msz=='\0')
			break;

		_tcscat_s(outbuf, bufchars, msz);
		_tcscat_s(outbuf, bufchars, _T(";"));

		//my_tprintf(_T("  %s\n"), msz);
		msz += onelen+1;
	}
	return outbuf;
}

BOOL CALLBACK EnumUILanguagesProc(LPTSTR lpUILanguageString, LONG_PTR lParam)
{
	my_tprintf(_T("UIlang has: %s\n"), lpUILanguageString);

	WCHAR mszLangchk[80];
	_tcscpy_s(mszLangchk, lpUILanguageString);
	mszLangchk[_tcslen(lpUILanguageString)+1] = '\0'; // need double NULL
	WCHAR arFallback[80] = L"";
	DWORD nFallback = 80;
	DWORD LangAttr;
	BOOL succ = GetUILanguageInfo(MUI_LANGUAGE_NAME, mszLangchk, 
		arFallback, &nFallback, &LangAttr);

	if(succ)
	{
		TCHAR concat[40] = {};
		my_tprintf(_T("  Fallback: %s , Attr=0x%04X\n"), 
			join_msz_strings(arFallback, nFallback, concat, ARRAYSIZE(concat)),
			LangAttr
			);
	}

	return true;
}


void test()
{
	BOOL succ = 0, succ2 = 0;
	TCHAR outbuf[80]={}, outbuf2[80]={}, outbuf3[80]={};
#if 0
	TCHAR langtag[LOCALE_NAME_MAX_LENGTH]={};
	GetUserDefaultLocaleName(langtag, ARRAYSIZE(langtag));
	NLSVERSIONINFOEX nlsver = {sizeof(NLSVERSIONINFOEX)};
	GetNLSVersionEx(COMPARE_STRING, langtag, &nlsver);

	const TCHAR * langtag= L"uz-UZ";
	succ = IsValidLocaleName(langtag); 
	int ret = GetLocaleInfoEx(langtag, LOCALE_SSCRIPTS, outbuf, 80);
#endif
	ULONG langs = 0;
	ULONG bufsize = 80, bufsize2 = 80;
 	succ = GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &langs, outbuf, &bufsize);

	succ2 = GetUILanguageFallbackList(outbuf2, 80, &bufsize2);

	wprintf(L"%s\n", join_msz_strings(outbuf2, bufsize2, outbuf3, 80));

	succ = EnumUILanguages(EnumUILanguagesProc, 
		MUI_LANGUAGE_NAME, 
		0);

	const WCHAR *mszLangchk = L"de-CH\0";
	WCHAR arFallback[80] = L"XYZ";
	DWORD nFallback = 80;
	DWORD LangAttr;
	succ = GetUILanguageInfo(MUI_LANGUAGE_NAME, mszLangchk, 
		arFallback, &nFallback, &LangAttr);
}

int _tmain(int argc, TCHAR *argv[])
{
test();
	setlocale(LC_CTYPE, "");
//	setlocale(LC_ALL, "cht_JPN.936"); // OK for VC2010 CRT, ="Chinese (Traditional)_Japan.936"

	app_print_version(argv[0], g_szversion);

	apply_startup_user_params(argv+1);

	if(g_set_thread_lcid>0)
	{
		my_tprintf(_T("Startup: Call SetThreadLocale(0x%04X); \n"), g_set_thread_lcid);
		BOOL succ = SetThreadLocale(g_set_thread_lcid);
		if(succ)
		{
			LCID lcid2 = GetThreadLocale();
			if(g_set_thread_lcid==lcid2) // OK
			{
				if(g_set_crtlocale==NULL)
					g_set_crtlocale = _T(""); // so that setlocale is called later
				
			}
			else
			{
				my_tprintf(_T("[Unexpect] GetThreadLocale() does NOT report the LCID wet just set!\n"));
			}
		}
		else
		{
			my_tprintf(_T("Startup:      SetThreadLocale() fail. WinErr=%d\n"), GetLastError());
		}
	}

	if(g_set_crtlocale)
	{
		my_tprintf(_T("Startup: Call setlocale(LC_CTYPE, \"%s\"); \n"), g_set_crtlocale);
		const TCHAR *locret = _tsetlocale(LC_CTYPE, g_set_crtlocale);
		my_tprintf(_T("  > %s\n"), locret);
	}

	do_work();

	return 0;
}
