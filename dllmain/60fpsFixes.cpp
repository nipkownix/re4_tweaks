#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"

static uint32_t* ptrGameFrameRate;

int intCurrentFrameRate()
{
	return *(int32_t*)(ptrGameFrameRate);
}

double* ptrMaxFrameTime = nullptr;

struct INIConfig
{
	int resolutionWidth;
	int resolutionHeight;
	int resolutionRefreshRate;
	int vsync;
	int fullscreen;
	int adapter;
	int shadowQuality;
	int motionblurEnabled;
	int ppEnabled;
	int useHDTexture;
	int antialiasing;
	int anisotropy;
	int variableframerate;
	int subtitle;
	int laserR;
	int laserG;
	int laserB;
	int laserA;
};
void(_cdecl* ConfigReadINI)(INIConfig* config);

void Init_60fpsFixes()
{
	auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
	ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);

	// Treasure items fall speed
	pattern = hook::pattern("DC 25 ? ? ? ? 6A ? 83 EC ? 8D BE ? ? ? ? D9 9E ? ? ? ? B9");
	struct TreasureSpeed
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaMulti = 10.0;
			float newMulti = static_cast<float>(30) / intCurrentFrameRate() * 10;

			if (cfg.bFixFallingItemsSpeed)
				_asm{fsub newMulti}
			else
				_asm {fsub vanillaMulti}
		}
	}; injector::MakeInline<TreasureSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Ammo boxes fall speed
	pattern = hook::pattern("DC 0D ? ? ? ? D8 83 ? ? ? ? D9 9B ? ? ? ? D9 83");
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

	// Fix games framelimiter to improve support for framerates greater than 60
	pattern = hook::pattern("00 00 00 20 11 11 91 3F"); // default is 0.1666666..., luckily only one instance of it in the game EXE
	ptrMaxFrameTime = pattern.count(1).get(0).get<double>();

	// Hook call to ConfigReadINI so we can update frametime after INI has been read from
	pattern = hook::pattern("89 95 ? ? ? ? 89 85 ? ? ? ? E8 ? ? ? ? 83 C4 04 6A 00");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(12)).as_int(), ConfigReadINI);
	ConfigReadINI = ConfigReadINI;
	struct MaxFrameTime
	{
		void operator()(injector::reg_pack& regs)
		{
			INIConfig* config = (INIConfig*)regs.ecx;
			ConfigReadINI(config);

			if (config->variableframerate > 60)
			{
				injector::WriteMemory(ptrMaxFrameTime, (double)1 / (double)config->variableframerate, true);
			}
		}
	}; injector::MakeInline<MaxFrameTime>(pattern.count(1).get(0).get<uint32_t>(12));

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
					extern uint32_t* ptrpG;
					uint8_t* Global = *(uint8_t**)ptrpG;

					// Update ebx to make use of delta-time from pG+0x70
					float ebx = float(regs.ebx);
					ebx *= *(float*)(Global + 0x70);
					regs.ebx = uint32_t(ebx);
				}

				// Code that we overwrote
				regs.eax = *(uint32_t*)regs.esi;
				regs.edx = *(uint32_t*)(regs.eax + 0xC);
			}
		}; injector::MakeInline<AshleyBustFrametimeFix>(pattern.count(1).get(0).get<uint32_t>(6), pattern.count(1).get(0).get<uint32_t>(11));
	}
}