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

#include <stdio.h>
#include <string.h>

extern const char *get_ver_string();

int main(int argc, char *argv[])
{
	int i, j;;
	int isPrnHex = strstr(argv[0], "hex") ? 1 : 0;

#ifdef PRINT_VER_STRING
	printf("%s\n", get_ver_string());
#endif
	printf("argc=%d.\n", argc);

	for(i=0; argv[i]; i++)
	{
		printf("(%s%d)argv[%d]: ", isPrnHex?"H":"", (int)strlen(argv[i]), i);
		for(j=0; argv[i][j]; j++)
		{
			unsigned char uc = (unsigned char)argv[i][j];
			if(isPrnHex)
				printf("%02x ", uc);
			else
				printf("%c", uc);
		}
		printf("\n");
	}

	return 0;
}
