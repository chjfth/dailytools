/* This file has UTF8 BOM, so to contain UTF8 chars in .cpp source code,
and at the same time, the BOM makes MSVC compiler happy. */
#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <tchar.h>
#include <locale.h>
#include <conio.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>

void my_tprintf(const TCHAR *szfmt, ...)
{
	va_list args;
	va_start(args, szfmt);

	TCHAR buf[200] = {};
	_vsntprintf_s(buf, ARRAYSIZE(buf), szfmt, args);

	va_end(args);

	_tprintf(_T("%s"), buf);
	fflush(stdout); // important
}

struct EnumInfo_t
{
	int count;
	int empty;
};

BOOL CALLBACK EnumLocalesProcEx(LPWSTR lpLocaleString,  DWORD dwFlags, LPARAM lParam)
{
	int &count = ((EnumInfo_t*)lParam)->count;
	count++;

	if(!lpLocaleString || !lpLocaleString[0])
	{
		my_tprintf(_T("[%d] Empty!!!\n"), count);
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

	int slen = _tcslen(exflags);

	if(slen>=2)
	{
		if(exflags[slen-1]==_T('|'))
			exflags[slen-2] = _T('\0');
	}

	my_tprintf(_T("[%d] %s ; %s @ %s ; LCID=%s"), count, lpLocaleString, szLang, szRegn, szLCID);
	//
	if(exflags[0])
	{
		my_tprintf(_T(" (%s)"), exflags);
	}
	my_tprintf(_T("\n"));

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
	int key = _getch();
	int num = key - '0';
	if(num>=0 && num<=7)
		;
	else
		num = 0;

	my_tprintf(_T("%d\n"), num);
	return num;
}

int _tmain(int argc, TCHAR *argv[])
{
	_tsetlocale(LC_CTYPE, _T(""));

	int flags = 0;
	if(argc==1 || (flags=_ttoi(argv[1]))<0)
	{
		flags = AskUserForFlags();
	}

	EnumInfo_t exi = {};

	BOOL succ = EnumSystemLocalesEx(EnumLocalesProcEx, flags, (LPARAM)&exi, 0);
	if(succ)
	{
		if(exi.count==0)
			my_tprintf(_T("None.\n"));
		
		if(exi.empty>0)
			my_tprintf(_T("NOTE: There are %d empty locale-name given by EnumSystemLocalesEx().\n"), exi.empty);
	}
	else
	{
		my_tprintf(_T("EnumSystemLocalesEx() fail. WinErr=%d\n"), GetLastError());
	}

	return 0;
}
