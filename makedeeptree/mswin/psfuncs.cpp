#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include "osheader.h"
#include "..\psfuncs.h"

#include "..\..\__WinConst\mswin\WinError.itc.h"
using namespace itc;

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
		snprintf(errmsg, ARRAYSIZE(errmsg), 
			"CreateDirectory(\"%s\") fail with winerr=%s", dirpath, ITCS_WinError);
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

filehandle_t ps_create_new_file(const char *filepath)
{
	const char *thisfunc = "ps_create_new_file";

	char errmsg[4000] = {};

	HANDLE hfile = CreateFile(filepath,
		GENERIC_READ | GENERIC_WRITE,
		0, // shareMode
		NULL, // no security attribute
		CREATE_ALWAYS, // dwCreationDisposition
		0, // FILE_FLAG_OVERLAPPED,
		NULL);

	if (hfile == INVALID_HANDLE_VALUE)
	{
		snprintf(errmsg, ARRAYSIZE(errmsg),
			"CreateFile() creating file fail with winerr=%s"
			"filepath: %s\n"
			,
			ITCS_WinError, filepath);
		throw ErrMsg(thisfunc, errmsg);
	}

	return hfile;
}

void ps_write_file(filehandle_t hfile, const void *pbytes, int nbytes)
{
	const char *thisfunc = "ps_write_file";

	char errmsg[4000] = {};

	DWORD nbWritten = 0;
	BOOL succ = WriteFile(hfile, pbytes, nbytes, &nbWritten, NULL);
	if (!succ)
	{
		snprintf(errmsg, ARRAYSIZE(errmsg),
			"[ERROR] WriteFile() fail with winerr=%s"
			"file-handle: 0x%p\n"
			,
			ITCS_WinError, hfile);
		throw ErrMsg(thisfunc, errmsg);
	}
}

void ps_close_file(filehandle_t hfile)
{
	CloseHandle(hfile);
}

void ps_create_file_write_string(const char *filepath, const char *text)
{
	filehandle_t hfile = ps_create_new_file(filepath);
	ps_write_file(hfile, text, (int)strlen(text));
	ps_close_file(hfile);
}
