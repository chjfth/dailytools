#include "utils.h"
#include <time.h>

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

void see_strftime()
{
	// This output help us determine which locale space(s) determine
	// strftime()'s date-time format.
	
	my_tprintf(_T("Calling wcsftime() for 2022-09-17 09:17:03 (AM) ...\n"));

	struct tm tm1 = {};
	tm1.tm_year = 2022 - 1900, tm1.tm_mon = 9 - 1, tm1.tm_mday = 17;
	tm1.tm_hour = 9, tm1.tm_min = 17, tm1.tm_sec = 3;
	tm1.tm_wday = 6;

	TCHAR buf1[200] = {};
	_tcsftime(buf1, sizeof(buf1), _T("%x (%A); %X ; %z(%Z)"), &tm1);

	TCHAR buf2[200] = {};
	_tcsftime(buf2, sizeof(buf2), _T("%#c"), &tm1);

	my_tprintf(_T("    %s\n"), buf1);
	my_tprintf(_T("    %s\n"), buf2);
}

void see_LocaleDateFormat()
{
	struct ValNStr_st
	{
		DWORD val;
		const TCHAR* str;
	} ar_uselc[] = {
		{ LOCALE_INVARIANT , _T("LOCALE_INVARIANT")},
		{ LOCALE_SYSTEM_DEFAULT , _T("LOCALE_SYSTEM_DEFAULT")},
		{ LOCALE_USER_DEFAULT , _T("LOCALE_USER_DEFAULT")},
	};
	
	BOOL succ = 0;
	SYSTEMTIME st = {};
	GetLocalTime(&st);
	st.wYear = 2022, st.wMonth = 9, st.wDay = 15;

	for(int override=0; override<2; override++)
	{
		_tprintf(_T("Show date format for %04d-%02d-%02d: %s\n"),
			st.wYear, st.wMonth, st.wDay,
			override ? _T("(with user override)") : _T("(LOCALE_NOUSEROVERRIDE)")
			);

		int i;
		for(i=0; i<ARRAYSIZE(ar_uselc); i++)
		{
			TCHAR retbuf[100] = {};
			const ValNStr_st& uselc= ar_uselc[i];
			int retchars = GetDateFormat(uselc.val,
                DATE_SHORTDATE | (override ? 0 : LOCALE_NOUSEROVERRIDE),
                &st,
                NULL, // lpFormat, NULL means according to thread-locale 
                retbuf, ARRAYSIZE(retbuf));
			_tprintf(_T("  %30s : %s\n"), uselc.str, retbuf);
		}
	}	
}

void custom_test()
{
	// Execute extra experiment test here.
	
#ifdef CUSTOM_TEST_ON

	// Define env-var `PERSONAL_CL_DEFINES=CUSTOM_TEST_ON` to enable this block.

	verify_sample_codepages();

	see_strftime();

	see_LocaleDateFormat();
	
#endif
}
