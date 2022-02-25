#include <iostream>
#include "stdafx.h"
#include "Game.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "Logging/Logging.h"
#include "Patches.h"

static uint32_t* ptrGameFrameRate;

void Init_Misc()
{
	// Remove savegame SteamID checks, allows easier save transfers
	auto pattern = hook::pattern("8B 88 40 1E 00 00 3B 4D ? 0F 85 ? ? ? ?");
	Nop(pattern.count(1).get(0).get<uint8_t>(0x9), 6);
	Nop(pattern.count(1).get(0).get<uint8_t>(0x18), 6);

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
		auto pattern = hook::pattern("CB 4F 00 00 02 75 ? 83 FF ? 77 ? 0F B6 87 ? ? ? ? FF 24 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(5), (uint8_t)0xEB, true); // jne -> jmp

		Logging::Log() << "SilenceArmoredAshley enabled";
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
				}
				else {
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
}