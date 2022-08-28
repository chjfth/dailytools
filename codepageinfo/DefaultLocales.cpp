#include "utils.h"
#include "..\cinclude\dlptr_winapi.h"

const TCHAR *g_szversion = _T("1.0.2");

const TCHAR *StrLCID(LCID lcid)
{
	static TCHAR s_szLCID[20];
	_sntprintf_s(s_szLCID, ARRAYSIZE(s_szLCID), _T("0x%04X.%04X"), lcid>>16, lcid&0xffff);
	return s_szLCID;
}

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



int _tmain(int argc, TCHAR *argv[])
{
	_tsetlocale(LC_CTYPE, _T(""));

	app_print_version(argv[0], g_szversion);

	do_work();


	return 0;
}
