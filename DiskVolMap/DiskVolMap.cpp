/* 
	This program finds mapping between Windows drive-letter and \Device\HarddiskVolume1,
	\Device\HarddiskVolume2, \Device\HarddiskVolume3 etc.

	-- Jimm Chen 2021.11.12, 
	   Thanks to https://superuser.com/a/1401025/74107

Sample output:

(#1) \\?\Volume{77952243-35b0-11ea-adc6-806e6f6e6963}\   -   \Device\HarddiskVolume1
(#2) \\?\Volume{59d5a7e7-3533-11ea-aece-080027638633}\  D:\  \Device\HarddiskVolume3
(#3) \\?\Volume{77952244-35b0-11ea-adc6-806e6f6e6963}\  C:\  \Device\HarddiskVolume2
(#4) \\?\Volume{77952247-35b0-11ea-adc6-806e6f6e6963}\  X:\  \Device\CdRom0

C: \Device\HarddiskVolume2
D: \Device\HarddiskVolume3
H: \Device\VBoxMiniRdr\;H:\VBoxSvr\My-H
I: \Device\LanmanRedirector\;I:00000000000215d7\10.22.3.84\i
S: \Device\LanmanRedirector\;S:00000000000215d7\10.22.3.84\devshare
U: \Device\LanmanRedirector\;U:00000000000215d7\10.22.3.190\d$
V: \Device\LanmanRedirector\;V:00000000000215d7\10.22.3.153\d$
X: \Device\CdRom0

*/

#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <stdio.h>
#include <windows.h>

void PrintVolMap()
{
	TCHAR volname[MAX_PATH] = {}, mountpoint[MAX_PATH] = {}, devpath[MAX_PATH] = {};

	HANDLE hfind = FindFirstVolume(volname, ARRAYSIZE(volname));
	BOOL succ = (hfind==INVALID_HANDLE_VALUE) ? false : true;
	// -- typical value like this:
	//    \\?\Volume{77952243-35b0-11ea-adc6-806e6f6e6963}\

	int i = 0;
	while(succ)
	{
		_tprintf(_T("(#%d) %s  "), ++i, volname);

		DWORD outchars = 0;
		succ = GetVolumePathNamesForVolumeName(volname, mountpoint, ARRAYSIZE(mountpoint), &outchars);
		if(succ) {
			if(mountpoint[0])
				_tprintf(_T("%s  "), mountpoint); // usually, C:\ 
			else
				_tprintf(_T(" -   "), mountpoint);
		}
		else
			_tprintf(_T("<get-mountpoint-error>  "), mountpoint);

		int volname_len = (int)lstrlen(volname);
		if(volname[volname_len-1]==_T('\\'))
			volname[volname_len-1] = _T('\0');

		outchars = QueryDosDevice(volname+4, devpath, ARRAYSIZE(devpath));
		// -- typical value like this:
		//    \Device\HarddiskVolume1

		if( outchars>0 && outchars<ARRAYSIZE(devpath) )
			_tprintf(_T("%s\n"),devpath);
		else
			_tprintf(_T("<no-devpath>\n"),devpath);

		///

		succ = FindNextVolume(hfind, volname, ARRAYSIZE(volname));
	};

	if(hfind!=INVALID_HANDLE_VALUE)
		FindVolumeClose(hfind);

	return;
}

void PrintByDriveLetters()
{
	// Some entries may not be listed by PrintVolMap(), so we need this.
	// Reason, Dos-device symlinks from-and-to a drive-letter can be 
	// created/deleted individually.

	TCHAR ltr[3] = _T("A:");
	TCHAR ltrmap[26][MAX_PATH];
	int idx = 0;
	for(; idx<26; idx++)
	{
		ltr[0] = 'A' + idx;
		ltrmap[idx][0] = _T('\0');
		DWORD outchars = QueryDosDevice(ltr, ltrmap[idx], ARRAYSIZE(ltrmap[idx]));

		if( outchars>0 && outchars<ARRAYSIZE(ltrmap[idx]))
			_tprintf(_T("%s %s\n"), ltr, ltrmap[idx]);
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	PrintVolMap();

	_tprintf(_T("\n"));

	PrintByDriveLetters();
}
