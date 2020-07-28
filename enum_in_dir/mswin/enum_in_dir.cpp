#define UNICODE
#define _UNICODE
	// We are to use Unicode variant of FindFirstFile, FindNextFile etc.
	/* And note: Since we use WriteConsoleW to character output, the 
	 output cannot be redirected to a file -- that's the decision of
	 Microsoft. */

#include <stdio.h>
#include <string.h>
#include <windows.h>
//#include <wchar.h>

#define GetEleQuan_i(array) ((int)(sizeof(array)/sizeof(array[0])))

void 
Wprintf(const WCHAR *szfmt, ...)
{
	DWORD nWr;
	WCHAR wbuf[2000];
	va_list args;
	va_start(args, szfmt);
	_vsnwprintf(wbuf, GetEleQuan_i(wbuf)-1, szfmt, args);
	WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), 
		wbuf, wcslen(wbuf), &nWr, NULL);
	va_end(args);
}

int wmain(int argc, WCHAR *argv[])
{
	if (argc != 2)
	{
		printf("usage: enum_indir <directory_name>\n");
		return 1;
	}

	WIN32_FIND_DATA finddata = {0};
	WCHAR szFindPattern[MAX_PATH+2] = {0};
	wcsncpy(szFindPattern, argv[1], GetEleQuan_i(szFindPattern)-3);
	int tlen = wcslen(szFindPattern);
	if(szFindPattern[tlen-1]!=L'\\')
		szFindPattern[tlen++] = L'\\';
	szFindPattern[tlen++] = L'*';

	HANDLE hFind = FindFirstFileW(szFindPattern, &finddata);
	if(hFind==INVALID_HANDLE_VALUE)
	{
		Wprintf(L"FindFirstFile(\"%s\") failed, WinErr=%d.\n",
			szFindPattern, GetLastError());
		return 1;
	}

	int i, j;
	for(i=0; ; i++)
	{
		// Show files/dirs found.

		Wprintf(L"[%d]: %d/%d chars in filename/shortname.\n", i+1, 
			wcslen(finddata.cFileName), wcslen(finddata.cAlternateFileName));
	
		Wprintf(L"Normal name: %s\n", finddata.cFileName);
		Wprintf(L"        Hex: ");
		for(j=0; finddata.cFileName[j]; j++)
			Wprintf(L"%02X ", finddata.cFileName[j]);

		Wprintf(L"\n");
		Wprintf(L" Short name: %s\n", finddata.cAlternateFileName);
		Wprintf(L"        Hex: ");
		for(j=0; finddata.cAlternateFileName[j]; j++)
			Wprintf(L"%02X ", finddata.cAlternateFileName[j]);

		Wprintf(L"\n\n");
		
		if(!FindNextFileW(hFind, &finddata))
			break;

	}

	FindClose(hFind);

	return 0;
}
