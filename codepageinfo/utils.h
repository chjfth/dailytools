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
#include "..\cinclude\dlptr_winapi.h"


const TCHAR *app_GetFilenamePart(const TCHAR *pPath);

void app_print_version(const TCHAR *argv0, const TCHAR *verstr);

int my_getch_noblock(unsigned char default_key='0');

void my_tprintf(const TCHAR *szfmt, ...);

inline void newline()
{
	my_tprintf(_T("\n"));
}

const TCHAR *StrLCID(LCID lcid);

const TCHAR *app_GetWindowsVersionStr3();

const TCHAR *Desctext_from_LCID(LCID lcid, bool need_native_name=false);
