#include <iostream>
#include "stdafx.h"
#include "Game.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "Logging/Logging.h"

static uint32_t* ptrGameFrameRate;

int intCurrentFrameRate()
{
	return *(int32_t*)(ptrGameFrameRate);
}

uint32_t ModelForceRenderAll_EndTick = 0;

// Called by AddOtModelPosRadius to check if model is in view of camera
bool(*collision_sphere_hexahedron)(void*, void*);
bool collision_sphere_hexahedron_Hook(void* a1, void* a2)
{
	if (ModelForceRenderAll_EndTick > GlobalPtr()->frameCounter_530C)
		return true;
	return collision_sphere_hexahedron(a1, a2);
}

void(*NowLoadingOff)();
void NowLoadingOff_Hook()
{
	NowLoadingOff();

	// Enable model hack for 1 tick after loading
	// Forces game to fully process all models (rendering etc)
	// Without this game could lag significantly when many newly-seen models are on screen
	// By forcing this processing for all models, it seems something is cached
	// which stops lag from occurring when the models are viewed later on
	// (by enabling this hack after load screen, the player will also view any perf drop from it as part of loading too :)
	if (cfg.bPrecacheModels)
		ModelForceRenderAll_EndTick = GlobalPtr()->frameCounter_530C + 1;
}

void Init_60fpsFixes()
{
	auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
	ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);

	// Treasure items fall speed
	{
		auto pattern = hook::pattern("DC 25 ? ? ? ? 6A ? 83 EC ? 8D BE ? ? ? ? D9 9E ? ? ? ? B9");
		struct TreasureSpeed
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaMulti = 10.0;
				float newMulti = static_cast<float>(30) / intCurrentFrameRate() * 10;

				if (cfg.bFixFallingItemsSpeed)
					_asm {fsub newMulti}
				else
					_asm {fsub vanillaMulti}
			}
		}; injector::MakeInline<TreasureSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Ammo boxes fall speed
	{
		auto pattern = hook::pattern("DC 0D ? ? ? ? D8 83 ? ? ? ? D9 9B ? ? ? ? D9 83");
		struct AmmoBoxSpeed
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaMulti = 10.0;
				float newMulti = static_cast<float>(30) / intCurrentFrameRate() * 10;

				if (cfg.bFixFallingItemsSpeed)
					_asm {fmul newMulti}
				else
					_asm {fmul vanillaMulti}
			}
		}; injector::MakeInline<AmmoBoxSpeed>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));
	}

	// Fix character backwards turning speed
	{
		struct TurnSpeedSubtract
		{
			void operator()(injector::reg_pack& regs)
			{
				float cPlayer__SPEED_WALK_TURN = 0.04188790545f; // game calcs this on startup, always seems to be same value
				float newTurnSpeed = GlobalPtr()->deltaTime_70 * cPlayer__SPEED_WALK_TURN;

				if (cfg.bFixTurningSpeed)
					_asm {fsub newTurnSpeed}
				else
					_asm {fsub cPlayer__SPEED_WALK_TURN}
			}
		};

		pattern = hook::pattern("D8 25 ? ? ? ? D9 ? A4 00 00 00");

		// 0x7636d9 - pl_R1_Back
		// 0x766069 - pl_R1_Crouch
		// 0x7660b6 - pl_R1_Crouch
		// 0x766167 - pl_R1_Crouch
		// 0x7661b4 - pl_R1_Crouch
		// 0x9001a3 - pl_R1_KlauserAttack
		for (int i = 0; i < 6; i++)
			injector::MakeInline<TurnSpeedSubtract>(pattern.count(6).get(i).get<uint32_t>(0), pattern.count(6).get(i).get<uint32_t>(6));

		struct TurnSpeedAdd
		{
			void operator()(injector::reg_pack& regs)
			{
				float cPlayer__SPEED_WALK_TURN = 0.04188790545f; // game calcs this on startup, always seems to be same value
				float newTurnSpeed = GlobalPtr()->deltaTime_70 * cPlayer__SPEED_WALK_TURN;

				if (cfg.bFixTurningSpeed)
					_asm {fadd newTurnSpeed}
				else
					_asm {fadd cPlayer__SPEED_WALK_TURN}
			}
		};

		pattern = hook::pattern("D8 05 ? ? ? ? D9 ? A4 00 00 00");

		// 0x7636fa - pl_R1_Back
		// 0x76607b - pl_R1_Crouch
		// 0x7660a4 - pl_R1_Crouch
		// 0x766179 - pl_R1_Crouch
		// 0x7661a2 - pl_R1_Crouch
		for (int i = 0; i < 5; i++)
			injector::MakeInline<TurnSpeedAdd>(pattern.count(5).get(i).get<uint32_t>(0), pattern.count(5).get(i).get<uint32_t>(6));

		struct TurnSpeedLoad
		{
			void operator()(injector::reg_pack& regs)
			{
				float cPlayer__SPEED_WALK_TURN = 0.04188790545f; // game calcs this on startup, always seems to be same value
				float newTurnSpeed = GlobalPtr()->deltaTime_70 * cPlayer__SPEED_WALK_TURN;

				if (cfg.bFixTurningSpeed)
					_asm {fld newTurnSpeed}
				else
					_asm {fld cPlayer__SPEED_WALK_TURN}
			}
		};

		pattern = hook::pattern("D9 05 ? ? ? ? D8 86 A4 00 00 00 D9 9E A4 00 00 00");

		// 0x9001C0 - pl_R1_KlauserAttack
		injector::MakeInline<TurnSpeedLoad>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Copy delta-time related code from cSubChar::moveBust to cPlAshley::moveBust
	// Seems to make Ashley bust physics speed match between 30 & 60FPS
	{
		auto pattern = hook::pattern("57 E8 ? ? ? ? 8B 06 8B 50 0C 00 1D");

		// Hook struct
		struct AshleyBustFrametimeFix
		{
			void operator()(injector::reg_pack& regs)
			{
				if (cfg.bFixAshleyBustPhysics)
				{
					// Update ebx to make use of delta-time from pG+0x70
					float ebx = float(regs.ebx);
					ebx *= GlobalPtr()->deltaTime_70;
					regs.ebx = uint32_t(ebx);
				}

				// Code that we overwrote
				regs.eax = *(uint32_t*)regs.esi;
				regs.edx = *(uint32_t*)(regs.eax + 0xC);
			}
		}; injector::MakeInline<AshleyBustFrametimeFix>(pattern.count(1).get(0).get<uint32_t>(6), pattern.count(1).get(0).get<uint32_t>(11));
	}

	// Workaround for lag spike when many models are first shown on screen
	// forces game to treat all models as on-screen for a single tick after loading
	// allowing game to process/cache whatever data was causing lag spike
	pattern = hook::pattern("8B 4F 08 50 8D 55 ? 52 89 4D ? E8 ? ? ? ?"); // AddOtModelPosRadius
	auto caller = pattern.count(2).get(1).get<uint8_t>(0xB);
	ReadCall(caller, collision_sphere_hexahedron);
	InjectHook(caller, collision_sphere_hexahedron_Hook, PATCH_CALL);

	pattern = hook::pattern("39 91 80 84 00 00 0F 85 ? ? ? ? E8 ? ? ? ?"); // gameStageInit
	caller = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xC)).as_int();
	ReadCall(caller, NowLoadingOff);
	InjectHook(caller, NowLoadingOff_Hook, PATCH_JUMP);

	pattern = hook::pattern("D8 D1 DF E0 DD D9 F6 C4 41 75 ? D9 C0 D9 EE DA E9 DF E0 F6 C4 44 7A");

	struct ModelRenderDistHack
	{
		void operator()(injector::reg_pack& regs)
		{
			// original code we patched over...
			__asm
			{
				fcom st(1)
				fnstsw ax
				fstp st(1)
				mov ecx, [regs]
				mov[ecx]regs.eax, eax
			}
			// distance check skip
			if (ModelForceRenderAll_EndTick > GlobalPtr()->frameCounter_530C)
				regs.eax = 0x3125; // passes "test ah, 41h"
		}
	}; injector::MakeInline<ModelRenderDistHack>(pattern.count(2).get(1).get<uint8_t>(0), pattern.count(2).get(1).get<uint8_t>(6));

	Logging::Log() << __FUNCTION__ << " -> FPS fixes applied";
}
