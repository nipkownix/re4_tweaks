#pragma once

enum class LastDevice
{
	Keyboard,
	Mouse,
	XinputController,
	DinputController
};

void Init_KeyboardMouseTweaks();
void InventoryFlipBindings(UINT uMsg, WPARAM wParam);

LastDevice GetLastUsedDevice();