#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <Psapi.h>
#include <ShlObj.h> // IsUserAnAdmin()
#include <assert.h>
#include <stdio.h>

#include "utils.h"

#pragma comment(lib, "Psapi.lib") // GetModuleBaseName
#pragma comment(lib, "Shell32.lib") // IsUserAnAdmin


TCHAR* now_timestr(TCHAR buf[], int bufchars, bool ymd, bool add_millisec)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	buf[0]=_T('['); buf[1]=_T('\0'); buf[bufchars-1] = _T('\0');
	if(ymd) 
	{
		_sntprintf_s(buf, bufchars-1, _TRUNCATE, _T("%s%04d-%02d-%02d "), buf, 
			st.wYear, st.wMonth, st.wDay);
	}
	if(add_millisec)
	{
		_sntprintf_s(buf, bufchars-1, _TRUNCATE, _T("%s%02d:%02d:%02d.%03d]"), buf,
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	}
	else
	{
		_sntprintf_s(buf, bufchars-1, _TRUNCATE, _T("%s%02d:%02d:%02d]"), buf,
			st.wHour, st.wMinute, st.wSecond);
	}
	return buf;
}


static int s_dbgcount = 0;

void vaDbgTs(const TCHAR *fmt, ...)
{
	// Note: Each calling outputs one line, with timestamp prefix.
	// A '\n' will be added automatically at end.

	static int count = 0;
	static DWORD s_prev_msec = GetTickCount();

	DWORD now_msec = GetTickCount();

	TCHAR buf[1000] = {0};

	// Print timestamp to show that time has elapsed for more than one second.
	DWORD delta_msec = now_msec - s_prev_msec;
	if(delta_msec>=1000)
	{
		OutputDebugString(_T(".\n"));
	}

	TCHAR timebuf[40] = {};
	now_timestr(timebuf, ARRAYSIZE(timebuf));

	_sntprintf_s(buf, _TRUNCATE, _T("[%d]%s (+%3u.%03us) "), 
		++s_dbgcount,
		timebuf, 
		delta_msec/1000, delta_msec%1000);

	int prefixlen = (int)_tcslen(buf);

	va_list args;
	va_start(args, fmt);

	_vsntprintf_s(buf+prefixlen, ARRAYSIZE(buf)-3-prefixlen, _TRUNCATE, fmt, args);

	va_end(args);

	// add trailing \n
	int slen = (int)_tcslen(buf);
	if(slen==ARRAYSIZE(buf)-1)
		--slen;

	buf[slen] = '\n';
	buf[slen+1] = '\0';

	OutputDebugString(buf);

	s_prev_msec = now_msec;
}

void vaDbgS(const TCHAR *fmt, ...)
{
	// This only has Sequential prefix.

	TCHAR buf[1000] = {0};

	_sntprintf_s(buf, ARRAYSIZE(buf)-3, _TRUNCATE, TEXT("[%d] "), ++s_dbgcount); // prefix seq

	int prefixlen = (int)_tcslen(buf);

	va_list args;
	va_start(args, fmt);

	_vsntprintf_s(buf+prefixlen, ARRAYSIZE(buf)-3-prefixlen, _TRUNCATE, fmt, args);

	prefixlen = (int)_tcslen(buf);
	_tcsncpy_s(buf+prefixlen, 2, TEXT("\r\n"), _TRUNCATE); // add trailing \r\n

	va_end(args);

	// add trailing \n
	int slen = (int)_tcslen(buf);
	if(slen==ARRAYSIZE(buf)-1)
		--slen;

	buf[slen] = '\n';
	buf[slen+1] = '\0';

	OutputDebugString(buf);
}

const TCHAR *GetExeFilename() // the exe filename with .exe suffix
{
	static TCHAR s_exename[MAX_PATH] = _T("");
	if(!s_exename[0])
	{
		GetModuleBaseName(GetCurrentProcess(), NULL, s_exename, ARRAYSIZE(s_exename));
	}
	return s_exename;
}

const TCHAR *GetExeStemname() // the exe filename without .exe suffix
{
	static TCHAR s_exestem[MAX_PATH] = _T("");
	if(!s_exestem[0])
	{
		_tcscpy_s(s_exestem, GetExeFilename());

		int slen = (int)_tcslen(s_exestem);
		if(_tcsicmp(s_exestem+slen-4, _T(".exe"))==0)
			s_exestem[slen-4] = '\0';
	}
	return s_exestem;
}

int vaMsgBox(HWND hwnd, UINT utype, const TCHAR *szTitle, const TCHAR *szfmt, ...)
{
	TCHAR szModuleName[40] = {};

	va_list args;
	va_start(args, szfmt);

	if(!szTitle) {
		szTitle = GetExeFilename();
	}

	TCHAR msgtext[4000] = {};
	_vsntprintf_s(msgtext, _TRUNCATE, szfmt, args);

	int ret = MessageBox(hwnd, msgtext, szTitle, utype);

	va_end(args);
	return ret;
}

BOOL vaSetWindowText(HWND hwnd, const TCHAR *szfmt, ...)
{
	va_list args;
	va_start(args, szfmt);

	TCHAR msgtext[400] = {};
	_vsntprintf_s(msgtext, _TRUNCATE, szfmt, args);

	BOOL succ = SetWindowText(hwnd, msgtext);

	va_end(args);
	return succ;
}

BOOL vlSetDlgItemText(HWND hwnd, int nIDDlgItem, const TCHAR *szfmt, va_list args)
{
	TCHAR tbuf[64000] = {};
	_vsntprintf_s(tbuf, _TRUNCATE, szfmt, args);
	BOOL succ = SetDlgItemText(hwnd, nIDDlgItem, tbuf);
	return succ;	
}

BOOL vaSetDlgItemText(HWND hwnd, int nIDDlgItem, const TCHAR *szfmt, ...)
{
	va_list args;
	va_start(args, szfmt);
	BOOL succ = vlSetDlgItemText(hwnd, nIDDlgItem, szfmt, args);
	va_end(args);
	return succ;
}

void vlAppendText_mled(HWND hedit, const TCHAR *szfmt, va_list args)
{
	TCHAR tbuf[64000] = {};
	_vsntprintf_s(tbuf, _TRUNCATE, szfmt, args);

	int pos = GetWindowTextLength (hedit);
	// -- [2024-10-26] Note: when pos reaches 30000, Edit_SetSel() will fail with
	// WinErr=5 (ERROR_ACCESS_DENIED).
	// User needs to raise the limit by calling:
	//   Edit_LimitText(hedit, 200*1024); // EM_LIMITTEXT

	Edit_SetSel(hedit, pos, pos);
	Edit_ReplaceSel(hedit, tbuf);
}

void vaAppendText_mled(HWND hedit, const TCHAR *szfmt, ...)
{
	va_list args;
	va_start(args, szfmt);
	vlAppendText_mled(hedit, szfmt, args);
	va_end(args);
}

//////////////////////////////////////////

typedef void PROC_WM_TIMER_call_once(void *usercontext);

struct WM_TIMER_call_once_st
{
	HWND hwnd;
	PROC_WM_TIMER_call_once *userproc;
	void *usercontext;
};

void CALLBACK WM_TIMER_call_once_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	WM_TIMER_call_once_st *indata = (WM_TIMER_call_once_st*)idEvent;

	indata->userproc(indata->usercontext);

	KillTimer(indata->hwnd, idEvent);
	delete indata;
}


bool WM_TIMER_call_once(HWND hwnd, int delay_millisec, 
	PROC_WM_TIMER_call_once *userproc, void *usercontext)
{
	// TODO: If hwnd==NULL, maybe we can create a hidden window automatically.
	assert(hwnd);
	if(!hwnd)
		return false;

	WM_TIMER_call_once_st *indata = new WM_TIMER_call_once_st;
	indata->hwnd = hwnd;
	indata->userproc = userproc;
	indata->usercontext = usercontext;

	UINT_PTR uret = SetTimer(hwnd, (UINT_PTR)indata, delay_millisec, WM_TIMER_call_once_TimerProc);
	if(uret!=(UINT_PTR)indata)
	{
		delete indata;
		return false;
	}

	return true;
}

unsigned __int64 get_qpf()
{
	LARGE_INTEGER li = {};
	BOOL succ = QueryPerformanceFrequency(&li);
	if(!succ)
		return -1;
	else
		return li.QuadPart;
}

unsigned __int64 get_qpc()
{
	LARGE_INTEGER li = {};
	BOOL succ = QueryPerformanceCounter(&li);
	if(!succ)
		return -1;
	else
		return li.QuadPart;
}

DWORD TrueGetMillisec()
{
	static __int64 s_qpf = get_qpf();

	// We should use QueryPerformanceCounter(), 
	// bcz GetTickCount only has 15.6 ms resolution.
	DWORD millisec = DWORD(get_qpc()*1000 / s_qpf);
	return millisec;
}

const TCHAR *str_ANSIorUnicode()
{
	return sizeof(TCHAR)==1 ? _T("ANSI") : _T("Unicode");
}

void util_SetDlgDefaultButton(HWND hwndDlg, UINT idDefault) 
{
	// Provided by PSSA2000 TokenMaster.cpp 

	// Get that last default control
	UINT nOld = (UINT) SendMessage(hwndDlg, DM_GETDEFID, 0, 0);

	// Reset the current default push button to a regular button.
	if (HIWORD(nOld) == DC_HASDEFID)
	{
		SendDlgItemMessage(hwndDlg, LOWORD(nOld), BM_SETSTYLE, 
			BS_PUSHBUTTON, // make it a normal button
			(LPARAM) TRUE);
	}

	// Update the default push button's control ID.
	SendMessage(hwndDlg, DM_SETDEFID, idDefault, 0L);

	// Set the new style.
	SendDlgItemMessage(hwndDlg, idDefault, BM_SETSTYLE, 
		BS_DEFPUSHBUTTON, // make it a stand-out button
		(LPARAM) TRUE); 
}

#pragma comment(lib, "Shell32.lib")

BOOL Is_UserAnAdmin()
{
	// Define this wrapper instead of using IsUserAnAdmin(), bcz on VC2015 blames warning on a buggy line from ShlObj.h:
	/*
1>C:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A\include\ShlObj.h(1151): warning C4091: 'typedef ': ignored on left of 'tagGPFIDL_FLAGS' when no variable is declared
1>         utils.cpp

The buggy line is(L#1146):

typedef enum tagGPFIDL_FLAGS
{
	GPFIDL_DEFAULT    = 0x0000,      // normal Win32 file name, servers and drive roots included
	GPFIDL_ALTNAME    = 0x0001,      // short file name
	GPFIDL_UNCPRINTER = 0x0002,      // include UNC printer names too (non file system item)
};

WinSDK 8.1 has fixed it.
	*/
	
	return IsUserAnAdmin();
}
