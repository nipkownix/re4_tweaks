#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"

bool bIsThinking;

void Init_UI_MouseTweaks()
{

	auto pattern = hook::pattern("8b 97 ? ? ? ? 6a ? 50 89 4d");
	struct pzzlCursorDisp_hook
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.edx = *(int32_t*)(regs.edi + 0x2AC);


			// stuff

		}
	}; injector::MakeInline<pzzlCursorDisp_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));



	// Hooks to determine wheater or not the "extra" left-side board is showing.
	// Ideally we'd use IDSystem::unitPtr to get it, but this is easier.
	pattern = hook::pattern("80 88 ? ? ? ? ? 6a ? 6a ? b9 ? ? ? ? e8 ? ? ? ? 80 08 ? 80 a0");
	struct PzzlThinking_init_hook
	{
		void operator()(injector::reg_pack& regs)
		{
			*(uint8_t*)(regs.eax + 0x8A) |= 0xF;

			bIsThinking = true;
		}
	}; injector::MakeInline<PzzlThinking_init_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	pattern = hook::pattern("80 88 ? ? ? ? ? 6A 01 56 E8 ? ? ? ? 83 C4 08 B0 01");
	struct back2PieceSelect_hook
	{
		void operator()(injector::reg_pack& regs)
		{
			*(uint8_t*)(regs.eax + 0x8A) |= 0xF;

			bIsThinking = false;
		}
	}; injector::MakeInline<back2PieceSelect_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));





}