#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <locale.h>

int _tmain(int argc, TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	
	_tprintf(_T("Hello, ichsize!\n"));
	_tprintf(_T("sizeof(TCHAR)=%d\n"), sizeof(TCHAR));
	return 0;
}

