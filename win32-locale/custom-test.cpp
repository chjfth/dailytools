#include "utils.h"

void verify_sample_codepages()
{
	my_tprintf(_T("\n"));
	my_tprintf(_T("Running check_three_codepages():\n"));

	const char pansi[] = "\xC0\xC1\xE8\xE9";
	char ansihex[40] = {};

	_setmode(_fileno(stdout), _O_TEXT);
	printf("Input ANSI bytes: %s\n", HexdumpA(pansi, sizeof(pansi)-1, ansihex, ARRAYSIZE(ansihex)));

	_setmode(_fileno(stdout), _O_U8TEXT);
	
	WCHAR wcbuf[80] = {};
	WCHAR hexdumpbuf[250] = {};
	size_t wcret = 0;

	wcret = MultiByteToWideChar(CP_ACP, 0, pansi, -1, wcbuf, ARRAYSIZE(wcbuf));
	if(wcret>0)
	{
		my_tprintf(_T("MultiByteToWideChar(CP_ACP, ...): %s\n"), wcbuf);
		my_tprintf(_T("    %s\n"), HexdumpW(wcbuf, -1, hexdumpbuf, ARRAYSIZE(hexdumpbuf)));
	}
	
	wcret = MultiByteToWideChar(CP_OEMCP, 0, pansi, -1, wcbuf, ARRAYSIZE(wcbuf));
	if (wcret > 0)
	{
		my_tprintf(_T("MultiByteToWideChar(CP_OEMCP, ...): %s\n"), wcbuf);
		my_tprintf(_T("    %s\n"), HexdumpW(wcbuf, -1, hexdumpbuf, ARRAYSIZE(hexdumpbuf)));
	}

	wcret = MultiByteToWideChar(CP_THREAD_ACP, 0, pansi, -1, wcbuf, ARRAYSIZE(wcbuf));
	if (wcret > 0)
	{
		my_tprintf(_T("MultiByteToWideChar(CP_THREAD_ACP, ...): %s\n"), wcbuf);
		my_tprintf(_T("    %s\n"), HexdumpW(wcbuf, -1, hexdumpbuf, ARRAYSIZE(hexdumpbuf)));
	}

	errno_t err = 0;

	err = mbstowcs_s(&wcret, wcbuf, pansi, sizeof(pansi)-1);
	if(!err)
	{
		my_tprintf(_T("mbstowcs_s(): %s\n"), wcbuf);
		my_tprintf(_T("    %s\n"), HexdumpW(wcbuf, -1, hexdumpbuf, ARRAYSIZE(hexdumpbuf)));
	}
}


void custom_test()
{
	// Execute extra experiment test here.
	
#ifdef CUSTOM_TEST_ON

	// Define env-var `PERSONAL_CL_DEFINES=CUSTOM_TEST_ON` to enable this block.

	verify_sample_codepages();
	
#endif
}
