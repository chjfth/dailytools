#ifndef __dlptr_winapi_h_
#define __dlptr_winapi_h_

#include <windows.h>

namespace {

    const void *FUNCPTR_UNINIT = (void*)8;

    static inline HMODULE load_module(const wchar_t* dllname) {
        return ::LoadLibraryW(dllname);
    }

    static inline HMODULE load_module(const char* dllname) {
        return ::LoadLibraryA(dllname);
    }

    class Hmodule {
        HMODULE m_hmodule;
    public:
        Hmodule(const char* dllname) : m_hmodule(load_module(dllname)) {}
        Hmodule(const wchar_t* dllname) : m_hmodule(load_module(dllname)) {}
        ~Hmodule() { if (m_hmodule)  ::FreeLibrary(m_hmodule); }
        operator HMODULE() const {
            return m_hmodule;
        }
    };


    template<typename FuncType, FuncType* template_id>
    static inline FuncType* dlptr_get_func(const char* dllname, const char *func_name)
    {
        static FuncType* func = (FuncType*)FUNCPTR_UNINIT;

        if (func != FUNCPTR_UNINIT)
            return func;

        static const Hmodule hmodule(dllname);
        if (!hmodule)
        {
            return func = nullptr;
        }

        func = (FuncType*)::GetProcAddress(hmodule, func_name);
        return func;
    }

} // namespace


#define DLPTR_WINAPI_GET_FUNC_(dllname, apiname) dlptr_get_func<decltype(apiname), apiname>(dllname, #apiname)


#define DEFINE_DLPTR_WINAPI(dllfilename, apiname) \
	static auto dlptr_ ## apiname = DLPTR_WINAPI_GET_FUNC_(dllfilename, apiname); \

/* Example Usage: 

//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <tchar.h>

DEFINE_DLPTR_WINAPI("kernel32.dll", GetWindowsDirectoryW)
DEFINE_DLPTR_WINAPI("kernel32.dll", GetSystemDirectoryW)

// [Stramphibian ready] This will auto select MessageBoxA or MessageBoxW
DEFINE_DLPTR_WINAPI("user32.dll", MessageBox) 

// You may "instantiate" a callable future-WINAPI as a function-pointer.
//
// WinSDK 1703 will have NTDDI_WIN10_RS1 defined in sdkddkver.h; older WinSDK does not.
#ifndef NTDDI_WIN10_RS1 
// Supply future function prototypes, so they can be used in older SDK.
UINT WINAPI GetDpiForSystem();
UINT WINAPI GetDpiForWindow(HWND hwnd);
#endif
//
DEFINE_DLPTR_WINAPI("user32.dll", GetDpiForSystem) // since Win10.1607
DEFINE_DLPTR_WINAPI("user32.dll", GetDpiForWindow) // since Win10.1607

int main()
{
	WCHAR windir[MAX_PATH], sysdir[MAX_PATH];
	dlptr_GetWindowsDirectoryW(windir, MAX_PATH);
	dlptr_GetSystemDirectoryW(sysdir, MAX_PATH);

	wprintf(L"windir=%s\n", windir);
	wprintf(L"sysdir=%s\n", sysdir);

	// When you need to call one future-WINAPI, first check for null pointer.

	UINT windpi = dlptr_GetDpiForWindow ? 
		dlptr_GetDpiForWindow(GetDesktopWindow()) 
		: 0;
	printf("GetDpiForWindow() = %d\n", windpi); // Will print 96 etc on Win10.1607+

	dlptr_MessageBox(NULL, _T("Hello."), _T("Title"), MB_OK);

	return 0;
}

*/


#endif
