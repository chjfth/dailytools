#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <locale.h>

#include <mswin/winuser.itc.h>

TCHAR* now_timestr(TCHAR buf[], int bufchars, bool ymd=false)
{
	SYSTEMTIME st = {0};
	GetLocalTime(&st);
	buf[0]=_T('['); buf[1]=_T('\0'); buf[bufchars-1] = _T('\0');
	if(ymd) {
		_sntprintf_s(buf, bufchars-1, _TRUNCATE, _T("%s%04d-%02d-%02d "), buf, 
			st.wYear, st.wMonth, st.wDay);
	}
	_sntprintf_s(buf, bufchars-1, _TRUNCATE, _T("%s%02d:%02d:%02d.%03d]"), buf,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
	return buf;
}

void PrnTs(const TCHAR *fmt, ...)
{
	static int count = 0;
	static DWORD s_prev_msec = GetTickCount();

	DWORD now_msec = GetTickCount();

	TCHAR buf[2000] = {0};

	// Print timestamp to show that time has elapsed for more than one second.
	DWORD delta_msec = now_msec - s_prev_msec;
	if(delta_msec>=1000)
	{
		_tprintf(_T(".\n"));
	}

	TCHAR timebuf[40] = {};
	now_timestr(timebuf, ARRAYSIZE(timebuf));

	_sntprintf_s(buf, _TRUNCATE, _T("%s (+%3u.%03us) "), 
		timebuf, 
		delta_msec/1000, delta_msec%1000);

	int prefixlen = (int)_tcslen(buf);

	va_list args;
	va_start(args, fmt);

	_vsntprintf_s(buf+prefixlen, ARRAYSIZE(buf)-prefixlen-1, // -1 for trailing \n
		_TRUNCATE, fmt, args); 

	va_end(args);

	// add trailing \n
	int slen = (int)_tcslen(buf);
	buf[slen] = '\n';
	buf[slen+1] = '\0';

	_tprintf(_T("%s"), buf);
#ifdef _DEBUG
	OutputDebugString(buf);
#endif
	s_prev_msec = now_msec;
}

HHOOK g_hhook;

bool g_isDelayInHook = false;
int g_DelayConstantMs = 0;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT &ki = *(KBDLLHOOKSTRUCT*)lParam;
	UINT wm_xxx = (UINT)wParam;

	PrnTs(_T("KBD: <%d> %-12s, %-16s, Scancode=0x%02X(%d)")
		,
		nCode,
		ITCS(wm_xxx, itc::WM_xxx),  // %-12s
		ITCSv(ki.vkCode, itc::VK_xxx), // %-16s
		ki.scanCode, ki.scanCode
		);

	PrnTs(_T("     flags: %s"), ITCSv(ki.flags, itc::LLKHF_xxx));

	if(g_isDelayInHook && g_DelayConstantMs==0)
	{
		if(ki.vkCode>='1' && ki.vkCode<='9')
		{
			int msec = 500 * (ki.vkCode-'1'+1);
			PrnTs(_T("Delay %dms"), msec);
			Sleep(msec);
		}
		else if(ki.vkCode=='0')
		{
			PrnTs(_T("!!! Got '0', not calling CallNextHookEx().\n"));
			return 0;
		}
	}
	else if(g_isDelayInHook && g_DelayConstantMs>0)
	{
		PrnTs(_T("Delay %dms"), g_DelayConstantMs);
		Sleep(g_DelayConstantMs);
	}

	LRESULT lret = CallNextHookEx(g_hhook, nCode, wParam, lParam);
	return lret;
}

void do_work()
{
	MSG msg = {};
	while (GetMessage (&msg, NULL, 0, 0))
	{
		DispatchMessage (&msg) ;
	}
}

int _tmain(int argc, TCHAR* argv[])
{
	setlocale(LC_ALL, "");

	if(argc==1)
	{
		_tprintf(_T("Hint: Pass param 'D', so that pressing key '1'~'9' would delay hookproc return.\n"));
		_tprintf(_T("Hint: Pass param 'D100', so that each hookproc will delay 100 millisec.\n"));
	}
	else
	{
		if(argv[1][0]=='D')
		{
			g_isDelayInHook = true;

			if(argv[1][1]=='\0') // single 'D'
			{ 
				_tprintf(_T("Pressing key '1'~'9' would delay hookproc return, multiple of 500ms.\n"));
				_tprintf(_T("So that we can observe when our hookproc is banned by Windows.\n"));
				_tprintf(_T("And, pressing key '0' to deliberately NOT calling CallNextHookEx().\n"));
				_tprintf(_T("\n"));
			}
			else
			{
				g_DelayConstantMs = _tcstoul(argv[1]+1, nullptr, 10);
				_tprintf(_T("Echo hookproc will Delay %d millisec.\n"), g_DelayConstantMs);
			}
		}
	}

	PrnTs(_T("Calling SetWindowsHookEx(WH_KEYBOARD_LL)..."));

	HHOOK hhook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	if(hhook)
		PrnTs(_T("Success SetWindowsHookEx(WH_KEYBOARD_LL), hhook=0x%p"), (void*)hhook);
	else
		PrnTs(_T("Error SetWindowsHookEx(WH_KEYBOARD_LL), WinErr=%d"), GetLastError());
	
	g_hhook = hhook;
	do_work();

	PrnTs(_T("Calling UnhookWindowsHookEx()..."));
	UnhookWindowsHookEx(hhook);

	return 0;
}

