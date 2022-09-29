/*
This program is for Windows XP, by calling EnumSystemLocales().
The newer API EnumSystemLocalesEx() is supported only since Windows Vista.
*/
#include "utils.h"

const TCHAR *g_szversion = _T("1.0.0");

struct EnumInfo_t
{
	int callbacks;
	int count;
	int empty;
} g_exi;

BOOL CALLBACK EnumLocalesProc(LPWSTR pszLcid)
{
	// pszLcid points to a string like "00000409", "00020804".
	// so, it is a numeric LCID in string form.

	EnumInfo_t &exi = g_exi;
	exi.callbacks++;

	if(!pszLcid || !pszLcid[0])
	{
		my_tprintf(_T("[callback #%d] Empty!!!\n"), exi.callbacks);
		exi.empty ++ ;
		return TRUE;
	}

	exi.count++;

	LCID lcid = _tcstoul(pszLcid, NULL, 16);
	
	TCHAR szLang[40] = {}, szRegn[40] = {};
	GetLocaleInfo(lcid, LOCALE_SENGLISHLANGUAGENAME, szLang, ARRAYSIZE(szLang));
	GetLocaleInfo(lcid, LOCALE_SENGLISHCOUNTRYNAME, szRegn, ARRAYSIZE(szRegn));

	TCHAR szACP[10] = {}, szOCP[10] = {};
	GetLocaleInfo(lcid, LOCALE_IDEFAULTANSICODEPAGE, szACP, ARRAYSIZE(szACP));
	GetLocaleInfo(lcid, LOCALE_IDEFAULTCODEPAGE, szOCP, ARRAYSIZE(szOCP));

	TCHAR szLangtag[20] = {}, sz_Langtag_[20] = {};
	if( GetLocaleInfo(lcid, LOCALE_SNAME, szLangtag, ARRAYSIZE(szLangtag)) )
	{
		// `LOCALE_SNAME` is valid on Vista+, got "en-US", "zh-CN" etc.
		_sntprintf_s(sz_Langtag_, ARRAYSIZE(sz_Langtag_), _T("(%s)"), szLangtag);
	}

	my_tprintf(_T("[%d] %s %s; %s @ %s ; ANSI/OEM[%s/%s]\n"), exi.count, 
		HexstrLCID(lcid), sz_Langtag_, szLang, szRegn, szACP, szOCP);

	return TRUE;
}

int AskUserForFlags()
{
	my_tprintf(_T("Select what to enumerate:\n"));
	my_tprintf(_T("[1] LCID_INSTALLED\n"));
	my_tprintf(_T("[2] LCID_SUPPORTED\n"));
	my_tprintf(_T("[4] LCID_ALTERNATE_SORTS\n"));
	my_tprintf(_T("Select: "));
	int key = my_getch_noblock('1');
	int num = key - '0';
	if(num>=0 && num<=4)
		; // valid input
	else
		num = 1;

	my_tprintf(_T("%d\n"), num);
	return num;
}


int _tmain(int argc, TCHAR *argv[])
{
	// Param1: dwFlags passed to EnumSystemLocales().
	// If omit, select interactively.

	_tsetlocale(LC_CTYPE, _T(""));

	app_print_version(argv[0], g_szversion);

	const TCHAR *pfn = app_GetFilenamePart(argv[0]);

	if(argc==1)
	{
		my_tprintf(_T("Hint: You can pass one param for EnumSystemLocales() flags,\n"));
		my_tprintf(_T("    so that this program will not ask you interactively.\n"));
		my_tprintf(_T("For example, to list LCID_INSTALLED:\n"));
		my_tprintf(_T("    %s 1\n"), pfn);
	}

	EnumInfo_t &exi = g_exi;

	int flags = 0;
	if(argc<=1 || (flags=_ttoi(argv[1]))<0)
	{
		flags = AskUserForFlags();
	}

	BOOL succ = EnumSystemLocales(EnumLocalesProc, flags);
	if(succ)
	{
		if(exi.count==0)
			my_tprintf(_T("None.\n"));
		
		if(exi.empty>0)
			my_tprintf(_T("Callbacks:%d , Shown:%d, and %d empty-string given by EnumSystemLocalesEx().\n"), 
				exi.callbacks, exi.count, exi.empty);
	}
	else
	{
		my_tprintf(_T("EnumSystemLocales() fail. WinErr=%d\n"), GetLastError());
	}

	return 0;
}
