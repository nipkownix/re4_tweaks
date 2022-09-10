#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Settings.h"
#include "input.hpp"

uintptr_t* ptrKnife_r3_downMovAddr;

int32_t* MouseDeltaX;
int32_t* MouseDeltaY;

std::vector<uint32_t> mouseTurnModifierCombo;

bool ParseMouseTurnModifierCombo(std::string_view in_combo)
{
	if (in_combo.empty())
		return false;

	mouseTurnModifierCombo.clear();
	mouseTurnModifierCombo = ParseKeyCombo(in_combo);

	return mouseTurnModifierCombo.size() > 0;
}

bool bPrevMouseTurnState = false;
bool bMouseTurnStateChanged = false;

bool isMouseTurnEnabled()
{
	bool modifierPressed = pInput->is_combo_down(&mouseTurnModifierCombo);

	bool state = pConfig->bUseMouseTurning;

	// Invert mouse turning setting if modifier pressed
	// If mouse turning enabled: modifier disables turning, allows camera movement
	// If mouse turning disabled: modifier enables mouse turning temporarily
	// (if user wants to completely disable mouse turning, they can clear the combination)
	if (modifierPressed)
		state = !state;

	bMouseTurnStateChanged = bPrevMouseTurnState != state;

	// Reset camera position if modifier is pressed
	if (bMouseTurnStateChanged)
	{
		*fMousePosX = 0.0f;
		*AnalogRX_8 = 0;
	}

	bPrevMouseTurnState = state;

	return state;
}

void MouseTurn(float TurnDelayFactor = 20.0f)
{
	if (isController())
		return;

	if (!isMouseTurnEnabled())
		return;

	if ((pConfig->iMouseTurnType == MouseTurnTypes::TypeA))
	{
		// This has to be changed based on C_RANGE_1097
		float fRangeMulti;
		if (pConfig->bCameraImprovements)
			fRangeMulti = 3.0f;
		else
			fRangeMulti = 2.0f;

		// TurnDelayFactor is used to alter how long it will take for the player rotation to reach the camera rotation.
		// Higher values = slower speed.
		float factor = *fCameraPosX / TurnDelayFactor;

		PlayerPtr()->ang_A0.y += factor;
		*fMousePosX += factor / fRangeMulti;
	}
	else // Type B
	{
		float SpeedMulti = 900.0f;

		// "Classic" aiming mode (0x00) needs lower sensitivity here.
		if (GetMouseAimingMode() == MouseAimingModes::Classic)
			SpeedMulti = 1300.0f;

		PlayerPtr()->ang_A0.y += (-*MouseDeltaX / SpeedMulti) * pConfig->fTurnTypeBSensitivity;
	}
}

bool __cdecl KeyOnCheck_hook(KEY_BTN a1)
{
	// Enable the turning animation/action if the mouse is moving

	// Type A doesn't need this
	if (pConfig->iMouseTurnType == MouseTurnTypes::TypeA)
		return game_KeyOnCheck_0(a1);

	bool ret = false;
	bool isMovingMouse = false;

	if (a1 == KEY_BTN::KEY_LEFT)
		isMovingMouse = (*MouseDeltaX < 0);
	else if (a1 == KEY_BTN::KEY_RIGHT)
		isMovingMouse = (*MouseDeltaX > 0);

	switch (LastUsedDevice()) {
		case InputDevices::DinputController:
		case InputDevices::XinputController:
			ret = game_KeyOnCheck_0(a1);
			break;
		case InputDevices::Keyboard:
		case InputDevices::Mouse:
			if (isMouseTurnEnabled() && isMovingMouse)
				ret = true;
			else
				ret = game_KeyOnCheck_0(a1);
			break;
	}

	return ret;
}

void Init_MouseTurning()
{
	auto pattern = hook::pattern("DB 05 ? ? ? ? D9 45 ? D9 C0 DE CA D9 C5");
	MouseDeltaX = (int32_t*)*pattern.count(1).get(0).get<uint32_t>(2);
	MouseDeltaY = MouseDeltaX + 1;

	// Keep CameraXpos at 0f while isMouseTurnEnabled
	pattern = hook::pattern("D9 05 ? ? ? ? DE C2 D9 C9 D9 1D ? ? ? ? D9 85");
	struct CameraPositionWriter
	{
		void operator()(injector::reg_pack& regs)
		{
			float CameraXpos;

			switch (LastUsedDevice()) {
			case InputDevices::DinputController:
			case InputDevices::XinputController:
				CameraXpos = *fMousePosX;
				break;
			case InputDevices::Keyboard:
			case InputDevices::Mouse:
				if (isMouseTurnEnabled() && (pConfig->iMouseTurnType == MouseTurnTypes::TypeB)) // Camera should be free for Type A
					CameraXpos = 0.0f;
				else
					CameraXpos = *fMousePosX;
				break;
			}

			_asm {fld CameraXpos}

		}
	}; injector::MakeInline<CameraPositionWriter>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Trigger left turn
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 8A 86");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyOnCheck_hook, PATCH_CALL);

	// Trigger right turn
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 53");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyOnCheck_hook, PATCH_CALL);

	// pl_R1_Turn (left and right separated) TODO: Combine them?
	pattern = hook::pattern("A1 ? ? ? ? 83 E0 ? 33 C9 0B C1 75 ? 89 9E");
	struct pl_R1_Turn_left
	{
		void operator()(injector::reg_pack& regs)
		{
			// Code we replaced
			regs.eax = (uint32_t)Key_btn_on();

			switch (LastUsedDevice()) {
			case InputDevices::DinputController:
			case InputDevices::XinputController:
				break;
			case InputDevices::Keyboard:
			case InputDevices::Mouse:
				// Get pointer to cModel->MotInfo_1D8
				MOTION_INFO* mi_ptr = (MOTION_INFO*)(regs.esi + 0x1D8);

				// Check if default key is being pressed
				bool isPressingDefaultKey = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_LEFT) == (uint64_t)KEY_BTN::KEY_LEFT);

				// Make game go back to the default procedure if the user started to hold the default key after moving the mouse
				if (isPressingDefaultKey && (mi_ptr->Mot_attr_40 == 4))
					regs.eax = 0;
				else if (isMouseTurnEnabled() && (*MouseDeltaX < -8) && !isPressingDefaultKey)
				{
					// Setting this flag to 4 makes MotionMove not apply any rotation/position changes
					// Very lucky for us, but I'm not sure what the real intent of this value/flag even is
					mi_ptr->Mot_attr_40 = 4;

					regs.eax = 0x8;

					MouseTurn();
				}
				break;
			}
		}
	}; injector::MakeInline<pl_R1_Turn_left>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	pattern = hook::pattern("A1 ? ? ? ? 83 E0 ? 33 C9 0B C1 75 ? 89 8E");
	struct pl_R1_Turn_right
	{
		void operator()(injector::reg_pack& regs)
		{
			// Code we replaced
			regs.eax = (uint32_t)Key_btn_on();

			switch (LastUsedDevice()) {
			case InputDevices::DinputController:
			case InputDevices::XinputController:
				break;
			case InputDevices::Keyboard:
			case InputDevices::Mouse:
				// Get pointer to cModel->MotInfo_1D8
				MOTION_INFO* mi_ptr = (MOTION_INFO*)(regs.esi + 0x1D8);

				// Check if default key is being pressed
				bool isPressingDefaultKey = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_RIGHT) == (uint64_t)KEY_BTN::KEY_RIGHT);

				// Make game go back to the default procedure if the user started to hold the default key after moving the mouse
				if (isPressingDefaultKey && (mi_ptr->Mot_attr_40 == 4))
					regs.eax = 0;
				else if (isMouseTurnEnabled() && (*MouseDeltaX > 8) && !isPressingDefaultKey)
				{
					// Setting this flag to 4 makes MotionMove not apply any rotation/position changes
					// Very lucky for us, but I'm not sure what the real intent of this value/flag even is
					mi_ptr->Mot_attr_40 = 4;

					regs.eax = 0x4;

					MouseTurn();
				}
				break;
			}
		}
	}; injector::MakeInline<pl_R1_Turn_right>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// pl_R1_Turn180
	pattern = hook::pattern("0F B6 86 ? ? ? ? 33 DB 2B C3 74 ? 48 74 ? 8B 16");
	struct TurnHook180
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int8_t*)(regs.esi + 0xFE);

			MouseTurn();
		}
	}; injector::MakeInline<TurnHook180>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	// pl_R1_Walk
	pattern = hook::pattern("8B EC 83 EC ? 53 56 8B 75 ? 33 DB 38 9E");
	struct TurnHookWalk
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebp = *(int32_t*)(regs.esp);
			*(int32_t*)(regs.esp) -= 0x8;

			MouseTurn();
		}
	}; injector::MakeInline<TurnHookWalk>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// pl_R1_Run
	pattern = hook::pattern("8B EC 83 EC ? 53 56 8B 75 ? 0F B6 86 ? ? ? ? 33 DB");
	struct TurnHookRun
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebp = *(int32_t*)(regs.esp);
			*(int32_t*)(regs.esp) -= 0x8;

			MouseTurn(5.0f);
		}
	}; injector::MakeInline<TurnHookRun>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// pl_R1_Back
	pattern = hook::pattern("83 3D ? ? ? ? ? 75 ? D9 05 ? ? ? ? 8B 8E ? ? ? ? 83 EC ? D9");
	struct TurnHookWalkingBack
	{
		void operator()(injector::reg_pack& regs)
		{
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
			if (isMouseTurnEnabled() && (*MouseDeltaX != 0) && isKeyboardMouse())
				regs.eax = 0x8;
		}
	}; injector::MakeInline<Knife_r3_downHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	if (pConfig->bUseMouseTurning)
		spd::log()->info("{} -> MouseTurning enabled", __FUNCTION__);
}