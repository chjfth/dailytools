/* 2021.10.18 Jimm Chen

purwin-args.cpp, show Windows API CommandLineToArgvW()'s behaivor.

Compile it with VC2010, not linking any MSVCRT libraries, resulting exe is 2560 bytes.

	cl purewin-args.cpp /UNICODE /GS- /link /nodefaultlib /subsystem:console kernel32.lib shell32.lib user32.lib

We'll see that CommandLineToArgvW's behavior is not the same as that of main()'s argv[].
What's more, it is NOT the same as cmd's command line parsing into %1, %2, %3 etc.
So, I think CommandLineToArgvW is quite useless in real world.

You may try it with:

	purewin-args.exe C:\1st-param """Debug ANSI"" 2.2-param" 3rd-param

*/

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ShellAPI.h> // CommandLineToArgvW

int __stdcall mainCRTStartup()
{
	LPWSTR pszWholeCmdline = GetCommandLineW();
	int numArgs = 0;
	LPWSTR *Argv = CommandLineToArgvW(pszWholeCmdline, &numArgs);

	HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD nWritten = 0;
	WCHAR tbuf[1000];

	for(int i=0; i<numArgs; i++)
	{
		wsprintfW(tbuf, L"(%d)Argv[%d]: %s\n", lstrlenW(Argv[i]), i, Argv[i]);
		WriteConsoleW(hout, tbuf, lstrlenW(tbuf), &nWritten, NULL);
	}

	return 0;
}
