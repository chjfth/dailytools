#ifndef __demobox_h_
#define __demobox_h_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// This is legacy demobox function:
INT_PTR WINAPI DemoBox_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// This is the extra JULayout enabler function that demobox needs to call.
void DemoBox_EnableJULayout(HWND hdlg, void *lParam_WM_INITDIALOG);


#endif
