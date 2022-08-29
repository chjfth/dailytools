#include "utils.h"

enum Filter_et
{
	Filter_None = 0,
	Filter_LangRegn = 1,    // need only <lang>-<REGION> locales
	Filter_Neutral = 2, // need only neutral locales
};

struct EnumInfo_t
{
	int callbacks;
	int count;
	int empty;
	Filter_et filter;
};


BOOL CALLBACK EnumLocalesProcEx(LPWSTR lpLocaleString,  DWORD dwFlags, LPARAM lParam)
{
	int &callbacks = ((EnumInfo_t*)lParam)->callbacks;
	callbacks++;

	int &count = ((EnumInfo_t*)lParam)->count;

	if(!lpLocaleString || !lpLocaleString[0])
	{
		my_tprintf(_T("[callback #%d] Empty!!!\n"), callbacks);
		((EnumInfo_t*)lParam)->empty ++ ;
		return TRUE;
	}

	TCHAR self[LOCALE_NAME_MAX_LENGTH+1] = {};
	GetLocaleInfoEx(lpLocaleString, LOCALE_SNAME, self, ARRAYSIZE(self));
	if(_tcscmp(lpLocaleString, self)!=0)
	{
		my_tprintf(_T("[PANIC] Locale-name round-trip query not match! \"%s\" -> \"%s\"\n"),
			lpLocaleString, self);
	}

	TCHAR szLang[40] = {}, szRegn[40] = {};
	GetLocaleInfoEx(lpLocaleString, LOCALE_SENGLISHLANGUAGENAME, szLang, ARRAYSIZE(szLang));
	GetLocaleInfoEx(lpLocaleString, LOCALE_SENGLISHCOUNTRYNAME, szRegn, ARRAYSIZE(szRegn));

	TCHAR exflags[80] = {};
	if(dwFlags&LOCALE_REPLACEMENT)
		_tcscat_s(exflags, _T("LOCALE_REPLACEMENT |"));
	if(dwFlags&LOCALE_NEUTRALDATA)
		_tcscat_s(exflags, _T("LOCALE_NEUTRALDATA |"));
	if(dwFlags&LOCALE_SPECIFICDATA)
		_tcscat_s(exflags, _T("LOCALE_SPECIFICDATA |"));

	TCHAR szLCID[20] = {};
	LCID lcid = LocaleNameToLCID(lpLocaleString, LOCALE_ALLOW_NEUTRAL_NAMES); // LOCALE_ALLOW_NEUTRAL_NAMES effective since Win7
	_sntprintf_s(szLCID, ARRAYSIZE(szLCID), _T("0x%04X.%04X"), lcid>>16, lcid&0xFFFF);

	int slen = (int)_tcslen(exflags);

	if(slen>=2)
	{
		if(exflags[slen-1]==_T('|'))
			exflags[slen-2] = _T('\0');
	}

	const Filter_et &filter = ((EnumInfo_t*)lParam)->filter;

	if(filter==Filter_None 
		|| ((filter==Filter_LangRegn) && (dwFlags&LOCALE_SPECIFICDATA))
		|| ((filter==Filter_Neutral) && (dwFlags&LOCALE_NEUTRALDATA)) 
		)
	{
		count++;

		my_tprintf(_T("[%d] %s ; %s @ %s ; LCID=%s"), count, lpLocaleString, szLang, szRegn, szLCID);
		//
		if(exflags[0])
		{
			my_tprintf(_T(" (%s)"), exflags);
		}
		my_tprintf(_T("\n"));
	}

	// TEST "localized" names. Why still get English text?
	GetLocaleInfoEx(lpLocaleString, LOCALE_SLOCALIZEDLANGUAGENAME, szLang, ARRAYSIZE(szLang));
	GetLocaleInfoEx(lpLocaleString, LOCALE_SLOCALIZEDCOUNTRYNAME, szRegn, ARRAYSIZE(szRegn));
	TCHAR tbuf[100];
	_sntprintf_s(tbuf, ARRAYSIZE(tbuf), _T("Local: %s @ %s\n"), szLang, szRegn);
	OutputDebugString(tbuf);

	return TRUE;
}

int AskUserForFlags()
{
	my_tprintf(_T("Select what to enumerate:\n"));
	my_tprintf(_T("[0] LOCALE_ALL\n"));
	my_tprintf(_T("[1] LOCALE_WINDOWS\n"));
	my_tprintf(_T("[2] LOCALE_SUPPLEMENTAL\n"));
	my_tprintf(_T("[4] LOCALE_ALTERNATE_SORTS\n"));
	my_tprintf(_T("Select: "));
	int key = my_getch_noblock();
	int num = key - '0';
	if(num>=0 && num<=7)
		; // valid input
	else
		num = 0;

	my_tprintf(_T("%d\n"), num);
	return num;
}

Filter_et AskForFilters()
{
	my_tprintf(_T("Select filter for LOCALE_WINDOWS:\n"));
	my_tprintf(_T("[0] Show all\n"));
	my_tprintf(_T("[1] Show only <lang>-<REGION> entries (LOCALE_SPECIFICDATA)\n"));
	my_tprintf(_T("[2] Show only neutral entries (LOCALE_NEUTRALDATA)\n"));
	my_tprintf(_T("Select: "));
	int key = my_getch_noblock();
	int num = key - '0';
	if(num>=0 && num<=2)
		; // valid input
	else
		num = 0;

	my_tprintf(_T("%d\n"), num);
	return (Filter_et)num;
}

int _tmain(int argc, TCHAR *argv[])
{
	// Param1: dwFlags passed to EnumSystemLocalesEx().
	// If omit, select interactively.
	// 
	// Param2: Filter the entries enumerated. User can choose display
	// only <lang>-<REGION> locales, or *neutral* locales.
	// If omit, select interactively.

	_tsetlocale(LC_CTYPE, _T(""));

	const TCHAR *pfn = app_GetFilenamePart(argv[0]);

	if(argc==1)
	{
		my_tprintf(_T("Hint: You can pass two params for EnumSystemLocalesEx() flags, and filters,\n"));
		my_tprintf(_T("    so that this program will not ask you interactively.\n"));
		my_tprintf(_T("For example, to list LOCALE_WINDOWS with only <lang>-<REGION> entries:\n"));
		my_tprintf(_T("    %s 1 1\n"), pfn);
	}

	EnumInfo_t exi = {};

	int flags = 0;
	if(argc<=1 || (flags=_ttoi(argv[1]))<0)
	{
		flags = AskUserForFlags();
	}

	if(flags & LOCALE_WINDOWS)
	{
		// We only apply filter to LOCALE_WINDOWS, bcz, for LOCALE_ALTERNATE_SORTS,
		// neither LOCALE_SPECIFICDATA or LOCALE_NEUTRALDATA is seen from the callback.
		if(argc<=2 || (exi.filter=(Filter_et)_ttoi(argv[2]))<0)
		{
			exi.filter = AskForFilters();
		}
	}

	BOOL succ = EnumSystemLocalesEx(EnumLocalesProcEx, flags, (LPARAM)&exi, 0);
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
		my_tprintf(_T("EnumSystemLocalesEx() fail. WinErr=%d\n"), GetLastError());
	}

	return 0;
}
