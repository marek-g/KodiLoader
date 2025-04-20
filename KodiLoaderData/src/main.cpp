#include <stdio.h>
#include <windows.h>
#include <MMSystem.h>
#include "../../detours/detours.h"


static void Init()
{
}


// Variables

DWORD *_is_top_most_ptr = (DWORD*)0x52554C;
DWORD *_free_apect_rate_ptr = (DWORD*)0x525490;
DWORD *_auto_hide_buttons_ptr = (DWORD*)0x525550;


// WindowProc

typedef int(__fastcall *window_proc)(int self, void *notUsed, unsigned int wParam, unsigned int uMsg, unsigned int lParam);
window_proc _window_proc = (window_proc)0x43B9C0;


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
// __window_proc
//

int __fastcall __window_proc(int self, void *notUsed, unsigned int uMsg, unsigned int wParam, unsigned int lParam)
{
	HWND hWnd = *((HWND *)self + 8);

	if (uMsg == WM_CREATE)
	{
		make_top_most(self);
	}

	if (uMsg == WM_GETMINMAXINFO)
	{
		// set window minimum size

		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 160;
		lpMMI->ptMinTrackSize.y = 90;
		return 0;
	}

	if (uMsg == WM_MOUSEWHEEL || uMsg == 11653 /* mouse hook */)
	{
		// disable mouse wheel (do not change volume)
		return 0;
	}

	if (uMsg == WM_LBUTTONDBLCLK)
	{
		// fullscreen
		toggle_fullscreen(self);
		return 0;
	}

	if (uMsg == 9903)
	{
		// mute off
		_isSound = true;
	}

	if (uMsg == 9904)
	{
		// mute on
		_isSound = false;
	}

	if (uMsg == WM_NCRBUTTONUP || uMsg == WM_RBUTTONUP)
	{
		// create popup menu

		HMENU mainMenu = CreatePopupMenu();

		AppendMenu(mainMenu, 0, 0xA000A, L"Pokaż przyciski");
		if (_show_buttons)
		{
			CheckMenuItem(mainMenu, 0xA000A, MF_CHECKED);
		}
		AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);
		bool isRecording = *((void**)0x525A1C) && *((DWORD*)*((void**)0x525A1C) + 429);
		bool isPaused = *(DWORD*)(self + 5636);
		AppendMenu(mainMenu, 0, 0xA0006, isRecording ? L"Zatrzymaj nagrywanie" : L"Nagrywaj wideo");
		AppendMenu(mainMenu, isRecording ? 0 : MF_DISABLED, 0xA0007, isPaused ? L"Kontynuuj nagrywanie" : L"Pauza nagrywania");
		AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);
		AppendMenu(mainMenu, 0, 0xA000B, L"Przechwyć obraz");
		AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);

		AppendMenu(mainMenu, 0, 0xA000C, _isSound ? L"Wyłącz dźwięk" : L"Włącz dźwięk");
		AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);

		//AppendMenu(mainMenu, 0, 0xA0008, L"Udost�pnianie obrazu");
		//AppendMenu(mainMenu, 0, 0xA0009, L"Konfiguracja...");
		//AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);

		HMENU subMenu1 = CreatePopupMenu();
		AppendMenu(subMenu1, 0, 0xC8, L"16:9");
		AppendMenu(subMenu1, 0, 0xC9, L"16:10");
		AppendMenu(subMenu1, 0, 0xCA, L"4:3");
		AppendMenu(subMenu1, 0, MF_SEPARATOR, 0);
		AppendMenu(subMenu1, 0, 0x64, L"Zawsze na wierzchu");
		if (*_is_top_most_ptr)
		{
			CheckMenuItem(subMenu1, 0x64, MF_CHECKED);
		}
		AppendMenu(subMenu1, 0, 0x65, L"Zachowaj aspekt");
		if (!*_free_apect_rate_ptr)
		{
			CheckMenuItem(subMenu1, 0x65, MF_CHECKED);
		}
		//AppendMenu(subMenu1, 0, 0x66, L"Ukrywaj przyciski");
		//if (*_auto_hide_buttons_ptr)
		//{
		//	CheckMenuItem(subMenu1, 0x66, MF_CHECKED);
		//}
		AppendMenu(subMenu1, 0, 0xA0001, L"Zminimalizuj");
		AppendMenu(subMenu1, 0, 0xA0005, L"Pe�ny ekran");
		AppendMenu(mainMenu, MF_POPUP, (UINT_PTR)subMenu1, L"Okno");

		//AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);
		//AppendMenu(mainMenu, 0, 0xA0002, L"Pomoc");
		//AppendMenu(mainMenu, 0, 0xA0003, L"O programie");
		AppendMenu(mainMenu, 0, MF_SEPARATOR, 0);
		AppendMenu(mainMenu, 0, 0xA0004, L"Wyjdź");

		POINT cursorPos;
		GetCursorPos(&cursorPos);
		TrackPopupMenu(mainMenu, TPM_RIGHTBUTTON, cursorPos.x, cursorPos.y, 0, hWnd, 0);
		PostMessageA(hWnd, WM_NULL, 0, 0);

		DestroyMenu(subMenu1);
		DestroyMenu(mainMenu);

		return 0;
	}

	if (uMsg == WM_COMMAND)
	{
		// handle popup command

		switch (wParam)
		{
		case 0xA0001:
			// minimize window
			ShowWindow(hWnd, SW_MINIMIZE);
			return 0;

		case 0xA0002:
			// help
			//_cwnd_move_window((self + 1096), 0, 0, 5, 13, 13, 1);
			//ShowWindow(*((HWND*)(self + 1096 + 8)), SW_HIDE);
			//SendMessage(*((HWND*)(self + 1096 + 8)), BM_CLICK, 0, 0);
			return 0;

		case 0xA0003:
			// about
			//SendMessage(*((HWND*)(self + 1296 + 8)), BM_CLICK, 0, 0);
			return 0;

		case 0xA0004:
			// exit
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			SendMessage(hWnd, WM_QUIT, 0, 0);
			return 0;

		case 0xA0005:
			// fullscreen
			toggle_fullscreen(self);
			return 0;

		case 0xA0006:
			// record
			_record(self, 0);
			return 0;

		case 0xA0007:
			// record pause
			_record_pause(self, 0);
			return 0;

		case 0xA0008:
			// video broadcast share
			_video_broadcast_share(self, 0);
			return 0;

		case 0xA0009:
			// show configuration
			_show_config_dialog(self, 0);
			return 0;

		case 0xA000A:
			// show / hide buttons
			_show_buttons = !_show_buttons;
			{
				RECT clientRect;
				GetClientRect(hWnd, &clientRect);
				SendMessage(hWnd, WM_SIZE, 0, MAKELPARAM(clientRect.right - clientRect.left,
					clientRect.bottom - clientRect.top));
			}
			return 0;

		case 0xA000B:
			// capture image
			{
				SYSTEMTIME st;
				GetSystemTime(&st);

				char *video_path = (char*)*((char**)0x5254BC);
				char path[1024];
				sprintf(path, "%sIMG_%04d%02d%02d_%02d.%02d.%02d.jpg", video_path,
					st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

				char *old = (char*)*(LPCSTR *)(self + 3248);
				*(char**)(self + 3248) = path;
				_capture_image(self, 0);
				*(char**)(self + 3248) = old;
			}
			return 0;

		case 0xA000C:
			// toggle sound
			SendMessage(hWnd, _isSound ? 9904 : 9903, 0, 0);
			return 0;

		default:
		break;
		}
	}

	return _window_proc(self, notUsed, uMsg, wParam, lParam);
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

		DetourAttach(&(PVOID&)_window_proc, __window_proc);
		DetourAttach(&(PVOID&)_window_layout, __window_layout);
       
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

		DetourDetach(&(PVOID&)_window_proc, __window_proc);
		DetourDetach(&(PVOID&)_window_layout, __window_layout);
        
		error = DetourTransactionCommit();
    }
    return TRUE;
}

