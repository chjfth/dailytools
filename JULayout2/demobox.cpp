#define WIN32_LEAN_AND_MEAN
#include <WindowsX.h>
#include <tchar.h>
#include <stdio.h>
#include "resource.h"
#include "CmnHdr-Jeffrey.h" // chHANDLE_DLGMSG()

#include "demobox.h"

// This file contains demobox dialogbox *legacy* implementation,
// with only an extra call to DemoBox_EnableJULayout().
//
// DemoBox_EnableJULayout() is a function provided by outside JULayout-enabler.
// The enabler code knows control-ids and positions of the original demobox,
// so that he(enabler) can adjust JUL params from within DemoBox_EnableJULayout().
//
// Of course, different legacy dialog-box Foo, will pair with different
// Foo_EnableJULayout() enabler code.

///////////////////////////////////////////////////////////////////////////////

struct DlgPrivate_st
{
	// Anything you like here.
};


BOOL DemoBox_OnInitDialog(HWND hdlg, HWND hwndFocus, LPARAM lParam) 
{
	DlgPrivate_st *prdata = new DlgPrivate_st;
	SetWindowLongPtr(hdlg, DWLP_USER, (LONG_PTR)prdata);
	// -- This shows that JULayout does not occupy your DWLP_USER space.

	// Before returning from WM_INITDIALOG, add code to enable JULayout for this dlgbox.
	//
	DemoBox_EnableJULayout(hdlg, (void*)lParam);

	return FALSE; // no default focus (but why still see focus on Button A?)
}

void DemoBox_OnCommand(HWND hdlg, int id, HWND hwndCtl, UINT codeNotify) 
{
	DlgPrivate_st *prdata = (DlgPrivate_st*)GetWindowLongPtr(hdlg, DWLP_USER);
	(void)prdata;

	switch (id) 
	{{
	case IDOK:
	case IDCANCEL:
		EndDialog(hdlg, id);
		break;

	case IDC_BTNA:
		SetDlgItemText(hdlg, IDC_EDIT1, L"You clicked Button A.");
		break;
	case IDC_BTNB:
		SetDlgItemText(hdlg, IDC_EDIT1, L"You clicked Button B.");
		break;
	}}
}

INT_PTR WINAPI DemoBox_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
 	switch (uMsg) 
 	{
 		chHANDLE_DLGMSG(hdlg, WM_INITDIALOG,    DemoBox_OnInitDialog);
 		chHANDLE_DLGMSG(hdlg, WM_COMMAND,       DemoBox_OnCommand);
 	}
	return(FALSE);
}

