#pragma once
#include "..\includes\stdafx.h"

void Init_cfgMenu();
void cfgMenuBinding(HWND hWnd, UINT uMsg, WPARAM wParam);

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);