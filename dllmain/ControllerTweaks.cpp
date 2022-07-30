#include <iostream>
#include "dllmain.h"
#include "Settings.h"
#include "Patches.h"

uintptr_t* ptrAimSpeedFldAddr;

int* g_XInputDeadzone_LS;
int* g_XInputDeadzone_RS;

int iDefaultXIDeadzone_LS = 7849;
int iDefaultXIDeadzone_RS = 8689;

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

				if (pConfig->bOverrideControllerSensitivity && ((LastUsedDevice() == InputDevices::XinputController) || (LastUsedDevice() == InputDevices::DinputController)))
					aim_spd_val /= pConfig->fControllerSensitivity;

				_asm {fld aim_spd_val}
			}
		}; injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("DD 05 ? ? ? ? DC F9 DD 05 ? ? ? ? DC FA D9 01");
		injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		if (pConfig->bOverrideControllerSensitivity)
			spd::log()->info("{} -> Controller sensitivity changes applied", __FUNCTION__);
	}

	// Deadzone tweaks
	{
		// Nop the extra deadzones that were added for Xinput inside PadRead
		// No idea why this even exists since PadXinputRead also has deadzones for both sticks already. Oh well.
		if (pConfig->bRemoveExtraXinputDeadzone)
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

		pattern = hook::pattern("89 56 ? 89 46 ? 8B 0D ? ? ? ? F7 81 ? ? ? ? ? ? ? ? 0F");
		struct PadXinputReadDZwrite
		{
			void operator()(injector::reg_pack& regs)
			{
				*(uint32_t*)(regs.esi - 0x44) = regs.edx;
				*(uint32_t*)(regs.esi - 0x40) = regs.eax;

				switch (LastUsedDevice()) {
				case InputDevices::DinputController:
				case InputDevices::XinputController:
					if (pConfig->bOverrideXinputDeadzone)
					{
						*g_XInputDeadzone_LS = (int)(pConfig->fXinputDeadzone * iDefaultXIDeadzone_LS);
						*g_XInputDeadzone_RS = (int)(pConfig->fXinputDeadzone * iDefaultXIDeadzone_RS);
					}
					else
					{
						*g_XInputDeadzone_LS = iDefaultXIDeadzone_LS;
						*g_XInputDeadzone_RS = iDefaultXIDeadzone_RS;
					}
					break;
				case InputDevices::Keyboard:
				case InputDevices::Mouse:
					*g_XInputDeadzone_LS = iDefaultXIDeadzone_LS;
					*g_XInputDeadzone_RS = iDefaultXIDeadzone_RS;
					break;
				}
			}
		}; injector::MakeInline<PadXinputReadDZwrite>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		spd::log()->info("{} -> XInput deadzone changes applied", __FUNCTION__);
	}
}