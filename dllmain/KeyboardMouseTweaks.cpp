#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"

static uint32_t* ptrMouseDeltaX;
static uint32_t* ptrMovInputState;
static uint32_t* ptrMouseAimMode;

uintptr_t* ptrCamXmovAddr;
uintptr_t* ptrRifleMovAddr;
uint32_t* ptrCharRotationBase;

uintptr_t ptrAfterTurnRightAnimHook;
uintptr_t ptrAfterTurnLeftAnimHook;
uintptr_t ptrRetryLoadDLGstate;

uintptr_t* ptrInvMovAddr;
uintptr_t* ptrKnife_r3_downMovAddr;

DWORD _EAX;
DWORD _ECX;
DWORD _ESP;

int mousedelta;
int MovInputState;

void __declspec(naked) TurnRightAnimHook()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _ESP, esp
	}

	mousedelta = *(int32_t*)(ptrMouseDeltaX);
	MovInputState = *(int8_t*)(ptrMovInputState);

	// Enable the turning animation if the mouse is moving and we're not
	// trying to walk backwards / forwards, or run.
	if ((cfg.bUseMouseTurning) && (mousedelta > 0))
		if ((MovInputState != 0x01) && (MovInputState != 0x02) && (MovInputState != 0x41) && (MovInputState != 0x42))
			_EAX = 0x1;

	_asm
	{
		mov eax, _EAX
		mov ecx, _ECX
		mov esp, _ESP

		add esp, 0x8
		test al, al

		jmp ptrAfterTurnRightAnimHook
	}
}

void __declspec(naked) TurnLeftAnimHook()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _ESP, esp
	}

	mousedelta = *(int32_t*)(ptrMouseDeltaX);
	MovInputState = *(int8_t*)(ptrMovInputState);

	// Enable the turning animation if the mouse is moving and we're not
	// trying to walk backwards / forwards, or run.
	if ((cfg.bUseMouseTurning) && (mousedelta < 0))
		if ((MovInputState != 0x01) && (MovInputState != 0x02) && (MovInputState != 0x41) && (MovInputState != 0x42))
			_EAX = 0x1;

	_asm
	{
		mov eax, _EAX
		mov ecx, _ECX
		mov esp, _ESP

		add esp, 0x8
		test al, al

		jmp ptrAfterTurnLeftAnimHook
	}
}

void GetKBMPointers()
{
	auto pattern = hook::pattern("DB 05 ? ? ? ? D9 45 ? D9 C0 DE CA D9 C5");
	ptrMouseDeltaX = *pattern.count(1).get(0).get<uint32_t*>(2);

	pattern = hook::pattern("A1 ? ? ? ? 25 ? ? ? ? 33 C9 0B C1 74 ? B8 ? ? ? ? C3");
	ptrMovInputState = *pattern.count(1).get(0).get<uint32_t*>(1);

	pattern = hook::pattern("80 3D ? ? ? ? ? 0F B6 05");
	ptrMouseAimMode = *pattern.count(1).get(0).get<uint32_t*>(2);

	pattern = hook::pattern("A1 ? ? ? ? D9 80 ? ? ? ? 51 8D 90 ? ? ? ? D9 1C ? E8 ? ? ? ? 83 C4 ? 84 C0 0F 85 ? ? ? ? 8B 45 ? 8B 7D");
	ptrCharRotationBase = *pattern.count(1).get(0).get<uint32_t*>(1);
}

void MouseTurn()
{
	// TODO: Maybe we'd get smoother results if we calculated the delta ourselves instead of using what the game provides.
	int mousedelta = *(int32_t*)(ptrMouseDeltaX);

	float SpeedMulti = 700;

	// "Classic" aiming mode (0x00) needs lower sensitivity here.
	if (*(int8_t*)(ptrMouseAimMode) == 0x00)
		SpeedMulti = 1100;

	*(float*)(*ptrCharRotationBase + 0xA4) += -mousedelta / SpeedMulti;
}

void Init_KeyboardMouseTweaks()
{
	GetKBMPointers();

	///
	// bUseMouseTurning begin
	///

	// Camera control
	auto pattern = hook::pattern("A2 ? ? ? ? EB ? DD D8 8B 0D ? ? ? ? 8B 81 ? ? ? ? A9");
	ptrCamXmovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct CameraControl1
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!cfg.bUseMouseTurning)
				*(int8_t*)(ptrCamXmovAddr) = (int8_t)regs.eax;
		}
	}; injector::MakeInline<CameraControl1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	pattern = hook::pattern("C6 05 ? ? ? ? ? EB ? 83 F8 ? 7D ? C6 05 ? ? ? ? ? EB ? E8");
	struct CameraControl2
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!cfg.bUseMouseTurning)
				*(int8_t*)(ptrCamXmovAddr) = 0x7F;
		}
	}; injector::MakeInline<CameraControl2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	pattern = hook::pattern("C6 05 ? ? ? ? ? EB ? E8 ? ? ? ? EB");
	struct CameraControl3
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!cfg.bUseMouseTurning)
				*(int8_t*)(ptrCamXmovAddr) = -0x7F;
		}
	}; injector::MakeInline<CameraControl3>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	// Always center the camera behind the character
	pattern = hook::pattern("8B 16 8B 42 ? 8B CE FF D0 8B CE E8 ? ? ? ? 8B CE E8 ? ? ? ? 8B CE");
	struct CameraAutoCenter
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.edx = *(int32_t*)(regs.esi);
			regs.eax = *(int32_t*)(regs.edx + 0x54);

			if (cfg.bUseMouseTurning)
				regs.edi = 1;
		}
	}; injector::MakeInline<CameraAutoCenter>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Turning animations
	pattern = hook::pattern("83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 8A 86");
	ptrAfterTurnLeftAnimHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), TurnLeftAnimHook, true);

	pattern = hook::pattern("83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 53");
	ptrAfterTurnRightAnimHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), TurnRightAnimHook, true);

	// Actual turning
	// pl_R1_Turn
	pattern = hook::pattern("0F B6 86 ? ? ? ? 83 F8 ? 0F 87 ? ? ? ? 53 33 DB FF 24 85 ? ? ? ? 8B 86 ? ? ? ? 8B 88");
	struct TurnHookStill
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int8_t*)(regs.esi + 0xFE);

			if (cfg.bUseMouseTurning)
				MouseTurn();
		}
	}; injector::MakeInline<TurnHookStill>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
	
	// Walk/Run struct
	struct TurnHookWalkRun
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebp = *(int32_t*)(regs.esp);
			*(int32_t*)(regs.esp) -= 0x8;

			if (cfg.bUseMouseTurning)
				MouseTurn();
		}
	};
	
	// pl_R1_Walk
	pattern = hook::pattern("8B EC 83 EC ? 53 56 8B 75 ? 33 DB 38 9E");
	injector::MakeInline<TurnHookWalkRun>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// pl_R1_Run
	pattern = hook::pattern("8B EC 83 EC ? 53 56 8B 75 ? 0F B6 86 ? ? ? ? 33 DB");
	injector::MakeInline<TurnHookWalkRun>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// pl_R1_Back
	pattern = hook::pattern("83 3D ? ? ? ? ? 75 ? D9 05 ? ? ? ? 8B 8E ? ? ? ? 83 EC ? D9");
	struct TurnHookWalkingBack
	{
		void operator()(injector::reg_pack& regs)
		{
			if (cfg.bUseMouseTurning)
				MouseTurn();
		}
	}; injector::MakeInline<TurnHookWalkingBack>(pattern.count(3).get(1).get<uint32_t>(0), pattern.count(3).get(1).get<uint32_t>(7));
	injector::WriteMemory(pattern.count(3).get(1).get<uint32_t>(7), uint8_t(0xEB), true);

	// Cancel the Knife_r3_down animation if we move the mouse (indicating we want to turn ASAP)
	pattern = hook::pattern("A1 ? ? ? ? 25 ? ? ? ? 33 C9 0B C1 74 ? 8B 86 ? ? ? ? 39 48");
	ptrKnife_r3_downMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct Knife_r3_downHook
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int32_t*)ptrKnife_r3_downMovAddr;

			mousedelta = *(int32_t*)(ptrMouseDeltaX);

			if ((cfg.bUseMouseTurning) && (mousedelta != 0))
				regs.eax = 0x8;
		}
	}; injector::MakeInline<Knife_r3_downHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	///
	// bUseMouseTurning end
	///

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
}