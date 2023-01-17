/*
	This program demonstrates, what would be different before/after we call
	MSVCRT setloacale(LC_ALL, ""); .
*/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <locale.h>

WCHAR *HexdumpW(const WCHAR *pszw, int count, WCHAR *hexbuf, int bufchars)
{
	if (count < 0)
		count = (int)wcslen(pszw);

	for (int i = 0; i < count; i++)
	{
		_snwprintf_s(hexbuf + i * 5, bufchars - i * 5, _TRUNCATE,
			L"%04X ", (unsigned short)pszw[i]);
	}

	int wlen = (int)wcslen(hexbuf);
	if (wlen > 0 && hexbuf[wlen - 1] == L' ')
		hexbuf[wlen - 1] = L'\0';

	return hexbuf;
}

char *HexdumpA(const char *pbytes, int count, char *hexbuf, int bufchars)
{
	if (count < 0)
		count = (int)strlen(pbytes);

	for (int i = 0; i < count; i++)
	{
		_snprintf_s(hexbuf + i * 3, bufchars - i * 3, _TRUNCATE,
			"%02X ", (unsigned char)pbytes[i]);
	}

	int alen = (int)strlen(hexbuf);
	if (alen > 0 && hexbuf[alen - 1] == ' ')
		hexbuf[alen - 1] = L'\0';

	return hexbuf;
}


void print_sample()
{
	errno_t err = 0;
	char szerr[80];
	size_t retlen = 0; // will be TCHARs in output buffer
	char hexbufa[60];
	WCHAR hexbufw[60];

	const char pansi[] = "\xB5\xE7"; // '电' in GBK, '萇' in Big5

	printf("Input bytes: %s\n", HexdumpA(pansi, -1, hexbufa, ARRAYSIZE(hexbufa)));
	
	printf("printf: %s\n", pansi);

	WCHAR wcbuf[20] = {};
	err = mbstowcs_s(&retlen, wcbuf, pansi, sizeof(pansi));
	if(!err)
	{
		wprintf(L"  mbstowcs_s() : %s\n", HexdumpW(wcbuf, -1, hexbufw, ARRAYSIZE(hexbufw)));
		wprintf(L"  wprintf()    : %s\n", wcbuf);
	}
	else
	{
		strerror_s(szerr, err);
		printf("  mbstowcs_s() error! errno=%d, %s\n", err, szerr);
	}

	printf("\n");

	const WCHAR ustr[] = L"\x99AC"; // '馬'

	wprintf(L"Input WCHARs: %s\n", HexdumpW(ustr, -1, hexbufw, ARRAYSIZE(hexbufw)));
	
	wprintf(L"wprintf: %s\n", ustr);

	char ansibuf[20] = {};
	err = wcstombs_s(&retlen, ansibuf, ustr, ARRAYSIZE(ustr));
	if(!err)
	{
		printf("  wcstombs_s() : %s\n", HexdumpA(ansibuf, -1, hexbufa, ARRAYSIZE(hexbufa)));
		printf("  printf()     : %s\n", ansibuf);
	}
	else
	{
		strerror_s(szerr, err);
		printf("  wcstombs_s() error! (%d)%s\n", err, szerr);
		// -- probably EILSEQ(42)
	}

	printf("\n");
}

int main(int argc, char* argv[])
{
	printf("setlocale01 version 1.0, compiled with _MSC_VER=%d\n", _MSC_VER);

	const char *lcstr = "";
	if(argc>1)
	{
		lcstr = argv[1];
	}
	
	setvbuf(stdout, NULL, _IONBF, 0);

	WCHAR sysloc[20]={}, usrloc[20]={};
	GetSystemDefaultLocaleName(sysloc, 20); // Vista+
	GetUserDefaultLocaleName(usrloc, 20); // Vista+

	wprintf(L"GetSystemDefaultLocaleName() = %s\n", sysloc);
	wprintf(L"GetUserDefaultLocaleName()   = %s\n", usrloc);
	printf("GetConsoleOutputCP() = %d\n", GetConsoleOutputCP());

	printf("\n");

	printf("=============================\n");
	printf("Before setlocale(LC_ALL, \"%s\");\n", lcstr);
	printf("=============================\n");
	print_sample();

	const char *lcret = setlocale(LC_ALL, lcstr);

	printf("=============================\n");
	printf("After setlocale(LC_ALL, \"%s\");\n", lcstr);
	printf("> %s\n", lcret);
	printf("=============================\n");
	print_sample();
	
	return 0;
}
