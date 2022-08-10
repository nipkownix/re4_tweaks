#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "..\wrappers\wrapper.h"
#include "Settings.h"
#include "Game.h"
#include "input.hpp"
#include "gitparams.h"
#include "resource.h"
#include "Trainer.h"

std::string WrapperMode;
std::string WrapperName;
std::string rootPath;
std::string logPath;

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
	spd::log()->info("Running from: \"{}\"", rootPath);
	spd::log()->info("Game version: {}", GameVersion());

	// Populate keymap (needed before we ReadSettings(), otherwise, parsing hotkeys will fail)
	pInput->PopulateKeymap();

	// Read re4_tweaks settings
	pConfig->ReadSettings();

	// Log re4_tweaks settings
	pConfig->LogSettings();
	
	// Install input-related hooks
	pInput->InstallHooks();

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

	Trainer_Init();

	// Apply changes needed by the HD Project
	if (bIsUsingHDProject)
		Init_HDProject();
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

	// Store log path
	logPath = rootPath + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".log";
}

void LoadRealDLL(HMODULE hModule)
{
	// Get wrapper mode
	const char* RealWrapperMode = Wrapper::GetWrapperName((WrapperMode.size()) ? WrapperMode.c_str() : WrapperName.c_str());

	const char* wrappedDllPath = nullptr;

	// Read WrappedDLLPath from .ini file
	std::string iniPath = rootPath + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".ini";

	char userDllPath[MAX_PATH] = {};

	GetPrivateProfileStringA("MISC", "WrappedDLLPath", "", userDllPath, 256, iniPath.c_str());

	if (strlen(userDllPath) > 0)
	{
		char result[MAX_PATH] = {};

		strcat(result, rootPath.c_str());
		strcat(result, userDllPath);

		wrappedDllPath = result;

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

		LoadRealDLL(hModule);

		Init_ExceptionHandler();

		Init_Main();

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}
