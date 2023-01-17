// [2022-07-26] Windows wide-char version of showargs.

/* 
	This program shows the content of argv[0], argv[1]... , it's shown in
 two format:
	(1) in literal if argv[0] doesn't contain "hex" as a substring.
	(2) in hex if argv[0] does contain "hex" as a substring.

---Examples Output (1)---

> .\showargsw A电B "Two Words"
argc=3.
(11)argv[0]: .\showargsw
(3)argv[1]: A电B
(9)argv[2]: Two Words

---Examples Output (2)---

> .\showargswhex A电B "Two Words"
argc=3.
(H14)argv[0]: 002e 005c 0073 0068 006f 0077 0061 0072 0067 0073 0077 0068 0065 0078
(H3)argv[1]: 0041 7535 0042
(H9)argv[2]: 0054 0077 006f 0020 0057 006f 0072 0064 0073

*/

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <tchar.h>

extern const wchar_t *get_ver_string();

int wmain(int argc, wchar_t *argv[])
{
	setlocale(LC_ALL, "");
	
	int i, j;;
	int isPrnHex = wcsstr(argv[0], L"hex") ? 1 : 0;

#ifdef PRINT_VER_STRING
	wprintf(L"%s\n", get_ver_string());
#endif
	wprintf(L"argc=%d.\n", argc);

	for(i=0; argv[i]; i++)
	{
		wprintf(L"(%s%d)argv[%d]: ", isPrnHex?L"H":L"", (int)wcslen(argv[i]), i);
		for(j=0; argv[i][j]; j++)
		{
			wchar_t uc = (wchar_t)argv[i][j];
			if(isPrnHex)
				wprintf(L"%04x ", uc);
			else
				wprintf(L"%c", uc);
		}
		wprintf(L"\n");
	}

	return 0;
}
