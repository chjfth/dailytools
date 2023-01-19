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
#include <mbctype.h>  // _setmbcp()
#include <windows.h>
#include <assert.h>
#include "..\cinclude\dlptr_winapi.h"

#ifndef LOCALE_CUSTOM_USER_DEFAULT
#define LOCALE_CUSTOM_USER_DEFAULT 0x0C00
// -- the special LCID that means LOCALE_USER_DEFAULT but current locale-name
//    only has LangTag representation but not LCID-numeric representation.
#endif

bool Is_LCID_unspecified(LCID lcid);

const TCHAR *app_GetFilenamePart(const TCHAR *pPath);

void app_print_version(const TCHAR *argv0, const TCHAR *verstr);

int my_getch_noblock(unsigned char default_key='0');

void my_tprintf(const TCHAR *szfmt, ...);

inline void newline()
{
	my_tprintf(_T("\n"));
}

const TCHAR *HexstrLCID(LCID lcid, bool detect_unspecified=false);

const TCHAR *app_GetWindowsVersionStr3();

enum DepictLang_et
{
	DepictLang_English = 0,
	DepictLang_localized = 1, // current system's UI language
	DepictLang_native = 2, // the language current LCID argument is referring to

	DepictLang_SimuIntlcpl = 3, // produce the same list as intl.cpl user-locales list
};

const TCHAR *Desctext_from_LANGID(LANGID lcid, DepictLang_et dlang=DepictLang_English);

const TCHAR * app_WinErrStr(DWORD winerr=-1);

WCHAR *HexdumpW(const WCHAR *pszw, int count, WCHAR *hexbuf, int bufchars);

char *HexdumpA(const char *pbytes, int count, char *hexbuf, int bufchars);

bool ishexdigit(TCHAR c);

bool ishextoken(const TCHAR* psz);

int qsort_CompareString(void* context, const void* item1, const void* item2);

void vaDbgString(const TCHAR* szfmt, ...);

template<typename TEle>
int collect_hexrpw_from_argv(TCHAR** argv, TEle obuf[], int nebuf)
{
	int i;
	for (i = 0; i < nebuf; i++)
	{
		if (argv[i] == nullptr)
			break;

		if (!ishextoken(argv[i]))
		{
			my_tprintf(_T("[ERROR] The parameter \"%s\" is not a valid hex-token.\n"), argv[i]);
			exit(1);
		}

		obuf[i] = (TEle)_tcstoul(argv[i], nullptr, 16);
	}

	return i;
}

BOOL easySetClipboardText(const TCHAR Text[], int textchars = -1, HWND hwnd = 0);
