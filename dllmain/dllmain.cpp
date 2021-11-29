#include <iostream>
#include <charconv>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\ModUtils\MemoryMgr.h"
#include "..\external\MinHook\MinHook.h"
#include "WndProcHook.h"
#include "dllmain.h"
#include "cfgMenu.h"
#include "HandleLimits.h"
#include "Sofdec.h"
#include "FilterXXFixes.h"
#include "UltraWideFix.h"
#include "tool_menu.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "qtefixes.h"

std::string RealDllPath;
std::string WrapperMode;
std::string WrapperName;
std::string iniPath;
std::string game_version;

HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;

bool bCfgMenuOpen;
bool bConsoleOpen;
bool bShouldFlipX;
bool bShouldFlipY;
bool bisDebugBuild;

uintptr_t* ptrInvMovAddr;
uintptr_t* ptrNonBlurryVertex;
uintptr_t* ptrBlurryVertex;
uintptr_t* ptrRifleMovAddr;
uintptr_t ptrGXCopyTex;
uintptr_t ptrFilter03;
uintptr_t ptrAfterItemExamineHook;
uintptr_t ptrAfterEsp04TransHook;
uintptr_t ptrRetryLoadDLGstate;

static uint32_t* ptrGameFrameRate;

BOOL __stdcall SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	int windowX = cfg.iWindowPositionX < 0 ? 0 : cfg.iWindowPositionX;
	int windowY = cfg.iWindowPositionY < 0 ? 0 : cfg.iWindowPositionY;
	return SetWindowPos(hWnd, hWndInsertAfter, windowX, windowY, cx, cy, uFlags);
}

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

void HandleAppID()
{
	//Create missing steam_appid file
	const char *filename = "steam_appid.txt";
	if (!std::filesystem::exists(filename)) {
		std::ofstream appid(filename);
		appid << "254700";
		appid.close();
	}
}

void Init_Main()
{
	con.AddLogChar("Big ironic thanks to QLOC S.A.");

	// Detect game version
	auto pattern = hook::pattern("31 2E ? ? ? 00 00 00 6D 6F 76 69 65 2F 64 65 6D 6F 30 65 6E 67 2E 73 66 64");
	int ver = injector::ReadMemory<int>(pattern.get_first(2));

	if (ver == 0x362E30) {
		game_version = "1.0.6";
	} else if (ver == 0x302E31) {
		game_version = "1.1.0";
	} else {
		MessageBoxA(NULL, "This version of RE4 is not supported.\nre4_tweaks will be disabled.", "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		return;
	}

	// Check for part of gameDebug function, if exists this must be a debug-enabled build
	pattern = hook::pattern("6A 00 6A 00 6A 08 68 AE 01 00 00 6A 10 6A 0A");
	if (pattern.size() > 0)
	{
		game_version += "d";
		bisDebugBuild = true;
	}

	#ifdef VERBOSE
	con.AddConcatLog("Game version = ", game_version.data());
	#endif

	MH_Initialize();

	cfg.ReadSettings();

	HandleAppID();

	Init_WndProcHook();

	Init_cfgMenu();

	Init_HandleLimits();

	Init_FilterXXFixes();

	Init_QTEfixes();

	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (cfg.bFixUltraWideAspectRatio)
		Init_UltraWideFix();

	// SFD size
	if (cfg.bAllowHighResolutionSFD)
		Init_sofdec();

	// FOV
	if (cfg.fFOVAdditional > 0.0f)
	{
		// Hook function that loads the FOV
		auto pattern = hook::pattern("D9 45 ? 89 4E ? D9 5E ? 8B 8D ? ? ? ? 89 46 ? 8B 85 ? ? ? ? 8D 7E");
		struct ScaleFOV
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaFov = *(float*)(regs.ebp - 0x34);
				_asm
				{
					fld   vanillaFov
					fadd  cfg.fFOVAdditional
				}
				*(int32_t*)(regs.esi + 0x4) = regs.ecx;
			}
		}; injector::MakeInline<ScaleFOV>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
	
	// Fix vsync option
	if (cfg.bFixVsyncToggle)
	{
		auto pattern = hook::pattern("50 E8 ? ? ? ? C7 07 01 00 00 00 E9");
		injector::MakeNOP(pattern.get_first(6), 6);
	}

	// Apply window changes
	if (cfg.bWindowBorderless || cfg.iWindowPositionX > -1 || cfg.iWindowPositionY > -1)
	{
		// hook SetWindowPos, can't nop as it's used for resizing window on resolution changes
		auto pattern = hook::pattern("BE 00 01 04 00 BF 00 00 C8 00");
		injector::MakeNOP(pattern.get_first(0xA), 6);
		injector::MakeCALL(pattern.get_first(0xA), SetWindowPos_Hook, true);

		// nop MoveWindow
		pattern = hook::pattern("53 51 52 53 53 50");
		injector::MakeNOP(pattern.get_first(6), 6);

		if (cfg.bWindowBorderless)
		{
			// if borderless, update the style set by SetWindowLongA
			pattern = hook::pattern("25 00 00 38 7F 05 00 00 C8 00");
			injector::WriteMemory(pattern.get_first(5), uint8_t(0xb8), true); // mov eax, XXXXXXXX
			injector::WriteMemory(pattern.get_first(6), uint32_t(WS_POPUP), true);
		}
	}

	// Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action.
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

	// Check if the game is running at a valid frame rate
	if (!cfg.bIgnoreFPSWarning)
	{
		// Hook function to read the FPS value from config.ini
		auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
		ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);
		struct ReadFPS
		{
			void operator()(injector::reg_pack& regs)
			{
				int intIniFPS = regs.ecx;
				int intNewFPS;

				#ifdef VERBOSE
				con.AddConcatLog("Config.ini frame rate = ", intIniFPS);
				#endif

				if (intIniFPS != 30 && intIniFPS != 60) {

					// Calculate new fps
					if (intIniFPS > 45)
						intNewFPS = 60;
					else
						intNewFPS = 30;

					// Show warning
					std::string Msg = "ERROR: Invalid frame rate detected\n\nYour game appears to be configured to run at " + std::to_string(intIniFPS) +
						" FPS, which isn't supported and will break multiple aspects of the game.\nThis could mean you edited the game's \"config.ini\" file" +
						" and changed the \"variableframerate\" value to something other than 30 or 60.\n\nTo prevent the problems mentioned, re4_tweaks automatically" +
						" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' \"winmm.ini\" file.";

					MessageBoxA(NULL, Msg.c_str(), "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);

					// Use new FPS value
					*(int32_t*)(ptrGameFrameRate) = intNewFPS;
				} else {
					// Use .ini FPS value.
					*(int32_t*)(ptrGameFrameRate) = intIniFPS;
				}
			}
		}; injector::MakeInline<ReadFPS>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Replace MemorySwap with memcpy
	if (cfg.bUseMemcpy)
	{
		auto pattern = hook::pattern("8b 49 14 33 c0 a3 ? ? ? ? 85 c9 74 ? 8b ff f6 81 ? ? ? ? 20 8b 49 ? 74 ? b8 01 00 00 00 a3 ? ? ? ? 85 c9 75 ? c3");
		injector::MakeJMP(pattern.get_first(0), memcpy, true);
	}

	// Option to skip the intro logos when starting up the game
	if (cfg.bSkipIntroLogos)
	{
		pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);
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

	// Restore missing transparency in the item pickup screen by
	// removing a call to GXCopyTex inside ItemExamine::gxDraw
	pattern = hook::pattern("E8 ? ? ? ? D9 05 ? ? ? ? D9 7D ? 6A ? 0F B7 45");
	ptrGXCopyTex = injector::GetBranchDestination(pattern.get_first(0)).as_int();
	ptrAfterItemExamineHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), ItemExamineHook, true);


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
	}

	// Fix a problem related to a vertex buffer that caused the image to be slightly blurred
	{
		// Replacement buffer
		auto pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 6A ? 6A ? 52");
		ptrNonBlurryVertex = *pattern.count(1).get(0).get<uint32_t*>(7);

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
	}

	// Disable film grain (Esp04)
	{
		auto pattern = hook::pattern("55 8B EC 83 EC ? A1 ? ? ? ? 33 C5 89 45 ? 56 8B 75 ? 0F B6 4E");
		ptrAfterEsp04TransHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(6);
		injector::MakeNOP(pattern.get_first(0), 6);
		injector::MakeJMP(pattern.get_first(0), Esp04TransHook, true);
	}
  
	// Enable what was leftover from the dev's debug menu (called "ToolMenu")
	if (cfg.bEnableDebugMenu)
	{
		Init_ToolMenu();
		if (bisDebugBuild)
			Init_ToolMenuDebug();
	}

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

	// Enable MH hooks
	MH_EnableHook(MH_ALL_HOOKS);
}

void LoadRealDLL(HMODULE hModule)
{
	char configname[MAX_PATH];
	GetModuleFileNameA(hModule, configname, MAX_PATH);
	WrapperName.assign(strrchr(configname, '\\') + 1);
	std::transform(WrapperName.begin(), WrapperName.end(), WrapperName.begin(), ::tolower);

	// Store ini path
	std::string modulePath = configname;
	iniPath = modulePath.substr(0, modulePath.rfind('\\') + 1) + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".ini";

	// Get wrapper mode
	const char* RealWrapperMode = Wrapper::GetWrapperName((WrapperMode.size()) ? WrapperMode.c_str() : WrapperName.c_str());

	proxy_dll = Wrapper::CreateWrapper((RealDllPath.size()) ? RealDllPath.c_str() : nullptr, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr, WrapperName.c_str());
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		LoadRealDLL(hModule);

		#ifdef VERBOSE
		con.AddLogChar("\n");
		#endif

		Init_Main();

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}