#include "utils.h"

const TCHAR *g_szversion = _T("1.1.0");

struct EnumInfo_t
{
	int callbacks;
	int count;
	int empty;

	DWORD calling_dwFlag;
	DepictLang_et uselang;
};

BOOL CALLBACK EnumLocalesProcEx(LPWSTR lpLocaleString, DWORD dwFlags, LPARAM lParam)
{
	int &callbacks = ((EnumInfo_t*)lParam)->callbacks;
	callbacks++;

	int &count = ((EnumInfo_t*)lParam)->count;
	int calling_dwFlag = ((EnumInfo_t*)lParam)->calling_dwFlag;
	DepictLang_et uselang = ((EnumInfo_t*)lParam)->uselang;

	if(!lpLocaleString || !lpLocaleString[0])
	{
		my_tprintf(_T("[[callback #%d]] Empty!!!\n"), callbacks);
		((EnumInfo_t*)lParam)->empty ++ ;
		return TRUE;
	}

	// Some API behavior verification >>>

	TCHAR self[LOCALE_NAME_MAX_LENGTH+1] = {};
	GetLocaleInfoEx(lpLocaleString, LOCALE_SNAME, self, ARRAYSIZE(self));
	if(_tcscmp(lpLocaleString, self)!=0)
	{
		my_tprintf(_T("[PANIC] Locale-name round-trip query not match! \"%s\" -> \"%s\"\n"),
			lpLocaleString, self);
	}

	if (calling_dwFlag == LOCALE_WINDOWS)
		assert( dwFlags & (LOCALE_NEUTRALDATA|LOCALE_SPECIFICDATA) );
	else if (calling_dwFlag == LOCALE_NEUTRALDATA)
		assert(dwFlags & LOCALE_NEUTRALDATA);
	else if (calling_dwFlag == LOCALE_SPECIFICDATA)
		assert(dwFlags & LOCALE_SPECIFICDATA);

	// Some API behavior verification <<<

	LCTYPE lcLang = LOCALE_SENGLISHLANGUAGENAME;
	LCTYPE lcCountry = LOCALE_SENGLISHCOUNTRYNAME;
	if (uselang == DepictLang_localized)
		lcLang = LOCALE_SLOCALIZEDLANGUAGENAME, lcCountry = LOCALE_SLOCALIZEDCOUNTRYNAME;
	else if (uselang == DepictLang_native)
		lcLang = LOCALE_SNATIVELANGNAME, lcCountry = LOCALE_SNATIVECOUNTRYNAME;

	TCHAR szLang[40] = {}, szCountry[40] = {};
	GetLocaleInfoEx(lpLocaleString, lcLang, szLang, ARRAYSIZE(szLang));
	GetLocaleInfoEx(lpLocaleString, lcCountry, szCountry, ARRAYSIZE(szCountry));

	TCHAR szACP[10] = {}, szOCP[10] = {};
	GetLocaleInfoEx(lpLocaleString, LOCALE_IDEFAULTANSICODEPAGE, szACP, ARRAYSIZE(szACP));
	GetLocaleInfoEx(lpLocaleString, LOCALE_IDEFAULTCODEPAGE, szOCP, ARRAYSIZE(szOCP));

	TCHAR exflags[80] = {};
	if(dwFlags&LOCALE_REPLACEMENT)
		_tcscat_s(exflags, _T("LOCALE_REPLACEMENT |"));

	int slen = (int)_tcslen(exflags);
	if (slen >= 2)
	{
		if (exflags[slen - 1] == _T('|'))
			exflags[slen - 2] = _T('\0');
	}

	TCHAR szLCID[20] = {};
	LCID lcid = LocaleNameToLCID(lpLocaleString, LOCALE_ALLOW_NEUTRAL_NAMES); 
	// -- LOCALE_ALLOW_NEUTRAL_NAMES effective since Win7
	
	_sntprintf_s(szLCID, ARRAYSIZE(szLCID), _T("0x%04X.%04X"), lcid>>16, lcid&0xFFFF);

	count++;

	my_tprintf(_T("[%d] %s ; %s @ %s ; LCID=%s"), count, lpLocaleString, szLang, szCountry, szLCID);
	

	my_tprintf(_T(" ; ANSI/OEM[%s/%s]"), szACP, szOCP);

	if(exflags[0])
	{
		my_tprintf(_T(" (%s)"), exflags);
	}
	my_tprintf(_T("\n"));

	return TRUE;
}

static DWORD kbkey_to_dwFlag(int key, const TCHAR **ppSzFlag)
{
	const TCHAR* pszFlag = NULL;
	
	int dwFlag = -1; // -1 : invalid selection
	if (key >= '0' && key <= '7')
	{
		dwFlag = key - '0';
		if (dwFlag == 0)
			pszFlag = _T("LOCAL_ALL");
		else if (dwFlag == 1)
			pszFlag = _T("LOCALE_WINDOWS");
		else if (dwFlag == 2)
			pszFlag = _T("LOCALE_SUPPLIMENTAL");
		else if (dwFlag == 4)
			pszFlag = _T("LOCAL_ALTERNATE_SORTS");
	}
	else if (key == 'a' || key == 'A')
	{
		dwFlag = LOCALE_NEUTRALDATA;
		pszFlag = _T("LOCALE_NEUTRALDATA");
	}
	else if (key == 'b' || key == 'B')
	{
		dwFlag = LOCALE_SPECIFICDATA;
		pszFlag = _T("LOCALE_SPECIFICDATA");
	}
	else
	{
		return -1;
	}

	*ppSzFlag = pszFlag;
	return dwFlag;
}

DWORD AskUserForFlags()
{
	
	my_tprintf(_T("Select what to enumerate:\n"));
	my_tprintf(_T("[0] LOCALE_ALL (all of 4,A,B)\n"));
	my_tprintf(_T("[1] LOCALE_WINDOWS (all of A,B)\n"));
	my_tprintf(_T("[2] LOCALE_SUPPLEMENTAL\n"));
	my_tprintf(_T("[4] LOCALE_ALTERNATE_SORTS\n"));
	my_tprintf(_T("[A] LOCALE_NEUTRALDATA\n"));
	my_tprintf(_T("[B] LOCALE_SPECIFICDATA\n"));
	my_tprintf(_T("Select: "));
	int key = my_getch_noblock();

	const TCHAR* pszFlag = NULL;
	DWORD dwFlag = kbkey_to_dwFlag(key, &pszFlag);

	if(dwFlag==-1)
	{
		my_tprintf(_T("Invalid selection!\n"));
		return -1;
	}
	
	my_tprintf(_T("[%c] %s\n"), key, pszFlag);
	return dwFlag;
}

DepictLang_et AskForDepictLang()
{
	my_tprintf(_T("Select DepictLang to show the locale-names:\n"));
	my_tprintf(_T("[0] Use English.\n"));
	my_tprintf(_T("[1] Use your Windows UI language.\n"));
	my_tprintf(_T("[2] Use context, the language that current locale entry is referring to.\n"));
	my_tprintf(_T("Select: "));
	int key = my_getch_noblock();
	int num = key - '0';
	if(num>=0 && num<=2)
		; // valid input
	else
		num = 0;

	my_tprintf(_T("%d\n"), num);
	return (DepictLang_et)num;
}

int _tmain(int argc, TCHAR *argv[])
{
	_tsetlocale(LC_CTYPE, _T(""));
	_setmode(_fileno(stdout), _O_U8TEXT); // ensure WCHARs are passed to Console

	app_print_version(argv[0], g_szversion);

	const TCHAR *pfn = app_GetFilenamePart(argv[0]);

	if(argc==1)
	{
		my_tprintf(_T("Hint: You can pass two params for EnumSystemLocalesEx() flags, and filters,\n"));
		my_tprintf(_T("    so that this program will not ask you interactively.\n"));
		my_tprintf(_T("For example, to list LOCALE_WINDOWS with only <lang>-<Country> entries:\n"));
		my_tprintf(_T("    %s 1 1\n"), pfn);
	}

	EnumInfo_t exi = {};

	DWORD dwFlag = 0; // Only ONE-bit of flag is meaningful for each call of EnumSystemLocalesEx().
	if(argc<=1)
	{
		dwFlag = AskUserForFlags();
		if(dwFlag==-1)
			exit(1);
	}
	else
	{
		const TCHAR* pszFlag = NULL;
		dwFlag = kbkey_to_dwFlag(argv[1][0], &pszFlag);
		if (dwFlag == -1)
		{
			my_tprintf(_T("[ERROR] Invalid first parameter for dwFlag.\n"));
			exit(1);
		}

		my_tprintf(_T("Calling EnumSystemLocalesEx() with dwFlags=%s\n"), pszFlag);
	}

	exi.calling_dwFlag = dwFlag;
	
	if (argc <= 2)
		exi.uselang = AskForDepictLang();
	else
		exi.uselang = (DepictLang_et)_ttoi(argv[2]);

	BOOL succ = EnumSystemLocalesEx(EnumLocalesProcEx, dwFlag, (LPARAM)&exi, 0);
	if(succ)
	{
		if(exi.count==0)
			my_tprintf(_T("None.\n"));
		
		if (exi.empty == 0)
			my_tprintf(_T("Callbacks:%d\n"), exi.callbacks);
		else
			my_tprintf(_T("Callbacks:%d , Shown:%d, and %d empty-string given by EnumSystemLocalesEx().\n"), 
				exi.callbacks, exi.count, exi.empty);
	}
	else
	{
		my_tprintf(_T("EnumSystemLocalesEx() fail. WinErr=%d\n"), GetLastError());
	}

	return 0;
}
