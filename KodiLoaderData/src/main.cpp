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
bool _is_top_most = true;
bool _is_full_screen_set = false;
bool _is_full_screen = false;
RECT _window_size_before_full_screen;
HWND btn_mouse_mode = 0;
HWND btn_top_most = 0;
HWND btn_audio = 0;
HWND btn_quit = 0;
bool buttons_visible = false;

int btn_left_top = 10;
int btn_size = 25;

void ToggleMouseMode(HWND hwnd) {
	_mouse_controls_window = !_mouse_controls_window;
	SetFocus(hwnd);
	SetActiveWindow(hwnd);
	//ShowCursor(_mouse_controls_window);
}

void UpdateWindowFrame(HWND hwnd) {
	// remove icon from the task bar
	//::SetWindowLongPtrW(hwnd, GWL_EXSTYLE, static_cast<LONG>(WS_EX_TOOLWINDOW));

	if (!_is_full_screen_set) {
		_is_full_screen = (::GetWindowLongPtr(hwnd, GWL_STYLE) & WS_POPUP);
		if (_is_full_screen) {
			_window_size_before_full_screen.left = 100;
			_window_size_before_full_screen.top = 100;
			_window_size_before_full_screen.right = _window_size_before_full_screen.left + 1280;
			_window_size_before_full_screen.bottom = _window_size_before_full_screen.top + 720;
		}
		_is_full_screen_set = true;
	}

	// make window borderless
	::SetWindowLongPtr(hwnd, GWL_STYLE, static_cast<LONG>(WS_VISIBLE | WS_POPUP | WS_CLIPCHILDREN));

	// make window top-most
	::SetWindowPos(hwnd, _is_top_most ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
}

void ToggleFullScreen(HWND hwnd) {
	_is_full_screen = !_is_full_screen;

	if (_is_full_screen) {
		GetWindowRect(hwnd, &_window_size_before_full_screen);
	}

	SendMessage(hwnd, WM_KEYDOWN, (WPARAM)VK_OEM_5, (LPARAM)0);
	SendMessage(hwnd, WM_KEYUP, (WPARAM)VK_OEM_5, (LPARAM)0);

	SetTimer(hwnd, 1235, 10, NULL);
}

void CreateButtons(HWND hwnd) {
	if (btn_mouse_mode != 0) {
		DestroyWindow(btn_mouse_mode);
	}

	btn_mouse_mode = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"M",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE,  // Styles 
		btn_left_top,         // x position 
		btn_left_top,          // y position 
		btn_size,        // Button width
		btn_size,        // Button height
		hwnd,       // Parent window
		(HMENU)1234,// Id
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	CheckDlgButton(hwnd, 1234, BST_CHECKED);

	if (btn_top_most != 0) {
		DestroyWindow(btn_top_most);
	}

	btn_top_most = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"T",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE,  // Styles 
		btn_left_top + btn_size,         // x position 
		btn_left_top,          // y position 
		btn_size,        // Button width
		btn_size,        // Button height
		hwnd,       // Parent window
		(HMENU)1235,// Id
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	CheckDlgButton(hwnd, 1235, BST_CHECKED);

	if (btn_audio != 0) {
		DestroyWindow(btn_audio);
	}

	btn_audio = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"A",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_PUSHLIKE,  // Styles 
		btn_left_top + btn_size * 2,         // x position 
		btn_left_top,          // y position 
		btn_size,        // Button width
		btn_size,        // Button height
		hwnd,       // Parent window
		(HMENU)1236,// Id
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

	CheckDlgButton(hwnd, 1236, BST_CHECKED);

	if (btn_quit != 0) {
		DestroyWindow(btn_quit);
	}

	btn_quit = CreateWindow(
		L"BUTTON",  // Predefined class; Unicode assumed 
		L"Q",      // Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
		btn_left_top + btn_size * 3,         // x position 
		btn_left_top,          // y position 
		btn_size,        // Button width
		btn_size,        // Button height
		hwnd,       // Parent window
		(HMENU)NULL,// Id
		(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.
}

void ShowButtons(HWND hwnd) {
	buttons_visible = true;
	ShowWindow(btn_mouse_mode, SW_SHOW);
	ShowWindow(btn_top_most, SW_SHOW);
	ShowWindow(btn_audio, SW_SHOW);
	ShowWindow(btn_quit, SW_SHOW);
}

void HideButtons(HWND hwnd) {
	buttons_visible = false;
	ShowWindow(btn_mouse_mode, SW_HIDE);
	ShowWindow(btn_top_most, SW_HIDE);
	ShowWindow(btn_audio, SW_HIDE);
	ShowWindow(btn_quit, SW_HIDE);
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
		Kodi_MainWndProc(hwnd, uMsg, wParam, lParam);

		CreateButtons(hwnd);

		HideButtons(hwnd);

		SetTimer(hwnd, 1234, 10, NULL);

		return 0;
	}

	/*if (uMsg == WM_ACTIVATE) // || uMsg == WM_SIZE)
	{
		Kodi_MainWndProc(hwnd, uMsg, wParam, lParam);

		SetTimer(hwnd, 1234, 10, NULL);

		return 0;
	}*/

	// no borders
	/*if (uMsg == WM_NCCALCSIZE || uMsg == WM_NCPAINT)
	{
		return 0;
	}*/

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
			ToggleFullScreen(hwnd);
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

		
		if (xPos >= btn_left_top && yPos >= btn_left_top && yPos < btn_left_top + btn_size && xPos < btn_left_top + btn_size*4) {
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

		/*if (_mouse_controls_window) {
			return 0;
		}*/
	}


	if (uMsg == WM_COMMAND)
	{
		if (lParam == (int)btn_mouse_mode) {
			ToggleMouseMode(hwnd);
			//ShowCursor(_mouse_controls_window);
			
			return 0;
		}

		if (lParam == (int)btn_top_most) {
			_is_top_most = !_is_top_most;

			// make window top-most
			::SetWindowPos(hwnd, _is_top_most ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

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

	if (uMsg == WM_TIMER && wParam == 1234) {
		KillTimer(hwnd, 1234);

		UpdateWindowFrame(hwnd);
	}

	if (uMsg == WM_TIMER && wParam == 1235) {
		KillTimer(hwnd, 1235);

		UpdateWindowFrame(hwnd);
		SetWindowPos(hwnd, NULL, _window_size_before_full_screen.left, _window_size_before_full_screen.top,
			_window_size_before_full_screen.right - _window_size_before_full_screen.left,
			_window_size_before_full_screen.bottom - _window_size_before_full_screen.top,
			0);
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

