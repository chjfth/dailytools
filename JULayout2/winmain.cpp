#include <WindowsX.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"

#include "CmnHdr-Jeffrey.h"

#define JULAYOUT_IMPL
#include "JULayout2.h"

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HINSTANCE g_hinstExe;

///////////////////////////////////////////////////////////////////////////////

struct DlgPrivate_st
{
	// Anything you like here.
};

struct CtlParam_st // for one control
{
	int x1anco, y1anco, x2anco, y2anco;
	int ctlID; // if 0, mark array end
	const WCHAR *text;
};

struct DemoParam_st
{
	int nctlparam;      // number of control-params in ar_ctlparams
	const CtlParam_st *ar_cltparams;
	const TCHAR *title; // window/dlgbox title
};

void MyDlg_EnableJULayout(HWND hdlg, const DemoParam_st *dp)
{
	SetWindowText(hdlg, dp->title);

	JULayout *jul = JULayout::EnableJULayout(hdlg);

	for(int i=0; i<dp->nctlparam; i++)
	{
		const CtlParam_st &cp = dp->ar_cltparams[i];
		jul->AnchorControl(cp.x1anco, cp.y1anco, cp.x2anco, cp.y2anco, cp.ctlID);
		SetDlgItemText(hdlg, cp.ctlID, cp.text);
	}
}

BOOL DlgJUL_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	DlgPrivate_st *prdata = new DlgPrivate_st;
	SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)prdata);
	// -- This shows that JULayout does not occupy your DWLP_USER space.

	// Before returning from WM_INITDIALOG, add code to enable JULayout for this dlgbox.
	//
	DemoParam_st *dp = (DemoParam_st*)lParam;
	MyDlg_EnableJULayout(hwnd, dp);

	return FALSE; // no default focus (but why still see focus on Button A?)
}

void DlgJUL_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) 
{
	DlgPrivate_st *prdata = (DlgPrivate_st*)GetWindowLongPtr(hwnd, DWLP_USER);
	(void)prdata;

	switch (id) 
	{{
	case IDOK:
	case IDCANCEL:
		EndDialog(hwnd, id);
		break;

	case IDC_BTNA:
		SetDlgItemText(hwnd, IDC_EDIT1, L"You clicked Button A.");
		break;
	case IDC_BTNB:
		SetDlgItemText(hwnd, IDC_EDIT1, L"You clicked Button B.");
		break;
	}}
}

INT_PTR WINAPI DlgJUL_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
 	switch (uMsg) 
 	{
 		chHANDLE_DLGMSG(hwnd, WM_INITDIALOG,    DlgJUL_OnInitDialog);
 		chHANDLE_DLGMSG(hwnd, WM_COMMAND,       DlgJUL_OnCommand);
 	}
	return(FALSE);
}

///////

void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) 
{
	DlgPrivate_st *prdata = (DlgPrivate_st*)GetWindowLongPtr(hwnd, DWLP_USER);

	HWND hwndOwner = NULL;
	
	TCHAR title[80] = {};
	_sntprintf_s(title, _TRUNCATE, _T("JULayout Demo #%d"), id-IDC_DEMO1+1);


	switch (id) 
	{{
	case IDOK:
	case IDCANCEL:
		delete prdata;
		EndDialog(hwnd, id);
		break;
	
	case IDC_DEMO1:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 100, 100, IDC_EDIT1},
				{100, 0, 100, 0, IDC_BTNA, L"Button A ↑"},
				{100, 100, 100, 100, IDC_BTNB, L"Button B ↓"},
			};
			DemoParam_st dp = { ARRAYSIZE(ctlparams), ctlparams, title };
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_JULBOX), hwndOwner, DlgJUL_Proc, (LPARAM)&dp);
			break;
		}

	case IDC_DEMO2:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 100, 100, IDC_EDIT1},
				{100, 100, 100, 100, IDC_BTNA, L"Button A ↓"},
				{100, 100, 100, 100, IDC_BTNB, L"Button B ↓"},
			};
			DemoParam_st dp = { ARRAYSIZE(ctlparams), ctlparams, title };
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_JULBOX), hwndOwner, DlgJUL_Proc, (LPARAM)&dp);
			break;
		}
		
	case IDC_DEMO3:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 100, 100, IDC_EDIT1},
				{100, 50, 100, 50, IDC_BTNA, L"Button A ↓"},
				{100, 50, 100, 50, IDC_BTNB, L"Button B ↑"},
			};
			DemoParam_st dp = { ARRAYSIZE(ctlparams), ctlparams, title };
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_JULBOX), hwndOwner, DlgJUL_Proc, (LPARAM)&dp);
			break;
		}
		
	case IDC_DEMO4:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 70, 100, IDC_EDIT1},
				{70, 50, 100, 50, IDC_BTNA, L"↔30%"},
				{70, 50, 100, 50, IDC_BTNB, L"↔30%"},
			};
			DemoParam_st dp = { ARRAYSIZE(ctlparams), ctlparams, title };
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_JULBOX), hwndOwner, DlgJUL_Proc, (LPARAM)&dp);
			break;
		}

	case IDC_DEMO5:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 70, 100, IDC_EDIT1},
				{70, 0,  100, 50,  IDC_BTNA, L"↔30%  ↕50%"},
				{70, 50, 100, 100, IDC_BTNB, L"↔30%  ↕50%"},
			};
			DemoParam_st dp = { ARRAYSIZE(ctlparams), ctlparams, title };
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_JULBOX), hwndOwner, DlgJUL_Proc, (LPARAM)&dp);
			break;
		}
	}}
}

BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	chSETDLGICONS(hwnd, IDI_WINMAIN);
	return(TRUE);
}

INT_PTR WINAPI Dlg_Proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	switch (uMsg) 
	{
		chHANDLE_DLGMSG(hwnd, WM_INITDIALOG,    Dlg_OnInitDialog);
		chHANDLE_DLGMSG(hwnd, WM_COMMAND,       Dlg_OnCommand);
	}
	return(FALSE);
}


int WINAPI _tWinMain(HINSTANCE hinstExe, HINSTANCE, PTSTR pszCmdLine, int) 
{
	g_hinstExe = hinstExe;

	const WCHAR *mystr = L"My private string";
	DialogBoxParam(hinstExe, MAKEINTRESOURCE(IDD_WINMAIN), NULL, Dlg_Proc, (LPARAM)mystr);
	return(0);
}

