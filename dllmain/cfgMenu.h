#pragma once
#include "..\includes\stdafx.h"

void Init_cfgMenu();
void cfgMenuBinding(HWND hWnd, LPARAM lParam);
bool ParseConfigMenuKeyCombo(std::string_view in_combo);

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);