/* This file has UTF8 BOM, so to contain UTF8 chars in .cpp source code,
and at the same time, the BOM makes MSVC compiler happy. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <locale.h>
#include <windows.h>

TCHAR *GetFilenamePart(TCHAR *pPath)
{
	TCHAR *p = _tcsrchr(pPath, _T('\\'));
	return p ? p+1 : pPath;
}


struct SampleStr_st
{
	const WCHAR *pszw;
	int codepage;
	const char *psza;
};

SampleStr_st ar_samps[] =
{
	{ L"\x7535", 936, "\xB5\xE7" }, // 电 E7 94 B5
	{ L"\x96FB", 936, "\xEB\x8A" }, // 電 E9 9B BB
	{ L"\x96FB", 950, "\xB9\x71" }, // 電 E9 9B BB
	{ L"\xD55C", 949, "\xC7\xD1" }, // 한 ED 95 9C
};

void wprintf_Samples()
{
	int i;
	for(i=0; i<ARRAYSIZE(ar_samps); i++)
	{
		wprintf(L" => %s\n", ar_samps[i].pszw);
	}
}

int _tmain(int argc, TCHAR *argv[])
{
	setlocale(LC_ALL, ".UTF8");

	TCHAR *pfn = GetFilenamePart(argv[0]);
		// For MSVC, argv[0] always contains the full pathname.

	printf("ocp=949\n"); SetConsoleOutputCP(949);

	_tprintf(_T("Initial GetConsoleCP()       = %d\n"), GetConsoleCP());
	_tprintf(_T("Initial GetConsoleOutputCP() = %d\n"), GetConsoleOutputCP());

	wprintf_Samples();

	return 0;
}
