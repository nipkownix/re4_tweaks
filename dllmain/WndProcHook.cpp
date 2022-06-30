#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "cfgMenu.h"
#include "WndProcHook.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "input.hpp"
#include "Logging/Logging.h"
#include "Game.h"

WNDPROC oWndProc;
HWND hWindow;

static uint32_t* ptrMouseDeltaX;

int curPosX;
int curPosY;

void EnableClipCursor(HWND window) {
	RECT rect;
	GetClientRect(window, &rect);

	POINT ul{};
	ul.x = rect.left;
	ul.y = rect.top;

	POINT lr{};
	lr.x = rect.right;
	lr.y = rect.bottom;

	MapWindowPoints(window, nullptr, &ul, 1);
	MapWindowPoints(window, nullptr, &lr, 1);

	rect.left = ul.x;
	rect.top = ul.y;

	rect.right = lr.x;
	rect.bottom = lr.y;

	if (!ClipCursor(&rect)) {
		#ifdef VERBOSE
		con.AddLogChar("Unable to lock cursor");
		#endif
		return;
	}

	#ifdef VERBOSE
	con.AddLogChar("Cursor locked");
	#endif
}

void DisableClipCursor(bool bCenterCursor) {
	ClipCursor(nullptr);

	// Move cursor to the center of the window
	if (bCenterCursor)
	{
		RECT rect;
		if (GetWindowRect(hWindow, &rect))
		{
			static int width = rect.right - rect.left;
			static int height = rect.bottom - rect.top;

			SetCursorPos(width / 2, height / 2);

			// Try to show the cursor immediately
			while (ShowCursor(TRUE) < 0);
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			SendMessage(hWindow, WM_SETCURSOR, (WPARAM)hWindow, MAKELPARAM(HTCLIENT, WM_MOUSEMOVE));
		}
	}

	#ifdef VERBOSE
	con.AddLogChar("Cursor released");
	#endif
}

// Our new hooked WndProc function
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_KEYDOWN:
			#ifdef VERBOSE
			if (wParam == VK_NUMPAD3)
			{
				con.AddLogChar("Sono me... dare no me?");
			}
			#endif
			break;

		case WM_MOVE:
			// Get current window position
			curPosX = (int)(short)LOWORD(lParam);   // horizontal position 
			curPosY = (int)(short)HIWORD(lParam);   // vertical position 
			break;

		case WM_EXITSIZEMOVE:
			EnableClipCursor(hWindow);
			if (cfg.bRememberWindowPos)
			{
				// Write new window position
				#ifdef VERBOSE
				con.AddConcatLog("curPosX = ", curPosY);
				con.AddConcatLog("curPosY = ", curPosX);
				#endif

				CIniReader iniReader("");

				cfg.iWindowPositionX = curPosX;
				cfg.iWindowPositionY = curPosY;

				iniReader.WriteInteger("DISPLAY", "WindowPositionX", curPosX);
				iniReader.WriteInteger("DISPLAY", "WindowPositionY", curPosY);
			}
			break;

		case WM_ACTIVATEAPP:
			if (wParam == WA_ACTIVE)
			{
				// Lock cursor inside window
				EnableClipCursor(hWindow);
			}
			break;

		case WM_WINDOWPOSCHANGED:
			// Lock cursor inside window
			EnableClipCursor(hWindow);
			break;

		case WM_KILLFOCUS:
			// Clear the mouse delta value if we lose focus
			*(int32_t*)(ptrMouseDeltaX) = 0;
			_input->clear_mouse_delta();

			// Unlock cursor
			DisableClipCursor(true);
			break;

		case WM_CLOSE:
			ExitProcess(0);
			break;
	}

	if (bCfgMenuOpen)
	{
		// Clear the mouse delta value if the cfg menu is open
		*(int32_t*)(ptrMouseDeltaX) = 0;
		_input->clear_mouse_delta();
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// CreateWindowExA hook to get the hWindow and set up the WndProc hook
HWND __stdcall CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	int windowX = cfg.iWindowPositionX < 0 ? CW_USEDEFAULT : cfg.iWindowPositionX;
	int windowY = cfg.iWindowPositionY < 0 ? CW_USEDEFAULT : cfg.iWindowPositionY;

	HWND result = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, cfg.bWindowBorderless ? WS_POPUP : dwStyle, windowX, windowY, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	hWindow = result;

	Logging::Log() << __FUNCTION__ << " -> Window created. Registering for input...";

	// Register hWnd for input processing
	_input = input::register_window(hWindow);

	oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);

	return result;
}

void Init_WndProcHook()
{
	auto pattern = hook::pattern("DB 05 ? ? ? ? D9 45 ? D9 C0 DE CA D9 C5");
	ptrMouseDeltaX = *pattern.count(1).get(0).get<uint32_t*>(2);

	// CreateWindowEx hook
	pattern = hook::pattern("68 00 00 00 80 56 68 ? ? ? ? 68 ? ? ? ? 6A 00");
	injector::MakeNOP(pattern.get_first(0x12), 6);
	injector::MakeCALL(pattern.get_first(0x12), CreateWindowExA_Hook, true);
}