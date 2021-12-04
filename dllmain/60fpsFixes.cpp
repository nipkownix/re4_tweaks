#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"

static uint32_t* ptrGameFrameRate;

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
			int curFPS = *(int32_t*)(ptrGameFrameRate);
			float newMulti = static_cast<float>(30) / curFPS * 10;

			if (cfg.b60fpsFixes)
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
			int curFPS = *(int32_t*)(ptrGameFrameRate);
			float newMulti = static_cast<float>(30) / curFPS * 10;

			if (cfg.b60fpsFixes)
				_asm {fmul newMulti}
			else
				_asm {fmul vanillaMulti}
		}
	}; injector::MakeInline<AmmoBoxSpeed>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));

	// Aiming speed
	pattern = hook::pattern("DD 05 ? ? ? ? DC FA D9 01 DE CB D9 45");
	struct AimSpeed1
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaMulti = 10.0;
			int curFPS = *(int32_t*)(ptrGameFrameRate);
			float newMulti = static_cast<float>(30) / curFPS * 10;

			if (cfg.b60fpsFixes)
				_asm {fld newMulti}
			else
				_asm {fld vanillaMulti}
		}
	}; injector::MakeInline<AimSpeed1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	pattern = hook::pattern("DC 35 ? ? ? ? D8 F6 D8 F4 D8 49 ? DE EA");
	struct AimSpeed2
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaMulti = 10.0;
			int curFPS = *(int32_t*)(ptrGameFrameRate);
			float newMulti = static_cast<float>(30) / curFPS * 10;

			if (cfg.b60fpsFixes)
				_asm {fdiv newMulti}
			else
				_asm {fdiv vanillaMulti}
		}
	}; injector::MakeInline<AimSpeed2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	pattern = hook::pattern("DC 35 ? ? ? ? DE F3 D9 CA E9 ? ? ? ? D9");
	injector::MakeInline<AimSpeed2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
}