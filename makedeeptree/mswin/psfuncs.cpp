#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <strsafe.h>
//#include <stdexcept>
#include "osheader.h"
#include "..\psfuncs.h"

#if _MSC_VER < 1900
#error "This makedeeptree needs at least VC2015 to compile. Older compiler behaves abnormally on C++ exceptions."
#endif

//#define snprintf StringCchPrintf // VC2015 already provides snprintf.

unsigned int ps_GetMillisec()
{
	return GetTickCount();
}

void ps_create_dir_if_not_exist(const char *dirpath)
{
	const char *thisfunc = "ps_create_dir_if_not_exist";

	char errmsg[4000] = {};
	DWORD winerr = 0;
	DWORD attr = GetFileAttributes(dirpath);
	if(attr!=INVALID_FILE_ATTRIBUTES)
	{
		if(attr & FILE_ATTRIBUTE_DIRECTORY)
		{
			return; // ok, already a folder
		}
		else
		{	// meet something bad, report error
			snprintf(errmsg, ARRAYSIZE(errmsg),
				"Error: I see an existing path on disk that is not a directory:\n"
				"    %s\n"
				, dirpath);
			throw ErrMsg(thisfunc, errmsg);
		}
	}

	BOOL succ = CreateDirectory(dirpath, NULL);
	if(!succ)
	{
		winerr = GetLastError();
		snprintf(errmsg, ARRAYSIZE(errmsg), 
			"CreateDirectory(\"%s\") fail with winerr=%d", dirpath, winerr);
		throw ErrMsg(thisfunc, errmsg);
	}

	attr = GetFileAttributes(dirpath);
	if(attr==INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
	{
		snprintf(errmsg, ARRAYSIZE(errmsg),
			"Panic! CreateDirectory(\"%s\") reports success but the directory is NOT created.",
			dirpath);
		throw ErrMsg(thisfunc, errmsg);
	}
}

void ps_create_file_write_string(const char *filepath, const char *text)
{
	const char *thisfunc = "ps_create_file_write_string";

	DWORD winerr = 0;
	const int bufsize = 4000;
	char szWinErr[bufsize] = {};
	char errmsg[bufsize] = {};

	HANDLE hfile = CreateFile(filepath,
		GENERIC_READ | GENERIC_WRITE,
		0, // shareMode
		NULL, // no security attribute
		CREATE_ALWAYS, // dwCreationDisposition
		0, // FILE_FLAG_OVERLAPPED,
		NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		winerr = GetLastError();

		DWORD retchars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, winerr,
			0, // LANGID
			szWinErr, bufsize,
			NULL); // A trailing \r\n has been filled.

		snprintf(errmsg, ARRAYSIZE(errmsg),
			"CreateFile() creating file fail with winerr=%u: %s\n"
			"filepath: %s\n"
			,
			winerr, szWinErr, filepath);
		throw ErrMsg(thisfunc, errmsg);
	}

	int slen = (int)strlen(text);
	DWORD nbWritten = 0;
	BOOL succ = WriteFile(hfile, text, slen, &nbWritten, NULL);
	if(!succ)
	{
		CloseHandle(hfile);
		DWORD retchars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, winerr,
			0, // LANGID
			szWinErr, bufsize,
			NULL); // A trailing \r\n has been filled.

		snprintf(errmsg, ARRAYSIZE(errmsg),
			"WriteFile() fail with winerr=%u: %s\n"
			"filepath: %s\n"
			,
			winerr, szWinErr, filepath);
		throw ErrMsg(thisfunc, errmsg);
	}

	CloseHandle(hfile);
}


