#include "stdafx.h"
#include "dllmain.h"
#include "Game.h"
#include "ConsoleWnd.h"

std::string gameVersion;
bool gameIsDebugBuild = false;

std::string GameVersion()
{
	return gameVersion;
}

bool GameVersionIsDebug()
{
	return gameIsDebugBuild;
}

GLOBALS** pG_ptr = nullptr;
GLOBALS* GlobalPtr()
{
	if (!pG_ptr)
		return nullptr;

	return *pG_ptr;
}

SYSTEM_SAVE** pSys_ptr = nullptr;
SYSTEM_SAVE* SystemSavePtr()
{
	if (!pSys_ptr)
		return nullptr;

	return *pSys_ptr;
}

cPlayer** pPL_ptr = 0;
cPlayer* PlayerPtr()
{
	if (!pPL_ptr)
		return nullptr;

	return *pPL_ptr;
}

bool Init_Game()
{
	// Detect game version
	auto pattern = hook::pattern("31 2E ? ? ? 00 00 00 6D 6F 76 69 65 2F 64 65 6D 6F 30 65 6E 67 2E 73 66 64");
	int ver = injector::ReadMemory<int>(pattern.count(1).get(0).get<uint32_t>(2));

	if (ver == 0x362E30) {
		gameVersion = "1.0.6";
	}
	else if (ver == 0x302E31) {
		gameVersion = "1.1.0";
	}
	else {
		MessageBoxA(NULL, "This version of RE4 is not supported.\nre4_tweaks will be disabled.", "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		return false;
	}

	// Check for part of gameDebug function, if exists this must be a debug-enabled build
	pattern = hook::pattern("6A 00 6A 00 6A 08 68 AE 01 00 00 6A 10 6A 0A");
	if (pattern.size() > 0)
	{
		gameVersion += "d";
		gameIsDebugBuild = true;
	}

	#ifdef VERBOSE
	con.AddConcatLog("Game version = ", GameVersion().data());
	#endif

	// Grab pointer to pG (pointer to games Global struct)
	pattern = hook::pattern("A1 ? ? ? ? B9 FF FF FF 7F 21 48 ? A1");
	pG_ptr = *pattern.count(1).get(0).get<GLOBALS**>(1);

	// pSys pointer
	pattern = hook::pattern("00 80 00 00 83 C4 ? E8 ? ? ? ? A1 ? ? ? ?");
	pSys_ptr = *pattern.count(1).get(0).get<SYSTEM_SAVE**>(13);

	// pPL pointer
	pattern = hook::pattern("A1 ? ? ? ? D8 CC D8 C9 D8 CA D9 5D ? D9 45 ?");
	pPL_ptr = *pattern.count(1).get(0).get<cPlayer**>(1);

	return true;
}
