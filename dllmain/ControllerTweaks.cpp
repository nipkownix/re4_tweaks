#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "KeyboardMouseTweaks.h"
#include "Logging/Logging.h"

uintptr_t* ptrAimSpeedFldAddr;

void Init_ControllerTweaks()
{
	// Aiming speed
	auto pattern = hook::pattern("DD 05 ? ? ? ? DC F9 DD 05 ? ? ? ? DC FA D9 01 DE CB D9 45");
	ptrAimSpeedFldAddr = *pattern.count(1).get(0).get<uint32_t*>(2);
	struct AimSpeed
	{
		void operator()(injector::reg_pack& regs)
		{
			double aim_spd_val = *(double*)ptrAimSpeedFldAddr;

			if ((GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController))
				aim_spd_val /= cfg.fControllerSensitivity;

			_asm
			{
				fld aim_spd_val
			}
		}
	}; injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	pattern = hook::pattern("DD 05 ? ? ? ? DC F9 DD 05 ? ? ? ? DC FA D9 01");
	injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	if (cfg.fControllerSensitivity != 1.0f)
		Logging::Log() << __FUNCTION__ << " -> Controller sensitivity changes applied";
}