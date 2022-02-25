#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "..\Wrappers\wrapper.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "LoggingInit.h"
#include "Game.h"
#include "Patches.h"
#include "Logging/Logging.h"

std::string WrapperMode;
std::string WrapperName;
std::string rootPath;

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

	if (!Init_Game())
		return;

	Logging::Log() << "Game version = " << GameVersion();

	// Log re4_tweaks settings
	LogSettings();

	// Make sure steam_appid.txt exists
	HandleAppID();
	
	// Various display-related tweaks
	Init_DisplayTweaks();

	// Install input-related hooks
	Init_Input();

	// Install a WndProc hook and register the resulting hWnd for input
	Init_WndProcHook();

	// Increase some game limits
	Init_HandleLimits();

	// QTE-related changes
	Init_QTEfixes();

	// Fixes FPS-related issues
	Init_60fpsFixes();

	Init_Misc();

	Init_KeyboardMouseTweaks();

	Init_ControllerTweaks();

	Init_ExceptionHandler();
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
