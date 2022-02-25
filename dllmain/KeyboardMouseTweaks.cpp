#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "MouseTurning.h"
#include "KeyboardMouseTweaks.h"
#include "60fpsFixes.h"
#include "Logging/Logging.h"
#include "WndProcHook.h"
#include "input.hpp"

uintptr_t* ptrRifleMovAddr;
uintptr_t* ptrInvMovAddr;
uintptr_t* ptrFocusAnimFldAddr;
uintptr_t ptrRetryLoadDLGstate;

static uint32_t* ptrLastUsedController;
static uint32_t* ptrMouseSens;
static uint32_t* ptrMouseAimMode;

int iMinFocusTime;

LastDevice GetLastUsedDevice()
{
	// 0 = Keyboard?
	// 1 = Mouse
	// 2 = Xinput Controller
	// 3 = Dinput Controller

	switch (*(int32_t*)(ptrLastUsedController))
	{
	case 0:
		return LastDevice::Keyboard;
	case 1:
		return LastDevice::Mouse;
	case 2:
		return LastDevice::XinputController;
	case 3:
		return LastDevice::DinputController;
	default:
		return LastDevice::Keyboard;
	}
}

int g_MOUSE_SENS()
{
	return *(int8_t*)(ptrMouseSens);
}

int intMouseAimingMode()
{
	return *(int8_t*)(ptrMouseAimMode);
}

uint8_t* g_KeyIconData = nullptr;

// Data generated by game for (LANG_ENGLISH, SUBLANG_ENGLISH_US)
uint8_t KeyIconData_US[1024] =
{
		0x00, 0x00, 0x00, 0x00, 0x1B, 0x1B, 0x00, 0x1B, 0x31, 0x21, 0x00, 0x31, 0x32, 0x22, 0x00, 0x32,
		0x33, 0xA3, 0x00, 0x33, 0x34, 0x24, 0x80, 0x34, 0x35, 0x25, 0x00, 0x35, 0x36, 0x5E, 0x00, 0x36,
		0x37, 0x26, 0x00, 0x37, 0x38, 0x2A, 0x00, 0x38, 0x39, 0x28, 0x00, 0x39, 0x30, 0x29, 0x00, 0x30,
		0x2D, 0x5F, 0x00, 0x2D, 0x3D, 0x2B, 0x00, 0x3D, 0x08, 0x08, 0x00, 0x08, 0x09, 0x09, 0x00, 0x09,
		0x71, 0x51, 0x00, 0x51, 0x77, 0x57, 0x00, 0x57, 0x65, 0x45, 0xE9, 0x45, 0x72, 0x52, 0x00, 0x52,
		0x74, 0x54, 0x00, 0x54, 0x79, 0x59, 0x00, 0x59, 0x75, 0x55, 0xFA, 0x55, 0x69, 0x49, 0xED, 0x49,
		0x6F, 0x4F, 0xF3, 0x4F, 0x70, 0x50, 0x00, 0x50, 0x5B, 0x7B, 0x00, 0x5B, 0x5D, 0x7D, 0x00, 0x5D,
		0x0D, 0x0D, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x61, 0x41, 0xE1, 0x41, 0x73, 0x53, 0x00, 0x53,
		0x64, 0x44, 0x00, 0x44, 0x66, 0x46, 0x00, 0x46, 0x67, 0x47, 0x00, 0x47, 0x68, 0x48, 0x00, 0x48,
		0x6A, 0x4A, 0x00, 0x4A, 0x6B, 0x4B, 0x00, 0x4B, 0x6C, 0x4C, 0x00, 0x4C, 0x3B, 0x3A, 0x00, 0x3B,
		0x27, 0x40, 0x00, 0x27, 0x60, 0xAC, 0xA6, 0x60, 0x00, 0x00, 0x00, 0x00, 0x23, 0x7E, 0x5C, 0x23,
		0x7A, 0x5A, 0x00, 0x5A, 0x78, 0x58, 0x00, 0x58, 0x63, 0x43, 0x00, 0x43, 0x76, 0x56, 0x00, 0x56,
		0x62, 0x42, 0x00, 0x42, 0x6E, 0x4E, 0x00, 0x4E, 0x6D, 0x4D, 0x00, 0x4D, 0x2C, 0x3C, 0x00, 0x2C,
		0x2E, 0x3E, 0x00, 0x2E, 0x2F, 0x3F, 0x00, 0x2F, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x2A, 0x00, 0x2A,
		0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2D, 0x2D, 0x00, 0x2D, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x2B, 0x00, 0x2B, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5C, 0x7C, 0x00, 0x5C, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x09, 0x09, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

void (*Init_KeyIconMapping)();
void Init_KeyIconMapping_Hook()
{
	// Run original func
	Init_KeyIconMapping();

	// Check if users keyboard layout is one we know game supports
	auto userLanguage = GetKeyboardLayout(0);
	switch (PRIMARYLANGID(LOWORD(userLanguage)))
	{
	case LANG_CHINESE:
	case LANG_ENGLISH:
	case LANG_FRENCH:
	case LANG_GERMAN:
	case LANG_ITALIAN:
	case LANG_JAPANESE:
	case LANG_SPANISH:
	case LANG_POLISH: // not actually translated, but game seems to support it for keyicons

		return; // Game works with these langs fine, no change needed
	}

	// Lang might not be supported, replace keydata with known-good US data
	memcpy(g_KeyIconData, KeyIconData_US, sizeof(KeyIconData_US));
}

void InventoryFlipBindings(UINT uMsg, WPARAM wParam)
{
	switch (uMsg) {
	case WM_KEYDOWN:
		if (wParam == cfg.KeyMap(cfg.sFlipItemLeft.data(), true) || wParam == cfg.KeyMap(cfg.sFlipItemRight.data(), true))
			bShouldFlipX = true;
		else if (wParam == cfg.KeyMap(cfg.sFlipItemUp.data(), true) || wParam == cfg.KeyMap(cfg.sFlipItemDown.data(), true))
			bShouldFlipY = true;
		break;

	case WM_KEYUP:
		if (wParam == cfg.KeyMap(cfg.sFlipItemLeft.data(), true) || wParam == cfg.KeyMap(cfg.sFlipItemRight.data(), true))
			bShouldFlipX = false;
		else if (wParam == cfg.KeyMap(cfg.sFlipItemUp.data(), true) || wParam == cfg.KeyMap(cfg.sFlipItemDown.data(), true))
			bShouldFlipY = false;
		break;
	}
}

static uint32_t* ptrMouseDeltaX;
static uint32_t* ptrMouseDeltaY;

void Init_KeyboardMouseTweaks()
{
	auto pattern = hook::pattern("A1 ? ? ? ? 85 C0 74 ? 83 F8 ? 74 ? 81 F9");
	ptrLastUsedController = *pattern.count(1).get(0).get<uint32_t*>(1);

	// g_MOUSE_SENS pointer
	pattern = hook::pattern("0F B6 05 ? ? ? ? 89 85 ? ? ? ? DB 85 ? ? ? ? DC 35");
	ptrMouseSens = *pattern.count(1).get(0).get<uint32_t*>(3);

	// Aiming mode pointer
	pattern = hook::pattern("80 3D ? ? ? ? ? 0F B6 05");
	ptrMouseAimMode = *pattern.count(1).get(0).get<uint32_t*>(2);

	Init_MouseTurning();

	// X mouse delta adjustments
	pattern = hook::pattern("A3 ? ? ? ? E8 ? ? ? ? A3 ? ? ? ? 8B 85 ? ? ? ? EB ? D9 C9");
	ptrMouseDeltaX = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct MouseDeltaX
	{
		void operator()(injector::reg_pack& regs)
		{
			int delta_factor = (intCurrentFrameRate() / 30);
			int cur_delta = regs.eax;

			if (cfg.bUseRawMouseInput)
			{
				*(int32_t*)(ptrMouseDeltaX) = (int32_t)((_input->raw_mouse_delta_x() / 10.0f) * g_MOUSE_SENS());

				if (cfg.bFixAimingSpeed)
					*(int32_t*)(ptrMouseDeltaX) *= delta_factor;
			}
			else if (cfg.bFixAimingSpeed)
			{
				*(int32_t*)(ptrMouseDeltaX) = cur_delta * delta_factor;
			}
			else
				*(int32_t*)(ptrMouseDeltaX) = cur_delta;
		}
	}; injector::MakeInline<MouseDeltaX>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Y mouse delta adjustments
	pattern = hook::pattern("A3 ? ? ? ? E8 ? ? ? ? A3 ? ? ? ? 8B 85 ? ? ? ? EB ? E8");
	ptrMouseDeltaY = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct MouseDeltaY
	{
		void operator()(injector::reg_pack& regs)
		{
			int delta_factor = (intCurrentFrameRate() / 30);
			int cur_delta = regs.eax;

			if (cfg.bUseRawMouseInput)
			{
				*(int32_t*)(ptrMouseDeltaY) = -(int32_t)((_input->raw_mouse_delta_y() / 6.0f) * g_MOUSE_SENS());

				if (cfg.bFixAimingSpeed) 
					*(int32_t*)(ptrMouseDeltaY) *= delta_factor;
			}
			else if (cfg.bFixAimingSpeed)
			{
				*(int32_t*)(ptrMouseDeltaY) = cur_delta * delta_factor;
			}
			else
				*(int32_t*)(ptrMouseDeltaY) = cur_delta;
		}
	}; injector::MakeInline<MouseDeltaY>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	if (cfg.bUseRawMouseInput)
		Logging::Log() << __FUNCTION__ << " -> Raw mouse input enabled";

	// Prevent some negative mouse acceleration from being applied
	pattern = hook::pattern("83 3D ? ? ? ? ? 75 ? 83 3D ? ? ? ? 00 75 0E 85 C0 75 ? D9 EE D9");
	struct PlWepLockCtrlHook
	{
		void operator()(injector::reg_pack& regs)
		{
			int DeltaX = *(int32_t*)(ptrMouseDeltaX);

			// Mimic what CMP does, since we're overwriting it.
			if (DeltaX > 0)
			{
				// Clear both flags
				regs.ef &= ~(1 << regs.zero_flag);
				regs.ef &= ~(1 << regs.carry_flag);
			}
			else if (DeltaX < 0)
			{
				// ZF = 0, CF = 1
				regs.ef &= ~(1 << regs.zero_flag);
				regs.ef |= (1 << regs.carry_flag);
			}
			else if (DeltaX == 0)
			{
				// ZF = 1, CF = 0
				regs.ef |= (1 << regs.zero_flag);
				regs.ef &= ~(1 << regs.carry_flag);
			}

			if ((GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
			{
				if (cfg.bUseRawMouseInput)
				{
					// Force aiming mode to "Modern". This seems to break "Classic" mode somewhat, and that mode is irrelevant anyways.
					if (intMouseAimingMode() == 0x00)
						*(int8_t*)(ptrMouseAimMode) = 0x01;

					// Clear both flags so we skip the section that does the actual negative acceleration
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}
			}
		}
	};
	
	injector::MakeInline<PlWepLockCtrlHook>(pattern.count(2).get(0).get<uint32_t>(0), pattern.count(2).get(0).get<uint32_t>(7));
	injector::MakeInline<PlWepLockCtrlHook>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(7));

	// Disable camera lock on "Modern" mouse setting
	struct CameraLockCmp
	{
		void operator()(injector::reg_pack& regs)
		{
			int aimingMode = *(int8_t*)(ptrMouseAimMode);

			// Mimic what CMP does, since we're overwriting it.
			if (aimingMode > 0)
			{
				// Clear both flags
				regs.ef &= ~(1 << regs.zero_flag);
				regs.ef &= ~(1 << regs.carry_flag);
			}
			else if (aimingMode < 0)
			{
				// ZF = 0, CF = 1
				regs.ef &= ~(1 << regs.zero_flag);
				regs.ef |= (1 << regs.carry_flag);
			}
			else if (aimingMode == 0)
			{
				// ZF = 1, CF = 0
				regs.ef |= (1 << regs.zero_flag);
				regs.ef &= ~(1 << regs.carry_flag);
			}

			if ((GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
			{
				if (cfg.bUnlockCameraFromAim)
				{
					// Make the game think the aiming mode is "Classic"
					regs.ef |= (1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}
			}
		}
	}; 

	pattern = hook::pattern("80 3D ? ? ? ? ? D9 41 ? D9 5D ? 74");
	injector::MakeInline<CameraLockCmp>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	pattern = hook::pattern("80 3D ? ? ? ? ? 0F 84 ? ? ? ? A1 ? ? ? ? 85 C0 74");
	injector::MakeInline<CameraLockCmp>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

	pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? A1 ? ? ? ? 85 C0 74 ? 83 F8 ? 74 ? D9 05");
	injector::MakeInline<CameraLockCmp>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

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

	Logging::Log() << __FUNCTION__ << " -> Keyboard inventory item flipping enabled";

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
	
		if (cfg.bFixRetryLoadMouseSelector)
			Logging::Log() << __FUNCTION__ << " -> FixRetryLoadMouseSelector applied";
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

		if (cfg.bFixSniperZoom)
			Logging::Log() << __FUNCTION__ << " -> FixSniperZoom applied";
	}

	// Fix the "focus animation" not looking as strong as when triggered with a controller
	if (cfg.bFixSniperFocus)
	{
		pattern = hook::pattern("8B F1 8B 4D ? 57 85 C9 74 ? D9 56 ? 88 56");
		struct FixScopeZoomFocus
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.esi = regs.ecx;
				regs.ecx = *(int32_t*)(regs.ebp + 0x8);

				// This makes it so the focus animation has to play for at least X ammount of time
				if ((GetLastUsedDevice() == LastDevice::Keyboard) || (GetLastUsedDevice() == LastDevice::Mouse))
				{
					if (regs.ecx == 1)
					{
						iMinFocusTime = intCurrentFrameRate() / 12;
					}
					else if (regs.ecx == 0)
					{
						iMinFocusTime--;
					}

					if (iMinFocusTime > 0)
						regs.ecx = 1;
					else
						regs.ecx = 0;
				}
			}
		}; injector::MakeInline<FixScopeZoomFocus>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	
		// This jl instruction makes the focus animation stop almost immediately when using the mouse. Noping it doesn't seem to affect the controller at all.
		pattern = hook::pattern("7C ? C6 06 ? EB ? C7 46 ? ? ? ? ? EB ? DD D8 83 3D");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 2, true);

		Logging::Log() << __FUNCTION__ << " -> FixSniperFocus applied";
	}

	// Hooks to allow toggling sprint instead of needing to hold it
	{
		// pl_R1_Run first KEY_RUN check
		pattern = hook::pattern("83 E0 01 33 D2 0B C2 74 ? 8B C1 83 E0 40"); // 7639EB
		struct SprintToggleHook1
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.eax &= 1; // Code we overwrote

				if (!cfg.bUseSprintToggle)
					regs.edx ^= regs.edx;
			}
		}; injector::MakeInline<SprintToggleHook1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// pl_R1_Run second KEY_RUN check
		pattern = hook::pattern("8B C1 83 E0 ? 33 D2 0B C2 74 ? 8B C1 83 E0 ? 0B C2 74 ? 8B CE"); // 763AE8
		struct SprintToggleHook2
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.eax = regs.ecx; // Code we overwrote

				if (!cfg.bUseSprintToggle)
					regs.eax &= 0x40;
			}
		}; injector::MakeInline<SprintToggleHook2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	}

	if (cfg.bFallbackToEnglishKeyIcons)
	{
		// Get pointer to key icon data buffer
		pattern = hook::pattern("53 56 57 68 00 04 00 00 6A 00 68 ? ? ? ?");
		g_KeyIconData = *pattern.count(1).get(0).get<uint8_t*>(0xB);

		// Hook Init_KeyIconMapping so we can replace key data if needed
		pattern = hook::pattern("53 57 E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 8B 35 ? ? ? ? 8B 1D ? ? ? ?");

		auto jmp_Init_KeyIconMapping = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0x11)).as_int();

		ReadCall(jmp_Init_KeyIconMapping, Init_KeyIconMapping);
		InjectHook(jmp_Init_KeyIconMapping, Init_KeyIconMapping_Hook);

		Logging::Log() << __FUNCTION__ << " -> FallbackToEnglishKeyIcons enabled";

		if (cfg.bVerboseLog)
		{
			char logBuf[100];

			Logging::Log() << "+------------------+------------------+";
			Logging::Log() << "| Keyboard/locale info:               |";

			// KeyboardLayout
			wchar_t KeyboardLayoutBuff[25];

			HKL userKeyboardLayout = GetKeyboardLayout(GetWindowThreadProcessId(hWindow, NULL));
			int userKeyboardLayoutID = PRIMARYLANGID(LOWORD(userKeyboardLayout));
			LCIDToLocaleName(userKeyboardLayoutID, KeyboardLayoutBuff, ARRAYSIZE(KeyboardLayoutBuff), 0);

			sprintf(logBuf, "%-28s%9ws", "| KeyboardLayout: ", KeyboardLayoutBuff);
			Logging::Log() << logBuf << " |";

			// UserDefaultLCID
			wchar_t UserDefaultLCIDBuff[25];

			LCIDToLocaleName(GetUserDefaultLCID(), UserDefaultLCIDBuff, ARRAYSIZE(UserDefaultLCIDBuff), 0);

			sprintf(logBuf, "%-28s%9ws", "| UserDefaultLCID: ", UserDefaultLCIDBuff);
			Logging::Log() << logBuf << " |";

			// SystemDefaultLCID
			wchar_t SystemDefaultLCIDBuff[25];

			LCIDToLocaleName(GetSystemDefaultLCID(), SystemDefaultLCIDBuff, ARRAYSIZE(SystemDefaultLCIDBuff), 0);

			sprintf(logBuf, "%-28s%9ws", "| SystemDefaultLCID: ", SystemDefaultLCIDBuff);
			Logging::Log() << logBuf << " |";

			// UserDefaultUILanguage
			wchar_t UserDefaultUILanguageBuf[100];

			GetLocaleInfo(GetUserDefaultUILanguage(), LOCALE_SNAME, UserDefaultUILanguageBuf, 100);

			sprintf(logBuf, "%-28s%9ws", "| UserDefaultUILanguage: ", UserDefaultUILanguageBuf);
			Logging::Log() << logBuf << " |";

			// SystemDefaultUILanguage
			wchar_t SystemDefaultUILanguageBuf[100];

			GetLocaleInfo(GetSystemDefaultUILanguage(), LOCALE_SNAME, SystemDefaultUILanguageBuf, 100);

			sprintf(logBuf, "%-28s%9ws", "| SystemDefaultUILanguage: ", SystemDefaultUILanguageBuf);
			Logging::Log() << logBuf << " |";

			// UserDefaultLangID
			wchar_t UserDefaultLangIDBuf[100];

			GetLocaleInfo(GetUserDefaultLangID(), LOCALE_SNAME, UserDefaultLangIDBuf, 100);

			sprintf(logBuf, "%-28s%9ws", "| UserDefaultLangID: ", UserDefaultLangIDBuf);
			Logging::Log() << logBuf << " |";

			// SystemDefaultLangID
			wchar_t SystemDefaultLangIDBuf[100];

			GetLocaleInfo(GetSystemDefaultLangID(), LOCALE_SNAME, SystemDefaultLangIDBuf, 100);

			sprintf(logBuf, "%-28s%9ws", "| SystemDefaultLangID: ", SystemDefaultLangIDBuf);
			Logging::Log() << logBuf << " |";

			Logging::Log() << "+------------------+------------------+";
		}
	}
}
