/*
This program shows various layers of locales on Windows system.
This should help user discriminate the abstract and ubiquitous word "locale".
*/

#include "utils.h"
#include "..\cinclude\dlptr_winapi.h"

const TCHAR *g_szversion = _T("1.0.3");

LCID g_set_thread_lcid = 0; // If not 0, will call SetThreadLocale() with this value.

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


void verify_locname_lcid_match(const TCHAR *locname, LCID lcid)
{
	// Locale-name is like: en-US
	// LCID is like: 0x00000409

	LCID lcid2 = LocaleNameToLCID(locname, LOCALE_ALLOW_NEUTRAL_NAMES); //  LOCALE_ALLOW_NEUTRAL_NAMES since Win7
	// -- TODO: Check for retval LOCALE_CUSTOM_DEFAULT and LOCALE_CUSTOM_UNSPECIFIED
	if(lcid!=lcid2)
	{
		my_tprintf(_T("  [unexpect] LocaleNameToLCID(\"%s\") returns %s (not match!)\n"), 
			locname, StrLCID(lcid2));
	}
	// 

	TCHAR locname2[LOCALE_NAME_MAX_LENGTH+1] = {};
	LCIDToLocaleName(lcid, locname2, LOCALE_NAME_MAX_LENGTH, LOCALE_ALLOW_NEUTRAL_NAMES);
	//
	if(_tcscmp(locname, locname2)!=0)
	{
		my_tprintf(_T("  [unexpect] LCIDToLocaleName(0x%08u) returns: %s (not match!)\n"),
			lcid, locname2);
	}
}

void LL2_print_ansicodepage_and_oemcodepage(LCID lcid)
{
	my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE (ANSI codepage): %s\n"), 
		get_ll2info(lcid, LOCALE_IDEFAULTANSICODEPAGE));

	my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE      (OEM codepage): %s\n"), 
		get_ll2info(lcid, LOCALE_IDEFAULTCODEPAGE));
}

void do_work()
{
	LCID lcid = 0; 
	TCHAR locname[LOCALE_NAME_MAX_LENGTH+1] = {};
	LANGID langid = 0;
		
	/// System-level ///

	lcid = GetSystemDefaultLCID();
	my_tprintf(_T("GetSystemDefaultLCID() => %s (LangID=%u, decimal)\n"), 
		StrLCID(lcid), LANGIDFROMLCID(lcid));

	if(dlptr_GetSystemDefaultLocaleName)
	{
		locname[0] = 0;
		dlptr_GetSystemDefaultLocaleName(locname, LOCALE_NAME_MAX_LENGTH);
		my_tprintf(_T("GetSystemDefaultLocaleName() => %s\n"), locname);

		verify_locname_lcid_match(locname, lcid);

		LL2_print_ansicodepage_and_oemcodepage(lcid);
	}
	else
		print_api_notavai(_T("GetSystemDefaultLocaleName"));

	langid = GetSystemDefaultUILanguage();
	my_tprintf(_T("GetSystemDefaultUILanguage() => 0x%04X\n"), langid);

	newline();

	/// User-level ///

	lcid = GetUserDefaultLCID();
	my_tprintf(_T("GetUserDefaultLCID()   => %s (LangID=%u, decimal)\n"), 
		StrLCID(lcid), LANGIDFROMLCID(lcid));

	if(dlptr_GetUserDefaultLocaleName)
	{
		locname[0] = 0;
		dlptr_GetUserDefaultLocaleName(locname, LOCALE_NAME_MAX_LENGTH);
		my_tprintf(_T("GetUserDefaultLocaleName()   => %s\n"), locname);

		verify_locname_lcid_match(locname, lcid);

		LL2_print_ansicodepage_and_oemcodepage(lcid);
	}
	else
		print_api_notavai(_T("GetUserDefaultLocaleName"));

	langid = GetUserDefaultUILanguage();
	my_tprintf(_T("GetUserDefaultUILanguage()   => 0x%04X\n"), langid);

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

	const TCHAR *psz_start_lcid = _T("");

	int params = 0;
	for(; *argv!=NULL; argv++, params++)
	{
		if(_tcsnicmp(*argv, szThreadLcid, nzThreadLcid)==0)
		{
			psz_start_lcid = (*argv)+nzThreadLcid;
		}
		else
			break;
	}

	if(psz_start_lcid[0])
	{
		g_set_thread_lcid = _tcstoul(psz_start_lcid, NULL, 0);
	}

	return params;
}


int _tmain(int argc, TCHAR *argv[])
{
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
			if(g_set_thread_lcid==lcid2)
			{
				my_tprintf(_T("Startup: Call setlocale(LC_CTYPE, \"\"); \n"));
				setlocale(LC_CTYPE, "");
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

		newline();
	}

	do_work();

	return 0;
}
