/******************************************************************************
Module:  UILayout.h
Notices: Copyright (c) 2000 Jeffrey Richter
Purpose: This class manages child window positioning and sizing when a parent 
         window is resized.
         See Appendix B.
[2017-06-18] Updated by Chj, renamed to JULayout.
******************************************************************************/

#ifndef __JULayout_h_
#define __JULayout_h_


///////////////////////////////////////////////////////////////////////////////


//#include "..\CmnHdr.h"                 // See Appendix A.


///////////////////////////////////////////////////////////////////////////////

#define JULAYOUT_MAX_CONTROLS 200 

class JULayout 
{
public:
	void Initialize(HWND hwndParent, int nMinWidth = 0, int nMinHeight = 0);
   
	// Anco: Anchor coefficient, this value should be a percent value 0~100, 
	// 0 means left-most or top-most, 100 means right-most or bottom most.
	bool AnchorControl(int x1Anco, int y1Anco, int x2Anco, int y2Anco, int nCtrlID, bool fRedraw=false);
	bool AnchorControls(int x1Anco, int y1Anco, int x2Anco, int y2Anco, bool fRedraw, ...);

	BOOL AdjustControls(int cx, int cy);
	void HandleMinMax(PMINMAXINFO pMinMax) 
	{ 
		pMinMax->ptMinTrackSize = m_ptMinParentDims; 
	}

private:
	struct Ancofs_st {
		int Anco; // Anchor coefficient, this value should be a percent value 0~100, 
		int Offset; // pixel offset added to Anco-baseline
	};

	struct CtrlInfo_st {
		int         m_nID; 
		BOOL        m_fRedraw;
		Ancofs_st pt1x, pt1y; // pt1 means the north-west corner of the control
		Ancofs_st pt2x, pt2y; // pt2 means the south-east corner of the control
	}; 

   static void PixelFromAnchorPoint(int cxParent, int cyParent, int xAnco, int yAnco, PPOINT ppt)
   {
	   ppt->x = cxParent*xAnco/100;
	   ppt->y = cyParent*yAnco/100;
   }

private:    
   CtrlInfo_st m_CtrlInfo[JULAYOUT_MAX_CONTROLS]; // Max controls allowed in a dialog template
   int     m_nNumControls;
   HWND    m_hwndParent;
   POINT   m_ptMinParentDims; 
}; 


///////////////////////////////////////////////////////////////////////////////


#ifdef JULAYOUT_IMPL

void JULayout::Initialize(HWND hwndParent, int nMinWidth, int nMinHeight) 
{
	m_hwndParent = hwndParent;
	m_nNumControls = 0;

	if ((nMinWidth == 0) || (nMinHeight == 0)) {
		RECT rc;
		GetWindowRect(m_hwndParent, &rc);
		m_ptMinParentDims.x = rc.right  - rc.left; 
		m_ptMinParentDims.y = rc.bottom - rc.top; 
	}
	if (nMinWidth  != 0) 
		m_ptMinParentDims.x = nMinWidth;
	if (nMinHeight != 0) 
		m_ptMinParentDims.y = nMinHeight; 
}


///////////////////////////////////////////////////////////////////////////////


bool JULayout::AnchorControl(int x1Anco, int y1Anco, int x2Anco, int y2Anco, int nID, bool fRedraw) 
{
	if(m_nNumControls>=JULAYOUT_MAX_CONTROLS)
		return false;

	HWND hwndControl = GetDlgItem(m_hwndParent, nID);
	if(hwndControl == NULL) 
		return false;
	
	CtrlInfo_st &cinfo = m_CtrlInfo[m_nNumControls];

	cinfo.m_nID = nID;
	cinfo.m_fRedraw = fRedraw;

	cinfo.pt1x.Anco = x1Anco; cinfo.pt1y.Anco = y1Anco;
	cinfo.pt2x.Anco = x2Anco; cinfo.pt2y.Anco = y2Anco; 

	RECT rcControl;
	GetWindowRect(hwndControl, &rcControl);  // Screen coords of control
	// Convert coords to parent-relative coordinates
	MapWindowPoints(HWND_DESKTOP, m_hwndParent, (PPOINT) &rcControl, 2);

	RECT rcParent; 
	GetClientRect(m_hwndParent, &rcParent);

	POINT pt; 
	PixelFromAnchorPoint(rcParent.right, rcParent.bottom, x1Anco, y1Anco, &pt);
	cinfo.pt1x.Offset = rcControl.left - pt.x; 
	cinfo.pt1y.Offset = rcControl.top - pt.y;

	PixelFromAnchorPoint(rcParent.right, rcParent.bottom, x2Anco, y2Anco, &pt);
	cinfo.pt2x.Offset = rcControl.right - pt.x;
	cinfo.pt2y.Offset = rcControl.bottom - pt.y;

	m_nNumControls++;
	return true;
}

bool JULayout::AnchorControls(int x1Anco, int y1Anco, int x2Anco, int y2Anco, bool fRedraw, ...) 
{
	bool fOk = true;

	va_list arglist;
	va_start(arglist, fRedraw);
	int nID = va_arg(arglist, int);
	while (fOk && (nID != -1)) 
	{
		fOk = fOk && AnchorControl(x1Anco, y1Anco, x2Anco, y2Anco, nID, fRedraw);
		nID = va_arg(arglist, int);
	}           
	va_end(arglist);
	return(fOk);
}

BOOL JULayout::AdjustControls(int cx, int cy) 
{
	bool fOk = false;

	// Create region consisting of all areas occupied by controls
	HRGN hrgnPaint = CreateRectRgn(0, 0, 0, 0);
	int i;
	for(i=0; i<m_nNumControls; i++) 
	{
		HWND hwndControl = GetDlgItem(m_hwndParent, m_CtrlInfo[i].m_nID);
		RECT rcControl; 
		GetWindowRect(hwndControl, &rcControl);  // Screen coords of control
		// Convert coords to parent-relative coordinates
		MapWindowPoints(HWND_DESKTOP, m_hwndParent, (PPOINT) &rcControl, 2);

		HRGN hrgnTemp = CreateRectRgnIndirect(&rcControl);
		CombineRgn(hrgnPaint, hrgnPaint, hrgnTemp, RGN_OR);
		DeleteObject(hrgnTemp);
	}

	for(i=0; i<m_nNumControls; i++) 
	{
		CtrlInfo_st &cinfo = m_CtrlInfo[i];

		// Get control's upper/left position w/respect to parent's width/height
		RECT rcControl; 
		PixelFromAnchorPoint(cx, cy, cinfo.pt1x.Anco, cinfo.pt1y.Anco, (PPOINT)&rcControl);
		rcControl.left += cinfo.pt1x.Offset; 
		rcControl.top  += cinfo.pt1y.Offset; 

		// Get control's lower/right position w/respect to parent's width/height
		PixelFromAnchorPoint(cx, cy, cinfo.pt2x.Anco, cinfo.pt2y.Anco, (PPOINT)&rcControl.right);
		rcControl.right  += cinfo.pt2x.Offset;
		rcControl.bottom += cinfo.pt2y.Offset;

		// Position/size the control
		HWND hwndControl = GetDlgItem(m_hwndParent, cinfo.m_nID);
		MoveWindow(hwndControl, rcControl.left, rcControl.top, 
			rcControl.right - rcControl.left, 
			rcControl.bottom - rcControl.top, FALSE);
		
		if (m_CtrlInfo[i].m_fRedraw) {
			InvalidateRect(hwndControl, NULL, FALSE);
		} else {
			// Remove the regions occupied by the control's new position
			HRGN hrgnTemp = CreateRectRgnIndirect(&rcControl);
			CombineRgn(hrgnPaint, hrgnPaint, hrgnTemp, RGN_DIFF);
			DeleteObject(hrgnTemp);
			// Make the control repaint itself
			InvalidateRect(hwndControl, NULL, TRUE);
			SendMessage(hwndControl, WM_NCPAINT, 1, 0);
			UpdateWindow(hwndControl);
		}
	}

	// Paint the newly exposed portion of the dialog box's client area
	HDC hdc = GetDC(m_hwndParent);
	HBRUSH hbrColor = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
	FillRgn(hdc, hrgnPaint, hbrColor);
	DeleteObject(hbrColor);
	ReleaseDC(m_hwndParent, hdc);
	DeleteObject(hrgnPaint);
	return(fOk);
}

#endif   // JULAYOUT_IMPL

///////////////////////////////////////////////////////////////////////////////


#endif // __JULayout_h_
