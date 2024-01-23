// [2005-02-10] Both for Windows and Unix
// [2024-01-09] On Linux, there is stock printenv command available.

/* 
	This program shows the environment variable(env-var) values for itself.
 Your program parameters tells what env-vars to show.
 The values are shown in two formats:
	(1) in literal if argv[0] doesn't contain "hex" as a substring.
	(2) in hex if argv[0] does contain "hex" as a substring.

---Examples Output (1)---

$ ./show-envvar OS HOME TEMP
OS=
HOME=/home/chj
TEMP=

---Examples Output (2)---

$ ./show-envvar-hex OS HOME TMP
OS=
HOME=2f 68 6f 6d 65 2f 63 68 6a
TEMP=

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	int i, j;
	int isPrnHex = strstr(argv[0], "hex") ? 1 : 0;

	for(i=1; argv[i]; i++)
	{
		const char * val = getenv(argv[i]);
		printf("%s=", argv[i]);
		if(val)
		{
			for(j=0; val[j]; j++)
			{
				unsigned char uc = (unsigned char)val[j];
				if(isPrnHex)
					printf("%02x ", uc);
				else
					printf("%c", uc);
			}
		}
		printf("\n");
	}

	return 0;
}
