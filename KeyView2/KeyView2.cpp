/*--------------------------------------------------------
KEYVIEW2.C -- Displays Keyboard and Character Messages
(c) Charles Petzold, 1998
[2015-12-18 ~] Some updates by chj:
 * Show current Charset value and FontFace name on title bar.
 * Add a Seq(sequence number) column for easier investigation.
 * Right click context menu to clear or copy screen content.
 * Buffered lines quantity can be configured, default 5000.
 * Show hex as well as decimal for VK code and scan code.
 * v1.8: Show GetACP() value on title.
 * v1.8: On startup, try to select the Charset that matches
   user's default input-locale, instead of DEFAULT_CHARSET.
 * v1.9: For Keyview2A, cope with the case that a Unicode codepoint 
   is packed in one ANSI WM_CHAR message. (Win10.21H2, UTF8ACP on)
--------------------------------------------------------*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include <psapi.h> // GetProcessImageFileName 

#include "easyclipboard.h"
#include "dbgprint.h"
#include "iversion.h"
#include "resource.h"

#ifdef UNICODE
#define APPNAME TEXT("KeyView2U") 
#define TVARIANT TEXT("Unicode")
#else
#define APPNAME TEXT("KeyView2A") 
#define TVARIANT TEXT("ANSI")
#endif

#define CHARSET_UNINITIALIZED (-1)

const int UI_KEYNAME_LIMIT = 16;

const int bsTitlePrefix = 80;
TCHAR szTitlePrefix[bsTitlePrefix];

const TCHAR szTop_s1[] = TEXT ("  Seq Message        VKcode,Keyname  Char        ");
const TCHAR szTop_s2[] = TEXT ("Repeat Scancode Ext ALT Prev Tran") ;
const TCHAR szUnd[] = TEXT ("  ___ _______        ______________  ____        ")
	TEXT ("______ ________ ___ ___ ____ ____") ;
const int g_chars_per_line = ARRAYSIZE(szUnd)-1;
const int g_chars_per_line_rn = g_chars_per_line+2; // including trailing \r\n

int g_max_store_lines = 5000;
	// when copy to clipboard, max such lines are copied
	// may be changed by command-line parameter
const int g_max_store_lines_max = 99999;
	// can change it to 99 for testing, you'll see display Seq rewind to 0

int g_keydes_all_bufchars = 0; // assign at program start 
TCHAR *g_keydes_for_clipboard = NULL; // assign to a dynamic array

int g_seq = 0;
int g_LinesToDraw = 0; 
MSG  *g_armsg ;

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM) ;

void process_cmd_options(int argc, TCHAR *argv[])
{
	if(argc==0)
		return;

	do
	{
		TCHAR *pcur = argv[0], *pnext = argv[1];

		if(_tcscmp(pcur, _T("-b"))==0)
		{
			if(pnext)
			{
				g_max_store_lines = _tcstoul(pnext, nullptr, 0);
				argv++;
			}
		}

		argv++;
	} while(*argv);
}

const TCHAR *myGetCharsetName(DWORD charset)
{
	if(charset==CHARSET_UNINITIALIZED)
		return TEXT("unset");

	struct cs2name_st { BYTE charset; const TCHAR *name; };
	// -- .charset is BYTE bcz TEXTMETRIC.tmCharSet is so.

	static cs2name_st cs2names[] =
	{
		{ ANSI_CHARSET, TEXT("ANSI or West European") }, // very trick inside
		{ DEFAULT_CHARSET, TEXT("Default") }, // relates to GetACP(), will not get this from WM_INPUTLANGCHANGE 
		{ SYMBOL_CHARSET, TEXT("Symbol") },            // 2
		{ SHIFTJIS_CHARSET, TEXT("Japanese") },        // 128
		{ HANGUL_CHARSET, TEXT("Korean") },            // 129
		{ GB2312_CHARSET, TEXT("Chinese GBK") },       // 134
		{ CHINESEBIG5_CHARSET, TEXT("Chinese Big5") }, // 136
		{ HEBREW_CHARSET, TEXT("Hebrew") },   // 177
		{ ARABIC_CHARSET, TEXT("Arabic") },   // 178
		{ GREEK_CHARSET, TEXT("Greek") },     // 161
		{ TURKISH_CHARSET, TEXT("Turkish") }, // 162
		{ VIETNAMESE_CHARSET, TEXT("Vietnamese") }, // 163
		{ BALTIC_CHARSET, TEXT("Baltic") },   // 186
		{ RUSSIAN_CHARSET, TEXT("Russian") }, // 204
		{ THAI_CHARSET, TEXT("Thai") },	      // 222
		{ EASTEUROPE_CHARSET, TEXT("Central/East European") },  // 238
		{ OEM_CHARSET, TEXT("OEM") }, // 255
	};

	int i;
	for(i=0; i<ARRAYSIZE(cs2names); i++)
	{
		if(charset==cs2names[i].charset)
			return cs2names[i].name;
	}
	return TEXT("unknown");
}

bool Is_CharMsg(UINT message)
{
	return (message == WM_CHAR || message == WM_SYSCHAR ||
		message == WM_DEADCHAR || message == WM_SYSDEADCHAR);
}

bool Is_KeyStrokeMsg(UINT message)
{
	return (message == WM_KEYDOWN || message == WM_KEYUP ||
		message == WM_SYSKEYDOWN || message == WM_SYSKEYUP);
}

#define WM_Trigger_INPUTLANGCHANGE (WM_USER+1)

void Trigger_WM_INPUTLANGCHANGE(HWND hwnd)
{
	// [2023-01-21] Chj:
	// Purpose: On program startup, we do not see WM_INPUTLANGCHANGE, 
	// until human user initiates Input-language change.
	// Before WM_INPUTLANGCHANGE, we use this function to trigger 
	// WM_INPUTLANGCHANGE so that we can get current-charset value. 
	//
	// But, this trick succeeds only when at least TWO input-langs are available.

	BOOL succ = 0;
	HKL curhkl = GetKeyboardLayout(0);

	HKL arHkl[100] = {};
	int nkl = GetKeyboardLayoutList(ARRAYSIZE(arHkl), arHkl);
	assert(nkl>0);

	if(nkl==1)
		return;

	PostMessage(hwnd, WM_Trigger_INPUTLANGCHANGE, (WPARAM)HKL_NEXT, 0);
	PostMessage(hwnd, WM_Trigger_INPUTLANGCHANGE, (WPARAM)curhkl, 0);
}

int WINAPI _tWinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
					PTSTR __szCmdLine_not_used, int iCmdShow)
{
	int argc = 0;
	const TCHAR *fullcmdline = GetCommandLine();
#if (defined UNICODE || defined _UNICODE) 
	TCHAR **argv = CommandLineToArgvW(fullcmdline, &argc);
	// TCHAR **argv = __wargv; // __wargv is NULL on a VS2010 compiled program on Win7.
#else
	argc = __argc;
	TCHAR ** argv = __argv;
#endif

	static TCHAR szAppName[] = APPNAME ;
	HWND         hwnd ;
	MSG          msg ;
	WNDCLASS     wndclass ;

	HICON hicon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	if(!hicon) {
		MessageBox(NULL, 
			TEXT("This EXE does not have resource(.res) linked in. Call the dev to fix it."), 
			TEXT("Error"), MB_ICONEXCLAMATION);
		return 4;
	}

	StringCchPrintf(szTitlePrefix, bsTitlePrefix, _T("KeyView2 (%s) v%d.%d"),
		TVARIANT, THISLIB_VMAJOR, THISLIB_VMINOR);

	process_cmd_options(argc-1, argv+1);

	g_keydes_all_bufchars = g_chars_per_line_rn*g_max_store_lines;
	g_keydes_for_clipboard = new TCHAR[g_keydes_all_bufchars+1];

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(IDI_ICON));
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	//wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); 
		// use control-panel defined window default background color
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = szAppName ;

	if (!RegisterClass (&wndclass))
	{
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), 
			szAppName, MB_ICONERROR) ;
		return 0 ;
	}

	hwnd = CreateWindow (szAppName, szTitlePrefix,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL) ;

	ShowWindow (hwnd, iCmdShow) ;
	UpdateWindow (hwnd) ;

	while (GetMessage (&msg, NULL, 0, 0))
	{
		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	}

	delete []g_keydes_for_clipboard;
	
	return 0 ;
}

const TCHAR * DWORDto0x4x(DWORD n)
{
	static TCHAR s_buf[12];
	_sntprintf_s(s_buf, _TRUNCATE, _T("0x%04X"), n);
	return s_buf;
}

void GetKeyDes(const MSG &msg, TCHAR s1[], int s1size, TCHAR s2[], int s2size)
{
	// KeyDes: key description text
	//
	// keydes is separated into two sections:
	// * section 1: Seq Message Key Char
	// * section 2: Repeat Scan Ext ALT Prev Tran

	static TCHAR * szYes  = TEXT ("Yes") ;
	static TCHAR * szNo   = TEXT ("No") ;
	static TCHAR * szDown = TEXT ("Down") ;
	static TCHAR * szUp   = TEXT ("Up") ;
	
	static TCHAR * szKeyMsgName [] = { 
		TEXT ("WM_KEYDOWN"),    TEXT ("WM_KEYUP"), 
		TEXT ("WM_CHAR"),       TEXT ("WM_DEADCHAR"), 
		TEXT ("WM_SYSKEYDOWN"), TEXT ("WM_SYSKEYUP"), 
		TEXT ("WM_SYSCHAR"),    TEXT ("WM_SYSDEADCHAR") 
	} ;

	const TCHAR *keymsg_name = szKeyMsgName[msg.message-WM_KEYFIRST];

	TCHAR szKeyName[64] = _T("(unknown)");
	GetKeyNameText ((LONG)msg.lParam, szKeyName, ARRAYSIZE(szKeyName)); // VK name

	int keyname_len = _tcslen(szKeyName);

	// keydes section 1
	if(Is_KeyStrokeMsg(msg.message))	
	{
		static const TCHAR szfmt_stroke_msg[] = TEXT("%5u %-13s %3d(%02Xh) %-*.*s%c   ");
		StringCchPrintfEx(s1, s1size, NULL, NULL, 
			STRSAFE_FILL_BEHIND_NULL, // opt, can be 0
			szfmt_stroke_msg
			,
			msg.time % (g_max_store_lines_max+1),     // %5u
			keymsg_name,  // %-13s _Message_ (WM_KEYDOWN etc)
			msg.wParam,   // %3d   _VKcode_ (VK code, decimal)
			msg.wParam,   // %02X  _VKcode_ (VK code, hex)
			UI_KEYNAME_LIMIT, UI_KEYNAME_LIMIT, szKeyName,    // %-*.*s (Keyname)
			keyname_len<=UI_KEYNAME_LIMIT ? TEXT(' ') : TEXT('?') // '?' to indicate text truncation
			);
		// Memo: French keyboard, the dead-char "ACCENT CIRCONFLEXE" (at right-side of P) is 18 chars long.
	}
	else
	{
		static const TCHAR szfmt_char_msg1[] = TEXT("%5u %-13.13s              %10s ");
		//
		StringCchPrintfEx(s1, s1size, NULL, NULL, 
			STRSAFE_FILL_BEHIND_NULL, // opt, can be 0
			szfmt_char_msg1
			, 
			msg.time % (g_max_store_lines_max+1),     // %5u
			keymsg_name,  // %-13.13s _Message_ (WM_CHAR etc)
			DWORDto0x4x((DWORD)msg.wParam) // %10s  _Char_ (WM_CHAR character code, 0x0041 or 0xE0Bd85)
			);

		int part1len = (int)_tcslen(s1);

		TCHAR part2str[5] = {}; // UTF8 max 4-bytes + NUL

		bool is_packed_utf8 = msg.wParam > 0xFFFF;
		if(is_packed_utf8)
		{
#ifdef UNICODE
			assert(0);
#else
			// [2023-01-21] Chj: 
			// If the UTF8 sequence is [E0 BD 91], we'll have msg.Param=0x91BDE0 .
			// To see this, in Win10.21H2, add a Tibetan keyboard layout and strike
			// keyboard physical key a-z, 1~9 etc.
			//
			// But be aware, as of Win10.21H2, not all input-lang's stock keyboard layout
			// sends UTF8 sequence this way. For example, with UTF8ACP option on, 
			// Amharic keyboard sends U+12A0 as 3 WM_CHAR messages, each carrying 
			// one UTF8 byte-sequence, E1 8A A0, respectively.
			//
			*(DWORD*)part2str = msg.wParam;
#endif
		}
		else
		{
			 part2str[0] = (TCHAR)msg.wParam;
		}

		static const TCHAR szfmt_char_msg2[] = TEXT("%-4s ");
		//
		StringCchPrintfEx(s1+part1len, s1size-part1len, NULL, NULL, 
			STRSAFE_FILL_BEHIND_NULL, // opt, can be 0
			szfmt_char_msg2, part2str);
	}

	// keydes section 2
	StringCchPrintfEx(s2, s2size, NULL, NULL, 
		STRSAFE_FILL_BEHIND_NULL, // opt, can be 0
		TEXT("%6u %3d(%02Xh) %3s %3s %4s %4s")
		,
		LOWORD (msg.lParam), // %6u Repeat(count)
		HIWORD (msg.lParam) & 0xFF, // %4d scan code (decimal)
		HIWORD (msg.lParam) & 0xFF, // %4d scan code (hex)
		0x01000000 & msg.lParam ? szYes  : szNo,
		0x20000000 & msg.lParam ? szYes  : szNo,
		0x40000000 & msg.lParam ? szDown : szUp,
		0x80000000 & msg.lParam ? szUp   : szDown
		); // szBuffer is guaranteed to be NUL-terminated

	assert(_tcslen(s1)+_tcslen(s2)==g_chars_per_line);
}

int Do_WM_COMMAND(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	(void)lParam;
	int idcmd = LOWORD(wParam);

	if(idcmd==IDM_CLEAR)
	{
		g_seq = 0;
		g_LinesToDraw = 0;
		InvalidateRect(hwnd, NULL, TRUE);
	}
	else if(idcmd==IDM_SET_BUFFER_LINES)
	{
		//TCHAR exename[200]={0};
		//GetModuleFileName(NULL, exename, ARRAYSIZE(exename)-1); 
			// This returns absolute path of the exe.
		//GetProcessImageFileName(GetCurrentProcess(), exename, ARRAYSIZE(exename)-1);
			// This returns something like "\Device\HarddiskVolume9\w\personal\chj\...\KeyView2A.exe"

		TCHAR tbuf[200]={0};
		StringCchPrintf(tbuf, ARRAYSIZE(tbuf)-1
			, 
			TEXT("Current buffer lines is %d. It can be changed by using command line parameter (max %d).\n")
			TEXT("For example:\n")
			TEXT("\n")
			TEXT("  %s -b 9999\n")
			, 
			g_max_store_lines, g_max_store_lines_max,
			get_exename());
		MessageBox(hwnd, tbuf, TEXT("Info"), MB_ICONINFORMATION);
	}
	else if(idcmd==IDM_COPY) 
	{
		if(g_seq==0)
			return 0;

		// clear old clipboard buffer 
		int i;
		for(i=0; i<g_keydes_all_bufchars; i++)
			g_keydes_for_clipboard[i] = TEXT(' ');
		g_keydes_for_clipboard[g_keydes_all_bufchars] = TEXT('\0');

		// determine seq_start, the first seq that is still in pmsg buffer.
		int seq_start = -1;
		if(g_seq<=g_max_store_lines)
			seq_start = 1;
		else
			seq_start = g_seq-g_max_store_lines+1;

		int seq_lines = g_seq - seq_start + 1;

		// copy MSG info(as displayed on screen) to clipboard, up to MAX_STORE_LINES
		TCHAR keydes_s1[80], keydes_s2[80]; //, keydes_all[160];
		TCHAR *pfill = g_keydes_for_clipboard;
		for(i=0; i<seq_lines; i++)
		{
			// hint: first MSG is at tail of pmsg[]
			GetKeyDes(g_armsg[seq_lines-1-i], keydes_s1, ARRAYSIZE(keydes_s1), keydes_s2, ARRAYSIZE(keydes_s2));
			
			StringCchPrintf(pfill, g_chars_per_line+1, TEXT("%s%s"), keydes_s1, keydes_s2);

			pfill[g_chars_per_line] = TEXT('\r');
			pfill[g_chars_per_line+1] = TEXT('\n');

			pfill += g_chars_per_line_rn;
		}
		*pfill = TEXT('\0');

		BOOL b = easySetClipboardText(g_keydes_for_clipboard, -1);
		if(!b) {
			MessageBox(hwnd, TEXT("Unexpected: Put clipboard fail!"), APPNAME, 
				MB_ICONEXCLAMATION);
		}
	}
	return 0;
}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int s_HKL_charset = CHARSET_UNINITIALIZED; // dwCharSet=DEFAULT_CHARSET; 
	// -- v1.8: init to CHARSET_UNINITIALIZED(-1).
	static int   cxClientMax, cyClientMax, cxClient, cyClient, cxChar, cyChar ;
	static int cWndLinesMax; // lines current window height can hold 
		// lines to draw on screen, grow from 0, will not exceed cWndLinesMax
	static RECT  rectScroll ;
	static int drawwidth_s1;

	static HMENU s_popmenu;
	HMENU hmenu_tmp; int pos;
	POINT point; SIZE sizexy;
	BOOL b;

	HDC          hdc ;
	int          i;
	PAINTSTRUCT  ps ;
	TCHAR        keydes_s1[80], keydes_s2[80];
	TEXTMETRIC   tm ;
	TCHAR szTitle[200], szFontface[32];
	TCHAR szKbLayoutName[KL_NAMELENGTH];

	switch (message)
	{{
	case WM_RBUTTONDOWN:
		point.x = LOWORD (lParam) ;
		point.y = HIWORD (lParam) ;
		ClientToScreen (hwnd, &point) ;
		
		EnableMenuItem(s_popmenu, IDM_COPY , g_seq>0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(s_popmenu, IDM_CLEAR, g_seq>0 ? MF_ENABLED : MF_GRAYED);
		
		TrackPopupMenu(s_popmenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL) ;
		return 0 ;

	case WM_INPUTLANGCHANGE:
		s_HKL_charset = (DWORD)wParam ;
		assert(s_HKL_charset<256);

		// fall through
	
	case WM_CREATE:

		if(s_HKL_charset==CHARSET_UNINITIALIZED)
		{
			Trigger_WM_INPUTLANGCHANGE(hwnd);
		}

		if(!s_popmenu)
		{
			s_popmenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1));
			s_popmenu = GetSubMenu(s_popmenu, 0) ; 
		}
		// fall through to WM_DISPLAYCHANGE

	case WM_DISPLAYCHANGE:
	{
		// Get maximum size of client area

		cxClientMax = GetSystemMetrics (SM_CXMAXIMIZED) ;
		cyClientMax = GetSystemMetrics (SM_CYMAXIMIZED) ;

		// Get character size for fixed-pitch font

		hdc = GetDC (hwnd) ;
		SelectObject (hdc, CreateFont (0, 0, 0, 0, 0, 0, 0, 0,
			s_HKL_charset==CHARSET_UNINITIALIZED ? DEFAULT_CHARSET : s_HKL_charset, 
			0, 0, 0, FIXED_PITCH, NULL)) ; 
		GetTextMetrics (hdc, &tm) ;
		cxChar = tm.tmAveCharWidth ;
		cyChar = tm.tmHeight ;

		GetTextFace(hdc, ARRAYSIZE(szFontface), szFontface);

		if(tm.tmCharSet!=s_HKL_charset)
		{
			dbgprint(_T("[UNEXPECT] s_HKL_charset(%d) not equal to Font \"%s\" charset(%d)."),
				s_HKL_charset, szFontface, tm.tmCharSet);
		}

		GetKeyboardLayoutName(szKbLayoutName);

		_sntprintf_s(szTitle, _TRUNCATE, 
			TEXT("%s - ACP=%d HKL[name=\"%s\" value=0x%08X Charset=%d(%s)] Fontface=\"%s\""),
			szTitlePrefix, 
			GetACP(),
			szKbLayoutName, 
			GetKeyboardLayout(0),
			s_HKL_charset, myGetCharsetName(s_HKL_charset),
			szFontface
			);
		SetWindowText(hwnd, szTitle);

		b = GetTextExtentPoint32(hdc, szTop_s1, lstrlen(szTop_s1), &sizexy);
		drawwidth_s1 = sizexy.cx;

		DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT))) ;
		ReleaseDC (hwnd, hdc) ;

		// Allocate memory for display lines

		if (g_armsg)
			free (g_armsg) ;

		cWndLinesMax = cyClientMax/cyChar;
		g_armsg = (MSG*) malloc(g_max_store_lines * sizeof(MSG)) ;
		g_LinesToDraw = 0 ;
		g_seq = 0;
		// fall through
	}

	case WM_SIZE:
		if (message == WM_SIZE)
		{
			cxClient = LOWORD (lParam) ;
			cyClient = HIWORD (lParam) ;
		}
		// Calculate scrolling rectangle

		rectScroll.left   = 0 ;
		rectScroll.right  = cxClient ;
		rectScroll.top    = cyChar ;
		rectScroll.bottom = cyChar * (cyClient / cyChar) ;

		InvalidateRect (hwnd, NULL, TRUE) ;

		if (message == WM_INPUTLANGCHANGE)
			return TRUE ;

		return 0 ;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_CHAR:
	case WM_DEADCHAR:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_SYSCHAR:
	case WM_SYSDEADCHAR: 
		// Rearrange storage array
		// Chj: Newer MSG is inserted at head of array, but will display at bottom of window.
		for (i = g_max_store_lines-1; i>0 ; i--)
		{
			g_armsg[i] = g_armsg[i - 1] ;
		}
		// Store new message
		g_armsg[0].hwnd = hwnd ;
		g_armsg[0].message = message ;
		g_armsg[0].wParam = wParam ;
		g_armsg[0].lParam = lParam ;
		g_armsg[0].time = ++g_seq; // we borrow .time member to store MSG's seq.

		g_LinesToDraw++;
		g_LinesToDraw = min(g_LinesToDraw, cWndLinesMax) ;
		g_LinesToDraw = min(g_LinesToDraw, g_max_store_lines);

		if(Is_KeyStrokeMsg(message))
		{
			TCHAR szKeyName[64] = _T("(unknown)");
			GetKeyNameText ((LONG)lParam, szKeyName, ARRAYSIZE(szKeyName)); // VK name

			int keyname_len = _tcslen(szKeyName);

			if(keyname_len>UI_KEYNAME_LIMIT)
			{	// Output lengthy Keyname to debugging channel.
				dbgprint(_T("#%-5u GetKeyNameText() is %d chars : %s"), g_seq, keyname_len, szKeyName);
			}
		}

		// Scroll up the display
		ScrollWindow (hwnd, 0, -cyChar, &rectScroll, &rectScroll) ;

		break ;        // ie, call DefWindowProc so Sys messages work

	case WM_UNICHAR:
		// [2023-01-21] Chj: Not seeing this yet.
		dbgprint(_T("WM_UNICHAR: wParam=0x%04X , lParam=0x%04X.%04X"),
			(DWORD)wParam, HIWORD(lParam), LOWORD(lParam));
		break;

	case WM_IME_CHAR:
		dbgprint(_T("WM_IME_CHAR: (chrvalue)wParam=0x%04X , lParam=0x%04X.%04X"),
			(DWORD)wParam, HIWORD(lParam), LOWORD(lParam));
		break;

	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps) ;

		SelectObject (hdc, CreateFont (0, 0, 0, 0, 0, 0, 0, 0,
			s_HKL_charset==CHARSET_UNINITIALIZED ? DEFAULT_CHARSET : s_HKL_charset,
			0, 0, 0, FIXED_PITCH, NULL)) ; 

		SetBkMode (hdc, TRANSPARENT) ;
		TextOut (hdc, 0,            0, szTop_s1, lstrlen (szTop_s1));
		TextOut (hdc, drawwidth_s1, 0, szTop_s2, lstrlen (szTop_s2));
		TextOut (hdc, 0, 0, szUnd, lstrlen (szUnd)) ;

		for(i=0; i<min(g_LinesToDraw, cyClient/cyChar-1); i++) // -1 to exclude the head line
		{
			GetKeyDes(g_armsg[i], keydes_s1, ARRAYSIZE(keydes_s1), keydes_s2, ARRAYSIZE(keydes_s2));
			
			// Chj: X position of keydes_s2 should be assigned explicitly, 
			// because we do not know whether the "%c" in szfmt_char_msg[] is 
			// a narrow character(English letter) or a wide character(Chinese character). 
			// A wide character will occupy twice the width of a narrow one on the screen.

			const int y = (cyClient/cyChar-1-i) * cyChar; 
			TextOut(hdc, 0,             y, keydes_s1, lstrlen(keydes_s1));
			TextOut(hdc, drawwidth_s1, y, keydes_s2, lstrlen(keydes_s2));
		}
		DeleteObject (SelectObject (hdc, GetStockObject (SYSTEM_FONT))) ;
		EndPaint (hwnd, &ps) ;
		return 0 ;

	case WM_COMMAND:
		Do_WM_COMMAND(hwnd, wParam, lParam);
		return 0;

	case WM_INITMENUPOPUP:
		hmenu_tmp = (HMENU)wParam;
		pos = (int)lParam;
		break;

	case WM_Trigger_INPUTLANGCHANGE:
	{
		HKL prev = ActivateKeyboardLayout((HKL)wParam, 0);
		assert(prev!=0);
		return 0;
	}
	case WM_DESTROY:

		if (g_armsg)
			free (g_armsg) ;

		PostQuitMessage (0) ;
		return 0 ;
	}}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}
