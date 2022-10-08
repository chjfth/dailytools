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

const TCHAR *app_GetFilenamePart(const TCHAR *pPath);

void app_print_version(const TCHAR *argv0, const TCHAR *verstr);

int my_getch_noblock(unsigned char default_key='0');

void my_tprintf(const TCHAR *szfmt, ...);

inline void newline()
{
	my_tprintf(_T("\n"));
}

const TCHAR *HexstrLCID(LCID lcid);

const TCHAR *app_GetWindowsVersionStr3();

enum DepictLang_et
{
	DepictLang_English = 0,
	DepictLang_localized = 1, // current system's UI language
	DepictLang_native = 2, // the language current LCID argument is referring to
};

const TCHAR *Desctext_from_LANGID(LANGID lcid, DepictLang_et dlang=DepictLang_English);

const TCHAR * app_WinErrStr(DWORD winerr=-1);

WCHAR *HexdumpW(const WCHAR *pszw, int count, WCHAR *hexbuf, int bufchars);

char *HexdumpA(const char *pbytes, int count, char *hexbuf, int bufchars);

bool ishexdigit(TCHAR c);