/*
This program shows various layers of locales on Windows system.
This should help user discriminate the abstract and ubiquitous word "locale".
*/

#include "utils.h"
#include <muiload.h>

const TCHAR *g_szversion = _T("1.5.1");

LCID g_set_thread_lcid = 0; // If not 0, will call SetThreadLocale() with this value.
const TCHAR *g_set_crtlocale = _T("");
int g_crtmbcp = 0;  // for CRT _setmbcp();
int g_set_threadui_lang = -1; // 0 consider a valid value to set
int g_consolecp = 0;
bool g_pause_on_quit = false;

WCHAR g_wsSetClipboard[100];
int g_nSetClipboard = 0;

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

// Dynamic loading of some Vista+ WinAPI, so this program runs on WinXP(if compiled with VC2010). 
DEFINE_DLPTR_WINAPI("kernel32.dll", GetSystemDefaultLocaleName)
DEFINE_DLPTR_WINAPI("kernel32.dll", GetUserDefaultLocaleName)
DEFINE_DLPTR_WINAPI("kernel32.dll", LCIDToLocaleName)
DEFINE_DLPTR_WINAPI("kernel32.dll", LocaleNameToLCID)
DEFINE_DLPTR_WINAPI("kernel32.dll", GetThreadUILanguage)

void verify_locname_lcid_match(const TCHAR *locname, LCID lcid)
{
	// Locale-name is like: en-US
	// LCID is like: 0x00000409

	if(!dlptr_LocaleNameToLCID)
		return;

	LCID lcid2 = dlptr_LocaleNameToLCID(locname, LOCALE_ALLOW_NEUTRAL_NAMES); //  LOCALE_ALLOW_NEUTRAL_NAMES since Win7
	if(lcid!=lcid2)
	{
		my_tprintf(_T("  [unexpect] LocaleNameToLCID(\"%s\") returns %s (not match!)\n"), 
			locname, HexstrLCID(lcid2));
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
	UINT true_acp = GetACP();
	if(verify_syscp)
	{
		UINT locale_acp = _ttoi(psz_ansi_codepage);
		if(true_acp==locale_acp)
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s (=GetACP())\n"), 
				psz_ansi_codepage);
		}
		else if(true_acp==CP_UTF8)
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s\n"),
				psz_ansi_codepage);
			my_tprintf(_T("  > But, GetACP()=  65001, so UTF8ACP is enabled on this OS.\n"));
		}
		else
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s !!!\n"), 
				psz_ansi_codepage);
			my_tprintf(_T("  > [PANIC!] This does NOT match GetACP()=%u !!!\n"), true_acp);
		}
	}
	else
	{
		my_tprintf(_T("  > LOCALE_IDEFAULTANSICODEPAGE  (ANSI codepage): %s\n"), 
			psz_ansi_codepage);
	}

	const TCHAR *psz_oem_codepage = get_ll2info(lcid, LOCALE_IDEFAULTCODEPAGE);
	UINT true_oemcp = GetOEMCP();
	if(verify_syscp)
	{
		UINT locale_oemcp = _ttoi(psz_oem_codepage);
		if(true_oemcp==locale_oemcp)
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s (=GetOEMCP())\n"), 
				psz_oem_codepage);
		}
		else if (true_oemcp==CP_UTF8)
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s\n"),
				psz_oem_codepage);
			my_tprintf(_T("  > But, GetOEMCP()=65001, so UTF8ACP is enabled on this OS.\n"));
		}
		else
		{
			my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s !!!\n"), 
				psz_oem_codepage);
			my_tprintf(_T("  > [PANIC!] This does NOT match GetOEMCP()=%u !!!\n"), true_oemcp);
		}		
	}
	else
	{
		my_tprintf(_T("  > LOCALE_IDEFAULTCODEPAGE       (OEM codepage): %s\n"), 
			psz_oem_codepage);
	}
}

void LL2_print_LANGID_Desctext(LANGID langid)
{
	my_tprintf(_T("  : LangID English   desc: %s\n"), Desctext_from_LANGID(langid, DepictLang_English));
	my_tprintf(_T("  : LangID Localized desc: %s\n"), Desctext_from_LANGID(langid, DepictLang_localized));
	my_tprintf(_T("  : LangID Native    desc: %s\n"), Desctext_from_LANGID(langid, DepictLang_native));
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

static const TCHAR *LANGID_NumericDesc(LANGID langid)
{
	static TCHAR s_szDesc[40];
	if(Is_LCID_unspecified(langid))
	{
		_sntprintf_s(s_szDesc, _TRUNCATE, _T("unspecified"));
	}
	else
	{
		_sntprintf_s(s_szDesc, _TRUNCATE, _T("LangID=%u decimal"), langid);
	}
	return s_szDesc;
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
	my_tprintf(_T("GetSystemDefaultUILanguage() => 0x%04X   (%u decimal)\n"), langid, langid);
	LL2_print_LANGID_Desctext(langid);

	/// GetUserDefaultUILanguage() ///

	langid = GetUserDefaultUILanguage();
	my_tprintf(_T("GetUserDefaultUILanguage()   => 0x%04X   (%u decimal)\n"), langid, langid);
	LL2_print_LANGID_Desctext(langid);

	/// GetThreadUILanguage() ///

	if (dlptr_GetThreadUILanguage)
	{
		langid = dlptr_GetThreadUILanguage();
		my_tprintf(_T("GetThreadUILanguage()        => 0x%04X   (%u decimal)\n"), langid, langid);
		LL2_print_LANGID_Desctext(langid);
	}

	newline();

	/// Win32 System-locale ///

	lcid = GetSystemDefaultLCID();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetSystemDefaultLCID()  => %s   (%s)\n"),
		HexstrLCID(lcid), LANGID_NumericDesc(langid));

	if (dlptr_GetSystemDefaultLocaleName)
	{
		locname[0] = 0;
		dlptr_GetSystemDefaultLocaleName(locname, LOCALE_NAME_MAX_LENGTH);
		my_tprintf(_T("GetSystemDefaultLocaleName() =>  %s\n"), locname);
		verify_locname_lcid_match(locname, lcid);
	}

	LL2_print_LANGID_Desctext(langid);
	LL2_print_ansicodepage_and_oemcodepage(lcid, true);

	newline();

	/// Win32 User-locale ///

	lcid = GetUserDefaultLCID();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetUserDefaultLCID()    => %s   (%s)\n"),
		HexstrLCID(lcid), LANGID_NumericDesc(langid));

	if (dlptr_GetUserDefaultLocaleName)
	{
		locname[0] = 0;
		dlptr_GetUserDefaultLocaleName(locname, LOCALE_NAME_MAX_LENGTH);
		my_tprintf(_T("GetUserDefaultLocaleName()   =>  %s\n"), locname);
		verify_locname_lcid_match(locname, lcid);
	}

	LL2_print_LANGID_Desctext(langid);
	LL2_print_ansicodepage_and_oemcodepage(lcid);

	newline();

	/// Thread-locale (almost obsolete since Win7) /// 

	lcid = GetThreadLocale();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetThreadLocale()       => %s   (%s)\n"),
		HexstrLCID(lcid), LANGID_NumericDesc(langid));

	LL2_print_LANGID_Desctext(langid);
	LL2_print_ansicodepage_and_oemcodepage(lcid);
	
	newline();

	/// GetKeyboardLayout() ///
	
	HKL curhkl = GetKeyboardLayout(0); // 0 means current thread
	langid = LOWORD(curhkl);
	dlptr_LCIDToLocaleName(langid, locname, LOCALE_NAME_MAX_LENGTH, 0);

	my_tprintf(_T("GetKeyboardLayout(0) = %s [%s]\n"), 
		HexstrLCID(langid), locname);

	if (g_nSetClipboard > 0)
	{
		easySetClipboardText(g_wsSetClipboard, g_nSetClipboard);

		my_tprintf(_T("  > Sent %d WCHARs to Clipboard as CF_UNICODETEXT.\n"), g_nSetClipboard);
		my_tprintf(_T("  > %s\n"), g_wsSetClipboard);
	}
	
	newline();
	
	/// Check/Probe what CRT locale() tells us. ///

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
		my_tprintf(_T("  [probed] .lc_codepage = %d (offset %d-int)\n"), lc_codepage, lccoffs);
	}
	
	const _digged_mbcinfo *p_mbcinfo = (_digged_mbcinfo*)(lcnow->mbcinfo);
	my_tprintf(_T("  [probed] .mb_codepage = %d\n"), 
		p_mbcinfo->mb_codepage);
}

void print_help()
{
	const TCHAR *helptext =
_T("Parameter help:\n")
_T("  uilangid:<uilangid>  Call SetThreadUILanguage(uilangid); on start. 0 is ok.\n")
_T("  thrdlcid:<thrlcid>   Call SetThreadLocale(thrlcid); on start.\n")
_T("  crtlocale:<locstr>   Call setlocale(LC_ALL, locstr); on start.\n")
_T("                       If <locstr> is '-', omit calling setlocale().\n")
_T("  crtmbcp:<mbcp>       Call _setmbcp(mbcp); on start.\n")
_T("  consolecp:<ccp>      Call SetConsoleCP(ccp); and SetConsoleOutputCP(ccp);.\n")
_T("  \n")
_T("  Trailing hex tokens constitute a WCHAR string, and are sent to Clipboard.\n")
_T("\n")
_T("Examples:\n")
_T("  DefaultLocales uilangid:0x0404 crtlocale:zh-TW consolecp:950\n")
_T("  DefaultLocales thrdlcid:0x0804 crtlocale:zh-CN consolecp:936\n")
_T("  DefaultLocales crtlocale:.65001 consolecp:65001\n")
_T("  DefaultLocales crtlocale:japanese_Japan\n")
_T("  DefaultLocales crtlocale:-\n")
_T("\n")
_T("  DefaultLocales 41 42 96FB\n")
_T("    -- This will send Unicode text \"AB電\" (3 WCHARs) to Clipboard.\n")
_T("\n")
_T("To pause before program quit, rename exe to have word \"pause\".\n")
;
	my_tprintf(_T("%s"), helptext);
}

int apply_startup_user_params(TCHAR *argv[])
{
	// If filename contains "pause", we'll pause(wait for a key) at program end.
	// 
	const TCHAR *exename = app_GetFilenamePart(argv[0]);
	if (_tcsstr(exename, _T("pause")) != NULL)
	{
		g_pause_on_quit = true;
	}

	argv++;

	if(*argv && _tcscmp(*argv, _T("-?"))==0)
	{
		print_help();
		exit(1);
	}
	
	const TCHAR szThrdLcid[]   = _T("thrdlcid:"); 
	const int   nzThrdLcid     = ARRAYSIZE(szThrdLcid)-1;
	// -- example: to call SetThreadLocale(0x411); jp-JP , use:
	//		thrdlcid:0x0411
	// or
	//		thrdlcid:1041
	//		
	// This param was named `threadlcid` before 1.4.0, we keep its compatibility
	const TCHAR szThreadLcid[] = _T("threadlcid:");
	const int   nzThreadLcid = ARRAYSIZE(szThreadLcid) - 1;

	const TCHAR szThreadUILang[] = _T("uilangid:");
	const int   nzThreadUILang = ARRAYSIZE(szThreadUILang) - 1;

	const TCHAR szCrtmbcp[] = _T("crtmbcp:");
	const int   nzCrtmbcp = ARRAYSIZE(szCrtmbcp)-1;
	
	const TCHAR szCrtLocale[]   = _T("crtlocale:");
	const int   nzCrtLocale     = ARRAYSIZE(szCrtLocale)-1;

	const TCHAR szConsoleCP[] = _T("consolecp:");
	const int   nzConsoleCP   = ARRAYSIZE(szConsoleCP)-1;

	const TCHAR szDEBUG[] = _T("debug:");
	const int   nzDEBUG = ARRAYSIZE(szDEBUG) - 1;

	int params = 0;
	for(; *argv!=NULL; argv++, params++)
	{
		if(_tcsnicmp(*argv, szThrdLcid, nzThrdLcid)==0)
		{
			const TCHAR *psz_thrdlcid = (*argv)+nzThrdLcid;
			g_set_thread_lcid = _tcstoul(psz_thrdlcid, NULL, 0);

			if(g_set_thread_lcid==0)
			{
				my_tprintf(_T("Invalid thrdlcid input value: %s\n"), psz_thrdlcid);
				exit(1);
			}
		}
		else if (_tcsnicmp(*argv, szThreadLcid, nzThreadLcid)==0) // old name for szThrdLcid
		{
			const TCHAR* psz_threadlcid = (*argv) + nzThreadLcid;
			g_set_thread_lcid = _tcstoul(psz_threadlcid, NULL, 0);

			if (g_set_thread_lcid == 0)
			{
				my_tprintf(_T("Invalid threadlcid input value: %s (Please use new name: thrdlcid)\n"), psz_threadlcid);
				exit(1);
			}
		}
		else if (_tcsnicmp(*argv, szThreadUILang, nzThreadUILang) == 0)
		{
			const TCHAR* psz_threaduilang = (*argv) + nzThreadUILang;
			g_set_threadui_lang = _tcstoul(psz_threaduilang, NULL, 0);

			if( !isdigit(psz_threaduilang[0]) || g_set_threadui_lang<0 )
			{
				my_tprintf(_T("Invalid uilangid input value: %s\n"), psz_threaduilang);
				exit(1);
			}
		}
		else if(_tcsnicmp(*argv, szCrtLocale, nzCrtLocale)==0)
		{
			g_set_crtlocale = (*argv)+nzCrtLocale;
		}
		else if(_tcsnicmp(*argv, szCrtmbcp, nzCrtmbcp)==0)
		{
			const TCHAR *psz_crtmbcp = (*argv) + nzCrtmbcp;
			g_crtmbcp = _tcstoul(psz_crtmbcp, NULL, 0);

			if(g_crtmbcp==0)
			{
				my_tprintf(_T("Invalid crtmbcp input value: %s\n"), psz_crtmbcp);
				exit(1);
			}
		}
		else if(_tcsnicmp(*argv, szConsoleCP, nzConsoleCP)==0)
		{
			const TCHAR *psz_consolecp = (*argv) + nzConsoleCP;
			g_consolecp = _tcstoul(psz_consolecp, NULL, 0);

			if(g_consolecp==0)
			{
				my_tprintf(_T("Invalid consolecp input value: %s\n"), psz_consolecp);
				exit(1);
			}
		}
		else if (_tcsnicmp(*argv, szDEBUG, nzDEBUG) == 0)
		{
			const TCHAR* psz_debug = (*argv) + nzDEBUG;
			if (_tcsicmp(psz_debug, _T("break")) == 0)
			{
				my_tprintf(_T("Startup: Now calling DebugBreak(), so you have a chance to attach Visual C++ debugger for this very consolecp.exe instance.\n"));
				DebugBreak();
			}
			else if (_tcsicmp(psz_debug, _T("pause")) == 0)
			{
				my_tprintf(_T("Startup: [PAUSE] Press a key to continue, or attach a debugger.\n"));
				_getch();
			}
		}
		else if(ishextoken(*argv))
		{
			// This marks the start of WCHAR stream params.
			// I will send Unicode text by this stream to the Clipboard, so that we can see
			// CF_LOCALE value in Clipboard is determined by GetKeyboardLayout().

			g_nSetClipboard = collect_hexrpw_from_argv(argv, g_wsSetClipboard, ARRAYSIZE(g_wsSetClipboard));
			break;
		}
		else
		{
			my_tprintf(_T("[ERROR] Unrecognized parameter: %s\n"), *argv);
			my_tprintf(_T("\n"));
			my_tprintf(_T("Run with \"%s -?\" to see valid parameters.\n"), exename);
			exit(1);
		}
	}

	return params;
}

TCHAR * join_msz_strings(const TCHAR *msz, int totchars, TCHAR outbuf[], int bufchars)
{
	outbuf[0] = 0;
	for(; ;)
	{
		int onelen = (int)_tcslen(msz);
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

	//setlocale(LC_ALL, "cht_JPN.936");
	// VC2010 style CRT-locale string (just memo, do not call it here):
	//		Chinese (Traditional)_Japan.936". 
	//		cht_Taiwan.950

	app_print_version(argv[0], g_szversion);

	apply_startup_user_params(argv);

	if (g_set_threadui_lang >= 0)
	{
		my_tprintf(_T("Startup: Call SetThreadUILanguage(0x%04X); \n"), g_set_threadui_lang);
		LANGID uilang_ret = SetThreadUILanguage(g_set_threadui_lang);
		if (g_set_threadui_lang == 0 || uilang_ret == g_set_threadui_lang)
		{
			my_tprintf(_T("  > SetThreadUILanguage() returns 0x%04X. Success.\n"), uilang_ret);
		}
		else
		{
			my_tprintf(_T("[Unexpect] SetThreadUILanguage() returns 0x%04X. %s\n"),
				uilang_ret, app_WinErrStr());
		}
	}

	if(g_set_thread_lcid>0)
	{
		my_tprintf(_T("Startup: Call SetThreadLocale(0x%04X); \n"), g_set_thread_lcid);
		BOOL succ = SetThreadLocale(g_set_thread_lcid);
		if(succ)
		{
			LCID lcid2 = GetThreadLocale();
			if(g_set_thread_lcid==lcid2) // OK
			{	
				my_tprintf(_T("  > Success. \n"), lcid2);
			}
			else
			{
				my_tprintf(_T("[Unexpect] GetThreadLocale()=0x%04X, does NOT match the LCID wet just set!\n"), lcid2);
				exit(1);
			}
		}
		else
		{
			my_tprintf(_T("Startup:      SetThreadLocale() fail. %s\n"), app_WinErrStr());
			exit(1);
		}
	}

	if(g_crtmbcp)
	{
		my_tprintf(_T("Startup: Call _setmbcp(%d); \n"), g_crtmbcp);
		_setmbcp(g_crtmbcp);
	}

	if(g_set_crtlocale[0]!='-')
	{
		my_tprintf(_T("Startup: Call setlocale(LC_ALL, \"%s\"); \n"), g_set_crtlocale);
		const TCHAR *locret = _tsetlocale(LC_ALL, g_set_crtlocale);

		if (locret)
		{
			my_tprintf(_T("  > %s\n"), locret);
		}
		else
		{
			my_tprintf(_T("  > setlocale() error. Probably your locale string is invalid.\n"));

			if (g_set_crtlocale[2] == '-')
			{
				my_tprintf(_T("  > Note: If _MSC_VER<1900 (before VC2015), \"zh-CN\" etc is not valid format.\n"));
			}

			exit(1);
		}
	}
	else
	{
		my_tprintf(_T("Omit calling setlocale(), so \"C\" locale will be in effect.\n"));
	}

	if(g_consolecp)
	{
		my_tprintf(_T("Startup: Set console input/output codepage to %d.\n"), g_consolecp);
		BOOL succ1 = SetConsoleCP(g_consolecp);
		if (!succ1)
			my_tprintf(_T("SetConsoleCP() fail. %s\n"), app_WinErrStr());
		
		BOOL succ2 = SetConsoleOutputCP(g_consolecp);
		if (!succ2)
			my_tprintf(_T("SetConsoleOutputCP() fail. %s\n"), app_WinErrStr());

		if (!succ1 || !succ2)
			exit(1);
	}

	do_work();

	extern void custom_test();
	custom_test();
	
	if(g_pause_on_quit)
	{
		// If user double clicks this command-line exe from Explorer, then he may need this.
		my_tprintf(_T("\n"));
		my_tprintf(_T("==Press a key to end this program.==\n"));
		_getch();
	}

	return 0;
}
