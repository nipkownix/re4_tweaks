#include <iostream>
#include "dllmain.h"
#include "Settings.h"
#include "input.hpp"
#include "Game.h"
#include "ConsoleWnd.h"
#include <iniReader.h>

HWND hWindow;

bool bIsMoving;

int curSizeX;
int curSizeY;
int curPosX;
int curPosY;

void EnableClipCursor(HWND window)
{
	if (re4t::cfg->bNeverHideCursor)
		return;

	if (window != GetForegroundWindow())
		return;

	if (bIsMoving)
		return;

	// Gotta set this to false so our hooks allow ClipCursor to go through if the UI is open...
	pInput->block_mouse_input(false, false);

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
		con.log("Unable to lock cursor");
		#endif
		return;
	}

	#ifdef VERBOSE
	con.log("Cursor locked");
	#endif
}

void DisableClipCursor(bool bCenterCursor)
{
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
	con.log("Cursor released");
	#endif
}

LRESULT(CALLBACK* GameWndProc)(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_KEYDOWN:
			#ifdef VERBOSE
			if (wParam == VK_NUMPAD3)
			{
				con.log("Sono me... dare no me?");
			}
			#endif
			break;

		case WM_MOVE:
			// Get current window position
			curPosX = (int)(short)LOWORD(lParam);   // horizontal position 
			curPosY = (int)(short)HIWORD(lParam);   // vertical position 
			break;

		case WM_SIZE:
			// Get current window position
			curSizeX = (int)LOWORD(lParam);   // horizontal size 
			curSizeY = (int)HIWORD(lParam);   // vertical size 
			break;

		case WM_ENTERSIZEMOVE:
			bIsMoving = true;
			break;

		case WM_EXITSIZEMOVE:
			bIsMoving = false;

			EnableClipCursor(hWindow);

			// Write new window position
			if (re4t::cfg->bRememberWindowPos)
			{
				#ifdef VERBOSE
				con.log("curPosX = %d", curPosY);
				con.log("curPosY = %d", curPosX);
				#endif

				iniReader ini;

				re4t::cfg->iWindowPositionX = curPosX;
				re4t::cfg->iWindowPositionY = curPosY;

				ini.setInt("DISPLAY", "WindowPositionX", curPosX);
				ini.setInt("DISPLAY", "WindowPositionY", curPosY);
				ini.writeIni();
			}

			// Resize game's resolution
			if (curSizeX != *bio4::g_D3D::Width_1 ||
				curSizeY != *bio4::g_D3D::Height_1)
			{
				bio4::D3D_SetupResolution(curSizeX, curSizeY);
				bio4::ScreenReSize(0x200, 0x1C0);
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
			pInput->clear_raw_mouse_delta();

			// Unlock cursor
			DisableClipCursor(true);
			break;

		case WM_CLOSE:
			DisableClipCursor(false);
			ExitProcess(0);
			break;
	}

	if (bCfgMenuOpen)
	{
		// Clear the mouse delta value if the cfg menu is open
		pInput->clear_raw_mouse_delta();
	}

	return GameWndProc(hWnd, uMsg, wParam, lParam);
}

// CreateWindowExA hook to get the hWindow and set up the WndProc hook
HWND __stdcall CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	// Perform LAA check/prompt before game window has a chance to be created
	LAACheck();

	hWindow = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, 0, 0, 0, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	
	// Register hWnd for input processing
	spd::log()->info("{} -> Window created; Registering for input", __FUNCTION__);

	pInput = re4t::input::register_window(hWindow);

	return hWindow;
}

BOOL __stdcall AdjustWindowRectEx_Hook(LPRECT lpRect, DWORD dwStyle, BOOL bMenu, DWORD dwExStyle)
{
	DWORD lStyle = GetWindowLong(hWindow, GWL_STYLE);
	DWORD lExStyle = GetWindowLong(hWindow, GWL_EXSTYLE);

	bool isFullscreen = *bio4::g_D3D::Fullscreen;

	if (re4t::cfg->bWindowBorderless && !isFullscreen)
	{
		lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
		lStyle |= (WS_POPUP);
	}
	else
	{
		lStyle |= (WS_MINIMIZEBOX | WS_SYSMENU);
		if (re4t::cfg->bEnableWindowResize && !isFullscreen)
			lStyle |= (WS_MAXIMIZEBOX | WS_THICKFRAME);
	}

	SetWindowLong(hWindow, GWL_STYLE, lStyle);

	return AdjustWindowRectEx(lpRect, lStyle, bMenu, lExStyle);
}

BOOL __stdcall SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	bool isFullscreen = *bio4::g_D3D::Fullscreen;

	int WindowX = X;
	int WindowY = Y;

	if (!isFullscreen)
	{
		RECT rect;
		GetWindowRect(hWnd, &rect);

		if (re4t::cfg->iWindowPositionX != -1)
			WindowX = re4t::cfg->iWindowPositionX;
		else
			WindowX = rect.left;

		if (re4t::cfg->iWindowPositionY != -1)
			WindowY = re4t::cfg->iWindowPositionY;
		else
			WindowY = rect.top;
	}

	return SetWindowPos(hWnd, hWndInsertAfter, WindowX, WindowY, cx, cy, uFlags);
}

BOOL __stdcall MoveWindow_Hook(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	bool isFullscreen = *bio4::g_D3D::Fullscreen;

	int WindowX = X;
	int WindowY = Y;

	if (!isFullscreen)
	{
		RECT rect;
		GetWindowRect(hWnd, &rect);

		if (re4t::cfg->iWindowPositionX != -1)
			WindowX = re4t::cfg->iWindowPositionX;
		else
			WindowX = rect.left;

		if (re4t::cfg->iWindowPositionY != -1)
			WindowY = re4t::cfg->iWindowPositionY;
		else
			WindowY = rect.top;
	}

	return MoveWindow(hWnd, WindowX, WindowY, nWidth, nHeight, bRepaint);
}

void re4t::init::WndProcHook()
{
	// CreateWindowEx hook
	auto pattern = hook::pattern("68 00 00 00 80 56 68 ? ? ? ? 68 ? ? ? ? 6A 00");
	injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(18), 6);
	injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(18), CreateWindowExA_Hook, true);

	// WndProc hook
	pattern = hook::pattern("C7 45 ? ? ? ? ? 89 75 ? 89 75 ? 89 45 ? FF 15 ? ? ? ? 6A");
	uint32_t* wndproc_addr = pattern.count(1).get(0).get<uint32_t>(3);
	uint32_t GameWndProcAddr = *(uint32_t*)wndproc_addr;
	GameWndProc = (LRESULT(WINAPI*)(HWND, UINT, WPARAM, LPARAM))GameWndProcAddr;
	injector::WriteMemory<uint32_t>(wndproc_addr, (uint32_t)&WndProc_hook, true);

	// Hook AdjustWindowRectEx call inside sub_93A0B0 to apply our style changes
	pattern = hook::pattern("FF 15 ? ? ? ? 8B 4D ? 2B 4D ? 8B 55");
	injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 6);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), AdjustWindowRectEx_Hook, PATCH_CALL);

	// Hook SetWindowPos call inside sub_93A0B0 to apply our custom X Y window position
	pattern = hook::pattern("FF 15 ? ? ? ? 56 53 57 8D 45 EC 50");
	injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 6);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), SetWindowPos_Hook, PATCH_CALL);

	// Same as before
	pattern = hook::pattern("FF 15 ? ? ? ? 8B 0D ? ? ? ? 6A 01");
	injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 6);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), MoveWindow_Hook, PATCH_CALL);
}