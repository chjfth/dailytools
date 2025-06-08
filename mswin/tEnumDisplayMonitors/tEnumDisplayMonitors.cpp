#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

#include "iversion.h"

#include "../utils.h"

#include <mswin/win32cozy.h> // for RECTtext
#include <mswin/win32clarify.h>

#define JULAYOUT_IMPL
#include <mswin/JULayout2.h>

#define JAUTOBUF_IMPL
#include <JAutoBuf.h>

#define WinMultiMon_IMPL
#include <mswin/WinMultiMon.h>

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HINSTANCE g_hinstExe;

struct DlgPrivate_st
{
	const TCHAR *mystr;
	int clicks;
};

JAutoBuf<OneMonitorInfo_st>
ui_RefreshMonitorInfo(HWND hdlg)
{
	vaSetDlgItemText(hdlg, IDC_EDIT_LOGMSG, _T(""));

	JAutoBuf<OneMonitorInfo_st> abMonsinfo;
	WinErr_t winerr = 0;
	do {
		winerr = doEnumDisplayMonitors(abMonsinfo, abMonsinfo, abMonsinfo);
	} while(winerr==ERROR_MORE_DATA);

	HWND hedit = GetDlgItem(hdlg, IDC_EDIT_LOGMSG);

	int i;
	int nMons = (int)abMonsinfo.Size();
	for(i=0; i<nMons; i++)
	{
		TCHAR rctMonitor[80], rctWorkArea[80];

		OneMonitorInfo_st &mi = abMonsinfo[i];
		vaAppendText_mled(hedit, 
			_T("[#%d] Monitor %s:\r\n")
			_T("  szDevice: %s\r\n")
			_T("  rcMonitor: %s\r\n")
			_T("  rcWorkArea: %s\r\n")
			_T("\r\n")
			,
			i+1, (mi.isPrimary ? _T("(Primary)") : _T("")),
			mi.szDevice,
			RECTtext(mi.rcMonitor, rctMonitor),
			RECTtext(mi.rcWorkArea, rctWorkArea)
			);
	}

	vaSetDlgItemText(hdlg, IDS_CountMonitors, _T("Total %d monitors"), nMons);

	return abMonsinfo;
}

#define BAD_POINT (-32001)

struct Enum_st
{
	int ptx, pty;
	RECT rcMon;
};

static BOOL CALLBACK OneMonitorEnumProc(
	HMONITOR hMonitor,  // handle to display monitor
	HDC hdcMonitor,     // handle to monitor DC
	LPRECT lprcMonitor, // monitor intersection rectangle
	LPARAM dwData       // user context
	)
{
	MONITORINFO minfo = {sizeof(MONITORINFO)};
	BOOL b = GetMonitorInfo(hMonitor, &minfo);
	if(!b)
		return FALSE;

	Enum_st &est = *(Enum_st*)dwData;

	est.rcMon = minfo.rcMonitor;
	
	TCHAR rctext[80];
	vaDbgTs(_T("For (%d,%d) OneMonitorEnumProc returns %s"), 
		est.ptx, est.pty, RECTtext(est.rcMon, rctext));
	
	return TRUE;
}



void ui_CheckPoint(HWND hdlg)
{
	BOOL bTrans = 0;
	int ptx = GetDlgItemInt(hdlg, IDE_PtX, &bTrans, bSigned_TRUE);
	int pty = GetDlgItemInt(hdlg, IDE_PtY, &bTrans, bSigned_TRUE);

	Enum_st est = { ptx, pty, BAD_POINT, BAD_POINT };

	RECT rcInput = { ptx, pty, ptx+1, pty+1 };
	BOOL succ = EnumDisplayMonitors(NULL, &rcInput, OneMonitorEnumProc, (LPARAM)&est);
	assert(succ);

	if(est.rcMon.left==BAD_POINT && est.rcMon.top==BAD_POINT)
	{
		SetDlgItemText(hdlg, IDE_CheckPointOutput, _T("Point not in any monitor"));
		return;
	}

	JAutoBuf<OneMonitorInfo_st> abMonsinfo = ui_RefreshMonitorInfo(hdlg);
	int nMons = (int)abMonsinfo.Size();
	for(int i=0; i<nMons; i++)
	{
		if( EqualRect(&abMonsinfo[i].rcMonitor, &est.rcMon) )
		{
			vaSetDlgItemText(hdlg, IDE_CheckPointOutput,
				_T("In [#%d] offset: %d,%d"), 
				i+1, est.ptx-est.rcMon.left, est.pty-est.rcMon.top);
			return;
		}
	}

	SetDlgItemText(hdlg, IDE_CheckPointOutput, _T("Uexpect! Point match fail!"));
}

void Dlg_OnCommand(HWND hdlg, int id, HWND hwndCtl, UINT codeNotify) 
{
	DlgPrivate_st &prdata = *(DlgPrivate_st*)GetWindowLongPtr(hdlg, DWLP_USER);

	switch (id) 
	{{
	case IDB_Refresh:
	{
		ui_RefreshMonitorInfo(hdlg);
		break;
	}
	case IDB_CheckPoint:
	{
		ui_CheckPoint(hdlg);
		break;
	}
	case IDOK:
	case IDCANCEL:
	{
		EndDialog(hdlg, id);
		break;
	}
	}}
}

static void Dlg_EnableJULayout(HWND hdlg)
{
	JULayout *jul = JULayout::EnableJULayout(hdlg);

	jul->AnchorControl(0,0, 100,100, IDC_EDIT_LOGMSG);

	jul->AnchorControls(0,100, 0,100, IDGB_CheckPoint,
		IDS_PtX, IDE_PtX, IDS_PtY, IDE_PtY, IDB_CheckPoint, IDE_CheckPointOutput,
		-1);

	// If you add more controls(IDC_xxx) to the dialog, adjust them here.
}

BOOL Dlg_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam) 
{
	SNDMSG(hdlg, WM_SETICON, TRUE, (LPARAM)LoadIcon(GetWindowInstance(hdlg), MAKEINTRESOURCE(IDI_WINMAIN)));

	DlgPrivate_st &prdata = *(DlgPrivate_st*)lParam;
	SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)&prdata);
	
	vaSetWindowText(hdlg, _T("tEnumDisplayMonitors v%d.%d.%d"), 
		tEnumDisplayMonitors_VMAJOR, tEnumDisplayMonitors_VMINOR, tEnumDisplayMonitors_VPATCH);
	
//	SetDlgItemText(hdlg, IDC_EDIT_LOGMSG, prdata.mystr);

	Dlg_EnableJULayout(hdlg);

	SetDlgItemText(hdlg, IDE_PtX, _T("0"));
	SetDlgItemText(hdlg, IDE_PtY, _T("0"));

	ui_RefreshMonitorInfo(hdlg);

	SetFocus(GetDlgItem(hdlg, IDB_Refresh));
	return FALSE; // FALSE to let Dlg-manager respect our SetFocus().
}

INT_PTR WINAPI UserDlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg) 
	{
		HANDLE_dlgMSG(hdlg, WM_INITDIALOG,    Dlg_OnInitDialog);
		HANDLE_dlgMSG(hdlg, WM_COMMAND,       Dlg_OnCommand);
	}
	return FALSE;
}


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR szParams, int) 
{
	g_hinstExe = hinstExe;

	InitCommonControls(); // WinXP requires this, to work with Visual-style manifest

	const TCHAR *szfullcmdline = GetCommandLine();
	vaDbgTs(_T("GetCommandLine() = %s"), szfullcmdline);

	DlgPrivate_st dlgdata = { _T("Hello.\r\nPrivate string here.") };
	DialogBoxParam(hinstExe, MAKEINTRESOURCE(IDD_WINMAIN), NULL, UserDlgProc, (LPARAM)&dlgdata);

	return 0;
}
