/*
[2023-07-03] This program creates a sub-process, waits for its completion,
and use GetProcessTimes() to report its time cost.
Yes, it is like the Unix/Linux `time` program.
*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <locale.h>

#include <mswin/trueGetTickCount.h>
#include <mswin/WinError.itc.h>

const TCHAR *version = _T("1.1");

typedef DWORD WinErr_t;

HANDLE myRunSubprocess(const TCHAR *pszExepath, const TCHAR *iSubCmdline, 
	bool isShowWindow=true)
{
	// Return the sub-process handle.

	TCHAR szCmdLine[64000] = {};

	if(_tcslen(iSubCmdline)>=ARRAYSIZE(szCmdLine))
	{
		_tprintf(_T("[timesub] Sorry, Sub-process command-line cannot exceed %d chars.\n"), 
			ARRAYSIZE(szCmdLine));
		return NULL;
	}

	_tcscpy_s(szCmdLine, iSubCmdline);
	// -- MSDN: szCmdLine[] may be modified inside; he injects temporal NUL after argv[0],
	//    then before CreateProcess() return, that user TCHAR is restored.
	//    So, MSDN requires the szCmdLine to be (non-const) TCHAR*. 

	WinErr_t winerr = 0;
	STARTUPINFO sti = {sizeof(STARTUPINFO)};
	sti.dwFlags = STARTF_USESHOWWINDOW;
	sti.wShowWindow = isShowWindow ? SW_SHOW : SW_HIDE;

	PROCESS_INFORMATION procinfo = {0};

	BOOL succ = CreateProcess(
		pszExepath,
		szCmdLine, 
		NULL, // process security attr
		NULL, // thread security attr
		FALSE, // bInheritHandles 
		isShowWindow ? 0 : CREATE_NO_WINDOW, // dwCreationFlags
		NULL, // lpEnvironment
		NULL, // lpCurrentDirectory  
		&sti, 
		&procinfo);
	if(!succ)
	{
		_tprintf(_T("[timesub] CreateProcess() fail, WinErr=%s.\n"), ITCS_WinError);
		return NULL;
	}

	//	_tprintf(_T("Subprocess PID = %u\n"), procinfo.dwProcessId);

	CloseHandle(procinfo.hThread);

	// Note: CloseHandle(procinfo.hProcess); will/should be done by the caller.

	// Wait for sub-process done.
	//
	BOOL waitre = WaitForSingleObject(procinfo.hProcess, INFINITE);
	if(waitre!=WAIT_OBJECT_0)
	{
		_tprintf(_T("[timesub] PANIC! WaitForSingleObject() on process-handle got WinErr=%s.\n"), ITCS_WinError);
	}

	return procinfo.hProcess;
}

const TCHAR *wincmdline_strip_argv0(
	const TCHAR *cmdline, const TCHAR *argv0, int *p_nspaces_after_argv0=0)
{
	// argv0 is definitely NOT surrounded by double-quotes.
	// cmdline may or may not be surrounded by double-quotes.
	//
	// Now checking cmdline, there may be some space-chars.
	// after argv0 part or after "argv0" part, we count the space-chars.

	const TCHAR * argv0_in_cmdline = _tcsstr(cmdline, argv0);
	if(!argv0_in_cmdline)
	{
		_tprintf(_T("[timesub] PANIC! Cannot find argv[0] in cmdline.\n"));
		exit(4);
	}

	const TCHAR *pv0tail = argv0_in_cmdline + _tcslen(argv0);

	if(pv0tail[0]=='"')
	{
		pv0tail++;
	}

	if(pv0tail[0]=='\0')
	{
		if(p_nspaces_after_argv0)
			*p_nspaces_after_argv0 = 0;

		return nullptr;
	}

	// Now count space-chars
	int nspc = 0;
	while( pv0tail[nspc] == ' ' )
		nspc++;

	if(nspc==0)
	{
		_tprintf(_T("[timesub] PANIC! No space delimiter found after argv[0] part.\n"));
		exit(4);		
	}

	if(p_nspaces_after_argv0)
		*p_nspaces_after_argv0 = nspc;

	if(pv0tail[nspc]!='\0')
		return pv0tail + nspc;
	else
		return nullptr;
}

__int64 diff_FILETIME(FILETIME ft1, FILETIME ft2)
{
	ULARGE_INTEGER ul1 = {ft1.dwLowDateTime, ft1.dwHighDateTime};
	ULARGE_INTEGER ul2 = {ft2.dwLowDateTime, ft2.dwHighDateTime};
	return ul1.QuadPart - ul2.QuadPart;
}

int _tmain(int argc, TCHAR* argv[])
{
	// Will use env-var SUBEXE's value as CreateProcess's first param(lpApplicationName).

	TCHAR exepath[MAX_PATH]={};
	GetEnvironmentVariable(_T("SUBEXE"), exepath, MAX_PATH-1);
	if(exepath[0])
	{
		_tprintf(_T("[timesub] Will call CreateProcess() with first param from env-var SUBEXE:\n"));
		_tprintf(_T("[timesub]    %s\n"), exepath);
	}
	else
	{
//		_tprintf(_T("[timesub] Will call CreateProcess() with first param=NULL. (no env-var SUBEXE)\n"));
	}

	const TCHAR *subexe_cmdline = wincmdline_strip_argv0(GetCommandLine(), argv[0]);

	if(!subexe_cmdline)
	{
		_tprintf(_T("[timesub] version %s\n"), version);
		_tprintf(_T("[timesub] No sub-process command-line provided. I will do nothing.\n"));
		return 0x404;
	}

	DWORD msec_start = trueGetTickCount();
	msec_start = trueGetTickCount(); // again to wipe off start jitter, maybe

	WinErr_t winerr = 0;
	HANDLE hProcess = myRunSubprocess(
		exepath[0] ? exepath : NULL,
		subexe_cmdline);

	if(!hProcess)
		return 0x405;

	DWORD msec_end = trueGetTickCount();

	DWORD subproc_exitcode = -1;
	BOOL succ = GetExitCodeProcess(hProcess, &subproc_exitcode);
	if(!succ)
	{
		winerr = GetLastError();
		_tprintf(_T("[timesub] PANIC! GetExitCodeProcess() got WinErr=%s.\n"), ITCS_WinError);
		exit(4);
	}

	FILETIME weCreate={}, weExit={}, wesKernel={}, wesUser={};
	succ = GetProcessTimes(hProcess, &weCreate, &weExit, &wesKernel, &wesUser);
	if(!succ)
	{
		winerr = GetLastError();
		_tprintf(_T("[timesub] PANIC! GetProcessTimes() got WinErr=%s.\n"), ITCS_WinError);
		exit(4);
	}

	CloseHandle(hProcess);

	// hns: count of 100ns
	__int64 hnsTimespan = diff_FILETIME(weExit, weCreate);
	
	ULARGE_INTEGER ulk = {wesKernel.dwLowDateTime, wesKernel.dwHighDateTime};
	__int64 hnsKrnl = ulk.QuadPart;

	ULARGE_INTEGER ulu = {wesUser.dwLowDateTime, wesUser.dwHighDateTime};
	__int64 hnsUser = ulu.QuadPart;

#define hns_m(hns)  ( (int) ((hns)/10/1000/1000/60) ) //  m(minute) part
#define hns_s(hns)  ( (int) ((hns)/10/1000/1000%60) ) //  s(second) part
#define hns_ms(hns) ( (int) ((hns)/10/1000%1000)    ) // ms(millisecond) part

	TCHAR report[300] = {};
	_sntprintf_s(report, _TRUNCATE, 
		_T("[timesub] wall %4dm%d.%03ds\n")
		_T("[timesub] user %4dm%d.%03ds\n")
		_T("[timesub] krnl %4dm%d.%03ds\n")
		,
		hns_m(hnsTimespan), hns_s(hnsTimespan), hns_ms(hnsTimespan),
		hns_m(hnsKrnl), hns_s(hnsKrnl), hns_ms(hnsKrnl),
		hns_m(hnsUser), hns_s(hnsUser), hns_ms(hnsUser)
		);

	_ftprintf_s(stderr, _T("\n%s\n"), report);

	// [2025-04-08] If GetProcessTimes() reports a walltime less than 16ms,
	// (could happen on a VMwks Win7 VM) hint user a more accurate time.
	DWORD msec_used = msec_end-msec_start;
	if(msec_used<16 && (int)msec_used<hns_ms(hnsTimespan))
	{
		_ftprintf_s(stderr, _T("[timesub] Note: Finer walltime used: %d ms\n"), msec_used);
	}

	// We follow sub-process's exit code.
	return subproc_exitcode;
}
