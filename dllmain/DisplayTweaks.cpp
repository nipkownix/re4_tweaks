#include <iostream>
#include "stdafx.h"
#include "Game.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "Logging/Logging.h"
#include "EndSceneHook.h"
#include "Patches.h"
#include "cfgMenu.h"

int g_UserRefreshRate;

uintptr_t ptrGXCopyTex;
uintptr_t ptrAfterItemExamineHook;
uintptr_t ptrFilter03;
uintptr_t ptrAfterEsp04TransHook;

uintptr_t* ptrNonBlurryVertex;
uintptr_t* ptrBlurryVertex;

uint32_t* ptrLaserR;
uint32_t* ptrLaserG;
uint32_t* ptrLaserB;
uint32_t* ptrLaserA;

void __declspec(naked) ItemExamineHook()
{
	if (!cfg.bRestorePickupTransparency)
		_asm
	{
		call ptrGXCopyTex
		jmp ptrAfterItemExamineHook
	}
	else
		_asm {jmp ptrAfterItemExamineHook}
}

void __declspec(naked) Esp04TransHook()
{
	if (!cfg.bDisableFilmGrain)
		_asm
	{
		push ebp
		mov  ebp, esp
		sub  esp, 0x68
		jmp ptrAfterEsp04TransHook
	}
	else
		_asm {ret}
}

BOOL __stdcall SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	int windowX = cfg.iWindowPositionX < 0 ? 0 : cfg.iWindowPositionX;
	int windowY = cfg.iWindowPositionY < 0 ? 0 : cfg.iWindowPositionY;
	return SetWindowPos(hWnd, hWndInsertAfter, windowX, windowY, cx, cy, uFlags);
}

BOOL __stdcall MoveWindow_Hook(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	// Early return if bWindowBorderless is set to true, as this call changes the window size to include borders.
	if (cfg.bWindowBorderless)
		return true;

	int windowX = cfg.iWindowPositionX < 0 ? 0 : cfg.iWindowPositionX;
	int windowY = cfg.iWindowPositionY < 0 ? 0 : cfg.iWindowPositionY;
	return MoveWindow(hWnd, windowX, windowY, nWidth, nHeight, bRepaint);
}

void Init_DisplayTweaks()
{
	// Fix broken effects
	Init_FilterXXFixes();

	// Install a D3D9 hook
	Init_D3D9Hook();

	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (cfg.bFixUltraWideAspectRatio)
		Init_UltraWideFix();

	// Hook function that loads the FOV
	{
		auto pattern = hook::pattern("D9 45 ? 89 4E ? D9 5E ? 8B 8D ? ? ? ? 89 46 ? 8B 85 ? ? ? ? 8D 7E");
		struct ScaleFOV
		{
			void operator()(injector::reg_pack& regs)
			{
				float FOV = *(float*)(regs.ebp - 0x34);

				if (cfg.fFOVAdditional > 0.0f)
					FOV += cfg.fFOVAdditional;

				_asm {fld FOV}

				*(int32_t*)(regs.esi + 0x4) = regs.ecx;
			}
		}; injector::MakeInline<ScaleFOV>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		if (cfg.fFOVAdditional > 0.0f)
			Logging::Log() << "FOV increased";
	}

	// Force v-sync off
	if (cfg.bDisableVsync)
	{
		// See D3D9Hook.cpp -> hook_Direct3D9::CreateDevice and hook_Direct3D9::Reset

		auto pattern = hook::pattern("8B 56 ? 8B 85 ? ? ? ? 83 C4 ? 52 68 ? ? ? ? 50");
		struct WriteIniHook
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.edx = 0; // This is what the game will write as the v-sync value inside config.ini

				regs.eax = *(int32_t*)(regs.ebp - 0x808);
			}
		}; injector::MakeInline<WriteIniHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(9));

		Logging::Log() << "Vsync disabled";
	}

	// Game is hardcoded to only allow 60Hz display modes for some reason...
	// Hook func that runs EnumAdapterModes & checks for 60Hz modes, make it use the refresh rate from Windows instead
	if (cfg.bFixDisplayMode)
	{
		if (cfg.iCustomRefreshRate > -1)
		{
			g_UserRefreshRate = cfg.iCustomRefreshRate;
		}
		else
		{
			DISPLAY_DEVICE device{ 0 };
			device.cb = sizeof(DISPLAY_DEVICE);
			DEVMODE lpDevMode{ 0 };
			if (EnumDisplayDevices(NULL, 0, &device, 0) == false ||
				EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &lpDevMode) == false)
			{
				#ifdef VERBOSE
				con.AddLogChar("Couldn't read the display refresh rate. Using fallback value.");
				#endif
				Logging::Log() << "FixDisplayMode > Couldn't read the display refresh rate. Using fallback value.";
				g_UserRefreshRate = 60; // Fallback value.
			}
			else
			{
				#ifdef VERBOSE
				con.AddLogChar("Device 0 name: %s", device.DeviceName);
				#endif
				g_UserRefreshRate = lpDevMode.dmDisplayFrequency;

				// Log display modes for troubleshooting
				if (cfg.bVerboseLog)
				{
					DEVMODE dm = { 0 };
					dm.dmSize = sizeof(dm);
					Logging::Log() << "+-------------+-------------+";
					Logging::Log() << "| Display modes:            |";
					for (int iModeNum = 0; EnumDisplaySettings(device.DeviceName, iModeNum, &dm) != 0; iModeNum++)
					{
						char resolution[100];

						sprintf(resolution, "%4dx%-4d", dm.dmPelsWidth, dm.dmPelsHeight);

						Logging::Log() << "| Mode " << std::setw(2) << iModeNum << " = " << resolution << " " << std::setw(3) << dm.dmDisplayFrequency << "Hz" << " |";
					}
					Logging::Log() << "+-------------+-------------+";
				}
			}
		}

		#ifdef VERBOSE
		con.AddConcatLog("New refresh rate = ", g_UserRefreshRate);
		#endif

		Logging::Log() << "FixDisplayMode -> New refresh rate = " << g_UserRefreshRate;

		auto pattern = hook::pattern("8B 45 ? 83 F8 ? 75 ? 8B 4D ? 8B 7D ? 3B 4D");
		struct DisplayModeFix
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.eax = g_UserRefreshRate;
				regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<DisplayModeFix>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Patch function that fills resolution list, normally it filters resolutions to only 60Hz modes, which is bad for non-60Hz players...
		// Changing it to check for desktop refresh-rate helped, but unfortunately D3D doesn't always match desktop refresh rate
		// Luckily it turns out this function has an unused code-path which filters by checking for duplicate width/height combos instead, so any refresh rate can be allowed
		// who knows why they chose not to use it...
		pattern = hook::pattern("81 FA 98 26 00 00 77 ? 38 45 ? 75 ?");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(11), uint8_t(0xEB), true);

		// Patch above function to only allow 59Hz+ modes
		// (workaround for game potentially displaying invalid modes that some displays expose)
		struct DisplayModeFilter
		{
			void operator()(injector::reg_pack& regs)
			{
				const int GameMaxHeight = 10480 - 600; // game does strange optimized check involving 600 taken away from edx...

				// Game does JA after the CMP we patched, so we set ZF and CF both to 1 here
				// then if we want game to take the JA, we clear them both
				regs.ef |= (1 << regs.zero_flag);
				regs.ef |= (1 << regs.carry_flag);

				// Make game skip any mode under 59, since they're likely invalid
				// (old game code would only accept 60Hz modes, so I think this is fine for us to do)
				int refreshRate = *(int*)(regs.ebp - 0xC);
				if (regs.edx > GameMaxHeight || refreshRate < 59)
				{
					// Clear both flags to make game take JA and skip this mode
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}
			}
		}; injector::MakeInline<DisplayModeFilter>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Apply window changes
	if (cfg.bWindowBorderless || cfg.iWindowPositionX > -1 || cfg.iWindowPositionY > -1)
	{
		// hook SetWindowPos, can't nop as it's used for resizing window on resolution changes
		auto pattern = hook::pattern("FF 15 ? ? ? ? 56 53 57 8D 45 EC 50 FF 15 ? ? ? ? 8B");
		injector::MakeNOP(pattern.get_first(0), 6);
		InjectHook(pattern.get_first(0), SetWindowPos_Hook, PATCH_CALL);

		// hook MoveWindow. Also can't nop this one as it seems to set up the correct window size on startup
		pattern = hook::pattern("FF 15 ? ? ? ? 8B 0D ? ? ? ? 6A 01");
		injector::MakeNOP(pattern.get_first(0), 6);
		InjectHook(pattern.get_first(0), MoveWindow_Hook, PATCH_CALL);

		if (cfg.bWindowBorderless)
		{
			// if borderless, update the style set by SetWindowLongA
			pattern = hook::pattern("25 00 00 38 7F 05 00 00 C8 00");
			injector::WriteMemory(pattern.get_first(5), uint8_t(0xb8), true); // mov eax, XXXXXXXX
			injector::WriteMemory(pattern.get_first(6), uint32_t(WS_POPUP), true);
		}
	}

	// Restore missing transparency in the item pickup screen by
	// removing a call to GXCopyTex inside ItemExamine::gxDraw
	{
		auto pattern = hook::pattern("E8 ? ? ? ? D9 05 ? ? ? ? D9 7D ? 6A ? 0F B7 45");
		ptrGXCopyTex = injector::GetBranchDestination(pattern.get_first(0)).as_int();
		ptrAfterItemExamineHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
		injector::MakeNOP(pattern.get_first(0), 5);
		injector::MakeJMP(pattern.get_first(0), ItemExamineHook, true);

		if (cfg.bRestorePickupTransparency)
			Logging::Log() << "RestorePickupTransparency enabled";
	}

	// Override laser sight colors
	{
		auto pattern = hook::pattern("D9 05 ? ? ? ? D9 99 ? ? ? ? 8B 55 ? D9 05 ? ? ? ? D9");
		ptrLaserR = *pattern.count(1).get(0).get<uint32_t*>(2);
		ptrLaserG = *pattern.count(1).get(0).get<uint32_t*>(17);
		ptrLaserB = *pattern.count(1).get(0).get<uint32_t*>(32);
		ptrLaserA = *pattern.count(1).get(0).get<uint32_t*>(47);

		pattern = hook::pattern("A1 ? ? ? ? D9 5D ? 83 F8 ? 75 ? 8B 15 ? ? ? ? D9 45 ? 8B 0D ? ? ? ? 83");
		struct DrawLaserStruct
		{
			void operator()(injector::reg_pack& regs)
			{
				if (cfg.bOverrideLaserColor)
				{
					*(float*)(ptrLaserR) = cfg.fLaserRGB[0] * 255.0f;
					*(float*)(ptrLaserG) = cfg.fLaserRGB[1] * 255.0f;
					*(float*)(ptrLaserB) = cfg.fLaserRGB[2] * 255.0f;

					// Seems the game disables reading custom RGB colors if the alpha is set to 255?
					// The alpha value also seems to be doing nothing useful. It does affect the opacity of the laser dot, but not the line itself.
					// Forcing 220 makes it look okay-ish.
					*(int8_t*)(ptrLaserA) = (int8_t)220;
				}
				else
				{
					*(float*)(ptrLaserR) = 255.0f;
					*(float*)(ptrLaserG) = 0.0f;
					*(float*)(ptrLaserB) = 0.0f;
					*(int8_t*)(ptrLaserA) = (int8_t)255;
				}

				regs.eax = *(int8_t*)(ptrLaserA);
			}
		}; injector::MakeInline<DrawLaserStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		pattern = hook::pattern("a1 ? ? ? ? 83 f8 ? 75 ? 8b 0d ? ? ? ? d9 45 ? 8b 15 ? ? ? ? 83 ec");
		injector::MakeInline<DrawLaserStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	}

	// Disable Filter03 for now, as we have yet to find a way to actually fix it
	{
		auto pattern = hook::pattern("E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 39 1D");
		ptrFilter03 = injector::GetBranchDestination(pattern.get_first(0)).as_int();
		struct DisableBrokenFilter03
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!cfg.bDisableBrokenFilter03)
					_asm {call ptrFilter03}
			}
		}; injector::MakeInline<DisableBrokenFilter03>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		Logging::Log() << "DisableBrokenFilter03 applied";
	}

	// Fix a problem related to a vertex buffer that caused the image to be slightly blurred
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 6A ? 6A ? 52");
		ptrNonBlurryVertex = *pattern.count(1).get(0).get<uint32_t*>(7); // Replacement buffer

		// Hook struct
		struct BlurryBuffer
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!cfg.bFixBlurryImage)
					regs.edx = *(int32_t*)ptrBlurryVertex;
				else
					regs.edx = *(int32_t*)ptrNonBlurryVertex;
			}
		};

		// First buffer
		pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 56 57 6A ? 6A");
		ptrBlurryVertex = *pattern.count(1).get(0).get<uint32_t*>(2);
		injector::MakeInline<BlurryBuffer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Second buffer
		pattern = hook::pattern("D9 5D ? FF D0 D9 E8 A1 ? ? ? ? 8B 08 8B 91 ? ? ? ? 6A ? 51 D9 1C ? 68");
		injector::MakeInline<BlurryBuffer>(pattern.count(1).get(0).get<uint32_t>(40), pattern.count(1).get(0).get<uint32_t>(46));

		if (cfg.bFixBlurryImage)
			Logging::Log() << "FixBlurryImage enabled";
	}

	// Disable film grain (Esp04)
	{
		auto pattern = hook::pattern("55 8B EC 83 EC ? A1 ? ? ? ? 33 C5 89 45 ? 56 8B 75 ? 0F B6 4E");
		ptrAfterEsp04TransHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(6);
		injector::MakeNOP(pattern.get_first(0), 6);
		injector::MakeJMP(pattern.get_first(0), Esp04TransHook, true);

		if (cfg.bDisableFilmGrain)
			Logging::Log() << "DisableFilmGrain applied";
	}

	// Disable Windows DPI scaling
	if (cfg.bFixDPIScale)
	{
		SetProcessDPIAware();
	}
}
