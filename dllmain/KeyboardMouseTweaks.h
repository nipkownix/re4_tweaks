#pragma once

enum class LastDevice
{
	Keyboard,
	Mouse,
	XinputController,
	DinputController
};

int intMouseAimingMode();

LastDevice GetLastUsedDevice();