#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "cfgMenu.h"
#include "WndProcHook.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "../external/imgui/imgui.h"
#include "LAApatch.h"
#include "MouseTurning.h"
#include "KeyboardMouseTweaks.h"
#include "ToolMenu.h"

WNDPROC oWndProc;
HWND hWindow;

static uint32_t* ptrMouseDeltaX;

void processRawMouse(RAWMOUSE rawMouse)
{
	ImGuiIO& io = ImGui::GetIO();

	switch (rawMouse.usButtonFlags)
	{
	case RI_MOUSE_LEFT_BUTTON_DOWN:
		io.MouseDown[0] = true;
		break;
	case RI_MOUSE_RIGHT_BUTTON_DOWN:
		io.MouseDown[1] = true;
		break;
	case RI_MOUSE_MIDDLE_BUTTON_DOWN:
		io.MouseDown[2] = true;
		break;
	case RI_MOUSE_LEFT_BUTTON_UP:
		io.MouseDown[0] = false;
		break;
	case RI_MOUSE_RIGHT_BUTTON_UP:
		io.MouseDown[1] = false;
		break;
	case RI_MOUSE_MIDDLE_BUTTON_UP:
		io.MouseDown[2] = false;
		break;
	case RI_MOUSE_WHEEL:
		io.MouseWheel += (float)(short)rawMouse.usButtonData / (float)WHEEL_DELTA;
		break;
	case RI_MOUSE_HWHEEL:
		io.MouseWheelH += (float)(short)rawMouse.usButtonData / (float)WHEEL_DELTA;
		break;
	default:
		break;
	}
}

// Our new hooked WndProc function
int curPosX;
int curPosY;
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Evaluate mouse turning state
	isMouseTurnEnabled(uMsg, wParam);
	
	// Key bindings for flipping inventory items on keyboard
	InventoryFlipBindings(uMsg, wParam);

	// Key binding for brining up the config menu
	cfgMenuBinding(hWindow, uMsg, wParam, lParam);

	// Key binding for brining up the config menu
	ToolMenuBinding(uMsg, wParam);

	#ifdef VERBOSE
	// Key binding for showing/hiding the console window
	ConsoleBinding(uMsg, wParam);
	#endif

	switch (uMsg) {
		case WM_MOVE:
			// Get current window position
			curPosX = (int)(short)LOWORD(lParam);   // horizontal position 
			curPosY = (int)(short)HIWORD(lParam);   // vertical position 
			break;

		case WM_EXITSIZEMOVE:
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

		case WM_KILLFOCUS:
			// Clear the mouse delta value if we lose focus
			*(int32_t*)(ptrMouseDeltaX) = 0;
			break;

		case WM_CLOSE:
			ExitProcess(0);
			break;

		// Raw input stuff for ImGui
		case WM_INPUT: {
			HRAWINPUT rawInput = (HRAWINPUT)lParam;
			UINT dwSize = 0;
			if (GetRawInputData((HRAWINPUT)rawInput, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER)) == -1) {
				break;
			}

			LPBYTE lpb = new BYTE[dwSize];
			if (lpb == nullptr) {
				break;
			}

			if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize) {
				con.AddLogChar("GetRawInputData returned an incorrect size");
			}

			RAWINPUT* raw = (RAWINPUT*)lpb;

			if (raw->header.dwType == RIM_TYPEMOUSE) {
				if (raw->data.mouse.usButtonFlags)
					processRawMouse(raw->data.mouse);
			}

			delete[]lpb;
			break;
		}
	}

	if (bCfgMenuOpen) {
		// Use raw mouse input because this game is weird and won't let me use window messages properly.
		RAWINPUTDEVICE rid[1];

		rid[0].usUsagePage = 0x01;
		rid[0].usUsage = 0x02;
		rid[0].dwFlags = RIDEV_INPUTSINK;
		rid[0].hwndTarget = hWindow;

		if (!RegisterRawInputDevices(rid, 1, sizeof(*rid)))
			con.AddLogChar("Failed to register for raw input!");

		// Clear the mouse delta value if the cfg menu is open
		*(int32_t*)(ptrMouseDeltaX) = 0;

		return ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	}
	else {
		return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
	}
}

// CreateWindowExA hook to get the hWindow and set up the WndProc hook
HWND __stdcall CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	int windowX = cfg.iWindowPositionX < 0 ? CW_USEDEFAULT : cfg.iWindowPositionX;
	int windowY = cfg.iWindowPositionY < 0 ? CW_USEDEFAULT : cfg.iWindowPositionY;

	HWND result = CreateWindowExA(dwExStyle, lpClassName, lpWindowName, cfg.bWindowBorderless ? WS_POPUP : dwStyle, windowX, windowY, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	hWindow = result;

	oWndProc = (WNDPROC)SetWindowLongPtr(hWindow, GWL_WNDPROC, (LONG_PTR)WndProc);

	return result;
}

void Init_WndProcHook()
{
	auto pattern = hook::pattern("DB 05 ? ? ? ? D9 45 ? D9 C0 DE CA D9 C5");
	ptrMouseDeltaX = *pattern.count(1).get(0).get<uint32_t*>(2);

	//CreateWindowEx hook
	pattern = hook::pattern("68 00 00 00 80 56 68 ? ? ? ? 68 ? ? ? ? 6A 00");
	injector::MakeNOP(pattern.get_first(0x12), 6);
	injector::MakeCALL(pattern.get_first(0x12), CreateWindowExA_Hook, true);
}