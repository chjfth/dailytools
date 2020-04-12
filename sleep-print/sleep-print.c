#include <stdio.h>
#include <stdlib.h>

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
"Print some text to stdout/stderr according to sleep (milliseconds) parameter input.\n"
"\n"		
"You can control whether one print is followed by a line feed character(LF, or say \\n).\n"
" * A number of multiples of 10 means LF is needed.\n"
" * A number not of multiples of 10 means LF is not needed.\n"
"\n"
"Example:\n"
"\n"
"  $ ./sleep-print 101 201 1000 1001 301\n"
"  Line1;Line1-1;Line1-2.\n"
"  Line2;Line2-1;\n"
"\n"
"If a number ends with 4 or 40, an extra piece of text will print to stderr.\n"
"\n"
"Example (stdout and stderr interleaving may vary):\n"
"\n"
"  [chj @chja20 /mnt/d/gitw/dailytools/sleep-print]\n"
"  $ time ./sleep-print  101 204 1040 1001 304 \n"
"  Line1;ErrL1-1(204);Line1-1;Line1-2.         \n"
"  ErrL1-2(1040).                              \n"
"  Line2;ErrL2-1(304);Line2-1;                 \n"
"  real    0m2.657s                            \n"
"  user    0m0.000s                            \n"
"  sys     0m0.000s                            \n"
"\n"
"  D:\\>sleep-print  101 204 1040 1001 304      \n"
"  Line1;Line1-1;ErrL1-1(204);Line1-2.         \n"
"  ErrL1-2(1040).                              \n"
"  Line2;Line2-1;ErrL2-1(304);                 \n"
"\n"
"This program is useful in testing whether a parent-process can grab a child-process stdout/stderr.\n"
	;
	fprintf(stderr, "%s", helptext);
}

void sleep_and_print(int n, char *argv[])
{
	int line=1, step=0;
	int i;
	
	for(i=0; i<n; i++)
	{
		int sleep_ms = atoi(argv[i]);
		if(sleep_ms<0)
			sleep_ms = 0;
		
		sleep_millisec(sleep_ms);
		
		if(sleep_ms%10==0)
		{
			if(step==0)
				printf("Line%d.\n", line);
			else
				printf("Line%d-%d.\n", line, step);
		}
		else
		{
			if(step==0)
				printf("Line%d;", line);
			else
				printf("Line%d-%d;", line, step);
		}

		if(sleep_ms%100 == 40)
		{
			fprintf(stderr, "ErrL%d-%d(%d).\n", line, step, sleep_ms);
		}
		else if(sleep_ms%10 == 4)
		{
			fprintf(stderr, "ErrL%d-%d(%d);", line, step, sleep_ms);
		}

		if(sleep_ms%10==0)
		{
			line++;
			step = 0;
		}
		else
		{
			step++;
		}

		fflush(stdout);
		fflush(stderr);
	}
}

int main(int argc, char **argv)
{
	if(argc==1)
	{
		print_help();
		exit(1);
	}

	sleep_and_print(argc-1, argv+1);

	return 0;
}
