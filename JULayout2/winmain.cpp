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

BOOL DlgJUL_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	DlgPrivate_st *prdata = new DlgPrivate_st;
	SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)prdata);

	JULayout *jul = JULayout::EnableJULayout(hwnd);

	CtlParam_st *cp = (CtlParam_st*)lParam;
	for(; cp->ctlID; cp++)
	{
		jul->AnchorControl(cp->x1anco, cp->y1anco, cp->x2anco, cp->y2anco, cp->ctlID);
		SetDlgItemText(hwnd, cp->ctlID, cp->text);
	}

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
				{0,0,0,0,0}
			};
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_DEMO1), hwndOwner, DlgJUL_Proc, (LPARAM)ctlparams);
			break;
		}

	case IDC_DEMO2:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 100, 100, IDC_EDIT1},
				{100, 100, 100, 100, IDC_BTNA, L"Button A ↓"},
				{100, 100, 100, 100, IDC_BTNB, L"Button B ↓"},
				{0,0,0,0,0}
			};
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_DEMO1), hwndOwner, DlgJUL_Proc, (LPARAM)ctlparams);
			break;
		}
		
	case IDC_DEMO3:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 100, 100, IDC_EDIT1},
				{100, 50, 100, 50, IDC_BTNA, L"Button A ↓"},
				{100, 50, 100, 50, IDC_BTNB, L"Button B ↑"},
				{0,0,0,0,0}
			};
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_DEMO1), hwndOwner, DlgJUL_Proc, (LPARAM)ctlparams);
			break;
		}
		
	case IDC_DEMO4:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 70, 100, IDC_EDIT1},
				{70, 50, 100, 50, IDC_BTNA, L"↔30%"},
				{70, 50, 100, 50, IDC_BTNB, L"↔30%"},
				{0,0,0,0,0}
			};
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_DEMO1), hwndOwner, DlgJUL_Proc, (LPARAM)ctlparams);
			break;
		}

	case IDC_DEMO5:
		{
			CtlParam_st ctlparams[] =
			{
				{0, 0, 70, 100, IDC_EDIT1},
				{70, 0,  100, 50,  IDC_BTNA, L"↔30%  ↕50%"},
				{70, 50, 100, 100, IDC_BTNB, L"↔30%  ↕50%"},
				{0,0,0,0,0}
			};
			DialogBoxParam(g_hinstExe, MAKEINTRESOURCE(IDD_DEMO1), hwndOwner, DlgJUL_Proc, (LPARAM)ctlparams);
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

