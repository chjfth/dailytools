// This is not a precise *escaping" program for passing argv[1], argv[2] ...
// to a .bat program. It is just enough for VSPG's requirement.
//
// Note: We should compile this cpp using VC2010+, bcz VC6's argv[] extracting
// behavior is NOT correct -- when there are nested double-quote chars on 
// CMD command line.

#include <stdio.h>

int wmain(int argc, wchar_t *argv[])
{
	int i=1;
	for(;;)
	{
		wprintf(L"%s", argv[i]);

		if(argv[++i]==NULL)
			break;

		wprintf(L" ");
	}

	return 0;
}

