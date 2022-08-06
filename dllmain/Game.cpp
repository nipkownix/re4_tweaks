#include "dllmain.h"
#include "Game.h"
#include "Patches.h"

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

uint32_t* ptrLastUsedDevice = nullptr;
InputDevices LastUsedDevice()
{
	return *(InputDevices*)(ptrLastUsedDevice);
}

bool isController()
{
	return ((LastUsedDevice() == InputDevices::DinputController) || (LastUsedDevice() == InputDevices::XinputController));
}

bool isKeyboardMouse()
{
	return ((LastUsedDevice() == InputDevices::Keyboard) || (LastUsedDevice() == InputDevices::Mouse));
}

uint32_t* ptrMouseSens = nullptr;
int g_MOUSE_SENS()
{
	return *(int8_t*)(ptrMouseSens);
}

uint32_t* ptrMouseAimMode = nullptr;
MouseAimingModes GetMouseAimingMode()
{
	return *(MouseAimingModes*)(ptrMouseAimMode);
}

void SetMouseAimingMode(MouseAimingModes newMode)
{
	*(int8_t*)(ptrMouseAimMode) = (int8_t)newMode;
	return;
}

static uint32_t* ptrKey_btn_on;
uint64_t  Key_btn_on()
{
	return *(uint64_t*)(ptrKey_btn_on);
}

static uint32_t* ptrKey_btn_trg;
uint64_t  Key_btn_trg()
{
	return *(uint64_t*)(ptrKey_btn_trg);
}

GLOBALS** pG_ptr = nullptr;
GLOBALS* GlobalPtr()
{
	if (!pG_ptr)
		return nullptr;

	return *pG_ptr;
}

DAMAGE* pD_ptr = nullptr;
DAMAGE* DamagePtr() {
	if (!pD_ptr)
		return nullptr;

	return pD_ptr;
}

SYSTEM_SAVE** pSys_ptr = nullptr;
SYSTEM_SAVE* SystemSavePtr()
{
	if (!pSys_ptr)
		return nullptr;

	return *pSys_ptr;
}

cPlayer** pPL_ptr = nullptr;
cPlayer* PlayerPtr()
{
	if (*pPL_ptr == nullptr)
		return nullptr;

	if (**(int**)pPL_ptr < 0x10000)
		return nullptr;

	return *pPL_ptr;
}

cPlayer** pAS_ptr = nullptr;
cPlayer* AshleyPtr()
{
	if (*pAS_ptr == nullptr)
		return nullptr;

	if (**(int**)pAS_ptr < 0x10000)
		return nullptr;

	return *pAS_ptr;
}

uint8_t** g_GameSave_BufPtr = nullptr;
uint8_t* GameSavePtr()
{
	if (!g_GameSave_BufPtr)
		return nullptr;

	return *g_GameSave_BufPtr;
}

bool IsGanado(int id) // same as games IsGanado func
{
	if (id == 0x4B || id == 0x4E)
		return 0;
	if ((unsigned int)(id - 0x40) <= 0xF)
		return 1;
	if (id < 0x10)
		return 0;
	return id <= 0x20;
}

// Original game funcs
bool(__cdecl* game_KeyOnCheck_0)(KEY_BTN a1);
void(__cdecl* game_C_MTXOrtho)(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far);

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
		::MessageBoxA(NULL, "This version of RE4 is not supported.\nre4_tweaks will be disabled.", "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
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

	// LastUsedDevice pointer
	pattern = hook::pattern("A1 ? ? ? ? 85 C0 74 ? 83 F8 ? 74 ? 81 F9");
	ptrLastUsedDevice = *pattern.count(1).get(0).get<uint32_t*>(1);

	// g_MOUSE_SENS pointer
	pattern = hook::pattern("0F B6 05 ? ? ? ? 89 85 ? ? ? ? DB 85 ? ? ? ? DC 35");
	ptrMouseSens = *pattern.count(1).get(0).get<uint32_t*>(3);

	// Mouse aiming mode pointer
	pattern = hook::pattern("80 3D ? ? ? ? ? 0F B6 05");
	ptrMouseAimMode = *pattern.count(1).get(0).get<uint32_t*>(2);

	// Pointer to Key_btn_on
	pattern = hook::pattern("A1 ? ? ? ? 33 C9 83 E0 ? 83 3D");
	ptrKey_btn_on = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Pointer to Key_btn_trg
	pattern = hook::pattern("A1 ? ? ? ? 25 ? ? ? ? 0B C1 74 ? 88 0D");
	ptrKey_btn_trg = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Grab pointer to pG (pointer to games Global struct)
	pattern = hook::pattern("A1 ? ? ? ? B9 FF FF FF 7F 21 48 ? A1");
	pG_ptr = *pattern.count(1).get(0).get<GLOBALS**>(1);

	// pDamage pointer
	pattern = hook::pattern("8A 8B C3 4F 00 00 89 45 0C");
	pD_ptr = *pattern.count(1).get(0).get<DAMAGE*>(-0xB);

	// pSys pointer
	pattern = hook::pattern("00 80 00 00 83 C4 ? E8 ? ? ? ? A1 ? ? ? ?");
	pSys_ptr = *pattern.count(1).get(0).get<SYSTEM_SAVE**>(13);

	// pPL pointer
	pattern = hook::pattern("A1 ? ? ? ? D8 CC D8 C9 D8 CA D9 5D ? D9 45 ?");
	pPL_ptr = *pattern.count(1).get(0).get<cPlayer**>(1);

	// pAS pointer
	pattern = hook::pattern("A8 02 74 16 8B 15");
	pAS_ptr = *pattern.count(1).get(0).get<cPlayer**>(6);

	// g_GameSave_BufPtr pointer (not actual name)
	pattern = hook::pattern("89 15 ? ? ? ? C7 05 ? ? ? ? A0 FA 0F 00");
	g_GameSave_BufPtr = *pattern.count(1).get(0).get<uint8_t**>(2);

	// Pointer to the original KeyOnCheck
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 53");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), game_KeyOnCheck_0);

	return true;
}
