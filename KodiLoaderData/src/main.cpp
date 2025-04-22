#include <stdio.h>
#include <windowsx.h>
#include <windows.h>
#include <MMSystem.h>
#include "../../detours/detours.h"


static void Init()
{
}

bool _mouse_controls_window = true;
bool _is_sound = true;
HWND btn_mouse_mode;
HWND btn_full_screen;
HWND btn_audio;
HWND btn_quit;
bool buttons_visible = false;

int btn_size = 50;

void ToggleMouseMode(HWND hwnd) {
	_mouse_controls_window = !_mouse_controls_window;
	SetFocus(hwnd);
	SetActiveWindow(hwnd);
	ShowCursor(_mouse_controls_window);
}

void ShowButtons(HWND hwnd) {
	buttons_visible = true;
	ShowWindow(btn_mouse_mode, SHOW_OPENWINDOW);
	ShowWindow(btn_full_screen, SHOW_OPENWINDOW);
	ShowWindow(btn_audio, SHOW_OPENWINDOW);
	ShowWindow(btn_quit, SHOW_OPENWINDOW);
}

void HideButtons(HWND hwnd) {
	buttons_visible = false;
	ShowWindow(btn_mouse_mode, HIDE_WINDOW);
	ShowWindow(btn_full_screen, HIDE_WINDOW);
	ShowWindow(btn_audio, HIDE_WINDOW);
	ShowWindow(btn_quit, HIDE_WINDOW);
}

void ToggleButtons(HWND hwnd) {
	if (buttons_visible) {
		HideButtons(hwnd);
	}
	else {
		ShowButtons(hwnd);
	}
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
	if (uMsg == WM_CREATE) {
		btn_mouse_mode = CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"M",      // Button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
			10,         // x position 
			10,          // y position 
			btn_size,        // Button width
			btn_size,        // Button height
			hwnd,       // Parent window
			(HMENU)NULL,// Id
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL);      // Pointer not needed.

		btn_full_screen = CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"F",      // Button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
			10 + btn_size,         // x position 
			10,          // y position 
			btn_size,        // Button width
			btn_size,        // Button height
			hwnd,       // Parent window
			(HMENU)NULL,// Id
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL);      // Pointer not needed.

		btn_audio = CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"A",      // Button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
			10 + btn_size*2,         // x position 
			10,          // y position 
			btn_size,        // Button width
			btn_size,        // Button height
			hwnd,       // Parent window
			(HMENU)NULL,// Id
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL);      // Pointer not needed.

		btn_quit = CreateWindow(
			L"BUTTON",  // Predefined class; Unicode assumed 
			L"Q",      // Button text 
			WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
			10 + btn_size*3,         // x position 
			10,          // y position 
			btn_size,        // Button width
			btn_size,        // Button height
			hwnd,       // Parent window
			(HMENU)NULL,// Id
			(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			NULL);      // Pointer not needed.

		HideButtons(hwnd);

		SetTimer(hwnd, 123, 1000, NULL);
	}

	if (uMsg == WM_ACTIVATE)
	{
		// remove icon from the task bar
		::SetWindowLongPtrW(hwnd, GWL_EXSTYLE, static_cast<LONG>(WS_EX_TOOLWINDOW));

		// make window top-most
		::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		//return 0;
	}

	// no borders
	if (uMsg == WM_NCCALCSIZE || uMsg == WM_NCPAINT)
	{
		return 0;
	}

	// move and resize with mouse in the client area
	if (uMsg == WM_NCHITTEST)
	{
		if (_mouse_controls_window) {
			int border_size = 8;

			POINTS pt = MAKEPOINTS(lParam);

			RECT rc;
			GetWindowRect(hwnd, &rc);

			if (pt.x >= rc.left && pt.x < rc.left + border_size)
			{
				if (pt.y >= rc.top && pt.y < rc.top + border_size) {
					return HTTOPLEFT;
				}
				if (pt.y < rc.bottom && pt.y >= rc.bottom - border_size)
				{
					return HTBOTTOMLEFT;
				}
				return HTLEFT;
			}
			else if (pt.x < rc.right && pt.x >= rc.right - border_size)
			{
				if (pt.y >= rc.top && pt.y < rc.top + border_size) {
					return HTTOPRIGHT;
				}
				if (pt.y < rc.bottom && pt.y >= rc.bottom - border_size)
				{
					return HTBOTTOMRIGHT;
				}
				return HTRIGHT;
			}
			else if (pt.y >= rc.top && pt.y < rc.top + border_size)
			{
				return HTTOP;
			}
			else if (pt.y < rc.bottom && pt.y >= rc.bottom - border_size)
			{
				return HTBOTTOM;
			}

			return HTCAPTION;
		}
	}

	if (uMsg == WM_SETCURSOR) {
		if (_mouse_controls_window) {
			if (LOWORD(lParam) == HTCAPTION) {
				SetCursor(LoadCursor(NULL, IDC_CROSS));
				return 1;
			}
		}
	}

	if (uMsg == WM_NCLBUTTONDBLCLK || uMsg == WM_LBUTTONDBLCLK) {
		if (_mouse_controls_window) {
			SendMessage(hwnd, WM_KEYDOWN, (WPARAM)VK_OEM_5, (LPARAM)0);
			SendMessage(hwnd, WM_KEYUP, (WPARAM)VK_OEM_5, (LPARAM)0);
			return 0;
		}
	}

	if (uMsg == WM_NCMOUSEMOVE || uMsg == WM_MOUSEMOVE) {
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (uMsg == WM_NCMOUSEMOVE) {
			RECT rect;
			GetWindowRect(hwnd, &rect);
			xPos -= rect.left;
			yPos -= rect.top;
		}

		
		if (xPos >= 10 && yPos >= 10 && yPos < 10 + btn_size && xPos < 10 + btn_size*4) {
			ShowButtons(hwnd);
			//ShowCursor(true);
		}
		else {
			HideButtons(hwnd);
			if (uMsg == WM_MOUSEMOVE) {
				//ShowCursor(false);
			}
		}

		ShowCursor(true);
	}


	if (uMsg == WM_COMMAND)
	{
		if (lParam == (int)btn_mouse_mode) {
			ToggleMouseMode(hwnd);
			return 0;
		}

		if (lParam == (int)btn_full_screen) {
			SendMessage(hwnd, WM_KEYDOWN, (WPARAM)VK_OEM_5, (LPARAM)0);
			SendMessage(hwnd, WM_KEYUP, (WPARAM)VK_OEM_5, (LPARAM)0);
			return 0;
		}

		if (lParam == (int)btn_audio) {
			_is_sound = !_is_sound;
			SendMessage(hwnd, WM_KEYDOWN, (WPARAM)VK_F8, (LPARAM)0);
			SendMessage(hwnd, WM_KEYUP, (WPARAM)VK_F8, (LPARAM)0);
			return 0;
		}

		if (lParam == (int)btn_quit) {
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			SendMessage(hwnd, WM_QUIT, 0, 0);
			return 0;
		}
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

	if (wcscmp(wcx->lpszClassName, L"Kodi") == 0) {
		Kodi_MainWndProc = wcx->lpfnWndProc;
		wcx->lpfnWndProc = Marek_MainWndProc;
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

