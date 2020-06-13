// Sleep for a number of milliseconds before exit.
// Sleep time can be assigned in exe filename, or in parameter. The latter takes precedence.

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
//
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void sleep_millisec(int ms)
{
	if(ms==0)
		return;
	Sleep(ms);
}
//
#else
//
#include <unistd.h>
void sleep_millisec(int ms)
{
	if(ms==0)
		return;
	usleep(ms*1000);
}
//
#endif


void print_help()
{
	const char *helptext = 
"msleep v1.0\n"
"This program sleeps for a number of milliseconds(ms) before exit.\n"
"\n"		
"If you run it with mere msleep or msleep.exe, this help message is displayed.\n"
"\n"
"To sleep for 2000ms, you can run:\n"
"\n"
"	msleep 2000\n"
"\n"
"or rename the executable to msleep2000, and run:\n"
"\n"
"	msleep2000\n"
"\n"
"Using hex value is OK as well, like:\n"
"\n"
"	msleep 0x7d0\n"
"	msleep0x7d0\n"
"\n"
"If parameter form is given, it takes precedence over the value in exe name.\n"
	;
	fprintf(stderr, "%s", helptext);
}

const char *get_filenam_ptr(const char *filepath)
{
	// https://stackoverflow.com/a/1575314 , split_path_file
	
    const char *slash = filepath, *next;
    while ((next = strpbrk(slash + 1, "\\/")))
    	slash = next;
    
    if (slash != filepath) // no slashes in filepath
    	slash++;
	
	return slash; // actually, the filename following the final slash
}

int get_value_in_filenam(const char *filepath)
{
	const char *pfilenam = get_filenam_ptr(filepath);
	const char *pdigit = pfilenam;
	while( !isdigit(*pdigit) && *pdigit!='\0' )
		pdigit++;
	
	if(*pdigit=='\0')
		return 0;
	
	return strtoul(pdigit, NULL, 0);
}

int main(int argc, char **argv)
{
	int ret = 0;
	int sleep_ms = 0;
	
	if(argc==1)
	{
		sleep_ms = get_value_in_filenam(argv[0]);
		
		if(sleep_ms==0)
		{
			print_help();
			exit(1);
		}
	}

	if(argc==2)
	{
		sleep_ms = strtoul(argv[1], NULL, 0);
	}
	
	printf("Sleep %d milliseconds before exit...\n", sleep_ms);

	sleep_millisec(sleep_ms);

	return 0;
}
