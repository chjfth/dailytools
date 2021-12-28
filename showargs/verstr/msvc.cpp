#include <stdio.h>

#define NUMCONST_TO_STR__(n) #n
#define NUMCONST_TO_STR(n) NUMCONST_TO_STR__(n)


#define VER_STR "_MSC_VER=" NUMCONST_TO_STR(_MSC_VER)

const char *get_ver_string()
{
	return VER_STR;
}
