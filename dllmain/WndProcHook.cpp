#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "cfgMenu.h"
#include "WndProcHook.h"

WNDPROC oWndProc;
HWND hWindow;

// Our new hooked WndProc function
int curPosX;
int curPosY;
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == getMapKey(flip_item_left, "vk") || wParam == getMapKey(flip_item_right, "vk"))
		{
			bShouldFlipX = true;
			Sleep(1);
		}
		else if (wParam == getMapKey(flip_item_up, "vk") || wParam == getMapKey(flip_item_down, "vk"))
		{
			bShouldFlipY = true;
			Sleep(1);
		}
		if (wParam == VK_F1)
		{
			bCfgMenuOpen ^= 1;
		}
		#ifdef VERBOSE
		else if (wParam == VK_NUMPAD3)
		{
			std::cout << "Sono me... dare no me?" << std::endl;
		}
		#endif
		break;
	case WM_MOVE:
		// Get current window position
		curPosX = (int)(short)LOWORD(lParam);   // horizontal position 
		curPosY = (int)(short)HIWORD(lParam);   // vertical position 
		break;
	case WM_EXITSIZEMOVE:
		if (bRememberWindowPos)
		{
			// Write new window position
			#ifdef VERBOSE
			std::cout << "curPosX = " << curPosX << std::endl;
			std::cout << "curPosY = " << curPosY << std::endl;
			#endif

			CIniReader iniReader("");

			iniReader.WriteInteger("DISPLAY", "WindowPositionX", curPosX);
			iniReader.WriteInteger("DISPLAY", "WindowPositionY", curPosY);
		}
		break;
	case WM_CLOSE:
		ExitProcess(0);
		break;
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

// CreateWindowExA hook to get the hWindow and set up the WndProc hook
HWND __stdcall CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	int windowX = iWindowPositionX < 0 ? CW_USEDEFAULT : iWindowPositionX;
	int windowY = iWindowPositionY < 0 ? CW_USEDEFAULT : iWindowPositionY;

	HWND result = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, bWindowBorderless ? WS_POPUP : dwStyle, windowX, windowY, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	hWindow = result;

	oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);

	return result;
}

void Init_WndProcHook()
{
	//CreateWindowEx hook
	auto pattern = hook::pattern("68 00 00 00 80 56 68 ? ? ? ? 68 ? ? ? ? 6A 00");
	injector::MakeNOP(pattern.get_first(0x12), 6);
	injector::MakeCALL(pattern.get_first(0x12), CreateWindowExA_Hook, true);
}