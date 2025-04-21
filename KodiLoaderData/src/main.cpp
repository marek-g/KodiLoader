#include <stdio.h>
#include <windows.h>
#include <MMSystem.h>
#include "../../detours/detours.h"


static void Init()
{
}

//
//
//
static LRESULT(CALLBACK* Kodi_MainWndProc)(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

LRESULT CALLBACK Marek_MainWndProc(
	HWND hwnd,        // handle to window
	UINT uMsg,        // message identifier
	WPARAM wParam,    // first message parameter
	LPARAM lParam)    // second message parameter
{
	if (uMsg == WM_CREATE)
	{
		//make_top_most(self);
		MessageBox(NULL, L"Create", L"Kodi", MB_OK);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
	}

	if (uMsg == WM_LBUTTONUP)
	{
		MessageBox(NULL, L"LButtonUp", L"Kodi", MB_OK);
		return 0;
	}


	return Kodi_MainWndProc(hwnd, uMsg, wParam, lParam);
}

//
//
//
static ATOM(WINAPI* Windows_RegisterClassExW)(const WNDCLASSEXW *wcx) = RegisterClassExW;

ATOM WINAPI Marek_RegisterClassExW(WNDCLASSEXW* wcx)
{
	wprintf(wcx->lpszClassName);

	//if (wcscmp(wcx->lpszClassName, L"BlankWindowClass") == 0) {
	if (wcscmp(wcx->lpszClassName, L"Kodi") == 0) {
		// 0x77e779a0
		wchar_t buf[1024];
		wsprintf(buf, L"%s: %x", wcx->lpszClassName, wcx->lpfnWndProc);
		Kodi_MainWndProc = wcx->lpfnWndProc;

		wcx->lpfnWndProc = Marek_MainWndProc;
		
		MessageBox(NULL, buf, L"Kodi", MB_OK);
	}

	return Windows_RegisterClassExW(wcx);
}

//
//
//
BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved)
{
    LONG error;
    (void)hinst;
    (void)reserved;

    if (DetourIsHelperProcess()) {
        return TRUE;
    }

    if (dwReason == DLL_PROCESS_ATTACH) {
        DetourRestoreAfterWith();

        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        
		Init();

		DetourAttach(&(PVOID&)Windows_RegisterClassExW, Marek_RegisterClassExW);
       
		error = DetourTransactionCommit();

        if (error != NO_ERROR)
		{
			wchar_t buf[1024];
			swprintf(buf, L"Detour error: %d\n", error);
			MessageBox(0, buf, L"Detour", MB_OK);
        }
    }
    else if (dwReason == DLL_PROCESS_DETACH) {
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());

		DetourDetach(&(PVOID&)Windows_RegisterClassExW, Marek_RegisterClassExW);
        
		error = DetourTransactionCommit();
    }
    return TRUE;
}

