/*
This program shows various layers of locales on Windows system.
This should help user discriminate the abstract and ubiquitous word "locale".
*/

#include "utils.h"
#include <muiload.h>

const TCHAR *g_szversion = _T("1.1.6");

LCID g_set_thread_lcid = 0; // If not 0, will call SetThreadLocale() with this value.
const TCHAR *g_set_crtlocale = NULL;
int g_consolecp = 0;
bool g_pause_on_quit = false;

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
	LL2_print_LANGID_Desctext(langid);

	/// WinAPI System-locale ///

	lcid = GetSystemDefaultLCID();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetSystemDefaultLCID()  => %s (LangID=%u, decimal)\n"), 
		HexstrLCID(lcid), langid);
	LL2_print_LANGID_Desctext(langid);

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
	LL2_print_LANGID_Desctext(langid);

	/// WinAPI User-locale ///

	lcid = GetUserDefaultLCID();
	langid = LANGIDFROMLCID(lcid);
	my_tprintf(_T("GetUserDefaultLCID()    => %s (LangID=%u, decimal)\n"), 
		HexstrLCID(lcid), langid);
	LL2_print_LANGID_Desctext(langid);

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
	TCHAR lcname[40] = _T("?");
	int retchars = LCIDToLocaleName(lcid, lcname, ARRAYSIZE(lcname), 0);
	if(retchars<=0)
	{
		my_tprintf(_T("[Unexpect!] LCIDToLocaleName(0x%04X ,...) fail!\n"), 
			lcid, app_WinErrStr());
	}
	my_tprintf(_T("GetThreadLocale() => %s (%s)\n"), HexstrLCID(lcid), lcname);
	
	LL2_print_LANGID_Desctext(LANGIDFROMLCID(lcid));
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

void print_help()
{
	const TCHAR *helptext =
_T("Parameter help:\n")
_T("  threadlcid:<lcid>      Call SetThreadLocale(lcid); on start.\n")
_T("  crtlocale:<locstr>     Call setlocale(LC_ALL, locstr); on start.\n")
_T("  consolecp:<ccp>        Call SetConsoleCP(ccp); and SetConsoleOutputCP(ccp);.\n")
_T("\n")
_T("Example:\n")
_T("  DefaultLocales threadlcid:0x0804 crtlocale:zh-CN consolecp:936\n")
_T("  DefaultLocales threadlcid:0x0404 crtlocale:zh-TW consolecp:950\n")
_T("  DefaultLocales crtlocale:.65001 consolecp:65001\n")
_T("  DefaultLocales crtlocale:japanese_Japan\n")
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
	
	const TCHAR szThreadLcid[]   = _T("threadlcid:");
	const int   nzThreadLcid     = ARRAYSIZE(szThreadLcid)-1;
	// -- example: to call SetThreadLocale(0x411); jp-JP , use:
	//		threadlcid:0x0411
	// or
	//		threadlcid:1041

	const TCHAR szCrtLocale[]   = _T("crtlocale:");
	const int   nzCrtLocale     = ARRAYSIZE(szCrtLocale)-1;

	const TCHAR szConsoleCP[] = _T("consolecp:");
	const int   nzConsoleCP   = ARRAYSIZE(szConsoleCP)-1;

	int params = 0;
	for(; *argv!=NULL; argv++, params++)
	{
		if(_tcsnicmp(*argv, szThreadLcid, nzThreadLcid)==0)
		{
			const TCHAR *psz_threadlcid = (*argv)+nzThreadLcid;
			g_set_thread_lcid = _tcstoul(psz_threadlcid, NULL, 0);
		}
		else if(_tcsnicmp(*argv, szCrtLocale, nzCrtLocale)==0) 
		{
			g_set_crtlocale = (*argv)+nzCrtLocale;
		}
		else if(_tcsnicmp(*argv, szConsoleCP, nzConsoleCP)==0)
		{
			const TCHAR *psz_consolecp = (*argv) + nzConsoleCP;
			g_consolecp = _tcstoul(psz_consolecp, NULL, 0);
		}
		else
		{
			my_tprintf(_T("[ERROR] Unrecognized parameter: %s\n"), *argv);
			my_tprintf(_T("\n"));
			my_tprintf(_T("Run with `%s -?` to see valid parameters.\n"), exename);
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

	setlocale(LC_ALL, "");
//	setlocale(LC_ALL, "cht_JPN.936"); // OK for VC2010 CRT, ="Chinese (Traditional)_Japan.936"

	app_print_version(argv[0], g_szversion);

	apply_startup_user_params(argv);

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
					g_set_crtlocale = _T(""); // so that setlocale() is called later
				
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

	if(g_set_crtlocale)
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

	if(g_pause_on_quit)
	{
		// If user double clicks this command-line exe from Explorer, then he may need this.
		my_tprintf(_T("\n"));
		my_tprintf(_T("==Press a key to end this program.==\n"));
		_getch();
	}
	
	return 0;
}
