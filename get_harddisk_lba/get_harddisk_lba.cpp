#include "stdafx.h"

// https://msdn.microsoft.com/en-gb/library/windows/desktop/aa363147%28v=vs.85%29.aspx


/* The code of interest is in the subroutine GetDriveGeometry. The 
   code in main shows how to interpret the results of the call. */

#define wszDriveFmt L"\\\\.\\PhysicalDrive%d"

BOOL GetDriveGeometry(int diskid, DISK_GEOMETRY *pdg, __int64 *pLBAs)
{
	// diskid: 0, 1, 2 ...

	HANDLE hDevice = INVALID_HANDLE_VALUE;  // handle to the drive to be examined 
	BOOL bResult   = FALSE;                 // results flag
	DWORD junk     = 0;                     // discard results

	WCHAR szDiskDev[64];
	wsprintfW(szDiskDev, wszDriveFmt, diskid);
	hDevice = CreateFileW(szDiskDev,      // drive to open
						0,                // no access to the drive
						FILE_SHARE_READ | // share mode
						FILE_SHARE_WRITE, 
						NULL,             // default security attributes
						OPEN_EXISTING,    // disposition
						0,                // file attributes
						NULL);            // do not copy file attributes

	if (hDevice == INVALID_HANDLE_VALUE)    // cannot open the drive
	{
		return (FALSE);
	}

	unsigned char dgex_buffer[1024];
	DISK_GEOMETRY_EX *pdgex = (DISK_GEOMETRY_EX*)dgex_buffer;

	bResult = DeviceIoControl(hDevice,                       // device to be queried
							IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, // operation to perform
							NULL, 0,                       // no input buffer
							pdgex, sizeof(dgex_buffer),    // output buffer
							&junk,                         // # bytes returned
							(LPOVERLAPPED) NULL);          // synchronous I/O

	CloseHandle(hDevice);

	if(!bResult)
		return FALSE;

	assert(pdgex->DiskSize.QuadPart%512==0);
	*pLBAs = pdgex->DiskSize.QuadPart / 512;

	*pdg = pdgex->Geometry;
	return (bResult);
}

double getfriendly(__int64 bytes, int radix_, WCHAR unit[1])
{
	__int64 radix = radix_;
	__int64 tbi = bytes/(radix*radix*radix*radix);
	if(tbi>=1) {
		unit[0] = 'T';
		return (double)bytes/(radix*radix*radix*radix);
	}

	__int64 gbi = bytes/(radix*radix*radix);
	if(gbi>=1) {
		unit[0] = 'G';
		return (double)bytes/(radix*radix*radix);
	}

	__int64 mbi = bytes/(radix*radix);
	if(mbi>=1) {
		unit[0] = 'M';
		return (double)bytes/(radix*radix);
	}

	__int64 kbi = bytes/(radix);
	unit[0] = 'K';
	return (double)bytes/(radix);
}

const WCHAR *FriendlyDiskSize(__int64 lba, WCHAR *buf, int bufchars)
{
	__int64 bytes = lba*512;
	
	WCHAR unit_i, unit_o;
	double XiB = getfriendly(bytes, 1024, &unit_i); // XiB implies TiB, GiB, MiB etc
	double XoB = getfriendly(bytes, 1000, &unit_o);
	unit_o -= 'A' - 'a'; // make it lower case

//	buf[bufchars-1] = L'\0';
//	_snwprintf(buf, bufchars-1, L"%.3g %ciB or %.3g %cB", XiB, unit_i, XoB, unit_o);
	//
	_snwprintf_s(buf, bufchars-1, _TRUNCATE, L"%.3g %ciB or %.3g %cB", XiB, unit_i, XoB, unit_o);
		// Result is like: "61 GiB or 65.5 GoB"
	return buf;
}

int wmain(int argc, wchar_t *argv[])
{
	DISK_GEOMETRY pdg = { 0 }; // disk drive geometry structure
	BOOL bResult = FALSE;      // generic results flag
	ULONGLONG DiskSize = 0;    // size of the drive, in bytes
	const int bufsize = 40;
	WCHAR szfriendly[bufsize];

	wprintf(L"Retrieving physical disk info...\n\n");

	int i;
	for(i=0; i<1000; i++)
	{
		__int64 LBAs = 0, LBAs_fake = 0;
		bResult = GetDriveGeometry(i, &pdg, &LBAs);

		if(!bResult) 
			break;
		
		__int64 c = pdg.Cylinders.QuadPart;
		int h = pdg.TracksPerCylinder;
		int s = pdg.SectorsPerTrack;
		LBAs_fake = c * h * s;
		DiskSize = LBAs*512;

		wprintf(L"Disk %d: LBAs=%I64d (%s), %I64d cylinders + %d sectors.\n",
			i,
			LBAs, FriendlyDiskSize(LBAs, szfriendly, bufsize),
			c, LBAs-LBAs_fake
			);

/*
		wprintf(L"Drive path      = %ws\n",   wszDrive);
		wprintf(L"Cylinders       = %I64d\n", pdg.Cylinders);
		wprintf(L"Tracks/cylinder = %ld\n",   (ULONG) pdg.TracksPerCylinder);
		wprintf(L"Sectors/track   = %ld\n",   (ULONG) pdg.SectorsPerTrack);
		wprintf(L"Bytes/sector    = %ld\n",   (ULONG) pdg.BytesPerSector);

		wprintf(L"Total Sectors   = %I64d\n", LBAs);
		
		wprintf(L"Disk size       = %I64d (Bytes)\n"
			L"                = %.2f (Gb)\n", 
			DiskSize, (double) DiskSize / (1024 * 1024 * 1024));
*/
	} 

	if(i==0)
	{
		wprintf(L"Unexpected: No disk info is available!\n");
		return 1;
	}
	else
	{
		wprintf(L"\nHint: 1GiB=1024*1024*1024 , 1gB=1000*1000*1000\n");
		return 0;
	}
}

