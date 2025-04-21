#include <stdio.h>
#include <windows.h>
#include <MMSystem.h>
#include "../../detours/detours.h"


static void Init()
{
}


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





static ATOM(WINAPI* Marek_RegisterClassExW)(const WNDCLASSEXW *wcx) = RegisterClassExW;

ATOM WINAPI Marek__RegisterClassExW(WNDCLASSEXW* wcx)
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

	return Marek_RegisterClassExW(wcx);
}





// Target pointer for the uninstrumented Sleep API.
//
static VOID(WINAPI* TrueSleep)(DWORD dwMilliseconds) = Sleep;

// Detour function that replaces the Sleep API.
//
VOID WINAPI TimedSleep(DWORD dwMilliseconds)
{
	MessageBox(NULL, L"Sleep", L"Kodi", MB_OK);
	TrueSleep(dwMilliseconds);
}



// Variables

DWORD *_is_top_most_ptr = (DWORD*)0x52554C;
DWORD *_free_apect_rate_ptr = (DWORD*)0x525490;
DWORD *_auto_hide_buttons_ptr = (DWORD*)0x525550;

// RegisterClassExW

//__stdcall RegisterClassExW(const WNDCLASSEXW*);


// CreateWindowExW

typedef HWND(__stdcall* create_window_ex_w)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
create_window_ex_w _create_window_ex_w = (create_window_ex_w)0x01E3E698;


// WindowProc

typedef int(__cdecl *window_proc)(void *notUsed, unsigned int wParam, unsigned int uMsg, unsigned int lParam);
window_proc _window_proc = (window_proc)0x8FB020;


// window_layout

typedef int(__fastcall *window_layout)(int self, void *notUsed, signed int width, int height);
window_layout _window_layout = (window_layout)0x43A1C0;



// helper methods

typedef int(__fastcall *sub_421090)(int self, void *notUsed, int a2, int a3, int a4, int a5);
sub_421090 _sub_421090 = (sub_421090)0x421090;

typedef int(__fastcall *sub_42D6E0)(int self, void *notUsed, int a2);
sub_42D6E0 _sub_42D6E0 = (sub_42D6E0)0x42D6E0;

typedef int(__fastcall *sub_42D5C0)(int self, void *notUsed, int a2, int a3, int a4, int a5, int a6);
sub_42D5C0 _sub_42D5C0 = (sub_42D5C0)0x42D5C0;

typedef int(__fastcall *sub_429CF0)(int self, void *notUsed);
sub_429CF0 _sub_429CF0 = (sub_429CF0)0x0429CF0;

typedef int(__fastcall *sub_428CE0)(int self, void *notUsed);
sub_428CE0 _sub_428CE0 = (sub_428CE0)0x428CE0;

typedef BOOL(*sub_42FA80)();
sub_42FA80 _sub_42FA80 = (sub_42FA80)0x42FA80;

typedef void*(__fastcall *fullscreen)(int cwnd, void *notUsed);
fullscreen _fullscreen = (fullscreen)0x43B2A0;

typedef void*(__fastcall *record)(int cwnd, void *notUsed);
fullscreen _record = (record)0x43AAA0;

typedef void*(__fastcall *record_pause)(int cwnd, void *notUsed);
record_pause _record_pause = (record_pause)0x43CD20;

typedef void*(__fastcall *capture_image)(int cwnd, void *notUsed);
capture_image _capture_image = (capture_image)0x43AE30;

typedef void*(__fastcall *video_broadcast_share)(int cwnd, void *notUsed);
video_broadcast_share _video_broadcast_share = (video_broadcast_share)0x404000;

typedef void*(__fastcall *show_config_dialog)(int cwnd, void *notUsed);
show_config_dialog _show_config_dialog = (show_config_dialog)0x42DB10;


//
// Some MFC methods
//

typedef void(__fastcall *cwnd_move_window)(int self, void *notUsed, int x, int y, int width, int height, BOOL repaint);
cwnd_move_window _cwnd_move_window = (cwnd_move_window)0x441A2A; // ?MoveWindow@CWnd@@QAEXHHHHH@Z MFC42

typedef BOOL(__fastcall *cwnd_show_window)(int self, void *notUsed, int cmdShow);
cwnd_show_window _cwnd_show_window = (cwnd_show_window)0x441A18; // ?ShowWindow@CWnd@@QAEHH@Z MFC42


//
//
//

bool _show_buttons = false;
bool _isSound = true;


void make_top_most(int self)
{
	*_is_top_most_ptr = true;
	HWND hWnd = *((HWND *)self + 8);
	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
}


void toggle_fullscreen(int self)
{
	bool wasTopMost = *_is_top_most_ptr;
	_fullscreen(self, 0);
	*_is_top_most_ptr = wasTopMost;

	bool is_fullscreen = *(DWORD *)(self + 3232);
	if (!is_fullscreen && *_is_top_most_ptr)
	{
		make_top_most(self);
	}
}

//
// __create_window_ex_w
//

HWND __stdcall __create_window_ex_w(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	return _create_window_ex_w(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

//
// __window_proc
//

int __cdecl __window_proc(void *notUsed, unsigned int uMsg, unsigned int wParam, unsigned int lParam)
{
	//HWND hWnd = *((HWND *)self + 8);

	if (uMsg == WM_CREATE)
	{
		MessageBox(NULL, L"WM_CREATE", L"Kodi", MB_OK);
		//make_top_most(self);
	}

	return _window_proc(notUsed, uMsg, wParam, lParam);
}


//
// __window_layout
//

int __fastcall __window_layout(int self, void *notUsed, signed int width, int height)
{
	int result; // eax@1
	int v5; // edi@10
	int v6; // edi@15

	result = *(DWORD *)(self + 3232);
	if (!result)
	{
		result = *(DWORD *)(self + 2936);
		if (result)
		{
			////////////////////////////////////////////////////////////////////////////////
			// MG: BEGIN

			if (!_show_buttons)
			{
				_cwnd_show_window((self + 2904), 0, 0);
				_cwnd_show_window((self + 896), 0, 0);
				_cwnd_show_window((self + 1096), 0, 0);
				_cwnd_show_window((self + 1296), 0, 0);

				if (*(DWORD *)(self + 3136))
				{
					if (*((void**)0x525A1C))
					{
						_sub_421090((int)*((void**)0x525A1C), 0, 4, 4, width - 8, height - 8);
						if (!*((DWORD *)*((void**)0x525A1C) + 447))
							_sub_42D6E0((self + 3104), 0, 0);
					}

					// video area
					_cwnd_move_window((self + 1496), 0, 4, 4, width - 8, height - 8, 1);
					_sub_42D5C0((self + 3104), 0, 4, 4, width - 8, height - 8, 1);

					_cwnd_show_window((self + 1568), 0, 0);
					_cwnd_show_window((self + 1704), 0, 0);
					_cwnd_show_window((self + 1904), 0, 0);
					_cwnd_show_window((self + 296), 0, 0);
					_cwnd_show_window((self + 2104), 0, 0);
					_cwnd_show_window((self + 96), 0, 0);
					_cwnd_show_window((self + 696), 0, 0);
					_cwnd_show_window((self + 2704), 0, 0);
					_cwnd_show_window((self + 2304), 0, 0);
					_cwnd_show_window((self + 496), 0, 0);
					_cwnd_show_window((self + 2504), 0, 0);
					_cwnd_show_window((self + 696), 0, 0);
				}

				return 0;
			}
			else
			{
				_cwnd_show_window((self + 2904), 0, 5);
				_cwnd_show_window((self + 896), 0, 5);
				_cwnd_show_window((self + 1096), 0, 5);
				_cwnd_show_window((self + 1296), 0, 5);

				_cwnd_show_window((self + 1568), 0, 5);
				_cwnd_show_window((self + 1704), 0, 5);
				_cwnd_show_window((self + 1904), 0, 5);
				_cwnd_show_window((self + 296), 0, 5);
				_cwnd_show_window((self + 2104), 0, 5);
				_cwnd_show_window((self + 96), 0, 5);
				_cwnd_show_window((self + 696), 0, 5);
				_cwnd_show_window((self + 2704), 0, 5);
				_cwnd_show_window((self + 2304), 0, 5);
				_cwnd_show_window((self + 496), 0, 5);
				_cwnd_show_window((self + 2504), 0, 5);
				_cwnd_show_window((self + 696), 0, 5);
			}

			// MG: END
			////////////////////////////////////////////////////////////////////////////////

			// window close button
			_cwnd_move_window((self + 2904), 0, width - 25, 5, 13, 13, 1);
			
			// window minimize button
			_cwnd_move_window((self + 896), 0, width - 50, 5, 13, 13, 1);
			
			// window help button
			_cwnd_move_window((self + 1096), 0, width - 75, 5, 13, 13, 1);
			
			// window info button
			_cwnd_move_window((self + 1296), 0, width - 100, 5, 13, 13, 1);
			
			if (*(DWORD *)(self + 3136))
			{
				if (*((void**)0x525A1C))
				{
					_sub_421090((int)*((void**)0x525A1C), 0, 7, 30, width - 13, height - 100);
					if (!*((DWORD *)*((void**)0x525A1C) + 447))
						_sub_42D6E0((self + 3104), 0, 0);
				}

				// video area
				_cwnd_move_window((self + 1496), 0, 7, 30, width - 13, height - 100, 1);
				_sub_42D5C0((self + 3104), 0, 7, 30, width - 13, height - 100, 1);

				// volume slider
				_cwnd_move_window((self + 1568), 0, width - 100, height - 35, 67, 12, 1);
				
				// volume icon
				_cwnd_move_window((self + 1704), 0, width - 30, height - 35, 13, 13, 1);
				
				if (*(DWORD *)(self + 5640) && *((signed int*)0x5254D4) < 3)
				{
					_cwnd_move_window((self + 1904), 0, width / 2 - 129, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 296), 0, width / 2 - 197, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 2104), 0, width / 2 - 265, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 96), 0, width / 2 - 333, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 696), 0, width / 2 - 61, height - 63, 56, 56, 1);
					_cwnd_move_window((self + 2704), 0, width / 2 + 5, height - 63, 56, 56, 1);
					_cwnd_move_window((self + 2304), 0, width / 2 + 91, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 496), 0, width / 2 + 159, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 2504), 0, width / 2 + 227, height - 50, 38, 38, 1);
					_cwnd_show_window((self + 696), 0, 5);
				}
				else
				{
					v5 = (width - 56) / 2;
					
					// record button
					_cwnd_move_window((self + 2704), 0, v5, height - 63, 56, 56, 1);
					
					// capture image
					_cwnd_move_window((self + 2304), 0, v5 + 86, height - 50, 38, 38, 1);

					// capture GIF
					_cwnd_move_window((self + 496), 0, v5 + 154, height - 50, 38, 38, 1);

					// setup
					_cwnd_move_window((self + 2504), 0, v5 + 222, height - 50, 38, 38, 1);

					// fullscreen
					_cwnd_move_window((self + 1904), 0, v5 - 68, height - 50, 38, 38, 1);

					// video broadcast share
					_cwnd_move_window((self + 296), 0, v5 - 136, height - 50, 38, 38, 1);

					// show file list
					_cwnd_move_window((self + 2104), 0, v5 - 204, height - 50, 38, 38, 1);

					// play mode
					_cwnd_move_window((self + 96), 0, v5 - 272, height - 50, 38, 38, 1);
					
					// pause
					_cwnd_show_window((self + 696), 0, 0);
				}
			}
			_sub_429CF0(*(int *)(self + 3224), 0);
			_sub_428CE0(*(int *)(self + 3228), 0);
			result = _sub_42FA80();
			if (result)
			{
				if (*(DWORD *)(self + 5640) && *((signed int*)0x5254D4) < 3)
				{
					_cwnd_move_window((self + 1904), 0, width / 2 - 129, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 2104), 0, width / 2 - 197, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 696), 0, width / 2 - 61, height - 63, 56, 56, 1);
					_cwnd_move_window((self + 2704), 0, width / 2 + 5, height - 63, 56, 56, 1);
					_cwnd_move_window((self + 2304), 0, width / 2 + 91, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 2504), 0, width / 2 + 159, height - 50, 38, 38, 1);
					_cwnd_show_window((self + 696), 0, 5);
				}
				else
				{
					v6 = (width - 56) / 2;
					_cwnd_move_window((self + 2704), 0, v6, height - 63, 56, 56, 1);
					_cwnd_move_window((self + 2304), 0, v6 + 86, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 2504), 0, v6 + 154, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 1904), 0, v6 - 68, height - 50, 38, 38, 1);
					_cwnd_move_window((self + 2104), 0, v6 - 136, height - 50, 38, 38, 1);
					_cwnd_show_window((self + 696), 0, 0);
				}
				_cwnd_show_window((self + 296), 0, 0);
				_cwnd_show_window((self + 496), 0, 0);
				result = _cwnd_show_window((self + 96), 0, 0);
			}
		}
	}

	// MG: enable video broadcast icon
	_cwnd_show_window((self + 296), 0, 5);
	_cwnd_move_window((self + 296), 0, width / 2 - 220, height - 50, 38, 38, 1);
	// MG: END

	return result;
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

		//DetourAttach(&(PVOID&)TrueSleep, TimedSleep);
		DetourAttach(&(PVOID&)Marek_RegisterClassExW, Marek__RegisterClassExW);
		//DetourAttach(&(PVOID&)_window_proc, __window_proc);
		//DetourAttach(&(PVOID&)_window_layout, __window_layout);
       
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

		//DetourDetach(&(PVOID&)TrueSleep, TimedSleep);
		DetourDetach(&(PVOID&)Marek_RegisterClassExW, Marek__RegisterClassExW);
		//DetourDetach(&(PVOID&)_window_proc, __window_proc);
		//DetourDetach(&(PVOID&)_window_layout, __window_layout);
        
		error = DetourTransactionCommit();
    }
    return TRUE;
}

