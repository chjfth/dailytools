﻿/*
This program shows various layers of locales on Windows system.
This should help user discriminate the abstract and ubiquitous word "locale".
*/

#include "utils.h"
#include <muiload.h>

const TCHAR *g_szversion = _T("1.1.0");

LCID g_set_thread_lcid = 0; // If not 0, will call SetThreadLocale() with this value.
const TCHAR *g_set_crtlocale = NULL;

////////

// Define this struct to reflect first few members of VC2019 CRT __crt_multibyte_data.
// Lucky to see this definition here is compatible with VC2010.
struct _digged_mbcinfo
{
	long refcount;
	int mb_codepage;  // the so-called multibyte-codepage in a locale struct
	int ismbcdoepage; // 1=DBCS codepage(GBK, Big5 etc), 0=SBCS codepage
};


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

void LL2_print_ansicodepage_and_oemcodepage(LCID lcid, bool verify_syscp=false)
{
	const TCHAR *psz_ansi_codepage = get_ll2info(lcid, LOCALE_IDEFAULTANSICODEPAGE);
	UINT acp = GetACP();
	if(verify_syscp)
	{
		if(_ttoi(psz_ansi_codepage)==acp)
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s (=GetACP())\n"), 
				psz_ansi_codepage);
		}
		else
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s !!!\n"), 
				psz_ansi_codepage);
			my_tprintf(_T("  > [PANIC!] This does NOT match GetACP()=%u !!!\n"), acp);
		}
	}
	else
	{
		my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s\n"), 
			psz_ansi_codepage);
	}

	const TCHAR *psz_oem_codepage = get_ll2info(lcid, LOCALE_IDEFAULTCODEPAGE);
	UINT oemcp = GetOEMCP();
	if(verify_syscp)
	{
		if(_ttoi(psz_oem_codepage)==oemcp)
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s (=GetOEMCP())\n"), 
				psz_oem_codepage);
		}
		else
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s !!!\n"), 
				psz_oem_codepage);
			my_tprintf(_T("  > [PANIC!] This does NOT match GetOEMCP()=%u !!!\n"), oemcp);
		}		
	}
	else
	{
		my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s\n"), 
			psz_oem_codepage);
	}
}

void LL2_print_LCID_Desctext(LCID lcid)
{
	my_tprintf(_T("  : LCID English   desc: %s\n"), Desctext_from_LCID(lcid, DepictLang_English));
	my_tprintf(_T("  : LCID Localized desc: %s\n"), Desctext_from_LCID(lcid, DepictLang_localized));
	my_tprintf(_T("  : LCID Native    desc: %s\n"), Desctext_from_LCID(lcid, DepictLang_native));
}

int detect_lc_codepage_offset()
{
	// MSVCRT does not provide an API to tell us what the .lc_codepage value is
	// from a struct pointed to _locale_t, so we need to probe it ourselves.
	// On VC2010, it is at 1-int, on VC2019, it is on 2-int.
	
	const int tcodepage = 1252;
	char locstr[10] = {};
	_snprintf_s(locstr, _TRUNCATE, ".%d", tcodepage);
	
	_locale_t lct = _create_locale(LC_CTYPE, locstr);
	const int *plocinfo = (int*)lct->locinfo;

	const int maxprobe = 10;
	int i;
	for(i=0; i<maxprobe; i++)
	{
		if (plocinfo[i] == tcodepage)
			break;
	}

	_free_locale(lct);

	if (i < maxprobe)
		return i; // return value is `int` count
	else
		return -1;
}

void do_work()
{
	LCID lcid = 0; 
	TCHAR locname[LOCALE_NAME_MAX_LENGTH+1] = {};
	LANGID langid = 0;
		
	/// Show console-codepage ///

	UINT orig_icp = GetConsoleCP();
	UINT orig_ocp = GetConsoleOutputCP();
	my_tprintf(_T("Current GetConsoleCP()       = %d\n"), orig_icp);
	my_tprintf(_T("Current GetConsoleOutputCP() = %d\n"), orig_ocp);

	newline();

	/// GetSystemDefaultUILanguage() ///
	
	langid = GetSystemDefaultUILanguage();
	my_tprintf(_T("GetSystemDefaultUILanguage() => 0x%04X\n"), langid);
	LL2_print_LCID_Desctext(langid);

	/// WinAPI System-locale ///

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

	LL2_print_ansicodepage_and_oemcodepage(lcid, true);

	newline();

	/// GetUserDefaultUILanguage() ///

	langid = GetUserDefaultUILanguage();
	my_tprintf(_T("GetUserDefaultUILanguage()   => 0x%04X\n"), langid);
	LL2_print_LCID_Desctext(langid);

	/// WinAPI User-locale ///

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

	newline();

	/// Thread locale /// 

	lcid = GetThreadLocale();
	my_tprintf(_T("GetThreadLocale()   => %s\n"), StrLCID(lcid));
	LL2_print_ansicodepage_and_oemcodepage(lcid);

	newline();

	/// Check/Probe what CRT locale() tells us.

	const TCHAR *crtlocstr = _tsetlocale(LC_ALL, NULL); // query current
	my_tprintf(_T("setlocale(LC_ALL, NULL) query returns: \n  %s\n"), crtlocstr);

	_locale_t lcnow = _get_current_locale();

	int lccoffs = detect_lc_codepage_offset();
	if(lccoffs<0)
	{
		my_tprintf(_T("[Unexpect] detect_lc_codepage_offset() fail!\n"));
	}
	else
	{
		int lc_codepage = *((int*)(lcnow->locinfo) + lccoffs);
		my_tprintf(_T("[probed] .lc_codepage = %d (offset %d-int)\n"), lc_codepage, lccoffs);
	}
	
	const _digged_mbcinfo *p_mbcinfo = (_digged_mbcinfo*)(lcnow->mbcinfo);
	my_tprintf(_T("[probed] .mb_codepage = %d\n"), 
		p_mbcinfo->mb_codepage);
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


int _tmain(int argc, TCHAR *argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);
	_setmode(_fileno(stdout), _O_U8TEXT);
	
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
			my_tprintf(_T("Startup:      SetThreadLocale() fail. [%s]\n"), app_WinErrStr());
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
