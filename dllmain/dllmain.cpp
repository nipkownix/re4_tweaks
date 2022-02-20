#include <iostream>
#include <charconv>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "Game.h"
#include "WndProcHook.h"
#include "dllmain.h"
#include "cfgMenu.h"
#include "HandleLimits.h"
#include "Sofdec.h"
#include "FilterXXFixes.h"
#include "UltraWideFix.h"
#include "ToolMenu.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "QTEFixes.h"
#include "60fpsFixes.h"
#include "KeyboardMouseTweaks.h"
#include "ExceptionHandler.h"
#include "ControllerTweaks.h"
#include "input.hpp"
#include "EndSceneHook.h"
#include <Logging/Logging.h>
#include "gitparams.h"
#include "resource.h"

std::string WrapperMode;
std::string WrapperName;
std::string rootPath;

std::ofstream LOG;

HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;
HMODULE g_module_handle = nullptr;

bool bCfgMenuOpen;
bool bConsoleOpen;
bool bShouldFlipX;
bool bShouldFlipY;
bool bisDebugBuild;

int g_UserRefreshRate;

const char* game_lang;

uintptr_t* ptrNonBlurryVertex;
uintptr_t* ptrBlurryVertex;
uintptr_t ptrGXCopyTex;
uintptr_t ptrFilter03;
uintptr_t ptrAfterItemExamineHook;
uintptr_t ptrAfterEsp04TransHook;

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

void Init_Logging()
{
	// Start logging
	std::string logPath = rootPath + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".log";
	Logging::Open(logPath);

	std::string commit = GIT_CUR_COMMIT;
	commit.resize(8);

	Logging::Log() << "Starting re4_tweaks v" << APP_VERSION << "-" << commit << "-" << GIT_BRANCH;

	Logging::LogComputerManufacturer();
	Logging::LogOSVersion();
	Logging::LogProcessNameAndPID();

	Logging::Log() << "Running from: " << rootPath;
}

void Init_Main()
{
	con.AddLogChar("Big ironic thanks to QLOC S.A.");

	if (!Init_Game())
		return;

	Logging::Log() << "Game version = " << GameVersion();

	// Make sure steam_appid.txt exists
	HandleAppID();
	
	// Install input-related hooks
	input::Init_Input();

	// Install a WndProc hook and register the resulting hWnd for input
	Init_WndProcHook();

	// Install a EndScene hook to display our own UI
	Init_EndSceneHook();

	// Increase some game limits
	Init_HandleLimits();

	// Fix broken effects
	Init_FilterXXFixes();

	// QTE-related changes
	Init_QTEfixes();

	// Fixes FPS-related issues
	Init_60fpsFixes();

	Init_KeyboardMouseTweaks();

	Init_ControllerTweaks();

	Init_ExceptionHandler();

	// Remove savegame SteamID checks, allows easier save transfers
	auto pattern = hook::pattern("8B 88 40 1E 00 00 3B 4D ? 0F 85 ? ? ? ?");
	Nop(pattern.count(1).get(0).get<uint8_t>(0x9), 6);
	Nop(pattern.count(1).get(0).get<uint8_t>(0x18), 6);

	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (cfg.bFixUltraWideAspectRatio)
		Init_UltraWideFix();

	// SFD size
	if (cfg.bAllowHighResolutionSFD)
		Init_sofdec();

	// FOV
	{
		// Hook function that loads the FOV
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
		auto pattern = hook::pattern("C7 07 ? ? ? ? E9 ? ? ? ? 8D 4E ? 51 8D 95 ? ? ? ? 52");
		injector::WriteMemory(pattern.get_first(2), uint32_t(0x0), true);

		pattern = hook::pattern("8B 56 ? 8B 85 ? ? ? ? 83 C4 ? 52 68 ? ? ? ? 50");
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

		pattern = hook::pattern("8B 45 ? 83 F8 ? 75 ? 8B 4D ? 8B 7D ? 3B 4D");
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

	// Mafia Leon on cutscenes
	if (cfg.bAllowMafiaLeonCutscenes)
	{
		auto pattern = hook::pattern("80 B9 ? ? ? ? 04 6A ? 6A ? 6A ? 6A ? 0F 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), (uint8_t)-1, true); // Allow the correct models to be used

		pattern = hook::pattern("8B 7D 18 75 ? 80 B8 ? ? ? ? 02 75 ? 6A ? 68");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(11), (uint8_t)-1, true); // Allow the correct animations to be used
		
		Logging::Log() << "AllowMafiaLeonCutscenes enabled";
	}

	// Silence armored Ashley
	if (cfg.bSilenceArmoredAshley)
	{
		auto pattern = hook::pattern("75 ? 83 FF ? 77 ? 0F B6 87 ? ? ? ? FF 24 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(0), (uint8_t)0xEB, true); // jne -> jmp

		Logging::Log() << "SilenceArmoredAshley enabled";
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
						" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' .ini file.";

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
	else
		Logging::Log() << "User decided to ignore the FPS warning";

	// Unlock JP-only classic camera angle during Ashley segment
	static uint8_t* pSys = nullptr;
	struct UnlockAshleyJPCameraAngles
	{
		void operator()(injector::reg_pack& regs)
		{
			bool unlock = *(uint32_t*)(pSys + 8) == 0 // pSys->language_8
				|| cfg.bAshleyJPCameraAngles;

			// set zero-flag if we're unlocking the camera, for the jz game uses after this hook
			if (unlock)
				regs.ef |= (1 << regs.zero_flag);
			else
				regs.ef &= ~(1 << regs.zero_flag); // clear zero_flag if unlock is false, in case it was set by something previously
		}
	};

	pattern = hook::pattern("8B 0D ? ? ? ? 80 79 08 00 75 ? 8A 80 A3 4F 00 00");
	pSys = *pattern.count(1).get(0).get<uint8_t*>(2);

	injector::MakeInline<UnlockAshleyJPCameraAngles>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));

	if (cfg.bAshleyJPCameraAngles)
		Logging::Log() << "AshleyJPCameraAngles enabled";

	// Option to skip the intro logos when starting up the game
	if (cfg.bSkipIntroLogos)
	{
		pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);

		Logging::Log() << "SkipIntroLogos enabled";
	}

	// Restore missing transparency in the item pickup screen by
	// removing a call to GXCopyTex inside ItemExamine::gxDraw
	pattern = hook::pattern("E8 ? ? ? ? D9 05 ? ? ? ? D9 7D ? 6A ? 0F B7 45");
	ptrGXCopyTex = injector::GetBranchDestination(pattern.get_first(0)).as_int();
	ptrAfterItemExamineHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
	injector::MakeNOP(pattern.get_first(0), 5);
	injector::MakeJMP(pattern.get_first(0), ItemExamineHook, true);

	if (cfg.bRestorePickupTransparency)
		Logging::Log() << "RestorePickupTransparency enabled";

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
  
	// Enable what was leftover from the dev's debug menu (called "ToolMenu")
	if (cfg.bEnableDebugMenu)
	{
		Init_ToolMenu();
		Init_ToolMenuDebug(); // mostly hooks for debug-build tool menu, but also includes hooks to slow down selection cursor

		Logging::Log() << "EnableDebugMenu applied";
	}

	// Add Handgun silencer to merchant sell list
	if (cfg.bAllowSellingHandgunSilencer)
	{
		auto pattern = hook::pattern("DB 00 AC 0D 01 00 FF FF 00 00 00 00 00 00 00 00 00 00 0B 01");

		struct PRICE_INFO // name from PS2/VR
		{
			unsigned short item_id_0;
			unsigned short price_2; // gets multiplied by 100 or 50
			unsigned short valid_4; // set to 1 on valid items? not sure if checked
		};

		// g_item_price_tbl has just enough room at the end to include 1 more item, so we'll add silencer to it there
		PRICE_INFO* g_item_price_tbl_130 = pattern.count(1).get(0).get<PRICE_INFO>(0);
		g_item_price_tbl_130[1].item_id_0 = 0x3F;
		g_item_price_tbl_130[1].price_2 = 3000; // buy price will be 30000, sell price should end up as 15000 - fair for a easter-egg item like this?
		g_item_price_tbl_130[1].valid_4 = 1;
		g_item_price_tbl_130[2].item_id_0 = 0xFFFF; // add new record end marker
		g_item_price_tbl_130[2].price_2 = 0;
		g_item_price_tbl_130[2].valid_4 = 0;

		// Add 1 to price table count
		pattern = hook::pattern("83 00 00 00 78 00 00 00");
		uint32_t* g_item_price_tbl_num = pattern.count(1).get(0).get<uint32_t>(0);
		*g_item_price_tbl_num += 1;

		Logging::Log() << "AllowSellingHandgunSilencer enabled";
	}

	// Allow physics to apply to tactical vest outfit
	// Some reason they specifically excluded that outfit inside the physics functions, maybe there's an issue with it somewhere
	// (Raz0r trainer overwrites this patch every frame for some reason, too bad)
	{
		auto pattern = hook::pattern("80 B8 C9 4F 00 00 02 74");
		injector::MakeNOP(pattern.count(2).get(0).get<uint8_t>(7), 2);
		injector::MakeNOP(pattern.count(2).get(1).get<uint8_t>(7), 2);
	}

	// Log current game language
	{
		auto pattern = hook::pattern("88 81 ? ? ? ? C3 8B FF");
		struct GameLangLog
		{
			void operator()(injector::reg_pack& regs)
			{
				*(uint8_t*)(regs.ecx + 0x4FA3) = (uint8_t)regs.eax;

				switch ((uint8_t)regs.eax)
				{
				case 2:
					game_lang = "English";
					break;				
				case 3:
					game_lang = "German";
					break;
				case 4:
					game_lang = "French";
					break;
				case 5:
					game_lang = "Spanish";
					break;
				case 6:
					game_lang = "Traditional Chinese";
					break;
				case 7:
					game_lang = "Simplified Chinese";
					break;
				case 8:
					game_lang = "Italian";
					break;
				default:
					game_lang = "Unknown"; // We should never reach this.
				}

				#ifdef VERBOSE
				con.AddConcatLog("Game language: ", game_lang);
				#endif

				Logging::Log() << "Game language: " << game_lang;
			}
		}; 
		
		// Japanese exe doesn't have the setLanguage function, so we end up hooking nothing
		if (pattern.size() != 1)
		{
			#ifdef VERBOSE
			con.AddLogChar("Game language: Japanese");
			#endif

			Logging::Log() << "Game language: Japanese";
		}
		else
			injector::MakeInline<GameLangLog>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
}

void StorePath(HMODULE hModule)
{
	// Store wrapper name
	char configname[MAX_PATH];
	GetModuleFileNameA(hModule, configname, MAX_PATH);
	WrapperName.assign(strrchr(configname, '\\') + 1);
	std::transform(WrapperName.begin(), WrapperName.end(), WrapperName.begin(), ::tolower);

	// Store root path
	std::string modulePath = configname;
	rootPath = modulePath.substr(0, modulePath.rfind('\\') + 1);
}

void LoadRealDLL(HMODULE hModule)
{
	// Get wrapper mode
	const char* RealWrapperMode = Wrapper::GetWrapperName((WrapperMode.size()) ? WrapperMode.c_str() : WrapperName.c_str());

	const char* wrappedDllPath = nullptr;
	if (cfg.sWrappedDllPath.size())
	{
		wrappedDllPath = cfg.sWrappedDllPath.c_str();
		// User has specified a DLL to wrap, make sure it exists first:
		if (GetFileAttributesA(wrappedDllPath) == 0xFFFFFFFF)
			wrappedDllPath = nullptr;
	}
	
	proxy_dll = Wrapper::CreateWrapper(wrappedDllPath, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr, WrapperName.c_str());
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		g_module_handle = hModule;

		StorePath(hModule);

		Init_Logging();

		cfg.ReadSettings();

		LoadRealDLL(hModule);

		Init_Main();

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
