#pragma once

enum class LastDevice
{
	Keyboard,
	Mouse,
	XinputController,
	DinputController
};

void InventoryFlipBindings(UINT uMsg, WPARAM wParam);
int intMouseAimingMode();

LastDevice GetLastUsedDevice();