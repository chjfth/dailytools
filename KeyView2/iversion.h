#ifndef _keyview_iversion_h
#define _keyview_iversion_h

#define THISLIB_VMAJOR 1
#define THISLIB_VMINOR 8
#define THISLIB_VBUILD 0

#define THISLIBstr__(n) #n
#define THISLIBstr(n) THISLIBstr__(n)

// The following 3 are used in .rc
#define THISLIB_VMAJORs THISLIBstr(THISLIB_VMAJOR)
#define THISLIB_VMINORs THISLIBstr(THISLIB_VMINOR)
#define THISLIB_VBUILDs THISLIBstr(THISLIB_VBUILD)

#define THISLIB_NAME "KeyView"

#endif
