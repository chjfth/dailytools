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


const TCHAR *app_GetFilenamePart(const TCHAR *pPath);

void app_print_version(const TCHAR *argv0, const TCHAR *verstr);

int my_getch_noblock();

void my_tprintf(const TCHAR *szfmt, ...);

const TCHAR *app_GetWindowsVersionStr3();

