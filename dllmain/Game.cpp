#include "dllmain.h"
#include "Game.h"
#include "Patches.h"
#include "SDK/filter00.h"

std::string gameVersion;
bool gameIsDebugBuild = false;

SND_CTRL* Snd_ctrl_work = nullptr; // extern inside Game.h
cLightMgr* LightMgr = nullptr; // extern inside light.h
cLightMgr__setEnv_Fn cLightMgr__setEnv = nullptr; // extern inside light.h
cFilter00Params* Filter00Params = nullptr; // extern inside filter00.h
cFilter00Params2* Filter00Params2 = nullptr; // extern inside filter00.h

// a lot missing from here sadly ;_;
std::unordered_map<int, std::string> UnitBeFlagNames =
{
	{ 0x1, "Alive" },
	{ 0x2, "Trans" },
	{ 0x8, "AltAmbientLightColor" }, // checked by LightSetModel, didn't notice any change
	{ 0x10, "DrawFootShadow" },
	{ 0x20, "Move" },
	{ 0x200, "Dead" }, // maybe dead flag? setting it seems to set Destruct flag, which unsets a bunch of flags including Alive
	{ 0x400, "Destruct" }, // checked by cManager<cEm>::dieCheck, calls destructor if set
	{ 0x800, "NoSuspend" }, // keep running even if suspend game flag is set
	{ 0x1000, "IgnoreDrawDistance" }, // looks like ModelTrans sets distance to 999999 if set
	{ 0x2000, "ContiguousParts" }, // optimization? checked by cModel::getPartsPtr
								   // seems to skip following nextParts_F4 pointers and accesses part by index directly if set
	{ 0x4000, "DisableAnimation" }, // code for this doesn't actually seem anim/motion related, but anims do get disabled by it...
	{ 0x8000, "DisableLighting" },
	{ 0x20000, "UseSimpleLighting" },
	{ 0x100000, "ClothNoScaleApply" }, // PenClothMove3
	{ 0x200000, "ClothReset" }, // PenClothMove3
	{ 0x1000000, "ApplyGlobalIllumination" }, // unsure, seems to adjust lighting color though
	//{ 0x8000000, "InvertShadowFlagBit6" }, // commonModelTrans, seems to invert check for flag 0x40 / bit6
};

std::unordered_map<int, std::string> EmNames =
{
	{0x00, "Player"},
	{0x50, "EmObj"},
	{0x51, "EmDoor"},
	{0x52, "EmWep"},
	{0x53, "EmBox"},
	//0x54 unused?
	{0x55, "EmRack"},
	{0x56, "EmWindow"},
	{0x57, "EmTorch"},
	{0x58, "EmBarrel"},
	{0x59, "EmTree"},
	{0x5A, "EmRock"},
	{0x5B, "EmSwitch"},
	{0x5C, "EmItem"},
	{0x5D, "EmHit"},
	{0x5E, "EmBarred"},
	{0x5F, "EmMine"},
	{0x60, "EmShield"},
	{0x61, "EmBar"},
};

std::string cEmMgr::EmIdToName(int id)
{
	if (EmNames.count(id))
		return EmNames[id];
	std::stringstream ss;
	ss << "Em" << std::setfill('0') << std::setw(2) << id;
	return ss.str();
}

std::string cUnit::GetBeFlagName(int flagIndex)
{
	int flagValue = (1 << flagIndex);
	if (UnitBeFlagNames.count(flagValue))
		return UnitBeFlagNames[flagValue];
	return "Bit" + std::to_string(flagIndex);
}

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

GLOBAL_WK** pG_ptr = nullptr;
GLOBAL_WK* GlobalPtr()
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

SYSTEM_SAVE_WORK** pSys_ptr = nullptr;
SYSTEM_SAVE_WORK* SystemSavePtr()
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

cEmMgr* EmMgr_ptr = nullptr;
cEmMgr* EmMgrPtr()
{
	return EmMgr_ptr;
}

TITLE_WORK* TitleWork_ptr = nullptr;
TITLE_WORK* TitleWorkPtr()
{
	return TitleWork_ptr;
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

const char* emlist_name[] = {
	"emleon00.esl",
	"emleon01.esl",
	"emleon02.esl",
	"emleon03.esl",
	"emleon04.esl",
	"emleon05.esl",
	"emleon06.esl",
	"emleon07.esl",
	"emleon08.esl",
	"emleon09.esl",
	"omake00.esl",
	"omake01.esl",
	"omake02.esl",
	"omake03.esl",
	"omake04.esl",
	"omake05.esl",
	"omake06.esl",
	"omake07.esl",
	"omake08.esl",
};

//
const char* emlist_enum_name[] = {
	"ST1_0",
	"ST1_1",
	"ST2_0",
	"ST2_1",
	"ST2_2",
	"ST2_3",
	"ST3_0",
	"ST3_1",
	"DUMMY",
	"BIO5",
	"ADA",
	"ETC",
	"ETC2",
	"PS2ST1",
	"PS2ST2",
	"PS2ST3",
	"PS2ST4",
	"PS2ST5",
	"PS2ETC"
};

const char* GetEmListName(int emListNumber)
{
	if (emListNumber >= 0 && emListNumber < 19)
		return emlist_name[emListNumber];
	return "unknown";
}

const char* GetEmListEnumName(int emListNumber)
{
	if (emListNumber >= 0 && emListNumber < 19)
		return emlist_enum_name[emListNumber];
	return "unknown";
}

// Original game funcs
bool(__cdecl* game_KeyOnCheck_0)(KEY_BTN a1);
void(__cdecl* game_C_MTXOrtho)(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far);


void* (__cdecl* mem_calloc)(size_t Size, char* Str, int a3, int a4, int a5);
void* __cdecl mem_calloc_TITLE_WORK_hook(size_t Size, char* Str, int a3, int a4, int a5)
{
	void* mem = mem_calloc(Size, Str, a3, a4, a5);
	TitleWork_ptr = (TITLE_WORK*)mem;
	return mem;
}

void(__cdecl* Mem_free)(void* mem);
void __cdecl Mem_free_TITLE_WORK_hook(void* mem)
{
	if (mem == TitleWork_ptr)
		TitleWork_ptr = nullptr;
	Mem_free(mem);
}

void AreaJump(uint16_t roomNo, Vec& position, float rotation)
{
	// proceed with the jumpening
	GLOBAL_WK* pG = GlobalPtr();

	// roomJumpExec begin
	// pG->flags_STOP_0_170[0] = 0xFFFFFFFF;
	pG->flags_DEBUG_2_68 |= 0x80000000;

	// copy what CRoomInfo::setNextPos does
	pG->nextRoomPosition_2C = position;
	pG->nextRoomRotationY_38 = rotation;
	pG->prevRoomId_4FB0 = pG->curRoomId_4FAC;
	pG->Part_old_4FB2 = pG->Part_4FAE;
	pG->RoomNo_next = roomNo;
	pG->Part_next_2A = 0;

	// roomJumpExec end

	// TODO: roomJumpExec runs these funcs, are they necessary at all?
#if 0
	typedef char(__fastcall* MessageControl__roomInit_Fn)(uint32_t ecx, uint32_t edx);
	MessageControl__roomInit_Fn MessageControl__roomInit = (MessageControl__roomInit_Fn)0x7196C0;
	MessageControl__roomInit(0xC17830, 0);

	typedef void(__fastcall* MessageControl__Delete_Fn)(uint32_t ecx, uint32_t edx, int idx);
	MessageControl__Delete_Fn MessageControl__Delete = (MessageControl__Delete_Fn)0x716F60;
	for (int i = 0; i < 0x10; i++)
		MessageControl__Delete(0xC17830, 0, i);
#endif

	pG->playerHpCur_4FB4 = pG->playerHpMax_4FB6;
	pG->r_continue_cnt_4FA0 = 0;

	// roomJumpExit
	pG->SetRoutine(GLOBAL_WK::Routine0::Doordemo, 0, 0, 0);

	// Force title screen to Exit state, allows AreaJump from main menu
	TITLE_WORK* titleWork = TitleWorkPtr();
	if (titleWork)
		titleWork->SetRoutine(TITLE_WORK::Routine0::Exit, 0, 0, 0);

	pG->Flags_SYSTEM_0_54[0] &= 0x40;
	pG->flags_DEBUG_0_60[0] &= ~0x80000000;
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
	pG_ptr = *pattern.count(1).get(0).get<GLOBAL_WK**>(1);

	// pDamage pointer
	pattern = hook::pattern("8A 8B C3 4F 00 00 89 45 0C");
	pD_ptr = *pattern.count(1).get(0).get<DAMAGE*>(-0xB);

	// pSys pointer
	pattern = hook::pattern("00 80 00 00 83 C4 ? E8 ? ? ? ? A1 ? ? ? ?");
	pSys_ptr = *pattern.count(1).get(0).get<SYSTEM_SAVE_WORK**>(13);

	// pPL pointer
	pattern = hook::pattern("A1 ? ? ? ? D8 CC D8 C9 D8 CA D9 5D ? D9 45 ?");
	pPL_ptr = *pattern.count(1).get(0).get<cPlayer**>(1);

	// pAS pointer
	pattern = hook::pattern("A8 02 74 16 8B 15");
	pAS_ptr = *pattern.count(1).get(0).get<cPlayer**>(6);

	// Pointer to Snd_ctrl_work struct
	pattern = hook::pattern("68 B8 00 00 00 6A 00 68 ? ? ? ?");
	Snd_ctrl_work = *pattern.count(1).get(0).get<SND_CTRL*>(8);

	// g_GameSave_BufPtr pointer (not actual name)
	pattern = hook::pattern("89 15 ? ? ? ? C7 05 ? ? ? ? A0 FA 0F 00");
	g_GameSave_BufPtr = *pattern.count(1).get(0).get<uint8_t**>(2);

	// Pointer to the original KeyOnCheck
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 53");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), game_KeyOnCheck_0);

	// pointer to EmMgr (instance of cManager<cEm>)
	pattern = hook::pattern("81 E1 01 02 00 00 83 F9 01 75 ? 50 B9 ? ? ? ? E8");
	EmMgr_ptr = *pattern.count(1).get(0).get<cEmMgr*>(0xD);

	// mem_calloc call for TITLE_WORK struct
	// (game doesn't store this in a global, so we need to capture it...)
	pattern = hook::pattern("6A 0D 6A 01 6A 00 6A 00 68 BC 00 00 00 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xD), mem_calloc);
	InjectHook(pattern.count(1).get(0).get<uint8_t>(0xD), mem_calloc_TITLE_WORK_hook, PATCH_CALL);

	// Mem_free call for TITLE_WORK, so we can set to nullptr once game frees it
	pattern = hook::pattern("56 E8 ? ? ? ? 6A 01 E8 ? ? ? ? 68 C0 01 00 00");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x1), Mem_free);
	InjectHook(pattern.count(1).get(0).get<uint8_t>(0x1), Mem_free_TITLE_WORK_hook, PATCH_CALL);

	// LightMgr pointer
	pattern = hook::pattern("6A 00 53 6A 00 57 B9 ? ? ? ?");
	LightMgr = (cLightMgr*)*pattern.count(1).get(0).get<uint32_t>(7);

	// Filter00Params ptrs
	pattern = hook::pattern("D9 EE 33 C0 D9 15 ? ? ? ? A2 ? ? ? ?");
	auto varPtr = pattern.count(3).get(0).get<uint32_t>(11);
	Filter00Params = (cFilter00Params*)*varPtr;

	varPtr = pattern.count(3).get(0).get<uint32_t>(22);
	Filter00Params2 = (cFilter00Params2*)*varPtr;

	// cLightMgr::setEnv ptr
	pattern = hook::pattern("55 8B EC 51 53 56 8B 75 ? 8B 46 ? A3 ? ? ? ? 8B D9 8B 4E ?");
	cLightMgr__setEnv = (cLightMgr__setEnv_Fn)pattern.count(1).get(0).get<uint8_t>(0);

	return true;
}
