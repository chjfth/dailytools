#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>

void 
my_tprintf(const TCHAR *szfmt, ...)
{
	DWORD nWr = 0;
	TCHAR tbuf[2000];
	va_list args;
	va_start(args, szfmt);

	_vsntprintf_s(tbuf, ARRAYSIZE(tbuf), _TRUNCATE, szfmt, args);

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), 
		tbuf, (DWORD)_tcslen(tbuf), &nWr, NULL);

	va_end(args);
}

int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 2)
	{
		my_tprintf(_T("usage: enum_indir <directory_name>\n"));
		return 1;
	}

	WIN32_FIND_DATA finddata = {0};
	TCHAR szFindPattern[MAX_PATH+2] = {0};

	_tcsncpy_s(szFindPattern, argv[1], ARRAYSIZE(szFindPattern)-3);

	int tlen = (int)_tcslen(szFindPattern);

	if(szFindPattern[tlen-1]!='\\')
		szFindPattern[tlen++] = '\\';

	szFindPattern[tlen++] = L'*';
	szFindPattern[tlen] = '\0';

	HANDLE hFind = FindFirstFile(szFindPattern, &finddata);
	if(hFind==INVALID_HANDLE_VALUE)
	{
		my_tprintf(_T("FindFirstFile(\"%s\") failed, WinErr=%d.\n"),
			szFindPattern, GetLastError());
		return 1;
	}

	int i, j;
	for(i=0; ; i++)
	{
		// Show files/dirs found.

		my_tprintf(_T("[%d]: %d/%d chars in filename/shortname.\n"), i+1, 
			(int)_tcslen(finddata.cFileName), 
			(int)_tcslen(finddata.cAlternateFileName));
	
		my_tprintf(_T("Normal name: %s\n"), finddata.cFileName);
		
		my_tprintf(_T("        Hex: "));
		for(j=0; finddata.cFileName[j]; j++)
			my_tprintf(_T("%02X "), (unsigned char)finddata.cFileName[j]);
		my_tprintf(_T("\n"));

		if(finddata.cAlternateFileName[0])
		{
			my_tprintf(_T(" Short name: %s\n"), finddata.cAlternateFileName);

			my_tprintf(_T("        Hex: "));
			for(j=0; finddata.cAlternateFileName[j]; j++)
				my_tprintf(_T("%02X "), (unsigned char)finddata.cAlternateFileName[j]);
			my_tprintf(_T("\n"));
		}
		
		my_tprintf(_T("\n"));
		
		if(!FindNextFile(hFind, &finddata))
			break;

	}

	FindClose(hFind);

	return 0;
}
