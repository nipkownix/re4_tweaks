#include <iostream>
#include "dllmain.h"
#include "Settings.h"
#include "Patches.h"

uintptr_t* ptrAimSpeedFldAddr;

int* g_XInputDeadzone_LS;
int* g_XInputDeadzone_RS;

int iDefaultXIDeadzone_LS = 7849;
int iDefaultXIDeadzone_RS = 8689;

float* fAnalogLX;
float* fAnalogLY;
float* fAnalogRX;
float* fAnalogRY;

void re4t::init::ControllerTweaks()
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

				if (re4t::cfg->bOverrideControllerSensitivity && isController())
					aim_spd_val /= re4t::cfg->fControllerSensitivity;

				_asm {fld aim_spd_val}
			}
		}; injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("DD 05 ? ? ? ? DC F9 DD 05 ? ? ? ? DC FA D9 01");
		injector::MakeInline<AimSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		if (re4t::cfg->bOverrideControllerSensitivity)
			spd::log()->info("{} -> Controller sensitivity changes applied", __FUNCTION__);
	}

	// Deadzone tweaks
	{
		// Nop the extra deadzones that were added for Xinput inside PadRead
		// No idea why this even exists since PadXinputRead also has deadzones for both sticks already. Oh well.
		if (re4t::cfg->bRemoveExtraXinputDeadzone)
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

		// Get pointers to the floats sub_964490 use for checking if the sticks are moving.
		// (Are these the origins of the AnalogR*_ and AnalogL*_ values?)
		pattern = hook::pattern("D9 81 ? ? ? ? D9 81 ? ? ? ? DA E9 DF E0 F6 C4 ? 7A ? D9 81 ? ? ? ? D9");
		fAnalogLX = (float*)*pattern.count(3).get(0).get<uint32_t>(2);
		fAnalogLY = fAnalogLX + 1;
		fAnalogRX = fAnalogLY + 1;
		fAnalogRY = fAnalogRX + 1;

		// Write new deadzone value right before they're loaded.
		pattern = hook::pattern("89 56 ? 89 46 ? 8B 0D ? ? ? ? F7 81 ? ? ? ? ? ? ? ? 0F");
		struct PadXinputReadDZwrite
		{
			void operator()(injector::reg_pack& regs)
			{
				*(uint32_t*)(regs.esi - 0x44) = regs.edx;
				*(uint32_t*)(regs.esi - 0x40) = regs.eax;

				if (re4t::cfg->bOverrideXinputDeadzone)
				{
					*g_XInputDeadzone_LS = (int)(re4t::cfg->fXinputDeadzone * iDefaultXIDeadzone_LS);
					*g_XInputDeadzone_RS = (int)(re4t::cfg->fXinputDeadzone * iDefaultXIDeadzone_RS);
				}
				else
				{
					*g_XInputDeadzone_LS = iDefaultXIDeadzone_LS;
					*g_XInputDeadzone_RS = iDefaultXIDeadzone_RS;
				}
			}
		}; injector::MakeInline<PadXinputReadDZwrite>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// This function (sub_964490) is responsible for updating the game's iLastUsedController variable. For Xinput, it seems to be just checking if the
		// floats aren't 0 before assuming a controller is being used, and writing Xinput as iLastUsedController.
		// OverrideXinputDeadzone revealed that, even in the vanilla game, this can cause false positives if the analog sticks have more than 0.1% of drift in any direction.
		// We add a bit of tolerance here to, hopefully, get rid of any false positives.
		auto pattern_begin = hook::pattern("75 71 D9 81 ? ? ? ? D9 81 ? ? ? ? DA E9 DF E0 F6 C4 ? 7A ? D9 81");
		auto pattern_end = hook::pattern("7A ? 83 C1 ? 81 F9 ? ? ? ? 0F 8C ? ? ? ? 8B 4D");
		struct LastUsedDeviceHook
		{
			void operator()(injector::reg_pack& regs)
			{
				float fDeadZone = 0.40f;
				bool bShouldUpdate = false;

				bShouldUpdate |= ((*fAnalogLX <= -fDeadZone) || (*fAnalogLX >= fDeadZone));
				bShouldUpdate |= ((*fAnalogLY <= -fDeadZone) || (*fAnalogLY >= fDeadZone));
				bShouldUpdate |= ((*fAnalogRX <= -fDeadZone) || (*fAnalogRX >= fDeadZone));
				bShouldUpdate |= ((*fAnalogRY <= -fDeadZone) || (*fAnalogRY >= fDeadZone));

				// Set PF if we want the input device to be updated to Xinput
				if (bShouldUpdate)
					regs.ef |= (1 << regs.parity_flag);
				else
					regs.ef &= ~(1 << regs.parity_flag);
			}
		}; injector::MakeInline<LastUsedDeviceHook>(pattern_begin.count(1).get(0).get<uint32_t>(2), pattern_end.count(1).get(0).get<uint32_t>(0));

		spd::log()->info("{} -> XInput deadzone changes applied", __FUNCTION__);
	}

	// Smooth analog turning
	// makes left stick movement similar to the 'Classic' control schemes found in later REs
	{
		// pl_R1_Walk
		auto pattern = hook::pattern("8B C1 83 E0 04 33 D2 0B C2 74 ? 8B ? ? ? ? ? D9");
		static float* cPlayer__SPEED_WALK_TURN = *pattern.count(1).get(0).get<float*>(28);
		struct WalkTurnHook
		{
			void operator()(injector::reg_pack& regs)
			{
				const float fDeadZone = 0.30f;

				if (isController() && re4t::cfg->bSmoothAnalogTurning)
				{
					if (abs(*fAnalogLX) > fDeadZone)
					{
						if (*fAnalogLX > 0.0f)
							PlayerPtr()->ang_A0.y -= GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_WALK_TURN * ((*fAnalogLX - fDeadZone) / (1.0f - fDeadZone));
						else
							PlayerPtr()->ang_A0.y += GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_WALK_TURN * -((*fAnalogLX + fDeadZone) / (1.0f - fDeadZone));
					}
				}
				else if ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_RIGHT) == (uint64_t)KEY_BTN::KEY_RIGHT)
				{
					PlayerPtr()->ang_A0.y -= GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_WALK_TURN;
				}
				else if ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_LEFT) == (uint64_t)KEY_BTN::KEY_LEFT)
				{
					PlayerPtr()->ang_A0.y += GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_WALK_TURN;
				}
			}
		}; injector::MakeInline<WalkTurnHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(82));

		// pl_R1_Run
		pattern = hook::pattern("8B C1 83 E0 04 33 D2 0B C2 74 ? D9 86 A4 00 00 00 8B ? ? ? ? ? D9");
		static float* cPlayer__SPEED_RUN_TURN = *pattern.count(1).get(0).get<float*>(31);
		static const float pl_speed2_xxx_1216 = 1.1f;
		struct RunTurnHook
		{
			void operator()(injector::reg_pack& regs)
			{
				const float fDeadZone = 0.30f;

				if (isController() && re4t::cfg->bSmoothAnalogTurning)
				{
					if (abs(*fAnalogLX) > fDeadZone)
					{
						if (*fAnalogLX > 0.0f)
							PlayerPtr()->ang_A0.y -= GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_RUN_TURN * ((*fAnalogLX - fDeadZone) / (1.0f - fDeadZone)) * pl_speed2_xxx_1216;
						else
							PlayerPtr()->ang_A0.y += GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_RUN_TURN * -((*fAnalogLX + fDeadZone) / (1.0f - fDeadZone)) * pl_speed2_xxx_1216;
					}
				}
				else if ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_RIGHT) == (uint64_t)KEY_BTN::KEY_RIGHT)
				{
					PlayerPtr()->ang_A0.y -= GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_RUN_TURN * pl_speed2_xxx_1216;
				}
				else if ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_LEFT) == (uint64_t)KEY_BTN::KEY_LEFT)
				{
					PlayerPtr()->ang_A0.y += GlobalPtr()->deltaTime_70 * *cPlayer__SPEED_RUN_TURN * pl_speed2_xxx_1216;
				}
			}
		}; injector::MakeInline<RunTurnHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(83));

		// p1_R1_Back
		// FrameRateFixes: Fix character backwards turning speed
		pattern = hook::pattern("8B C1 83 E0 04 33 D2 0B C2 74 ? D9 86 A4 00 00 00 D8");
		struct WalkTurnHookNoDeadzone
		{
			void operator()(injector::reg_pack& regs)
			{
				if (isController() && re4t::cfg->bSmoothAnalogTurning)
				{
					float deltaTime = re4t::cfg->bFixTurningSpeed ? GlobalPtr()->deltaTime_70 : 1.0f;
					if (*fAnalogLX > 0.0f)
						PlayerPtr()->ang_A0.y -= deltaTime * *cPlayer__SPEED_WALK_TURN * (*fAnalogLX / 1.0f);
					else
						PlayerPtr()->ang_A0.y += deltaTime * *cPlayer__SPEED_WALK_TURN * -(*fAnalogLX / 1.0f);
				}
				else if ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_RIGHT) == (uint64_t)KEY_BTN::KEY_RIGHT)
				{
					float deltaTime = re4t::cfg->bFixTurningSpeed ? GlobalPtr()->deltaTime_70 : 1.0f;
					PlayerPtr()->ang_A0.y -= deltaTime * *cPlayer__SPEED_WALK_TURN;
				}
				else if ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_LEFT) == (uint64_t)KEY_BTN::KEY_LEFT)
				{
					float deltaTime = re4t::cfg->bFixTurningSpeed ? GlobalPtr()->deltaTime_70 : 1.0f;
					PlayerPtr()->ang_A0.y += deltaTime * *cPlayer__SPEED_WALK_TURN;
				}
			}
		}; injector::MakeInline<WalkTurnHookNoDeadzone>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(62));

		// p1_R1_KlauserAttack
		pattern = hook::pattern("8B ? ? ? ? ? 8B C1 83 E0 04 33 D2 33 DB");
		injector::MakeInline<WalkTurnHookNoDeadzone>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(72));
	}

	// Fix joyFireOn bug with type III controls
	// When using type III config, knocking an enemy into a vulnerable state while in melee range with them interrupts your gunfire until you release the trigger
	// We just need to update joyFireOn's existing check for type II controls to check for type III controls as well
	{
		auto pattern = hook::pattern("8B ? ? ? ? ? 38 41 ? 74");
		struct joyFireOn_TypeIIIFix
		{
			void operator()(injector::reg_pack& regs)
			{
				if (SystemSavePtr()->pad_type_B == keyConfigTypes::TypeII || SystemSavePtr()->pad_type_B == keyConfigTypes::TypeIII)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<joyFireOn_TypeIIIFix>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(9));
	}
}