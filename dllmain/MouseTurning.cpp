#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "KeyboardMouseTweaks.h"

uintptr_t* ptrCamXmovAddr;
uintptr_t* ptrKnife_r3_downMovAddr;

uintptr_t ptrPSVECAdd;
uintptr_t ptrAfterTurnRightAnimHook;
uintptr_t ptrAfterTurnLeftAnimHook;
uintptr_t ptrAfterMotionMoveHook1;
uintptr_t ptrAfterMotionMoveHook2;

static uint32_t* ptrMouseDeltaX;
static uint32_t* ptrMovInputState;
static uint32_t* ptrMouseAimMode;

uint32_t* ptrCharRotationBase;

bool isTurn;

DWORD _EAX;
DWORD _ECX;
DWORD _ESP;

int intMouseDeltaX()
{
	// TODO: Maybe we'd get smoother results if we calculated the delta ourselves instead of using what the game provides.
    return *(int32_t*)(ptrMouseDeltaX);
}

int intMovInputState()
{
    return *(int8_t*)(ptrMovInputState);
}

std::vector<uint32_t> mouseTurnModifierCombo = { VK_LMENU }; // left ALT

bool ParseMouseTurnModifierCombo(std::string_view in_combo)
{
	mouseTurnModifierCombo = ParseKeyCombo(in_combo);
	return mouseTurnModifierCombo.size() > 0;
}

bool bPrevMouseTurnState = false;
bool bMouseTurnStateChanged = false;

bool MouseTurnEnabled()
{
	bool modifierPressed = mouseTurnModifierCombo.size() > 0;
	for (auto& key : mouseTurnModifierCombo)
	{
		if (!GetAsyncKeyState(key))
		{
			modifierPressed = false;
			break;
		}
	}

	bool state = cfg.bUseMouseTurning && !modifierPressed;
	bMouseTurnStateChanged = bPrevMouseTurnState != state;
	bPrevMouseTurnState = state;

	return state;
}

// Enable the turning animation if the mouse is moving and we're not
// trying to walk backwards / forwards, or run.
void __declspec(naked) TurnRightAnimHook()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _ESP, esp
	}

	if ((GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
	{
		if (MouseTurnEnabled() && (intMouseDeltaX() > 0))
			if ((intMovInputState() != 0x01) && (intMovInputState() != 0x02) && (intMovInputState() != 0x41) && (intMovInputState() != 0x42))
				_EAX = 0x1;
	}

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

// Enable the turning animation if the mouse is moving and we're not
// trying to walk backwards / forwards, or run.
void __declspec(naked) TurnLeftAnimHook()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _ESP, esp
	}

	if ((GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
	{
		if (MouseTurnEnabled() && (intMouseDeltaX() < 0))
			if ((intMovInputState() != 0x01) && (intMovInputState() != 0x02) && (intMovInputState() != 0x41) && (intMovInputState() != 0x42))
				_EAX = 0x1;
	}

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

// Only allow MotionMove to change the character's position if we're not trying to do so with the mouse
void __declspec(naked) MotionMoveHook1()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _ESP, esp
	}

	if ((GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController))
	{
		_asm {call ptrPSVECAdd}
	}
	else
	{
		if (MouseTurnEnabled())
		{
			if (isTurn && (intMovInputState() != 0x00))
				_asm {call ptrPSVECAdd}
			else if (!isTurn)
				_asm {call ptrPSVECAdd}
		}
		else
			_asm {call ptrPSVECAdd}
	}

	_asm
	{
		mov eax, _EAX
		mov ecx, _ECX
		mov esp, _ESP

		jmp ptrAfterMotionMoveHook1
	}
}

// Only allow MotionMove to change the character's rotation if we're not trying to do so with the mouse
void __declspec(naked) MotionMoveHook2()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _ESP, esp
	}

	if ((GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController))
	{
		_asm {call ptrPSVECAdd}
	}
	else
	{
		if (MouseTurnEnabled())
		{
			if (isTurn && (intMouseDeltaX() == 0))
				_asm {call ptrPSVECAdd}
			else if (!isTurn)
				_asm {call ptrPSVECAdd}
		}
		else
			_asm {call ptrPSVECAdd}
	}

	isTurn = false;

	_asm
	{
		mov eax, _EAX
		mov ecx, _ECX
		mov esp, _ESP

		jmp ptrAfterMotionMoveHook2
	}
}

void MouseTurn()
{
	float SpeedMulti = 900;

	// "Classic" aiming mode (0x00) needs lower sensitivity here.
	if (*(int8_t*)(ptrMouseAimMode) == 0x00)
		SpeedMulti = 1300;

	*(float*)(*ptrCharRotationBase + 0xA4) += (-intMouseDeltaX() / SpeedMulti) * cfg.fTurnSensitivity;
}

void GetMouseTurnPointers()
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

void Init_MouseTurning()
{
	GetMouseTurnPointers();

	// Camera control
	auto pattern = hook::pattern("A2 ? ? ? ? EB ? DD D8 8B 0D ? ? ? ? 8B 81 ? ? ? ? A9");
	ptrCamXmovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct CameraControl1
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!MouseTurnEnabled() || (GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController))
				*(int8_t*)(ptrCamXmovAddr) = (int8_t)regs.eax;
			if (bMouseTurnStateChanged)
				*(int8_t*)(ptrCamXmovAddr) = 0;
		}
	}; injector::MakeInline<CameraControl1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	pattern = hook::pattern("C6 05 ? ? ? ? ? EB ? 83 F8 ? 7D ? C6 05 ? ? ? ? ? EB ? E8");
	struct CameraControl2
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!MouseTurnEnabled() || (GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController))
				*(int8_t*)(ptrCamXmovAddr) = 0x7F;
			if (bMouseTurnStateChanged)
				*(int8_t*)(ptrCamXmovAddr) = 0;
		}
	}; injector::MakeInline<CameraControl2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	pattern = hook::pattern("C6 05 ? ? ? ? ? EB ? E8 ? ? ? ? EB");
	struct CameraControl3
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!MouseTurnEnabled() || (GetLastUsedDevice() == LastDevice::XinputController) || (GetLastUsedDevice() == LastDevice::DinputController))
				*(int8_t*)(ptrCamXmovAddr) = -0x7F;
			if (bMouseTurnStateChanged)
				*(int8_t*)(ptrCamXmovAddr) = 0;
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

			if ((MouseTurnEnabled() || bMouseTurnStateChanged) && (GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
				regs.edi = 1;
		}
	}; injector::MakeInline<CameraAutoCenter>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Turning animations
	// Trigger left turn
	pattern = hook::pattern("83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 8A 86");
	ptrAfterTurnLeftAnimHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), TurnLeftAnimHook, true);

	// Trigger right turn
	pattern = hook::pattern("83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 53");
	ptrAfterTurnRightAnimHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), TurnRightAnimHook, true);

	// MotionMove hook 1
	pattern = hook::pattern("E8 ? ? ? ? 8D 83 ? ? ? ? 50 8D 8D");
	ptrAfterMotionMoveHook1 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), MotionMoveHook1, true);

	// MotionMove hook 2
	pattern = hook::pattern("E8 ? ? ? ? 81 A3 ? ? ? ? ? ? ? ? 8B 0D");
	ptrPSVECAdd = injector::GetBranchDestination(pattern.get_first(0)).as_int();
	ptrAfterMotionMoveHook2 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), MotionMoveHook2, true);

	// Actual turning
	// pl_R1_Turn
	pattern = hook::pattern("0F B6 86 ? ? ? ? 83 F8 ? 0F 87 ? ? ? ? 53 33 DB FF 24 85 ? ? ? ? 8B 86 ? ? ? ? 8B 88");
	struct TurnHookStill
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int8_t*)(regs.esi + 0xFE);

			if (MouseTurnEnabled() && (GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
				MouseTurn();
		}
	}; injector::MakeInline<TurnHookStill>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	// pl_R1_Turn180
	pattern = hook::pattern("0F B6 86 ? ? ? ? 33 DB 2B C3 74 ? 48 74 ? 8B 16");
	injector::MakeInline<TurnHookStill>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	// Makes it so the full animation is played after mousedelta is past a certain value
	pattern = hook::pattern("A1 ? ? ? ? 83 E0 ? 33 C9 0B C1 75 ? 89 9E");
	struct pl_R1_Turn_left
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = intMovInputState();

			if (MouseTurnEnabled() && (intMouseDeltaX() < -8))
				regs.eax = 0x8;

			isTurn = true;
		}
	}; injector::MakeInline<pl_R1_Turn_left>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	pattern = hook::pattern("A1 ? ? ? ? 83 E0 ? 33 C9 0B C1 75 ? 89 8E");
	struct pl_R1_Turn_right
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = intMovInputState();

			if (MouseTurnEnabled() && (intMouseDeltaX() > 8))
				regs.eax = 0x4;

			isTurn = true;
		}
	}; injector::MakeInline<pl_R1_Turn_right>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Walk/Run struct
	struct TurnHookWalkRun
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebp = *(int32_t*)(regs.esp);
			*(int32_t*)(regs.esp) -= 0x8;

			if (MouseTurnEnabled() && (GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
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
			if (MouseTurnEnabled() && (GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
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
			if (MouseTurnEnabled() && (intMouseDeltaX() != 0) && (GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
				regs.eax = 0x8;
		}
	}; injector::MakeInline<Knife_r3_downHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
}