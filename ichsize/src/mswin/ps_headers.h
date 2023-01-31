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
#include <windows.h>

#include <tchar.h>
#include <locale.h>

typedef HANDLE FHANDLE; // file-handle type 

typedef __int64 int64;

#define FMT_i64d "%I64d"

#ifdef __cplusplus
} // extern"C"
#endif
//
#endif
