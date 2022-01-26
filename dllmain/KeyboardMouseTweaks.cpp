#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "MouseTurning.h"

uintptr_t* ptrRifleMovAddr;
uintptr_t* ptrInvMovAddr;
uintptr_t* ptrFocusAnimFldAddr;
uintptr_t ptrRetryLoadDLGstate;

static uint32_t* ptrLastUsedController;

int intLastUsedController()
{
	// 2 = Controller
	return *(int32_t*)(ptrLastUsedController);
}

void Init_KeyboardMouseTweaks()
{
	auto pattern = hook::pattern("A1 ? ? ? ? 85 C0 74 ? 83 F8 ? 74 ? 81 F9");
	ptrLastUsedController = *pattern.count(1).get(0).get<uint32_t*>(1);

	Init_MouseTurning();

	// Inventory item flip binding
	pattern = hook::pattern("A1 ? ? ? ? 75 ? A8 ? 74 ? 6A ? 8B CE E8 ? ? ? ? BB");
	ptrInvMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct InvFlip
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int32_t*)ptrInvMovAddr;

			if (bShouldFlipX)
			{
				regs.eax = 0x00300000;
				bShouldFlipX = false;
			}

			if (bShouldFlipY)
			{
				regs.eax = 0x00C00000;
				bShouldFlipY = false;
			}
		}
	}; injector::MakeInline<InvFlip>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Prevent the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action.
	{
		// Get pointer for the state. Only reliable way I found to achieve this.
		pattern = hook::pattern("C7 06 ? ? ? ? A1 ? ? ? ? F7 80 ? ? ? ? ? ? ? ? 74"); //0x48C1C0
		struct RLDLGState
		{
			void operator()(injector::reg_pack& regs)
			{
				*(int*)(regs.esi) = 0;
				ptrRetryLoadDLGstate = regs.esi + 0x3;
			}
		}; injector::MakeInline<RLDLGState>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Check if the "yes/no" prompt is open before sending any index updates
		pattern = hook::pattern("89 8F ? ? ? ? FF 85 ? ? ? ? 83 C6 ? 3B 77");
		struct MouseMenuSelector
		{
			void operator()(injector::reg_pack& regs)
			{
				if (cfg.bFixRetryLoadMouseSelector)
				{
					if (*(int32_t*)ptrRetryLoadDLGstate != 1)
					{
						*(int32_t*)(regs.edi + 0x160) = regs.ecx;
					}
				}
				else
				{
					*(int32_t*)(regs.edi + 0x160) = regs.ecx;
				}
			}
		}; injector::MakeInline<MouseMenuSelector>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Fix camera after zooming with the sniper
	{
		auto pattern = hook::pattern("A2 ? ? ? ? A2 ? ? ? ? EB ? 81 FB ? ? ? ? 75 ? 83");
		ptrRifleMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
		struct FixSniperZoom
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!cfg.bFixSniperZoom)
					*(int32_t*)ptrRifleMovAddr = regs.eax;
			}
		}; injector::MakeInline<FixSniperZoom>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	}

	// Fix the "focus animation" not looking as strong as when triggered with a controller
	if (cfg.bFixSniperFocus)
	{
		auto pattern = hook::pattern("D9 05 ? ? ? ? 8A 15 ? ? ? ? 56 8B F1");
		ptrFocusAnimFldAddr = *pattern.count(1).get(0).get<uint32_t*>(2);
		struct FixScopeZoomFocus
		{
			void operator()(injector::reg_pack& regs)
			{
				if (intLastUsedController() != 2)
					*(float*)ptrFocusAnimFldAddr = 12.0f; // Makes the focus animation last longer when using the mouse.
				else
					*(float*)ptrFocusAnimFldAddr = 5.0f;

				float focus_frame = *(float*)ptrFocusAnimFldAddr;

				_asm
				{
					fld focus_frame
				}
			}
		}; injector::MakeInline<FixScopeZoomFocus>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	
		// This jl instruction makes the focus animation stop almost immediately when using the mouse. Noping it doesn't seem to affect the controller at all.
		pattern = hook::pattern("7C ? C6 06 ? EB ? C7 46 ? ? ? ? ? EB ? DD D8 83 3D");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 2, true);
	}
}