#ifndef __tEnumDisplayMonitors_iversion_h_
#define __tEnumDisplayMonitors_iversion_h_

#define tEnumDisplayMonitors_VMAJOR 1
#define tEnumDisplayMonitors_VMINOR 0
#define tEnumDisplayMonitors_VPATCH 0
#define tEnumDisplayMonitors_VTAIL  1

#define tEnumDisplayMonitorsstr__(n) #n
#define tEnumDisplayMonitorsstr(n) tEnumDisplayMonitorsstr__(n)

// The following 4 are used in .rc
#define tEnumDisplayMonitors_VMAJORs tEnumDisplayMonitorsstr(tEnumDisplayMonitors_VMAJOR)
#define tEnumDisplayMonitors_VMINORs tEnumDisplayMonitorsstr(tEnumDisplayMonitors_VMINOR)
#define tEnumDisplayMonitors_VPATCHs tEnumDisplayMonitorsstr(tEnumDisplayMonitors_VPATCH)
#define tEnumDisplayMonitors_VTAILs  tEnumDisplayMonitorsstr(tEnumDisplayMonitors_VTAIL)

#define tEnumDisplayMonitors_NAME "tEnumDisplayMonitors"

enum {
	tEnumDisplayMonitors_vmajor = tEnumDisplayMonitors_VMAJOR,
	tEnumDisplayMonitors_vminor = tEnumDisplayMonitors_VMINOR,
	tEnumDisplayMonitors_vpatch = tEnumDisplayMonitors_VPATCH,
	tEnumDisplayMonitors_vtail = tEnumDisplayMonitors_VTAIL,
};


#endif
