#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <locale.h>
#include <windows.h>

int g_ncpsSupported = 0;
int g_ncpsInstalled = 0;
int *g_pncps;

TCHAR *GetFilenamePart(TCHAR *pPath)
{
	TCHAR *p = _tcsrchr(pPath, _T('\\'));
	return p ? p+1 : pPath;
}

BOOL CALLBACK procEnumCodepages(TCHAR *str) //LPTSTR
{
	_tprintf(_T("%d. %s\n"), ++(*g_pncps), str);
	return TRUE;
}

void EnumCodepages()
{
	g_pncps = &g_ncpsSupported;
	_tprintf(_T("EnumSystemCodePages(CP_SUPPORTED):\n"));
	EnumSystemCodePages(procEnumCodepages, CP_SUPPORTED);

	g_pncps = &g_ncpsInstalled;
	_tprintf(_T("EnumSystemCodePages(CP_INSTALLED):\n"));
	EnumSystemCodePages(procEnumCodepages, CP_INSTALLED);

	_tprintf(_T("CP_SUPPORTED: %d, CP_INSTALLED: %d.\n"), 
		g_ncpsSupported, g_ncpsInstalled);
}

void ShowCPInfo(int codepage)
{
/*
typedef struct _cpinfoex {
  UINT    MaxCharSize;
  BYTE    DefaultChar[MAX_DEFAULTCHAR];
  BYTE    LeadByte[MAX_LEADBYTES];
  WCHAR   UnicodeDefaultChar;
  UINT    CodePage;
  TCHAR   CodePageName[MAX_PATH];
} CPINFOEX, *LPCPINFOEX;
Members
*/	
	_tprintf(_T("Hardcoded consts: MAX_DEFAULTCHAR=%d, MAX_LEADBYTES=%d, MAX_PATH=%d.\n"), 
		MAX_DEFAULTCHAR, MAX_LEADBYTES, MAX_PATH);

	CPINFOEX cpix;
	if(!GetCPInfoEx(codepage, 0, &cpix))
	{
		_tprintf(_T("GetCPInfoEx(%d) error: Err=%d."), codepage, GetLastError());
		return;
	}

	_tprintf(_T("CPINFOEX.MaxCharSize=%d.\n"), cpix.MaxCharSize);

	int i;
	_tprintf(_T("CPINFOEX.DefaultChar: "));
	for(i=0; cpix.DefaultChar[i]; i++)
		_tprintf(_T("0x%02X "), cpix.DefaultChar[i]);
	printf("(%s)\n", cpix.DefaultChar); // always MBCS chars in `DefaultChar'

	_tprintf(_T("CPINFOEX.LeadByte: "));
	for(i=0; cpix.LeadByte[i]||cpix.LeadByte[i+1]; i+=2)
		_tprintf(_T("[0x%02X - 0x%02X] "), cpix.LeadByte[i], cpix.LeadByte[i+1]);
	_tprintf(_T("\n"));

	_tprintf(_T("CPINFOEX.UnicodeDefaultChar: "));
	_tprintf(_T("0x%04X "), cpix.UnicodeDefaultChar);
	wprintf(L"(%c)", cpix.UnicodeDefaultChar); // always Unicode char in `UnicodeDefaultChar'
	_puttchar(_T('\n'));

	_tprintf(_T("CPINFOEX.CodePageName: \"%s\"\n"), cpix.CodePageName);
}

int _tmain(int argc, TCHAR *argv[])
{
	setlocale(LC_ALL, "");

	TCHAR *pfn = GetFilenamePart(argv[0]);
		// For MSVC, argv[0] always contains the full pathname.

	if(argc==1)
	{
		EnumCodepages();
		
		_tprintf(_T("\n"));

		UINT acp = GetACP();
		_tprintf(_T("Current system codepage, as returned by GetACP() : %d\n"), acp);
		
		_tprintf(_T("\n"));
		
		_tprintf(_T("Use \"%s [codepage]\" to invoke GetCPInfoEx() for that codepage.\n"), pfn);
		_tprintf(_T("Example:\n"));
		_tprintf(_T("    %s 1252\n"), pfn);
		_tprintf(_T("    %s 936\n"), pfn);
		return 0;
	}

	ShowCPInfo(_ttoi(argv[1]));

	return 0;
}
