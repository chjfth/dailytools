#pragma once

#include <tchar.h>

template<typename T> T Min(T x, T y) { return x<y ? x : y; }
template<typename T> T Max(T x, T y) { return x>y ? x : y; }

#define HANDLE_dlgMSG(hwnd, message, fn) \
  case (message): \
  return SetDlgMsgResult( hwnd, message, HANDLE_##message((hwnd), (wParam), (lParam), (fn)) );
  // For message processing in a WinAPI user's dialog-procedure, we need a further step
  // beyond that of windowsx.h's HANDLE_MSG(). This HANDLE_dlgMSG() applies that further step.
  // Ref: Raymond Chen https://devblogs.microsoft.com/oldnewthing/20031107-00/?p=41923

TCHAR* now_timestr(TCHAR buf[], int bufchars, bool ymd=false, bool add_millisec=true);

void vaDbgTs(const TCHAR *fmt, ...);

void vaDbgS(const TCHAR *fmt, ...);

const TCHAR *GetExeFilename(); // the exe filename with .exe suffix
const TCHAR *GetExeStemname(); // the exe filename without .exe suffix

int vaMsgBox(HWND hwnd, UINT utype, const TCHAR *szTitle, const TCHAR *szfmt, ...);
#define NULL_TITLE NULL // use for szTitle param

BOOL vaSetWindowText(HWND hwnd, const TCHAR *szfmt, ...);

BOOL vlSetDlgItemText(HWND hwnd, int nIDDlgItem, const TCHAR *szfmt, va_list args);
BOOL vaSetDlgItemText(HWND hwnd, int nIDDlgItem, const TCHAR *szfmt, ...);

void vlAppendText_mled(HWND hedit, const TCHAR *szfmt, va_list args);
void vaAppendText_mled(HWND hedit, const TCHAR *szfmt, ...);

typedef void PROC_WM_TIMER_call_once(void *usercontext);
bool WM_TIMER_call_once(HWND hwnd, int delay_millisec, PROC_WM_TIMER_call_once *userproc, void *usercontext);

unsigned __int64 get_qpf();
unsigned __int64 get_qpc();

DWORD TrueGetMillisec();

const TCHAR *str_ANSIorUnicode();

void util_SetDlgDefaultButton(HWND hwndDlg, UINT idDefault);

BOOL Is_UserAnAdmin();
