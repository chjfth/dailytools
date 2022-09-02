/*--------------------------------------------------------
KEYVIEW2.C -- Displays Keyboard and Character Messages
(c) Charles Petzold, 1998
[2015-12-18] Some updates by chj:
 * Show current CharsetID and font face name on title bar.
 * Add a Seq(sequence number) column for easier investigation.
 * Right click context menu to clear to copy screen content,
   and the copied lines is default to 1000(larger than screen height).
 * Show hex as well as decimal for VK code and scan code.
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

#include <getopt/sgetopt.h>
#include <gadgetlib/T_string.h>
#include <gadgetlib/timefuncs.h>

#include "easyclipboard.h"
#include "dbgprint.h"
#include "iversion.h"
#include "resource.h"

#include <TScalableArray.h>

TScalableArray<int> g_sa;

#ifdef UNICODE
#define APPNAME TEXT("KeyView2U") 
#define TVARIANT TEXT("Unicode")
#else
#define APPNAME TEXT("KeyView2A") 
#define TVARIANT TEXT("ANSI")
#endif

// #define TITLE TEXT("KeyView2 (") TVARIANT TEXT(") v1.5")
const int bsTitlePrefix = 80;
TCHAR szTitlePrefix[bsTitlePrefix];

#define COUNT(ar) (sizeof(ar)/sizeof(ar[0]))

const TCHAR szTop_s1[] = TEXT ("  Seq Message        VKcode,Keyname  Char     ");
const TCHAR szTop_s2[] = TEXT ("Repeat Scancode Ext ALT Prev Tran") ;
const TCHAR szUnd[] = TEXT ("  ___ _______        ______________  ____     ")
	TEXT ("______ ________ ___ ___ ____ ____") ;
const int g_chars_per_line = COUNT(szUnd)-1;
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

void casual_test()
{
	g_sa.SetEleQuan(10, true); // no use, just trying my .h PDB-sewing
	struct tm tmu, tml;
	ggt_gmtime(1459575155, &tmu);
	ggt_localtime(1459575155, &tml); // << no use, just test my lib, Python verify: time.localtime(1459575155)
}

void process_cmd_options(int argc, TCHAR *argv[])
{
	const TCHAR *app_short_options = _T("b:");
	sgetopt_ctx *si = sgetopt_ctx_create();

	while(1)
	{
		int c = sgetopt(si, argc, argv, app_short_options);

		if(c == -1)
			break;

		if(c==_T('b'))
		{
			g_max_store_lines =T_atoi(si->optarg);
				// Note: If g_max_store_lines is less than window height by lines,
				// some lines may display as blank, which seems to be inevitable 
				// with the ScrollWindow() technique used here.
		}
	}

	sgetopt_ctx_delete(si);
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

	casual_test();
	process_cmd_options(argc, argv);

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
	return msg.wParam ;
}

const TCHAR *GetCharsetName(BYTE charsetid)
{
	struct cs2name_st { BYTE id; const TCHAR *name; };
	static cs2name_st cs2names[] =
	{
		{ 0, TEXT("ANSI or West European") },
		{ 1, TEXT("Default") }, // will not get this
		{ 2, TEXT("Symbol") },
		{ 128, TEXT("Japanese") },
		{ 129, TEXT("Korean") },
		{ 134, TEXT("Chinese GBK") }, // GB2312_CHARSET
		{ 136, TEXT("Chinese Big5") },
		{ 177, TEXT("Hebrew") },
		{ 178, TEXT("Arabic") },
		{ 161, TEXT("Greek") }, // GREEK_CHARSET
		{ 162, TEXT("Turkish") },
		{ 163, TEXT("Vietnamese") },
		{ 186, TEXT("Baltic") },
		{ 204, TEXT("Russian") },	
		{ 222, TEXT("Thai") },	
		{ 238, TEXT("Central European") }, // EASTEUROPE_CHARSET
		{ 255, TEXT("OEM") },	
	};
	
	int i;
	for(i=0; i<COUNT(cs2names); i++)
	{
		if(charsetid==cs2names[i].id)
			return cs2names[i].name;
	}
	return TEXT("unknown");
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

	bool is_char_msg = msg.message == WM_CHAR ||
		msg.message == WM_SYSCHAR ||
		msg.message == WM_DEADCHAR ||
		msg.message == WM_SYSDEADCHAR ;
	bool is_stroke_msg = !is_char_msg;
	
	TCHAR szKeyName[32] ;
	GetKeyNameText (msg.lParam, szKeyName, COUNT(szKeyName)); // VK name

	// keydes section 1
	if(is_stroke_msg)	
	{
		static const TCHAR szfmt_stroke_msg[] = TEXT("%5u %-13s %3d(%02Xh) %-16s%c");
		StringCchPrintfEx(s1, s1size, NULL, NULL, 
			STRSAFE_FILL_BEHIND_NULL, // opt, can be 0
			szfmt_stroke_msg,
			msg.time % (g_max_store_lines_max+1),     // %5u
			keymsg_name,  // %-13s _Message_ (WM_KEYDOWN etc)
			msg.wParam,   // %3d   _VKcode_ (VK code, decimal)
			msg.wParam,   // %02X  _VKcode_ (VK code, hex)
			szKeyName,    // %-16s (Keyname)
			TEXT(' '));
	}
	else
	{
		static const TCHAR szfmt_char_msg[] = TEXT("%5u %-13s                  0x%04X%1s%c ");
		StringCchPrintfEx(s1, s1size, NULL, NULL, 
			STRSAFE_FILL_BEHIND_NULL, // opt, can be 0
			szfmt_char_msg, 
			msg.time % (g_max_store_lines_max+1),     // %5u
			keymsg_name,  // %-13s _Message_ (WM_CHAR etc)
			msg.wParam,   // %04X  _Char_ (WM_CHAR character code, in 16-bit hex)
			TEXT(" "),    // %1s
			msg.wParam    // %c    _Char_'s printable form
			);
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
		//GetModuleFileName(NULL, exename, COUNT(exename)-1); 
			// This returns absolute path of the exe.
		//GetProcessImageFileName(GetCurrentProcess(), exename, COUNT(exename)-1);
			// This returns something like "\Device\HarddiskVolume9\w\personal\chj\...\KeyView2A.exe"

		TCHAR tbuf[200]={0};
		StringCchPrintf(tbuf, COUNT(tbuf)-1
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
			GetKeyDes(g_armsg[seq_lines-1-i], keydes_s1, COUNT(keydes_s1), keydes_s2, COUNT(keydes_s2));
			
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
	static DWORD dwCharSet = DEFAULT_CHARSET ;            
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
	TCHAR szTitle[128], szFontface[32];
	TCHAR szKbLayoutName[KL_NAMELENGTH];

	switch (message)
	{
	case WM_RBUTTONDOWN:
		point.x = LOWORD (lParam) ;
		point.y = HIWORD (lParam) ;
		ClientToScreen (hwnd, &point) ;
		
		EnableMenuItem(s_popmenu, IDM_COPY , g_seq>0 ? MF_ENABLED : MF_GRAYED);
		EnableMenuItem(s_popmenu, IDM_CLEAR, g_seq>0 ? MF_ENABLED : MF_GRAYED);
		
		TrackPopupMenu(s_popmenu, TPM_RIGHTBUTTON, point.x, point.y, 0, hwnd, NULL) ;
		return 0 ;

	case WM_INPUTLANGCHANGE:
		dwCharSet = wParam ;
		// fall through
	
	case WM_CREATE:
		if(!s_popmenu)
		{
			s_popmenu = LoadMenu(NULL, MAKEINTRESOURCE(IDR_MENU1));
			s_popmenu = GetSubMenu(s_popmenu, 0) ; 
		}
		// fall through to WM_DISPLAYCHANGE

	case WM_DISPLAYCHANGE:

		// Get maximum size of client area

		cxClientMax = GetSystemMetrics (SM_CXMAXIMIZED) ;
		cyClientMax = GetSystemMetrics (SM_CYMAXIMIZED) ;

		// Get character size for fixed-pitch font

		hdc = GetDC (hwnd) ;

		SelectObject (hdc, CreateFont (0, 0, 0, 0, 0, 0, 0, 0,
			dwCharSet, 0, 0, 0, FIXED_PITCH, NULL)) ; 

		GetTextMetrics (hdc, &tm) ;
		cxChar = tm.tmAveCharWidth ;
		cyChar = tm.tmHeight ;

		GetTextFace(hdc, COUNT(szFontface), szFontface);

		GetKeyboardLayoutName(szKbLayoutName);
		StringCchPrintf(szTitle, COUNT(szTitle), 
			TEXT("%s - Charset=%d(%s) Fontface=\"%s\" Keyboard[name=%s,HKL=%08X]"),
			szTitlePrefix, 
			tm.tmCharSet, GetCharsetName(tm.tmCharSet),
			szFontface,
			szKbLayoutName, GetKeyboardLayout(0)
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

		// Scroll up the display
		ScrollWindow (hwnd, 0, -cyChar, &rectScroll, &rectScroll) ;

		break ;        // ie, call DefWindowProc so Sys messages work

	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps) ;

		SelectObject (hdc, CreateFont (0, 0, 0, 0, 0, 0, 0, 0,
			dwCharSet, 0, 0, 0, FIXED_PITCH, NULL)) ; 

		SetBkMode (hdc, TRANSPARENT) ;
		TextOut (hdc, 0,            0, szTop_s1, lstrlen (szTop_s1));
		TextOut (hdc, drawwidth_s1, 0, szTop_s2, lstrlen (szTop_s2));
		TextOut (hdc, 0, 0, szUnd, lstrlen (szUnd)) ;

		for(i=0; i<min(g_LinesToDraw, cyClient/cyChar-1); i++) // -1 to exclude the head line
		{
			GetKeyDes(g_armsg[i], keydes_s1, COUNT(keydes_s1), keydes_s2, COUNT(keydes_s2));
			
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
		pos = lParam;
		break;

	case WM_DESTROY:

		if (g_armsg)
			free (g_armsg) ;

		PostQuitMessage (0) ;
		return 0 ;
	}
	return DefWindowProc (hwnd, message, wParam, lParam) ;
}

