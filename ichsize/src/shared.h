#ifndef _shared_h_
#define _shared_h_
//
#ifdef __cplusplus
extern"C" {
#endif

#include <ps_headers.h>

#ifdef __cplusplus
#ifndef ARRAYSIZE

// define ARRAYSIZE macro, borrowed from Visual C++ winnt.h, will used by Linux

extern"C++"
template <typename T, size_t N>
char (*
	RtlpNumberOf( T (& func_param)[N] )
)[N];

#define ARRAYSIZE(A)    ( sizeof(*RtlpNumberOf(A)) )

#endif // not defined ARRAYSIZE
#endif // has __cplusplus

typedef int RE_CODE;
#define NOERROR_0 0

// Exit value defines:
#define EXIT_SUCCESS 0
#define EXIT_INVALID_OPTION 1
#define EXIT_FILE_OPEN_ERROR 2
#define EXIT_GET_SIZE_ERROR 3
#define EXIT_SET_SIZE_ERROR 4



// Platform-specific functions

const TCHAR *OsErrStr();

const TCHAR *ps_get_os_errstring(int oserr, TCHAR *buf, int bufchars);
// Windows: oserr is from GetLastError()
// Linux: oserr is errno 

FHANDLE ps_openfile(const TCHAR *szfilename);
// -- return 0 on error.

void ps_closefile(FHANDLE fh);

int64 ps_get_filesize(FHANDLE fh);

RE_CODE ps_set_filesize(HANDLE fh, int64 NewSize);

int64 ps_str2i64(const TCHAR *s);

TCHAR *ps_fgets_stdin(TCHAR *buf, int bufchars);


#ifdef __cplusplus
} // extern"C"
#endif
//
#endif
