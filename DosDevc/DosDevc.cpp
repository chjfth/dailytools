#include <stdio.h>
#include <tchar.h>

#include <windows.h>

#pragma warning(disable:4996) // disable warning on _snprintf()

// Get the element quantity of an array
#define GetEleQuan(array) ( sizeof(array)/sizeof(array[0]) )
// Note that GetEleQuan() "returns" a unsigned int value !
#define GetEleQuan_i(array) ((int)GetEleQuan(array))

struct Winerr2Name_st
{
	DWORD err;
	const char *errA;
	const WCHAR *errW;
};

static Winerr2Name_st sar_WinerrMap[] =
{
#define WINERR(n) {n, #n, L ## #n }
	WINERR(ERROR_INVALID_FUNCTION),
	WINERR(ERROR_FILE_NOT_FOUND),
	WINERR(ERROR_PATH_NOT_FOUND),
	WINERR(ERROR_ACCESS_DENIED),
	WINERR(ERROR_INVALID_HANDLE), // when handle is force-closed by Process Explorer
	WINERR(ERROR_OPERATION_ABORTED),
	WINERR(ERROR_NOT_ENOUGH_MEMORY),
	WINERR(ERROR_OUTOFMEMORY),
	WINERR(ERROR_BAD_COMMAND),
	WINERR(ERROR_INVALID_PARAMETER),
	WINERR(ERROR_DEVICE_NOT_CONNECTED),
	WINERR(ERROR_DEV_NOT_EXIST),
	WINERR(ERROR_NOACCESS), // Invalid access to memory location
	WINERR(ERROR_SUCCESS),
	WINERR(ERROR_INVALID_NAME), // 'dosdevc \com1 chjc1' may get this, but may not
};

const char *Winerr2NameA(DWORD n)
{
	for(int i=0; i<GetEleQuan_i(sar_WinerrMap); i++)
	{
		if(sar_WinerrMap[i].err==n)
			return sar_WinerrMap[i].errA;
	}
	return "UnknownWinErrCode";
}

const WCHAR *Winerr2NameW(DWORD n)
{
	for(int i=0; i<GetEleQuan_i(sar_WinerrMap); i++)
	{
		if(sar_WinerrMap[i].err==n)
			return sar_WinerrMap[i].errW;
	}
	return L"UnknownWinErrCode";
}

void PrnWinerr(BOOL succ, const WCHAR *fmt, ...)
{
	if(succ)
		return;
	
	DWORD winerr = GetLastError();

	va_list args;
	va_start(args, fmt);

	WCHAR usertext[1000] = {0};
	_vsntprintf(usertext, GetEleQuan_i(usertext)-1, fmt, args);

	wprintf(L"%s WinErr=%u(%s)", usertext, winerr, Winerr2NameW(winerr));

	va_end(args);
}

void PrintMultisz(const WCHAR *bigbuf)
{
	const WCHAR *pnow = bigbuf;
	for(int i=0; ;i++)
	{
		int len = wcslen(pnow);
		wprintf(L"[%d] %s\n", i+1, pnow);

		pnow += len+1;
		if(*pnow==0)
			break;
	}
}

bool myListDosDevice()
{
	wprintf(L"List all dos devices:\n");

	static WCHAR bigbuf[256000] = {0};

	DWORD ret = QueryDosDevice(NULL, bigbuf, GetEleQuan_i(bigbuf));
	wprintf(L"QueryDosDevice() returns %u.\n", ret);
	if(ret==0) // error occurs
	{
		PrnWinerr(false, L"QueryDosDevice() fail.");
		return false;
	}
	else
	{
		PrintMultisz(bigbuf);
		return true;
	}
}

bool myQueryDosDevice(const WCHAR *doslink)
{
	wprintf(L"QueryDosDevice: '%s'\n", doslink);

	WCHAR target[MAX_PATH]={0};
	DWORD retchars = QueryDosDevice(doslink, target, GetEleQuan_i(target));
	if(retchars==0)
	{
		PrnWinerr(false, L"QueryDosDevice() fail.");
		return false;
	}
	else
	{
		wprintf(L"%s\n", target);
		return true;
	}
}

bool myRemoveDosDevice(const WCHAR *doslink)
{
	wprintf(L"Remove-DosDevice: '%s'\n", doslink);
	BOOL b = DefineDosDevice(DDD_REMOVE_DEFINITION, doslink, NULL);
	PrnWinerr(b, L"DefineDosDevice() fail.");

	return b ? true: false;
}

bool myDefineDosDevice(const WCHAR *doslink, const WCHAR *target)
{
	wprintf(L"DefineDosDevice('%s', '%s');\n", doslink, target);
	BOOL b = DefineDosDevice(DDD_RAW_TARGET_PATH, 
		doslink, target);
	PrnWinerr(b, L"DefineDosDevice() fail.");
	return b ? true : false;
}

int _tmain(int argc, _TCHAR* argv[])
{
	BOOL b = FALSE;

	WCHAR *src = L"COM14";
	WCHAR *dst = L"\\Device\\honeywell_cdc_AcmSerial0";

	if(argc<2)
	{
		printf("DosDevc compiled on %s %s\n", __DATE__, __TIME__);
		wprintf(L"Need parameter. Examples:\n");
		wprintf(L"To list all dos devices:\n");
		wprintf(L"  DosDevc *\n", src);
		wprintf(L"To query a doslink:\n");
		wprintf(L"  DosDevc %s\n", src);
		wprintf(L"To remove a doslink:\n");
		wprintf(L"  DosDevc %s -\n", src);
		wprintf(L"To add a doslink:\n");
		wprintf(L"  DosDevc %s %s\n", src, dst);
		return 1;
	}
	else if(argc==2)
	{
		src = argv[1];

		if(wcscmp(src, L"*")==0)
		{
			myListDosDevice();
		}
		else 
		{
			myQueryDosDevice(src);
		}
	}
	else if(argc==3)
	{
		src = argv[1];
		dst = argv[2];

		if(wcscmp(dst, L"-")==0)
		{
			myRemoveDosDevice(src);
		}
		else
		{
			myDefineDosDevice(src, dst);
		}
	}


	return 0;
}

