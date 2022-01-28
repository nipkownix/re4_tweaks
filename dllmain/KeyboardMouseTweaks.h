#pragma once

enum class LastDevice
{
	Keyboard,
	Mouse,
	XinputController,
	DinputController
};

void Init_KeyboardMouseTweaks();
LastDevice GetLastUsedDevice();