#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Settings.h"

float fDefaultC_RANGE = 59.0f;
float fCameraPosX_backup;

int m_trans_type_1B4; // Seems to be related to camera type?

float* C_RANGE_1097;
float* CamSmth__m_ratio_FC;

float* fCameraPosX; // m_direction_ratio_208
float* fCameraPosY; // m_depression_ratio_204 

float* wepPitch; // m3r.y

float* fMousePosX;
float* fMousePosY;

int8_t* AnalogRX_8;
int8_t* AnalogRY_9;

uintptr_t* knife_r3_set40_jmpAddr;

double CameraControl__getCameraPitch_hook()
{
	if (!pConfig->bCameraImprovements)
		return 0.0;

	if (isController())
		return 0.0;

	// We originally used fCameraPosY, but that value gets reset early in some cases, such as when using the knife, leading 
	// to it always reading "0.0". This really makes me think this whole CameraControl::getCameraPitch() stuff either never got too far into the 
	// game's development, or everything was stripped right after the GC release, with future changes not accounting for this legacy feature.
	double angle = (*fMousePosY * 127.0f) / *C_RANGE_1097;

	// Invert the value if needed.
	if (!FlagIsSet(SystemSavePtr()->flags_CONFIG_0, uint32_t(Flags_CONFIG::CFG_AIM_REVERSE)))
		angle = -angle;

	// By default we need to decrease the positive Y value a bit.
	// This value can change depending on the weapon.
	double dFactor = 3.0;

	switch (GlobalPtr()->weapon_no_4FC0){
	case 0x2E: // PRL 412
	case 0x0F: // Handcannon
		dFactor = 2.0;
	}

	if (angle > 0)
		angle /= dFactor;

	// Scale the value down a bit if Ashley is present due to camera shenanigans.
	if (m_trans_type_1B4 == 1)
		angle = angle - (angle / 4);

	return angle;
}

// MakeInline apparently can't change regs.esp, so we need to hook this differently.
void __declspec(naked) knife_r3_set40_hook()
{
	__asm {pushad}

	if (pConfig->bCameraImprovements)
	{
		__asm 
		{
			popad
			mov eax, 0x1 // Manipulate the test and jne instructions that comes a bit after this.
			add esp, 0x8
			test eax, eax
			jmp knife_r3_set40_jmpAddr
		}
	}
	else
	{
		__asm
		{
			popad
			add esp, 0x8
			test eax, eax
			jmp knife_r3_set40_jmpAddr
		}
	}
}

void Init_CameraTweaks()
{
	auto pattern = hook::pattern("E8 ? ? ? ? D9 5D EC D9 EE D9 45 EC");
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), CameraControl__getCameraPitch_hook, PATCH_JUMP);

	pattern = hook::pattern("D9 05 ? ? ? ? D9 C0 DE FA D9 C9 74");
	C_RANGE_1097 = (float*)*pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("D9 1D ? ? ? ? 8B 8B ? ? ? ? 8B 11 8B 42 ? FF D0 8B B3");
	CamSmth__m_ratio_FC = (float*)*pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("D9 15 ? ? ? ? D9 1D ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC CC 55");
	fCameraPosY = (float*)*pattern.count(1).get(0).get<uint32_t>(2);
	fCameraPosX = fCameraPosY + 1;

	pattern = hook::pattern("D8 0D ? ? ? ? D9 C9 DE CA DE C1 D9 1D ? ? ? ? D9 05 ? ? ? ? D9");
	wepPitch = (float*)*pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("D9 05 ? ? ? ? D8 D1 DF E0 D9 05");
	fMousePosX = (float*)*pattern.count(1).get(0).get<uint32_t>(2);
	fMousePosY = fMousePosX + 1;

	pattern = hook::pattern("A2 ? ? ? ? EB ? DD D8 8B 0D");
	AnalogRX_8 = (int8_t*)*pattern.count(1).get(0).get<uint32_t>(1);
	AnalogRY_9 = AnalogRX_8 + 1;

	// Hook PadRead to change the camera sensitivity
	pattern = hook::pattern("74 ? dd 05 ? ? ? ? eb ? dd 05 ? ? ? ? dc f9");
	struct CameraSens
	{
		void operator()(injector::reg_pack& regs)
		{
			double dMouseMulti = 0;

			// Original code
			if (GetMouseAimingMode() == MouseAimingModes::Modern)
				dMouseMulti = 20.0;
			else // Classic
				dMouseMulti = 70.0;

			dMouseMulti /= (double)pConfig->fCameraSensitivity;

			__asm {fld dMouseMulti}
		}
	}; injector::MakeInline<CameraSens>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(16));

	// Hook CameraSmooth::move to make the camera a bit more responsive
	pattern = hook::pattern("8B 15 ? ? ? ? D9 42 ? 83 C4 ? E8 ? ? ? ? D9 5D ? D9 45 ? 51 D9 5D");
	struct CSMoveCamDelta
	{
		void operator()(injector::reg_pack& regs)
		{
			// Original multi was just deltaTime
			float fNewMulti = GlobalPtr()->deltaTime_70;

			if (pConfig->bCameraImprovements && isKeyboardMouse())
				fNewMulti *= 1.5f;

			// Code we replaced
			__asm {fld fNewMulti}
			regs.eax = (uint32_t)GlobalPtr();
		}
	}; injector::MakeInline<CSMoveCamDelta>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(9));

	// Hook CameraQuasiFPS::checkCameraType to store m_trans_type_1B4, so we can check it inside getCameraPitch
	pattern = hook::pattern("0F B6 86 ? ? ? ? 8B 94 86 ? ? ? ? 89 96");
	struct checkCameraTypeHook
	{
		void operator()(injector::reg_pack& regs)
		{
			// Code we replaced
			regs.eax = *(uint8_t*)(regs.esi + 0x1B4);

			m_trans_type_1B4 = regs.eax;
		}
	}; injector::MakeInline<checkCameraTypeHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	// When aiming, this section changes the float used to populate AnalogRY_9 from absolute value to delta value.
	// After aiming, that value is never restored to what it was before.
	// Keeping the value from being changed at all doesn't seem to have any side-effects.
	pattern = hook::pattern("D9 15 ? ? ? ? D9 15 ? ? ? ? D9 C9 83 FB");
	struct PadReadResetXYAim
	{
		void operator()(injector::reg_pack& regs)
		{
			// We backup this value here to use it in our quickturn workaround.
			if (*fCameraPosX != 0.0f)
				fCameraPosX_backup = *fCameraPosX;

			if (!pConfig->bCameraImprovements || isController())
			{
				float tmp = 0.0f;
				float tmp2 = 0.0f;

				__asm
				{
					fst tmp
					fst tmp2
				}

				*fMousePosX = tmp;
				*fMousePosY = tmp2;
			}
		}
	}; injector::MakeInline<PadReadResetXYAim>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(12));

	// Same thing happens here.
	pattern = hook::pattern("D9 85 ? ? ? ? D9 15 ? ? ? ? D9 85 ? ? ? ? D9 15 ? ? ? ? EB");
	struct PadReadResetXYAim2
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!pConfig->bCameraImprovements || isController())
			{
				*fMousePosX = *(float*)(regs.ebp - 0x508);
				*fMousePosY = *(float*)(regs.ebp - 0x50C);
			}
		}
	}; injector::MakeInline<PadReadResetXYAim2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(24));

	// Replace AnalogRX with a float instead. Provides much smoother camera movement
	// when using the camera's full range (tweaked C_RANGE_1097 value).
	pattern = hook::pattern("DB 45 ? D9 5D ? D9 45 ? D9 C0 D9 EE");
	struct calcDepressionRatioAnalogRX
	{
		void operator()(injector::reg_pack& regs)
		{
			if (pConfig->bCameraImprovements && isKeyboardMouse())
			{
				// Invert the X float if it reaches its limit.
				// Makes it so we can fully rotate the camera.
				if ((*fMousePosX == 1.0f) || (*fMousePosX == -1.0f))
				{
					*fCameraPosX = -*fCameraPosX;
					*fMousePosX = -*fMousePosX;
				}

				// Gives the camera full range.
				*C_RANGE_1097 = 42.3f;

				*(float*)(regs.ebp - 0x8) = *fMousePosX * 127.0f;
			}
			else
			{
				*C_RANGE_1097 = fDefaultC_RANGE;
				*(float*)(regs.ebp - 0x8) = (float)*AnalogRX_8;
			}
		}
	}; injector::MakeInline<calcDepressionRatioAnalogRX>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Same as before, but for the Y
	pattern = hook::pattern("DB 45 ? D9 05 ? ? ? ? D9 C0 DE FA");
	struct calcDepressionRatioAnalogRY
	{
		void operator()(injector::reg_pack& regs)
		{
			float fNewY;
			float range = *C_RANGE_1097;

			if (pConfig->bCameraImprovements && isKeyboardMouse())
				fNewY = -(*fMousePosY * 127.0f);
			else
				fNewY = (float)*AnalogRY_9;

			__asm
			{
				fld fNewY
				fld range
			}
		}
	}; injector::MakeInline<calcDepressionRatioAnalogRY>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(9));

	// Keep fMousePosX at 0 while aiming to avoid possible weirdness.
	pattern = hook::pattern("D9 9E ? ? ? ? D9 EE 5F D9 9E ? ? ? ? 5E 8B E5 5D C3 83 3D");
	struct calcDepressionRatioKeepXYZero
	{
		void operator()(injector::reg_pack& regs)
		{
			// We backup this value here to use it in our quickturn workaround.
			if (*fCameraPosX != 0.0f)
				fCameraPosX_backup = *fCameraPosX;

			float tmp = 0.0f;
			__asm {fstp tmp}

			*(float*)(regs.esi + 0x204) = tmp;

			if (pConfig->bCameraImprovements && isKeyboardMouse())
				*fMousePosX = 0.0f;
		}
	}; injector::MakeInline<calcDepressionRatioKeepXYZero>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Hook CameraControl::Move to freeze fMousePosY/X when needed.
	pattern = hook::pattern("D9 1D ? ? ? ? 8B 8B ? ? ? ? 8B 11 8B 42 ? FF D0 8B B3");
	struct ccMoveHook
	{
		void operator()(injector::reg_pack& regs)
		{
			*fMousePosX = 0.0f;
			*fMousePosY = 0.0f;

			// Code we replaced
			float tmp = 0.0f;
			__asm {fstp tmp}

			*CamSmth__m_ratio_FC = tmp;
		}
	}; injector::MakeInline<ccMoveHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Hook pl_R1_Walk to provide the option to reset the camera pos when the player starts running.
	// Only relevant if MouseTurning is off, really.
	pattern = hook::pattern("c7 86 ? ? ? ? ? ? ? ? c6 86 ? ? ? ? ? db 45");
	struct pl_R1_Walk_runReset
	{
		void operator()(injector::reg_pack& regs)
		{
			// Code we replaced
			*(uint32_t*)(regs.esi + 0xFC) = 0x4020300;

			if (pConfig->bCameraImprovements 
				&& pConfig->bResetCameraWhenRunning 
				&& !pConfig->bUseMouseTurning
				&& isKeyboardMouse())
			{
				*fMousePosX = 0.0f;
				*fMousePosY = 0.0f;
				*fCameraPosX = 0.0f;
				*fCameraPosY = 0.0f;
			}
		}
	}; injector::MakeInline<pl_R1_Walk_runReset>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));

	// Reset AnalogRX_8 and fMousePosX when aiming begins
	pattern = hook::pattern("8B EC 8B 45 ? 8B 08 89 0D ? ? ? ? 8B 50 ? 89 15 ? ? ? ? 8B 40");
	struct CamCtrlShoulderSetAimHook
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebp = regs.esp;
			regs.eax = *(uint32_t*)(regs.ebp + 0x8);

			if (pConfig->bCameraImprovements && isKeyboardMouse())
			{
				// Reseting AnalogRX_8 here fixes a weird issue that would occur in the vanilla game.
				// When turning the camera to the side, pressing the aim button, and then releasing it, the camera
				// would turn to face the same "angle" it was before aiming. It is weird to explain.
				// I believe all these quirks were introduced when QLOC decided to hack their way around the original camera functions
				// to add mouse freelook, which uses absolute position, while keeping the aiming's delta position using the same
				// AnalogRY and AnalogRX vars. Not sure how hard it would have been to just split those up, but who knows.

				*fMousePosX = 0.0f;
				*AnalogRX_8 = 0;
			}
		}
	}; injector::MakeInline<CamCtrlShoulderSetAimHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// This section resets the camera X pos to 0 if we turn while walking. 
	// Maybe useful for controllers(?), but it is getting in our way for KB/M.
	pattern = hook::pattern("D9 15 ? ? ? ? D9 1D ? ? ? ? EB ? DD D8 A1");
	struct PadReadXreset
	{
		void operator()(injector::reg_pack& regs)
		{
			// MouseTurn type A needs this regardless of whether or not CameraImprovements is enabled.
			if (!pConfig->bCameraImprovements && !(pConfig->bUseMouseTurning && (pConfig->iMouseTurnType == MouseTurnTypes::TypeA)))
			{
				float tmp = 0.0f;
				__asm {fst tmp}

				*fMousePosX = tmp;
			}
		}
	}; injector::MakeInline<PadReadXreset>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Same as before, but for the Y pos
	struct PadReadYreset
	{
		void operator()(injector::reg_pack& regs)
		{
			float tmp = 0.0f;
			__asm {fstp tmp}

			// MouseTurn type A needs this regardless of whether or not CameraImprovements is enabled.
			if (!pConfig->bCameraImprovements && !(pConfig->bUseMouseTurning && (pConfig->iMouseTurnType == MouseTurnTypes::TypeA)))
			{
				*fMousePosY = tmp;
			}
		}
	}; injector::MakeInline<PadReadYreset>(pattern.count(1).get(0).get<uint32_t>(6), pattern.count(1).get(0).get<uint32_t>(12));

	// This section resets the camera Y pos to 0 after we stop aiming.
	// Same deal as before.
	pattern = hook::pattern("D9 15 ? ? ? ? 83 FB ? 75 ? 83 BD ? ? ? ? ? 75");
	struct PadReadYAfterAimReset
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!pConfig->bCameraImprovements && isKeyboardMouse())
			{
				*fMousePosY = 0.0f;
			}
		}
	}; injector::MakeInline<PadReadYAfterAimReset>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	
	// Due to our getCameraPitch hook potentially returning values higher than 1.0/lower than -1.0, we have to
	// make sure here that wepPitch is within bounds.
	// Clamping it inside our getCameraPitch hook doesn't work, because PlWepLockRand does something weird to the value afterwards.
	// This is just easier.
	pattern = hook::pattern("D9 15 ? ? ? ? 83 C4 ? D9 05 ? ? ? ? D9 C0 D9 EE DA E9 DF E0 F6 C4 ? 7A ? D9 C9 D9 1D");
	struct PlWepLockCtrlPitchClamp
	{
		void operator()(injector::reg_pack& regs)
		{
			// Reset fCameraPosX_backup after it has been used.
			fCameraPosX_backup = 0.0f;

			float tmp = 0.0f;
			__asm {fst tmp}

			if (!pConfig->bCameraImprovements || isController())
				*wepPitch = tmp;
			else
				*wepPitch = std::clamp(tmp, -1.0f, 1.0f);
		}
	}; injector::MakeInline<PlWepLockCtrlPitchClamp>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Clamp in PlSetLockPitch as well, just to be safe
	pattern = hook::pattern("D9 15 ? ? ? ? D9 E8 D9 EE DC E9 D9 C2 DE CA DE CA");
	injector::MakeInline<PlWepLockCtrlPitchClamp>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Customize the camera Y clamping code
	auto pattern_begin = hook::pattern("D9 05 ? ? ? ? D8 96 ? ? ? ? DF E0 F6 C4 ? 74 ? DD D8");
	auto pattern_end = hook::pattern("D9 86 ? ? ? ? D9 5D ? D9 45 ? D9 9E");
	struct calcDepressionRatio_yClamp
	{
		void operator()(injector::reg_pack& regs)
		{
			// Vanilla game clamps fCameraPosY here, but that value is derived from fMousePosY. This would make it so
			// the camera would reach its limit, but fMousePosY would continue increasing until reaching 1.0f, making it so
			// you had to move the mouse all the way down before fCameraPosY would begin decreasing again.
			// This solution is just better. We also allow fCameraPosY to go a bit above/beyond 1.0f/-1.0f to give a bit more
			// range to the camera.
			if (pConfig->bCameraImprovements && isKeyboardMouse())
				*fMousePosY = std::clamp(*fMousePosY, -0.44f, 0.44f);
			else // Vanilla behavior.
			{
				// The original code here was used just to clamp the fCameraPosY value.
				// Since hacking around (while still leaving the option to enable/disable our changes on the fly) 
				// proved to be a pain, we just replace everything with std::clamp. Seems to work fine.
				*fCameraPosY = std::clamp(*fCameraPosY, -1.0f, 1.0f);
			}
		}
	}; injector::MakeInline<calcDepressionRatio_yClamp>(pattern_begin.count(1).get(0).get<uint32_t>(0), pattern_end.count(1).get(0).get<uint32_t>(0));

	// Wep adjustments
	{
		// Enables an alternative code path that skips having to wait for the initial ready animation to be finished
		// before the aim moves to the correct place. Looks like this was added for Wii support, since it is originally
		// checking for WPadMode. We repalce the WPadMode check with our own bool here. Making it check for both would
		// be possible, and probably necessary if someone ever tries to restore Wii remote support, I guess.

		// Generic struct to replace the WPadMode cmp
		struct cmp_jne
		{
			void operator()(injector::reg_pack& regs)
			{
				if (pConfig->bCameraImprovements && isKeyboardMouse())
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		};

		// wep02_r3_ready00 -> Handguns
		auto pattern = hook::pattern("83 3D ? ? ? ? ? 75 ? E8 ? ? ? ? 32 C9");
		injector::MakeInline<cmp_jne>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// wep07_r3_ready00 -> Shotguns
		pattern = hook::pattern("39 1D ? ? ? ? 75 2A 88 9E ? ? ? ? 8B 0D ? ? ? ? 8B 41 44");
		injector::MakeInline<cmp_jne>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// wep11_r3_ready00 -> Machine guns
		pattern = hook::pattern("39 1D ? ? ? ? 75 ? C6 86 ? ? ? ? ? 8B 15 ? ? ? ? 0F B6 8A");
		injector::MakeInline<cmp_jne>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// wep14_r3_ready00 -> Mine thrower
		// This one is a bit different. We just need to not let the function set wepPitch to 0.
		pattern = hook::pattern("D9 15 ? ? ? ? 5F D9 15 ? ? ? ? D9 1D");
		struct wep14_r3_ready00WepPitchReset
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bCameraImprovements || isController())
				{
					*wepPitch = 0.0f;
				}
			}
		}; injector::MakeInline<wep14_r3_ready00WepPitchReset>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// wep51_r3_ready00 -> PRL 412
		pattern = hook::pattern("83 3D ? ? ? ? ? DC 0D ? ? ? ? D9 5D ? D9 45 ? D9 15 ? ? ? ? D9 15 ? ? ? ? D9 C9 D9 15 ? ? ? ? 75");
		injector::MakeInline<cmp_jne>(pattern.count(3).get(1).get<uint32_t>(0), pattern.count(3).get(1).get<uint32_t>(7));

		pattern = hook::pattern("83 3D ? ? ? ? ? 0F 85 ? ? ? ? 80 3D ? ? ? ? ? 74 ? A1 ? ? ? ? D9 86 ? ? ? ? 8B 50");
		injector::MakeInline<cmp_jne>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		pattern = hook::pattern("D9 15 ? ? ? ? D9 15 ? ? ? ? C6 05 ? ? ? ? ? D9 1D ? ? ? ? 5F");
		struct wep51_r3_ready00_3
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bCameraImprovements || isController())
				{
					*wepPitch = 0.0f;
				}
			}
		}; injector::MakeInline<wep51_r3_ready00_3>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// wep28_r3_ready00 -> Krauser's bow
		pattern = hook::pattern("39 1D ? ? ? ? 75 ? D9 EE 6A ? D9 15 ? ? ? ? 6A");
		injector::MakeInline<cmp_jne>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		/*
		// wep50_r3_ready00 -> Ada's bowgun. ## -->> Disabled because this path doesn't have the 30fps-on-60fps flag enabled, and that
		// is necessary to avoid a small glitch in the animation. Adding the flag causes even more glitches in the animation, sadly.
		pattern = hook::pattern("83 3D ? ? ? ? ? 0F 85 ? ? ? ? C6 86 ? ? ? ? ? 80 3D ? ? ? ? ? 74 6F");
		struct wep50_r3_ready00
		{
			void operator()(injector::reg_pack& regs)
			{
				if (pConfig->bCameraImprovements)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<wep50_r3_ready00>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
		*/

		// As a workaround to the bowgun issue above, we just skip the bowgun's initial animation.
		pattern = hook::pattern("6A ? 6A ? E8 ? ? ? ? 83 C4 ? 8B CE E8 ? ? ? ? 85 C0 74 ? C7 86");
		struct wep50_r3_ready10
		{
			void operator()(injector::reg_pack& regs)
			{
				if ((bool)regs.eax || (pConfig->bCameraImprovements && isKeyboardMouse()))
				{
					*(uint32_t*)(regs.esi + 0xFC) = 0x10600;

					// This workaround breaks quickturning, so we implement our own replacement here.
					float factor = fCameraPosX_backup * 0.8f;
					PlayerPtr()->ang_A0.y += factor;
					*fMousePosX += factor;
				}
			}
		}; injector::MakeInline<wep50_r3_ready10>(pattern.count(1).get(0).get<uint32_t>(21), pattern.count(1).get(0).get<uint32_t>(33));

		// wep19_r3_ready00 -> Grenades
		// Stop it from reseting the Y pos.
		pattern = hook::pattern("D9 15 ? ? ? ? 6A ? D9 15 ? ? ? ? 6A ? D9 1D ? ? ? ? 6A ? 8B 8E");
		struct wep19_r3_ready00
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bCameraImprovements || isController())
					*wepPitch = 0.0f;
			}
		}; injector::MakeInline<wep19_r3_ready00>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// knife_r3_set40 -> Knife.
		// The knife doesn't have the alternate Wii path, so we do someting similar to Ada's bowgun workaround from earlier.
		pattern = hook::pattern("83 C4 ? 85 C0 75 ? 8B 0D ? ? ? ? 8B C1");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 5, true);
		knife_r3_set40_jmpAddr = pattern.count(1).get(0).get<uintptr_t>(5);
		injector::MakeJMP(pattern.count(1).get(0).get<uint32_t>(0), knife_r3_set40_hook);

		// Implement quickturning with the knife. Not sure why this isn't a vanilla feature.
		pattern = hook::pattern("8B 8E ? ? ? ? 80 79 ? ? 7E ? 6A ? E8 ? ? ? ? 6A ? 8B");
		struct KnifeQuickTurn
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.ecx = *(uint32_t*)(regs.esi + 0x7D8);

				if (pConfig->bCameraImprovements && isKeyboardMouse())
				{
					PlayerPtr()->ang_A0.y += fCameraPosX_backup;
					*fMousePosX += fCameraPosX_backup;

					// Reset fCameraPosX_backup after it has been used.
					fCameraPosX_backup = 0.0f;
				}
			}
		}; injector::MakeInline<KnifeQuickTurn>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Hook knife_r3_down00 to make m_Work0_428 0 instead of 1. Keeps the camera/aim position from being reset in certain situations.
		pattern = hook::pattern("c7 86 ? ? ? ? ? ? ? ? eb ? e8 ? ? ? ? 8b 75");
		struct knife_r3_down00_Work
		{
			void operator()(injector::reg_pack& regs)
			{
				if (pConfig->bCameraImprovements && isKeyboardMouse())
				{
					*(uint32_t*)(regs.esi + 0x428) = 0;
				}
				else
					*(uint32_t*)(regs.esi + 0x428) = 1;
			}
		}; injector::MakeInline<knife_r3_down00_Work>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));

		// Keep the knife from writing 0 to Stick*Float when the animation ends
		pattern = hook::pattern("D9 15 ? ? ? ? 89 0D ? ? ? ? D9 15 ? ? ? ? 89 0D ? ? ? ? 89 0D");
		struct PadReadKnifeResetX
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bCameraImprovements || isController())
				{
					*fMousePosX = 0.0f;
				}
			}
		}; injector::MakeInline<PadReadKnifeResetX>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 15 ? ? ? ? 89 0D ? ? ? ? 89 0D ? ? ? ? 66 C7 05 ? ? ? ? ? ? 66");
		struct PadReadKnifeResetY
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bCameraImprovements || isController())
				{
					*fMousePosY = 0.0f;
				}
			}
		}; injector::MakeInline<PadReadKnifeResetY>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
}