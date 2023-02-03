#include "utils.h"

const TCHAR *g_szversion = _T("1.3.3");

struct EnumInfo_t
{
	int callbacks;
	int count;
	int empty;

	DWORD calling_dwFlag;
	DepictLang_et uselang;
};

bool is_localstring_empty(LPCWSTR lcstr)
{
	if (!lcstr || !lcstr[0])
		return true;
	else
		return false;
}

BOOL CALLBACK EnumLocalesProcEx(LPWSTR lpLocaleString, DWORD dwFlags, LPARAM lParam)
{
	EnumInfo_t *pei = (EnumInfo_t*)lParam;
	
	int &callbacks = pei->callbacks;
	callbacks++;

	int &count = pei->count;
	int calling_dwFlag = pei->calling_dwFlag;
	DepictLang_et uselang = pei->uselang;

	// Note: calling_dwFlag is the exact value we pass to EnumSystemLocalesEx() as 2nd param.
	// And, Windows does some tweak to calling_dwFlag and relay it to us as dwFlags.
	// For example, if calling_dwFlag==LOCALE_WINDOWS(1), windows will callback our
	// EnumLocalesProcEx() many times, sometimes with dwFlags==0x17, sometimes with dwFlags=0x27.
	
	if(is_localstring_empty(lpLocaleString))
	{
		my_tprintf(_T("[[callback #%d]] Empty!!!\n"), callbacks);
		((EnumInfo_t*)lParam)->empty ++ ;
		return TRUE;
	}

	count++;

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

	LCID lcid = LocaleNameToLCID(lpLocaleString, LOCALE_ALLOW_NEUTRAL_NAMES); 
	// -- LOCALE_ALLOW_NEUTRAL_NAMES effective since Win7
	
	my_tprintf(_T("[%d] %s ; %s @ %s ; LCID=%s"), 
		count, lpLocaleString, 
		szLang, szCountry, 
		HexstrLCID(lcid));

	my_tprintf(_T(" ; ANSI/OEM[%s/%s]"), szACP, szOCP);

	if(calling_dwFlag==0)
	{
		// test winapi ResolveLocaleName()
		//  
		TCHAR szRetLcname[LOCALE_NAME_MAX_LENGTH] = {};
		int retchars = ResolveLocaleName(lpLocaleString, szRetLcname, LOCALE_NAME_MAX_LENGTH);
		if (retchars > 0)
			vaDbgString(_T("[%d] ResolveLocaleName %s => %s\n"), count, lpLocaleString, szRetLcname);
		else
			vaDbgString(_T("[%d] ResolveLocaleName %s => WinErr: %d\n"), count, lpLocaleString, GetLastError());
	}
	if(exflags[0])
	{
		my_tprintf(_T(" (%s)"), exflags);
	}
	my_tprintf(_T("\n"));

	return TRUE;
}

BOOL CALLBACK EnumLocalesProc_Count(LPWSTR lpLocaleString, DWORD dwFlags, LPARAM lParam)
{
	int& Count = *(int*)lParam;

	if (!is_localstring_empty(lpLocaleString))
		Count++;

	return TRUE;
}

struct LocalePlate_st
{
	const TCHAR* lcstr;   // locale-name "en-US" etc
	const TCHAR* dispstr; // LOCALE_SLOCALIZEDDISPLAYNAME string
	DWORD lcid;
};

struct Collect_st
{
	LocalePlate_st* arPlate;
	int Count;
	int idx;
};

BOOL CALLBACK EnumLocalesProc_Collect(LPWSTR lpLocaleString, DWORD dwFlags, LPARAM lParam)
{
	Collect_st& collect = *(Collect_st*)lParam;

	if (is_localstring_empty(lpLocaleString))
		return TRUE;

	if(collect.idx < collect.Count)
	{
		int slen = (int)_tcslen(lpLocaleString);
		TCHAR* lcstr = new TCHAR[slen+1];
		_tcscpy_s(lcstr, slen+1, lpLocaleString);

		TCHAR szLcnameDisplay[80] = {};
		GetLocaleInfoEx(lpLocaleString, LOCALE_SLOCALIZEDDISPLAYNAME, szLcnameDisplay, ARRAYSIZE(szLcnameDisplay));
		slen = (int)_tcslen(szLcnameDisplay);

		TCHAR* dispstr = new TCHAR[slen + 1];
		_tcscpy_s(dispstr, slen + 1, szLcnameDisplay);
		
		collect.arPlate[collect.idx].lcstr = lcstr;
		collect.arPlate[collect.idx].dispstr = dispstr;
		collect.arPlate[collect.idx].lcid = LocaleNameToLCID(lpLocaleString, 0);

		collect.idx++;
	}
	return TRUE;
}

int LocalePlate_Compare(void* context, const void* item1, const void* item2)
{
	LocalePlate_st* p1 = (LocalePlate_st*)item1;
	LocalePlate_st* p2 = (LocalePlate_st*)item2;

	int cmpret = CompareString(LOCALE_USER_DEFAULT, 0, 
		p1->dispstr, (int)_tcslen(p1->dispstr), 
		p2->dispstr, (int)_tcslen(p2->dispstr));
	return cmpret - 2;
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

DepictLang_et AskForDepictLang(bool is_simu_intlcpl)
{
	int max_num = 2;
	
	my_tprintf(_T("Select in which language to show the locale-names:\n"));
	my_tprintf(_T("[0] Use English.\n"));
	my_tprintf(_T("[1] Use your Windows UI language.\n"));
	my_tprintf(_T("[2] Use context, the language that current locale entry is referring to.\n"));

	if(is_simu_intlcpl)
	{
		max_num = 4;
		my_tprintf(_T("[3] System-locales from Control Panel. (LOCALE_SLOCALIZEDDISPLAYNAME) \n"));
		my_tprintf(_T("[4]   User-locales from Control Panel. (LOCALE_SLOCALIZEDDISPLAYNAME) \n"));
	}	

	my_tprintf(_T("Select: "));
	int key = my_getch_noblock();

	int num = key - '0';

	if(num>=0 && num<=max_num)
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
		my_tprintf(_T("so that this program will not ask you interactively.\n"));
		my_tprintf(_T("\n"));
		my_tprintf(_T("For example, to get the same user-locales list as in intl.cpl, \n")
			       _T("which contains only <lang>-<Country> entries, use:\n"));
		my_tprintf(_T("\n"));
		my_tprintf(_T("    %s B 3\n"), pfn);
		my_tprintf(_T("\n"));
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
		exi.uselang = AskForDepictLang(dwFlag&LOCALE_SPECIFICDATA ? true : false);
	else
		exi.uselang = (DepictLang_et)_ttoi(argv[2]);

	if (exi.uselang!=DepictLang_SimuUsrlocs && exi.uselang!= DepictLang_SimuSyslocs)
	{
		BOOL succ = EnumSystemLocalesEx(EnumLocalesProcEx, dwFlag, (LPARAM)&exi, 0);

		if (succ)
		{
			if (exi.count == 0)
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
	}
	else
	{
		DWORD winerr = 0;
		Collect_st collect = {};
		BOOL succ = EnumSystemLocalesEx(EnumLocalesProc_Count, dwFlag, (LPARAM)&collect.Count, 0);

		collect.arPlate = new LocalePlate_st[collect.Count];
		memset(collect.arPlate, 0, sizeof(collect.arPlate[0]) * collect.Count);

		succ = EnumSystemLocalesEx(EnumLocalesProc_Collect, dwFlag, (LPARAM)&collect, 0);
		if (!succ)
			winerr = GetLastError();

		collect.Count = collect.idx; // .idx may be less than .Count

		// Sort the locales according to LOCALE_SLOCALIZEDDISPLAYNAME
		if(succ)
		{
			qsort_s(collect.arPlate, collect.Count, sizeof(LocalePlate_st), LocalePlate_Compare, nullptr);
		}

		int i, ncustom = 0, prnseq = 0;
		for(i=0; i<collect.Count; i++)
		{
			LocalePlate_st lcp = collect.arPlate[i];
			bool is_print = true;
			
			if(Is_LCID_customized(lcp.lcid))
			{
				ncustom++;

				if(exi.uselang!=DepictLang_SimuUsrlocs)
					is_print = false;
			}

			if(is_print)
			{
				prnseq++;
				
				my_tprintf(_T("[%d] %-12s ; %s ; %s\n"), prnseq,
					lcp.lcstr,
					HexstrLCID(lcp.lcid),
					lcp.dispstr);
			}
		}
		
		// Release memory by `collect`.
		//
		for (i = 0; i < collect.Count; i++)
		{
			delete collect.arPlate[i].lcstr;
			delete collect.arPlate[i].dispstr;
		}
		delete collect.arPlate;

		if(succ)
		{
			if(exi.uselang==DepictLang_SimuUsrlocs)
			{
				my_tprintf(
					_T("The above %d entries should match the user-locale list from intl.cpl, in the same order.\n")
					_T("(%d system-locales and %d custom-locales)\n")
					,
					prnseq,
					prnseq-ncustom, ncustom);

				assert(prnseq==collect.Count);
			}
			else
			{
				my_tprintf(
					_T("The above %d entries should match the system-locale list from intl.cpl, in the same order.\n")
					,
					prnseq);

				assert(prnseq+ncustom==collect.Count);
			}
		}
		else
		{
			my_tprintf(_T("EnumSystemLocalesEx() fail. WinErr=%d\n"), winerr);
		}
	}

	return 0;
}
