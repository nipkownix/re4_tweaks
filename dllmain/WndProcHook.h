#pragma once

struct key_state
{
	bool pressed = false;
	bool changed = false;
};

void Init_WndProcHook();
bool IsComboKeyPressed(std::vector<uint32_t>* KeyVector);

extern HWND hWindow;