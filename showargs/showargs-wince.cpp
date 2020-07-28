// [2005-02-10] Both for Windows and Unix

/* 
	This program shows the content of argv[0], argv[1]... , it's shown in
 two format:
	(1) in literal if argv[0] doesn't contain "hex" as a substring.
	(2) in hex if argv[0] does contain "hex" as a substring.

---Examples Output (1)---

$ ./showargs OneWord "Two Words"
argc=3.
(10)argv[0]: ./showargs
(7)argv[1]: OneWord
(9)argv[2]: Two Words

---Examples Output (2)---

$ ./showargshex OneWord "Two Words"
argc=3.
argc=3.
(H13)argv[0]: 2e 2f 73 68 6f 77 61 72 67 73 68 65 78
(H7)argv[1]: 4f 6e 65 57 6f 72 64
(H9)argv[2]: 54 77 6f 20 57 6f 72 64 73

*/

#include <tchar.h>
#include <stdio.h>
#include <string.h>

int wmain(int argc, WCHAR *argv[])
{
	int i, j;;
	int isPrnHex = wcsstr(argv[0], L"hex") ? 1 : 0;

	wprintf(L"argc=%d.\n", argc);

	for(i=0; argv[i]; i++)
	{
		wprintf(L"(%s%d)argv[%d]: ", isPrnHex?L"H":L"", wcslen(argv[i]), i);
		for(j=0; argv[i][j]; j++)
		{
			WCHAR uc = (WCHAR)argv[i][j];
			if(isPrnHex)
				wprintf(L"%02x ", uc);
			else
				wprintf(L"%c", uc);
		}
		wprintf(L"\n");
	}

	return 0;
}
