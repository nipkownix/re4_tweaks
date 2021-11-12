#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\MinHook\MinHook.h"
#include "game_flags.h"

extern std::string game_version;

// Funcs called by debug menu stuff
typedef void(__cdecl* DbMenuExec_Fn)();
typedef void(__cdecl* GameTask_callee_Fn)(void* a1);
typedef void* (__cdecl* GXLoadImage_Fn)(const char* path);
typedef void(__cdecl* GX_ScreenDisp_Fn)(void* a1, float a2, float a3, float a4, float a5, float a6, float a7, float a8, float a9, int a10);
typedef uint32_t(__cdecl* DvdReadN_Fn)(const char* a1, int a2, int a3, int a4, int a5, __int16 a6);
typedef int(__fastcall* cDvd__ReadCheck_Fn)(void* thisptr, void* unused, uint32_t a2, void* a3, void* a4, void* a5);
typedef int(__fastcall* cDvd__FileExistCheck_Fn)(void* thisptr, void* unused, const char* path, void* a2);
typedef void(__cdecl* CardSave_Fn)(uint8_t a1, char a2);
typedef void(*FlagEdit_die_Fn)();

DbMenuExec_Fn DbMenuExec = nullptr;
GameTask_callee_Fn GameTask_callee_Orig = nullptr;
GXLoadImage_Fn GXLoadImage = nullptr;
GX_ScreenDisp_Fn GX_ScreenDisp = nullptr;
DvdReadN_Fn DvdReadN = nullptr;
cDvd__ReadCheck_Fn cDvd__ReadCheck = nullptr;
cDvd__FileExistCheck_Fn cDvd__FileExistCheck = nullptr;
FlagEdit_die_Fn FlagEdit_die = nullptr;
CardSave_Fn CardSave = nullptr;

uintptr_t pG_ptr;
uintptr_t pSys_ptr;
uintptr_t pPL_ptr; // cPlayer*

uint32_t* DbgFontColorArray = nullptr;
uint32_t* PadButtonStates = nullptr;
uint32_t* FlagEdit_Backup_pG = nullptr;
void* cDvd = nullptr;
uint32_t* roomInfoAddr = nullptr;

struct ToolMenuEntry
{
	const char* Name;
	uint32_t Unk4;
	void* FuncPtr;
	uint32_t UnkC;
};
ToolMenuEntry* ToolMenuEntries = nullptr;

// Button definitions used by tool menus
enum class GamePadButton
{
	DPad_Left = 1,
	DPad_Right = 2,
	DPad_Down = 4,
	DPad_Up = 8,
	RT = 0x10,
	RB = 0x20,
	LB = 0x40,
	LT = 0x80,
	A = 0x100,
	B = 0x200,
	X = 0x400,
	Y = 0x800,
	Start = 0x1000,
	Back = 0x2000,
	RS = 0x4000,
	LS = 0x8000,
	LS_Left = 0x10000,
	LS_Right = 0x20000,
	LS_Down = 0x40000,
	LS_Up = 0x80000,
	RS_Left = 0x100000,
	RS_Right = 0x200000,
	RS_Down = 0x400000,
	RS_Up = 0x800000,
};


// Funcs hooked for debug menu stuff
typedef void(*MenuTask_Fn)();
typedef void(*systemRestartInit_Fn)();
typedef void* (__cdecl* titleInit_Fn)(void* a1);
typedef void(__cdecl* FlagEdit_main_Fn)(void* a1);

// restore code that loads dbgfont.tga texture
// 0x6DB765 into debug EXE
// DbgFont var doesn't seem to be anywhere in non-dbg EXE, so we'll define our own here
// the dbg EXE includes code in titleExit to unload DbgFont (at 0x6DD944), but we probably don't need to bother with it
void* DbgFont = nullptr;
titleInit_Fn titleInit_Orig = nullptr;
void* __cdecl titleInit_Hook(void* a1)
{
	void* ret = titleInit_Orig(a1);

	DbgFont = GXLoadImage("sv\\dbgfont.tga");

	return ret;
}

// restore screen-display part of eprintf
// calls into some GX-layer related function, luckily that func was left (unused) in the final EXE, seems to work fine
// 0x6DDC30 into debug EXE
void __cdecl eprintf_ScreenDisp(const char* str, float x, float y, int color)
{
	int length = strlen(str);

	double v7 = 0.9900000095367432;
	double v8 = 0.1428571492433548;
	double v9 = 0.0625;
	for (int i = 0; i < length; i++)
	{
		char ch = str[i] - 0x20;
		if (uint8_t(ch) > 0x61)
			continue;

		float v21 = (float)(ch / 0x10);
		float v15 = (float)(ch & 0xF);

		float v18 = v15 * v9;
		float v17 = v21 * v8;
		float v16 = (v15 + v7) * v9;
		float v20 = (v21 + v7) * v8;

		GX_ScreenDisp(DbgFont, x + (i * 8), y, 12.0, 18.0, v18, v17, v16, v20, color);
	}
}
char eprintf_buffer[0x80];
void eprintf_Hook(int a1, int a2, int a3, int a4, char* Format, ...)
{
	va_list va;
	va_start(va, Format);

	vsprintf_s(eprintf_buffer, Format, va);

	eprintf_ScreenDisp(eprintf_buffer, float(a1), float(a2 + 64), DbgFontColorArray[a3]);
}

#pragma pack(push, 1)
struct cPlayer // unsure if correct name!
{
	/* 0x000 */ uint8_t unk0[0x94];
	/* 0x094 */ float X;
	/* 0x098 */ float Y;
	/* 0x09C */ float Z;
	/* 0x0A0 */ float unkA0;
	/* 0x0A4 */ float Angle;
	/* goes to 7F0+ */
};
#pragma pack(pop)

// recreate gameDebug function from debug EXE, opens tool menu when button combo is pressed
// 0x63C610 into debug EXE
uint32_t PrevKeyboardState = 0;
bool ShowInfoDisplay = false;
void __cdecl gameDebug_recreation(void* a1)
{
	// Check pressed keys & emulate controller presses for tool-menu
	uint32_t keyboardState = 0;
	if (GetAsyncKeyState(VK_HOME))
		keyboardState |= uint32_t(GamePadButton::LT);
	if (GetAsyncKeyState(VK_END))
		keyboardState |= uint32_t(GamePadButton::LS);

	// Only emulate main keys if tool-menu is actually open
	uint8_t* pG = *(uint8_t**)pG_ptr;
	bool inDebugMenu = (*(uint32_t*)(pG + 0x60) & 0x80000000) != 0;
	if (inDebugMenu)
	{
		if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W'))
			keyboardState |= uint32_t(GamePadButton::DPad_Up);
		if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S'))
			keyboardState |= uint32_t(GamePadButton::DPad_Down);
		if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A'))
			keyboardState |= uint32_t(GamePadButton::DPad_Left);
		if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D'))
			keyboardState |= uint32_t(GamePadButton::DPad_Right);
		if (GetAsyncKeyState(VK_RETURN) || GetAsyncKeyState('F'))
			keyboardState |= uint32_t(GamePadButton::A);
		if (GetAsyncKeyState(VK_ESCAPE) || GetAsyncKeyState(VK_BACK))
			keyboardState |= uint32_t(GamePadButton::B);
		if (GetAsyncKeyState(VK_DELETE) || GetAsyncKeyState('Q'))
			keyboardState |= uint32_t(GamePadButton::LB);
		if (GetAsyncKeyState(VK_NEXT) || GetAsyncKeyState('E')) // pagedown
			keyboardState |= uint32_t(GamePadButton::RB);
	}

	if (keyboardState != PrevKeyboardState)
	{
		PadButtonStates[0] |= keyboardState;

		// toolmenu funcs check [4], [3] & [1] for some reason
		PadButtonStates[4] = PadButtonStates[3] = PadButtonStates[1] = PadButtonStates[0];

		PrevKeyboardState = keyboardState;
	}

	// Check for LT + LS buttons
	// (make sure they're the only ones pressed - if player opens a UI at same time it'll likely cause a hang...)
	bool isOnlyDebugComboPressed = PadButtonStates[0] == (uint32_t(GamePadButton::LT) | uint32_t(GamePadButton::LS));
	if (isOnlyDebugComboPressed)
	{
		// Only run DbMenuExec if debug menu isn't open already, otherwise game can hang
		if (!inDebugMenu)
			DbMenuExec();
	}

	if (ShowInfoDisplay)
	{
		cPlayer* pPL = *(cPlayer**)pPL_ptr;
		if (pPL)
			eprintf_Hook(32, 300, 0, 0, "[X %7.3f Y %7.3f Z %7.3f R %5.3f]", pPL->X, pPL->Y, pPL->Z, pPL->Angle);
	}

	GameTask_callee_Orig(a1);
}

MenuTask_Fn MenuTask_Orig = nullptr;
void MenuTask_Hook()
{
	uint8_t* pG = *(uint8_t**)pG_ptr;
	uint32_t* flags_DEBUG = (uint32_t*)(pG + 0x60);

	uint32_t offset = 0;
	uint32_t value = GetFlagValue(uint32_t(Flags_DEBUG::DBG_TEST_MODE), offset);
	flags_DEBUG[offset / 4] |= value;

	MenuTask_Orig();
}

uint32_t PrevButtons = 0;
FlagEdit_main_Fn FlagEdit_main_Orig = nullptr;
void FlagEdit_main_Hook(void* a1)
{
	uint32_t curButtons = PadButtonStates[3];
	PadButtonStates[3] = 0;

	if (PrevButtons != curButtons)
		PadButtonStates[3] = curButtons;

	FlagEdit_main_Orig(a1);

	PrevButtons = curButtons;
	PadButtonStates[3] = curButtons;
}

// Add code to read debug/roominfo.dat file into roomInfoAddr variable
// (debug EXE is missing this code, which stops AREA JUMP from working on that!)
// unfortunately the roominfo.dat provided with RE4 steam is missing the strings for some reason
// however, roominfo.gc contains the exact strings needed, even at the correct offsets etc!
// that file is endian-swapped though so can't be used directly...
// but the strings can be easily copied over by copying 0x2984-EOF over to roominfo.dat :)

const char* RoomInfoFileName = "debug/roomInfo_new.dat";

extern uint8_t* roomInfoDat; // replacement roomInfo inside roomInfo.cpp

typedef void(*systemRestartInit_Fn)();
systemRestartInit_Fn systemRestartInit_Orig = nullptr;
void systemRestartInit_Hook()
{
	systemRestartInit_Orig();

	// If roomInfo_new.dat exists we'll read from that instead (so users can still customize it if wanted)
	// But instead of using broken debug\roomInfo.data included with Steam UHD, we'll use our custom fixed one instead

	// not entirely sure how FileExistCheck works, game checks for 0xFFFFFFFF meaning doesn't exist, but it never returns that for us?
	// a2 seems to get set to 0xffffff if non-existing though, so we'll also check that too
	// (need to make sure file exists before calling DvdReadN on it, else game will hang if file doesn't exist first...)
	int a2 = 0;
	auto res = cDvd__FileExistCheck(cDvd, 0, RoomInfoFileName, &a2);
	if (res < 0 || a2 == 0xffffff)
	{
		// roomInfo_new.dat not found, use our fixed one
		*roomInfoAddr = (uint32_t)&roomInfoDat;
	}
	else
	{
		// Try reading roomInfo.dat from roomInfo_new.dat instead, so users can still customize it if wanted
		uint32_t handle = DvdReadN(RoomInfoFileName, 0, 0, 0, 0, 5);
		if (cDvd__ReadCheck(cDvd, 0, handle, 0, 0, roomInfoAddr) < 0)
			*roomInfoAddr = 0;
	}
}

void ToolMenu_Exit()
{
	uint8_t* pG = *(uint8_t**)pG_ptr;

	// Exit tool-menu state via FlagEdit_die
	// FlagEdit_die overwrites pG+0x170 with contents of 0xC6D008, so we need to write that first...
	*FlagEdit_Backup_pG = *(uint32_t*)(pG + 0x170);
	FlagEdit_die();

	// alternatively "TaskChain(MenuTask, 0);" should return us to tool-menu
	// but that requires another funcptr to track...
}

void ToolMenu_GoldMax()
{
	uint8_t* pG = *(uint8_t**)pG_ptr;
	*(uint32_t*)(pG + 0x4FA8) = 99999999;

	ToolMenu_Exit();
}

void ToolMenu_SecretOpen()
{
	uint8_t* pSys = *(uint8_t**)pSys_ptr;
	uint32_t* flags_EXTRA = (uint32_t*)(pSys + 4);

	// debug exe ORs these seperately, probably part of an enum or something
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_AIM_REVERSE));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_WIDE_MODE));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_LOCK_ON));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_BONUS_GET));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_VIBRATION));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_KNIFE_MODE));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_COSTUME));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_HARD_MODE));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_SW500));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_TOMPSON));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_ADA));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_HUNK));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_KLAUSER));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_WESKER));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_OMAKE_ADA_GAME));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_GET_OMAKE_ETC_GAME));

	ToolMenu_Exit();
}

void ToolMenu_MercenariesAllOpen()
{
	uint8_t* pSys = *(uint8_t**)pSys_ptr;
	uint32_t* flags_EXTRA = (uint32_t*)(pSys + 4);

	// TODO: confirm whether these are what this option actually unlocks in the debug build
	// (as these unlocks seem kinda strange)
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_AIM_REVERSE));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_VIBRATION));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::CFG_KNIFE_MODE));
	*flags_EXTRA |= GetFlagValue(uint32_t(Flags_EXTRA::EXT_COSTUME));

	ToolMenu_Exit();
}

const char* ToolMenu_UnusedName = "";

const char* ToolMenu_ToggleInfoDispName = "TOGGLE INFO Disp";
void ToolMenu_ToggleInfoDisp()
{
	ShowInfoDisplay = !ShowInfoDisplay;
	ToolMenu_Exit();
}

const char* ToolMenu_ToggleHUDName = "TOGGLE HUD";
void ToolMenu_ToggleHUD()
{
	uint8_t* pG = *(uint8_t**)pG_ptr;
	uint32_t* flags_DISP = (uint32_t*)(pG + 0x58);

	uint32_t offset = 0;
	int value = GetFlagValue(uint32_t(Flags_DISP::DPF_COCKPIT), offset);
	flags_DISP[offset / 4] ^= value;

	ToolMenu_Exit();
}

const char* ToolMenu_SaveGameName = "SAVE GAME";
void ToolMenu_SaveGame()
{
	CardSave(0, 1);

	ToolMenu_Exit();
}

void ToolMenu_GetPointers()
{
	auto pattern = hook::pattern("A1 ? ? ? ? 81 48 ? 00 00 00 80 A1 ? ? ? ? 8B 88 ? ? ? ?");
	DbMenuExec = (DbMenuExec_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	pattern = hook::pattern("53 E8 ? ? ? ? 8D 4D FC 51 8D 55 F8");
	auto fnCallPtr = pattern.count(1).get(0).get<uint8_t>(2);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GameTask_callee_Orig = (GameTask_callee_Fn)fnCallPtr;

	pattern = hook::pattern("55 8B EC 83 EC ? 56 8B 75 ? 56 E8 ? ? ? ? 50 68 ? ? ? ? 68 00 01 00 00 68 ? ? ? ?");
	GXLoadImage = (GXLoadImage_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	pattern = hook::pattern("D9 45 ? 03 C1 BA 30 00 00 00");
	GX_ScreenDisp = (GX_ScreenDisp_Fn)pattern.count(1).get(0).get<uint8_t>(-0x2A);

	pattern = hook::pattern("55 8B EC 56 6A 58 68 ? ? ? ? E8 ? ? ? ? 8B 45 ? 50 68 ? ? ? ?");
	DvdReadN = (DvdReadN_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	pattern = hook::pattern("55 8B EC 8B 55 ? 81 EC ? ? ? ? 8D 85 ? ? ? ? 50 52 E8 ? ? ? ? 83 F8 01 75");
	cDvd__ReadCheck = (cDvd__ReadCheck_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	pattern = hook::pattern("55 8B EC A1 ? ? ? ? 83 EC ? F7 40 54 00 00 02 00 74");
	cDvd__FileExistCheck = (cDvd__FileExistCheck_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	pattern = hook::pattern("55 8B EC 8A 45 0C A8 02 74 ?");
	CardSave = (CardSave_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	pattern = hook::pattern("A1 ? ? ? ? B9 FF FF FF 7F 21 48 ? A1");
	FlagEdit_die = (FlagEdit_die_Fn)pattern.count(1).get(0).get<uint8_t>(0);
	pG_ptr = *(uintptr_t*)pattern.count(1).get(0).get<uint8_t>(1);
	FlagEdit_Backup_pG = *(uint32_t**)pattern.count(1).get(0).get<uint8_t>(0x14);

	pattern = hook::pattern("FF FF FF FF FF FF 00 00 FF 00");
	DbgFontColorArray = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("99 56 8B 35 ? ? ? ? 89 55");
	auto varPtr = pattern.count(1).get(0).get<uint32_t>(4);
	PadButtonStates = (uint32_t*)*varPtr;

	pattern = hook::pattern("6A 40 52 B9 ? ? ? ? E8");
	varPtr = pattern.count(1).get(0).get<uint32_t>(4);
	cDvd = (void*)*varPtr;

	pattern = hook::pattern("FF BF FF FF 6A 04 E8 ? ? ? ? 83 C4 ? 85 C0 74 ? 8B 15 ? ? ? ?");
	varPtr = pattern.count(1).get(0).get<uint32_t>(20);
	roomInfoAddr = (uint32_t*)*varPtr;

	pattern = hook::pattern("00 80 00 00 83 C4 ? E8 ? ? ? ? A1 ? ? ? ?");
	varPtr = pattern.count(1).get(0).get<uint32_t>(13);
	pSys_ptr = (uintptr_t)*varPtr;

	pattern = hook::pattern("53 56 33 C0 BE ? ? ? ? 8D A4 24 00 00 00 00");
	varPtr = pattern.count(1).get(0).get<uint32_t>(5);
	ToolMenuEntries = (ToolMenuEntry*)*varPtr;

	pattern = hook::pattern("A1 ? ? ? ? D8 CC D8 C9 D8 CA D9 5D ? D9 45 ?");
	pPL_ptr = *pattern.count(1).get(0).get<uintptr_t>(1);
}

void ToolMenu_ApplyHooks()
{
	// Hook systemRestartInit so we can make it load in roomInfo.dat
	auto pattern = hook::pattern("53 56 E8 ? ? ? ? E8 ? ? ? ? 6A 01 E8 ? ? ? ? A1 ? ? ? ? 83 C0 ? 68");
	MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), systemRestartInit_Hook, (LPVOID*)&systemRestartInit_Orig);

	// Hook FlagEdit so we can slow it down by only exposing buttons to it when button state has changed at all
	pattern = hook::pattern("55 8B EC 56 8B 75 ? 57 33 FF F6 05");
	MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), FlagEdit_main_Hook, (LPVOID*)&FlagEdit_main_Orig);

	// If this isn't a debug build, add hooks to allow debug menu to be activated
	if (game_version.back() != 'd')
	{
		// hook GameTask_callee call to run our gameDebug recreation
		pattern = hook::pattern("53 E8 ? ? ? ? 8D 4D FC 51 8D 55 F8");
		injector::MakeCALL(pattern.count(1).get(0).get<uint32_t>(1), gameDebug_recreation, true);

		// Remove bzero call that clears flags every frame for some reason
		pattern = hook::pattern("83 C0 60 6A 10 50 E8");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(6), 5, true);

		// Hook MenuTask (task for tool-menu), since it seems the menu can sometimes be shown without the pG+0x60 flag being set
		// eg. when backing out of area-jump menu
		// (which could make game hang if this causes us to try opening tool-menu when it's already open...)
		pattern = hook::pattern("53 57 33 DB 53 E8 ? ? ? ? 6A 01 E8 ? ? ? ? 83 C4");
		MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), MenuTask_Hook, (LPVOID*)&MenuTask_Orig);

		// Hook eprintf to add screen-drawing to it, like the GC debug has
		// (required for debug-menu to be able to draw itself)
		pattern = hook::pattern("55 8B EC 8B 4D ? 8D 45 ? 50 51 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 5D C3");
		MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), eprintf_Hook, NULL);

		// Hook titleInit to make it load dbgFont for us
		pattern = hook::pattern("55 8B EC 53 68 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 33 DB 53 53 53 53 68 ? ? ? ? C6");
		MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), titleInit_Hook, (LPVOID*)&titleInit_Orig);
	}

	// Patches to change debug menu button bindings
	{
		// Allow tool-menu selection with X / A
		pattern = hook::pattern("E8 ? ? ? ? F7 05 ? ? ? ? 00 04 00 00");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(11), uint32_t(GamePadButton::X) | uint32_t(GamePadButton::A), true);
		// Change tool-menu return button to B / Back
		pattern = hook::pattern("F7 05 ? ? ? ? 00 12 00 00 BE FF FF FF 7F");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), uint32_t(GamePadButton::B) | uint32_t(GamePadButton::Back), true);
		// Allow flag toggle with X/A
		pattern = hook::pattern("83 C4 ? F7 05 ? ? ? ? 00 04 00 00 74 ? 8B");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(9), uint32_t(GamePadButton::X) | uint32_t(GamePadButton::A), true);
		// Change flag-menu return button to B/Back
		pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? F7 05 ? ? ? ? 00 01 00 00 5B");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(14), uint32_t(GamePadButton::B) | uint32_t(GamePadButton::Back), true);
		// Allow area jump with X/A/Start
		pattern = hook::pattern("83 C4 ? F7 05 ? ? ? ? 00 04 00 00 74 ? C6");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(9), uint32_t(GamePadButton::X) | uint32_t(GamePadButton::A) | uint32_t(GamePadButton::Start), true);
		// Change area jump return button to B/Back
		pattern = hook::pattern("C6 06 02 F7 05 ? ? ? ? 00 01 00 00 74");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(9), uint32_t(GamePadButton::B) | uint32_t(GamePadButton::Back), true);
	}

	// FlagEdit displays flags in wrong order, each 16-bits is displayed on the wrong line
	// (to be expected from a port from a big-endian machine like GC/Wii)
	// it reads flags in as 16-bit words, and then seems to have code to handle swapping the bits inside
	// since the flags are actually 32-bits, it reads the wrong part of that 32-bit dword
	// we can fix that by a simple xor though :)

	// very fortunately for us there's a useless instruction just before flag-reading code
	// patch it to "mov eax, edi; xor edi, 1; nop", to swap which part of 32-bit dword to read from
	{
		pattern = hook::pattern("33 FF 85 C0 0F 8E ? ? ? ? 8D 9B 00 00 00 00");

		uint8_t patchBytes[] = { 0x89, 0xf8, 0x83, 0xf0, 0x01, 0x90 };
		injector::WriteMemoryRaw(pattern.count(1).get(0).get<uint32_t>(10), patchBytes, 6, true);
		// patch read code to use eax register instead of edi
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(0x19), uint8_t(0x41), true);
		// patch loop to jump to start of our patched code
		pattern = hook::pattern("47 C1 F8 04 83 C4 24 3B F8 0F 8C");
		int32_t* jmpDest = pattern.count(1).get(0).get<int32_t>(11);
		injector::WriteMemory(jmpDest, *jmpDest - 6, true);
	}

	// Fixes for off-screen FlagEdit text
	{
		pattern = hook::pattern("6A 00 6A 06 6A F3");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(5), uint8_t(13), true);

		pattern = hook::pattern("6A 00 6A 00 6A 00 68 B8 00 00 00");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(5), uint8_t(13 + 13), true);

		pattern = hook::pattern("6A 00 6A 04 6A 00 68 08 01 00 00");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(5), uint8_t(13 + 13), true);

		pattern = hook::pattern("C1 F8 02 8D 44 38 01");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(6), uint8_t(3), true);

		pattern = hook::pattern("C1 EA 06 8D 54 0A 01");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(6), uint8_t(3), true);
	}

	// Allow RoomJump 1.1.0 to access stage 0 (test stages)
	if (game_version == "1.1.0")
	{
		pattern = hook::pattern("80 FB 05 7F ? 84 DB 7E ?");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(7), uint8_t(0x7C), true);
	}

	// Overwrite non-functional tool menu entries with replacements
	if (game_version.back() != 'd')
	{
		ToolMenuEntries[1].FuncPtr = &ToolMenu_GoldMax;
		ToolMenuEntries[15].FuncPtr = &ToolMenu_SecretOpen;
		ToolMenuEntries[29].FuncPtr = &ToolMenu_MercenariesAllOpen;

		// MUTEKI ON -> TOGGLE HUD
		ToolMenuEntries[2].Name = ToolMenu_ToggleHUDName;
		ToolMenuEntries[2].FuncPtr = &ToolMenu_ToggleHUD;

		//TODO: ToolMenuEntries[3].Name = ToolMenu_LightToolName;

		// MUGEN ON -> SAVE GAME
		ToolMenuEntries[4].Name = ToolMenu_SaveGameName;
		ToolMenuEntries[4].FuncPtr = &ToolMenu_SaveGame;

		// INFO Disp ON -> Toggle INFO Disp
		ToolMenuEntries[12].Name = ToolMenu_ToggleInfoDispName;
		ToolMenuEntries[12].FuncPtr = &ToolMenu_ToggleInfoDisp;

		// Clear non-functional options on non-debug builds
		if (game_version == "1.1.0" || game_version == "1.0.6")
		{
			for (int i = 0; i < 32; i++)
			{
				if (i == 0 || i == 1 || i == 2 || i == 4 || i == 12 || i == 15 || i == 16 || i == 29)
					continue;
				ToolMenuEntries[i].Name = ToolMenu_UnusedName;
				ToolMenuEntries[i].FuncPtr = nullptr; // some unused ones like "DEBUG OPTION" pointed to nullsub that'd hang game, null it out to prevent it
			}
		}
	}
}
