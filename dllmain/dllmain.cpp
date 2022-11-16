#include <iostream>
#include "dllmain.h"
#include "AutoUpdater.h"
#include "Patches.h"
#include "Settings.h"
#include "Game.h"
#include "input.hpp"
#include "gitparams.h"
#include "resource.h"
#include "Trainer.h"
#include "UI_DebugWindows.h"
#include "../wrappers/wrappers.h"

HMODULE g_module_handle = nullptr;

void HandleAppID()
{
	//Create missing steam_appid file
	if (!std::filesystem::exists("steam_appid.txt") && std::filesystem::exists("bio4.exe")) {
		std::ofstream appid("steam_appid.txt");
		appid << "254700";
		appid.close();
	}
}

void Init_Main()
{
	con.AddLogChar("Big ironic thanks to QLOC S.A.");

	// Get game pointers and version info
	if (!re4t::init::Game())
		return;
	
	// Make sure steam_appid.txt exists
	HandleAppID();

	// Initial logging
	std::string commit = GIT_CUR_COMMIT;
	commit.resize(8);

	spd::log()->info("Starting re4_tweaks v{0}-{1}-{2}", APP_VERSION, commit, GIT_BRANCH);
	spd::LogProcessNameAndPID();
	spd::log()->info("Running from: \"{}\"", WstrToStr(rootPath));
	spd::log()->info("Game version: {}", GameVersion());

	// Populate keymap (needed before we ReadSettings(), otherwise, parsing hotkeys will fail)
	pInput->PopulateKeymap();

	// Read re4_tweaks settings
	re4t::cfg->ReadSettings();

	// Log re4_tweaks settings
	re4t::cfg->LogSettings();
	
	// Install input-related hooks
	pInput->InstallHooks();

	// Parse any special command-line options
	re4t::init::CommandLine();

	// Check for re4_tweaks updates
	CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&updateCheck, NULL, 0, NULL);

	// Various display-related tweaks
	re4t::init::DisplayTweaks();

	re4t::init::AudioTweaks();

	re4t::init::CameraTweaks();

	re4t::init::KeyboardMouseTweaks();

	re4t::init::ControllerTweaks();

	// Install a WndProc hook and register the resulting hWnd for input
	re4t::init::WndProcHook();

	// Increase some game limits
	re4t::init::HandleLimits();

	// QTE-related changes
	re4t::init::QTEfixes();

	// Fixes FPS-related issues
	re4t::init::FrameRateFixes();

	re4t::init::Misc();

	if (re4t::cfg->bEnableModExpansion)
		re4t::init::ModExpansion();

	re4t::init::MathReimpl();

	re4t::init::DebugDisplay();

	Trainer_Init();

	// Apply changes needed by the HD Project
	if (re4t::cfg->bIsUsingHDProject)
		re4t::init::HDProject();
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		g_module_handle = hModule;

		Init_Wrappers();

		re4t::init::ExceptionHandler();

		Init_Main();

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
