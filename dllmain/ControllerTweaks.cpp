#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "KeyboardMouseTweaks.h"
#include "Logging/Logging.h"
#include "Patches.h"

uintptr_t* ptrAimSpeedFldAddr;

int* g_XInputDeadzone_LS;
int* g_XInputDeadzone_RS;

void Init_ControllerTweaks()
{
	// Aiming speed
	{
		auto pattern = hook::pattern("DD 05 ? ? ? ? DC F9 DD 05 ? ? ? ? DC FA D9 01 DE CB D9 45");
		ptrAimSpeedFldAddr = *pattern.count(1).get(0).get<uint32_t*>(2);
		struct AimSpeed
		{
			void operator()(injector::reg_pack& regs)
			{
				double aim_spd_val = *(double*)ptrAimSpeedFldAddr;

				if (cfg.bOverrideControllerSensitivity && ((GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController)))
					aim_spd_val /= cfg.fControllerSensitivity;

				_asm {fld aim_spd_val}
			}
		}; injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("DD 05 ? ? ? ? DC F9 DD 05 ? ? ? ? DC FA D9 01");
		injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		if (cfg.bOverrideControllerSensitivity)
			Logging::Log() << __FUNCTION__ << " -> Controller sensitivity changes applied";
	}

	// Deadzone tweaks
	{
		// Nop the extra deadzones that were added for Xinput inside PadRead
		// No idea why this even exists since PadXinputRead also has deadzones for both sticks already. Oh well.
		if (cfg.bRemoveExtraXinputDeadzone)
		{
			auto pattern = hook::pattern("C6 46 ? ? 8A 46 ? 3A C1 7D ? 81 4E ? ? ? ? ? EB"); // RS X
			injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 4, true);

			pattern = hook::pattern("C6 46 ? ? A1 ? ? ? ? 80 78 ? ? 75 ? F6 05"); // RS Y
			injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 4, true);
		}

		// Get deadzone pointers from PadXinputRead
		auto pattern = hook::pattern("0F B7 0D ? ? ? ? 51 52 E8 ? ? ? ? 0F B7 4E ? D9 5E");
		g_XInputDeadzone_LS = (int*)*pattern.count(1).get(0).get<uint32_t>(3);

		pattern = hook::pattern("0F B7 05 ? ? ? ? 50 51 E8 ? ? ? ? 0F B7 46 ? D9 5E");
		g_XInputDeadzone_RS = (int*)*pattern.count(1).get(0).get<uint32_t>(3);

		// Calculate new PadXinputRead deadzone
		if (cfg.bOverrideXinputDeadzone)
		{
			*g_XInputDeadzone_LS = (int)(cfg.fXinputDeadzone * 7849); // 7849 and 8689 are the default values
			*g_XInputDeadzone_RS = (int)(cfg.fXinputDeadzone * 8689);
		}
	}
}