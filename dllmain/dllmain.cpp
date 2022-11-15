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

std::string WrapperMode;
std::wstring wrapperName;
std::wstring rootPath;
std::wstring logPath;

HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;
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
	if (!Init_Game())
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
	pConfig->ReadSettings();

	// Log re4_tweaks settings
	pConfig->LogSettings();
	
	// Install input-related hooks
	pInput->InstallHooks();

	// Parse any special command-line options
	Init_CommandLine();

	// Check for re4_tweaks updates
	CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&updateCheck, NULL, 0, NULL);

	// Various display-related tweaks
	Init_DisplayTweaks();

	Init_AudioTweaks();

	Init_CameraTweaks();

	Init_KeyboardMouseTweaks();

	Init_ControllerTweaks();

	// Install a WndProc hook and register the resulting hWnd for input
	Init_WndProcHook();

	// Increase some game limits
	Init_HandleLimits();

	// QTE-related changes
	Init_QTEfixes();

	// Fixes FPS-related issues
	Init_60fpsFixes();

	Init_Misc();

	if (pConfig->bEnableModExpansion)
		Init_ModExpansion();

	Init_MathReimpl();

	Init_DebugDisplay();

	Trainer_Init();

	// Apply changes needed by the HD Project
	if (bIsUsingHDProject)
		Init_HDProject();
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

		Init_ExceptionHandler();

		Init_Main();

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
