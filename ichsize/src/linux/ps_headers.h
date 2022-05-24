#ifndef _ps_headers_h_
#define _ps_headers_h_
//
#ifdef __cplusplus
extern"C" {
#endif

// System headers first.

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

#include <locale.h>

typedef char TCHAR;
#define _T(str) str
#define _tprintf printf
#define _tcscmp strcmp
#define _tcslen strlen
typedef int HANDLE;


typedef int FHANDLE; // file-handle type 

typedef long long int64;


#ifdef __cplusplus
} // extern"C"
#endif
//
#endif
