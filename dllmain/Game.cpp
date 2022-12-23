#include "dllmain.h"
#include "Game.h"
#include "ConsoleWnd.h"
#include "SDK/filter00.h"

std::string gameVersion;
bool gameIsDebugBuild = false;

SND_CTRL* Snd_ctrl_work = nullptr; // extern inside Game.h
SUB_SCREEN* SubScreenWk = nullptr; // extern inside sscrn.h
pzlPlayer__ptrPiece_Fn pzlPlayer__ptrPiece = nullptr; // extern inside puzzle.h
CameraControl* CamCtrl = nullptr; // extern inside cam_ctrl.h
cPlayer__subScrCheck_Fn cPlayer__subScrCheck = nullptr; // extern inside player.h
uint32_t* cSofdec = nullptr;

// fade.h externs
j_j_j_FadeSet_Fn j_j_j_FadeSet = nullptr;

// light.h externs
cLightMgr* LightMgr = nullptr;
cLightMgr__setEnv_Fn cLightMgr__setEnv = nullptr;

// filter00.h externs
cFilter00Params* Filter00Params = nullptr;
cFilter00Params2* Filter00Params2 = nullptr;

// item.h externs
cItemMgr* ItemMgr = nullptr;
cItemMgr__search_Fn cItemMgr__search = nullptr;
cItemMgr__arm_Fn cItemMgr__arm = nullptr;

// event.h externs
EventMgr* EvtMgr = nullptr;
EventMgr__IsAliveEvt_Fn EventMgr__IsAliveEvt = nullptr;

// atari.h externs
cSatMgr* SatMgr = nullptr;
cSatMgr* EatMgr = nullptr;

// ID.h externs
IDSystem__set_Fn IDSystem__set = nullptr;
IDSystem__unitPtr_Fn IDSystem__unitPtr = nullptr;
IDSystem__kill_Fn IDSystem__kill = nullptr;
IDSystem__setTime_Fn IDSystem__setTime = nullptr;

// roomdata.h externs
cRoomData* RoomData = nullptr;
cRoomData__getRoomSavePtr_Fn cRoomData__getRoomSavePtr = nullptr;

// Original game funcs
namespace bio4 {
	uint32_t(__cdecl* SndCall)(uint16_t blk, uint16_t call_no, Vec* pos, uint8_t id, uint32_t flag, cModel* pMod);
	bool(__cdecl* SndStrReq_0)(uint32_t snd_id, int flg, int time, int vol);

	bool(__cdecl* SubScreenOpen)(SS_OPEN_FLAG open_flag, SS_ATTR_FLAG attr_flag);
	bool(__cdecl* CardCheckDone)();
	bool(__cdecl* CardLoad)(uint8_t a1);
	void(__cdecl* CardSave)(uint8_t terminal_no, uint8_t attr);

	void(__cdecl* GXSetBlendMode)(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor, GXLogicOp op);
	void(__cdecl* GXSetCullMode)(GXCullMode mode);
	void(__cdecl* GXSetZMode)(bool compare_enable, GXCompare func, bool update_enable);
	void(__cdecl* GXSetNumChans)(u8 nChans);
	void(__cdecl* GXSetChanCtrl)(GXChannelID chan, bool enable, GXColorSrc amb_src, GXColorSrc mat_src,
		u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn);
	void(__cdecl* GXSetNumTexGens)(u8 nTexGens);
	void(__cdecl* GXSetNumTevStages)(u8 nStages);
	void(__cdecl* GXSetTevOp)(GXTevStageID id, GXTevMode mode);
	void(__cdecl* GXSetTevOrder)(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color);
	void(__cdecl* GXClearVtxDesc)();
	void(__cdecl* GXSetVtxDesc)(GXAttr attr, GXAttrType type);
	void(__cdecl* GXSetVtxAttrFmt)(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac);
	void(__cdecl* GXLoadPosMtxImm)(const Mtx mtx, u32 id);
	void(__cdecl* GXSetCurrentMtx)(u32 id);
	void(__cdecl* GXBegin)(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
	void(__cdecl* GXEnd)(int a1);

	void(__cdecl* GXPosition3f32)(f32 x, f32 y, f32 z);
	void(__cdecl* GXColor4u8)(u8 r, u8 g, u8 b, u8 a);

	void(__cdecl* GXTexCoord2f32)(f32 s, f32 t);

	void(__cdecl* GXSetTexCopySrc)(u16 left, u16 top, u16 wd, u16 ht);
	void(__cdecl* GXSetTexCopyDst)(u16 wd, u16 ht, GXTexFmt fmt, bool mipmap);
	void(__cdecl* GXCopyTex)(void* dest, char clear, int a3);

	void(__cdecl* GXShaderCall)(int shaderNum);

	void(__cdecl* CameraCurrentProjection)();

	void(__cdecl* C_MTXOrtho)(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far);

	bool(__cdecl* KeyOnCheck_0)(KEY_BTN a1);

	void(__cdecl* KeyStop)(uint64_t un_stop_bit);

	void(__cdecl* SceSleep)(uint32_t ctr);

	void(__cdecl* QuakeExec)(uint32_t No, uint32_t Delay, int Time, float Scale, uint32_t Axis);
  
	bool(__cdecl* joyFireOn)();
};

// Current play time (H, M, S)
int iCurPlayTime[3] = { 0, 0, 0 };

// CPU/GPU usage meters
double* fCPUUsagePtr = nullptr;
double* fGPUUsagePtr = nullptr;

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
	{ 0x10000, "AttempedItemDrop" }, // set by EmSetDropItem, presense doesn't mean item is guaranteed, seems to be used just to prevent Em from trying to drop more than once
	{ 0x20000, "UseSimpleLighting" },
	{ 0x100000, "ClothNoScaleApply" }, // PenClothMove3
	{ 0x200000, "ClothReset" }, // PenClothMove3
	{ 0x1000000, "ApplyGlobalIllumination" }, // unsure, seems to adjust lighting color though
	//{ 0x8000000, "InvertShadowFlagBit6" }, // commonModelTrans, seems to invert check for flag 0x40 / bit6
};

std::unordered_map<int, std::string> EmNames =
{
	{0x00, "Player"},
	{0x01, "??"},
	{0x02, "Leon"},
	{0x03, "Ashley"},
	{0x04, "Luis"},
	{0x05, "Ashley"},
	{0x06, "Ganado"},
	{0x07, "Ganado"},
	{0x08, "Ganado"},
	{0x09, "Ganado"},
	{0x0A, "Ganado"},
	{0x0B, "Ganado"},
	{0x0C, "Ashley"},
	{0x0D, "Ganado"},
	{0x0E, "Jet-ski"},
	{0x0F, "Boat"},
	{0x11, "Zealot"},
	{0x12, "Ganado"},
	{0x13, "Ganado"},
	{0x13, "Ganado / Merchant"},
	{0x14, "Zealot / Merchant"},
	{0x15, "Ganado"},
	{0x16, "Ganado"},
	{0x17, "Ganado"},
	{0x18, "Merchant"},
	{0x19, "Zealot"},
	{0x1A, "Zealot"},
	{0x1B, "Garrador / Zealot"},
	{0x1C, "Garrador / Zealot"},
	{0x1D, "Soldier / J.J."},
	{0x1E, "Soldier / Merchant"},
	{0x1F, "Soldier"},
	{0x20, "SW Soldier / WW Super Salvador"},
	{0x21, "Dog"},
	{0x22, "Colmillo"},
	{0x23, "Crow"},
	{0x24, "Snake"},
	{0x25, "Parasite"},
	{0x26, "Cow"},
	{0x27, "Bass"},
	{0x28, "Chicken"},
	{0x29, "Bat"},
	{0x2A, "Bear trap / Mine trap"},
	{0x2B, "El Gigante"},
	{0x2C, "Verdugo"},
	{0x2D, "Novistador"},
	{0x2E, "Spider"},
	{0x2F, "Del Lago"},
	{0x31, "Saddler"},
	{0x32, "IT (U-3)"},
	{0x35, "Mendez"},
	{0x36, "Regenerator / Iron Maiden"},
	{0x38, "Salazar"},
	{0x39, "Krauser"},
	{0x3A, "Robot"},
	{0x3B, "Truck / Ammo wagon"},
	{0x3C, "Knight"},
	{0x3D, "Helicopter"},
	{0x3F, "Saddler"},
	{0x42, "SW Ganado"},
	{0x43, "SW Soldier"},
	{0x44, "SW Ganado"},
	{0x4E, "Cannon"},

	// Misc stuff
	{0x50, "Obj"},
	{0x51, "Door"},
	{0x52, "Wep"},
	{0x53, "Box"},
	//0x54 unused?
	{0x55, "Rack"},
	{0x56, "Window"},
	{0x57, "Torch"},
	{0x58, "Barrel"},
	{0x59, "Tree"},
	{0x5A, "Rock"},
	{0x5B, "Switch"},
	{0x5C, "Item"},
	{0x5D, "Hit"},
	{0x5E, "Barred"},
	{0x5F, "Mine"},
	{0x60, "Shield"},
	{0x61, "Bar"},
};

std::string cEmMgr::EmIdToName(int id, bool simplified)
{
	if (simplified)
		return EmNames[id];

	std::stringstream emIdStr;

	emIdStr << "cEm" << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << id;

	if (EmNames.count(id))
	{
		std::stringstream ss;
		ss << EmNames[id] << " (" << emIdStr.str() << ")";
		return ss.str();
	}

	return emIdStr.str();
}

cEm* cEmMgr::GetClosestEm(bool onlyValidEms, bool onlyEnemies, bool onlyESLSpawned, bool onlyTrans)
{
	auto& emMgr = *EmMgrPtr();
	cPlayer* player = PlayerPtr();

	float ClosestEmDist = 100000.0f;
	cEm* ClosestEmPtr = nullptr;

	// Get the closest Em
	for (auto& em : emMgr)
	{
		if (onlyValidEms && !em.IsValid())
			continue;

		if (onlyEnemies && !IsEnemy(em.id_100))
			continue;

		if (onlyESLSpawned && !em.IsSpawnedByESL())
			continue;

		if (onlyTrans && !em.IsTransSet())
			continue;

		float distFromPl = get_distance(em.pos_94.x, em.pos_94.y, em.pos_94.z, player->pos_94.x, player->pos_94.y, player->pos_94.z);

		// Must be something invalid
		if (distFromPl < 200.0f)
			continue;

		if (distFromPl < ClosestEmDist)
		{
			ClosestEmDist = distFromPl;
			ClosestEmPtr = &em;
		}
	}

	return ClosestEmPtr;
}

std::vector<cEm*> cEmMgr::GetVecClosestEms(int DesiredNumEms, float maxDistance, bool onlyValidEms, bool onlyEnemies, bool onlyESLSpawned, bool onlyTrans)
{
	auto& emMgr = *EmMgrPtr();
	cPlayer* player = PlayerPtr();

	// Get list of Ems and their distances
	std::vector<std::pair<float, cEm*>> dists;
	for (auto& em : emMgr)
	{
		if (onlyValidEms && !em.IsValid())
			continue;

		if (onlyEnemies && !IsEnemy(em.id_100))
			continue;

		if (onlyESLSpawned && !em.IsSpawnedByESL())
			continue;

		if (onlyTrans && !em.IsTransSet())
			continue;

		float distFromPl = get_distance(em.pos_94.x, em.pos_94.y, em.pos_94.z, player->pos_94.x, player->pos_94.y, player->pos_94.z);

		// Must be something invalid
		if (distFromPl < 200.0f)
			continue;

		// Too far
		if (distFromPl >= maxDistance)
			continue;

		dists.emplace_back(distFromPl, &em);
	}

	// Sort list. Shortest distance first.
	std::sort(dists.begin(), dists.end());

	// Put Em pointers in a vector and return
	std::vector<cEm*> EmVector;

	int i = 0;
	for (auto& emdists : dists)
	{
		EmVector.push_back(emdists.second);

		i++;

		if (i == DesiredNumEms)
			break;
	}

	return EmVector;
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

uint32_t* ptrGameVariableFrameRate;
int GameVariableFrameRate()
{
	return *(int32_t*)(ptrGameVariableFrameRate);
}

int CurrentFrameRate()
{
	return (int)std::round(30.0f / GlobalPtr()->deltaTime_70);
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

JOY* Joy_ptr = nullptr;
JOY* JoyPtr()
{
	return Joy_ptr;
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

IDSystem* IDSystem_ptr = nullptr;
IDSystem* IDSystemPtr()
{
	return IDSystem_ptr;
}

FADE_WORK(*FadeWork_ptr)[4];
FADE_WORK* FadeWorkPtr(FADE_NO no)
{
	return FadeWork_ptr[no];
}

itemPiece** g_p_Item_piece = nullptr; // not actual name...
itemPiece* ItemPiecePtr()
{
	if (!g_p_Item_piece)
		return nullptr;

	return *g_p_Item_piece;
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


bool IsEnemy(int id) // Since the game's IsGanado doesn't account for some enemies, we have this as an alternative
{
	bool blacklist = false;

	blacklist |= (id == 0x2A); // Mine trap / bear trap
	blacklist |= (id == 0x3B); // Truck / Wagon
	blacklist |= (id == 0x3D); // Mike's helicopter
	blacklist |= (id == 0x4E); // SW Ship cannon

	if (blacklist)
		return false;

	return (id > 0x10 && id < 0x4F);
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

bool WeaponChangeRequested = false;
void RequestWeaponChange()
{
	WeaponChangeRequested = true; // signal main thread
}

bool SaveGameRequested = false;
void RequestSaveGame()
{
	SaveGameRequested = true; // signal main thread
}

bool MerchantRequested = false;
void RequestMerchant()
{
	MerchantRequested = true; // signal main thread
}

void(__cdecl* WeaponChange)();
void(__fastcall* cSceSys__scheduler)(void* thisptr, void* unused);
void __fastcall cSceSys__scheduler_Hook(void* thisptr, void* unused)
{
	if (WeaponChangeRequested)
	{
		WeaponChangeRequested = false;
		WeaponChange();
	}

	if (SaveGameRequested)
	{
		SaveGameRequested = false;
		bio4::CardSave(0, 1);
	}

	if (MerchantRequested)
	{
		MerchantRequested = false;
		// If SubScreen is closed and game status is "playing"
		if (!SubScreenWk->open_flag_2C && GlobalPtr()->Rno0_20 == 0x3)
			bio4::SubScreenOpen(SS_OPEN_FLAG::SS_OPEN_SHOP, SS_ATTR_NULL);
	}

	cSceSys__scheduler(thisptr, unused);
}

bool AreaJump(uint16_t roomNo, Vec& position, float rotation)
{
	// proceed with the jumpening
	GLOBAL_WK* pG = GlobalPtr();

	// Only allow jumping after save has been loaded (after "PRESS ANY KEY" screen)
	// Otherwise there's a chance system info (unlocked flags etc) could be lost
	if (!bio4::CardCheckDone())
		return false;

	// roomJumpExec begin
	// pG->flags_STOP_0_170[0] = 0xFFFFFFFF;
	FlagSet(pG->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_ROOMJMP), true);

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

	// End any active Sofdec movie, else Title_task code will be stuck paused until movie ends
	*cSofdec |= 0x20; // adds flag to m_be_flag_0, same as what cSofdec::PlayCancel does

	// roomJumpExit
	pG->SetRoutine(GLOBAL_WK::Routine0::Doordemo, 0, 0, 0);

	// Force title screen to Exit state, allows AreaJump from main menu
	TITLE_WORK* titleWork = TitleWorkPtr();
	if (titleWork)
		titleWork->SetRoutine(TITLE_WORK::Routine0::Exit, 0, 0, 0);

	pG->Flags_SYSTEM_0_54[0] &= 0x40;
	FlagSet(pG->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_TEST_MODE), false);

	return true;
}

bool re4t::init::Game()
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
	con.log("Game version = %s", GameVersion().c_str());
	#endif

	// Pointer to users variableframerate setting value
	pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
	ptrGameVariableFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);

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

	// Pointer to Joy[4] array
	pattern = hook::pattern("83 E0 10 33 C9 0B C1 0F 84 ? ? ? ? 0F BE 05 ? ? ? ?");
	Joy_ptr = (JOY*)(*pattern.count(1).get(0).get<uint8_t*>(0x10) - 9);

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

	// GPU/CPU usage values
	pattern = hook::pattern("DD 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? DC 0D");
	fGPUUsagePtr = *pattern.count(1).get(0).get<double*>(2);
	fCPUUsagePtr = *pattern.count(1).get(0).get<double*>(0x17);

	// Pointer to KeyOnCheck_0
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 84 C0 74 ? C7 86 ? ? ? ? ? ? ? ? 5E B8 ? ? ? ? 5B 8B E5 5D C3 53");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4::KeyOnCheck_0);

	// Pointer to KeyStop
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 08 E8 ? ? ? ? 8B 0D ? ? ? ? F7 81 ? ? ? ? ? ? ? ? 74");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4::KeyStop);

	// Pointer to IDSystem
	pattern = hook::pattern("B9 ? ? ? ? E8 ? ? ? ? 8B ? ? ? ? ? 8B C8 D9");
	IDSystem_ptr = *pattern.count(1).get(0).get<IDSystem*>(1);

	// pointer to IDSystem::set
	pattern = hook::pattern("E8 ? ? ? ? 6A 29 68 FE 00 00 00 B9");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0)).as_int(), IDSystem__set);

	// pointer to IDSystem::unitPtr
	pattern = hook::pattern("E8 ? ? ? ? 8B ? ? ? ? ? 8B C8 D9 81 94 00 00 00 8B");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0)).as_int(), IDSystem__unitPtr);

	// pointer to IDSystem::kill
	pattern = hook::pattern("E8 ? ? ? ? 68 99 00 00 00 68 FF 00 00 00 B9 ? ? ? ? E8 ? ? ? ? 8B 46 20 8B");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0)).as_int(), IDSystem__kill);

	// pointer to IDSystem::setTime
	pattern = hook::pattern("E8 ? ? ? ? FE 46 01 5F 5E 8B E5 ");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0)).as_int(), IDSystem__setTime);

	// pointer to EmMgr (instance of cManager<cEm>)
	pattern = hook::pattern("81 E1 01 02 00 00 83 F9 01 75 ? 50 B9 ? ? ? ? E8");
	EmMgr_ptr = *pattern.count(1).get(0).get<cEmMgr*>(0xD);

	// mem_calloc call for TITLE_WORK struct
	// (game doesn't store this in a global, so we need to capture it...)
	pattern = hook::pattern("6A 0D 6A 01 6A 00 6A 00 68 BC 00 00 00 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xD), mem_calloc);
	InjectHook(pattern.count(1).get(0).get<uint8_t>(0xD), mem_calloc_TITLE_WORK_hook, PATCH_CALL);

	// pointer to FadeWork
	pattern = hook::pattern("68 ? ? ? ? E8 ? ? ? ? D9 EE D9 15 ? ? ? ? 83 C4 08 ");
	FadeWork_ptr = *pattern.count(1).get(0).get<FADE_WORK(*)[4]>(1);

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

	// ItemMgr
	pattern = hook::pattern("80 B9 C0 4F 00 00 10 0F 84 ? ? ? ? B9");
	ItemMgr = *pattern.count(1).get(0).get<cItemMgr*>(0xE);
	pattern = hook::pattern("83 C4 08 50 B9 ? ? ? ? E8 ? ? ? ? 85 C0 0F 94 45 FF");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(9), cItemMgr__search);
	pattern = hook::pattern("8B 0D ? ? ? ? 51 B9 ? ? ? ? E8 ? ? ? ? 5F 5E 5B 8B E5");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(12), cItemMgr__arm);

	// EvtMgr
	pattern = hook::pattern("75 ? 6A 00 6A 00 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 84 C0");
	EvtMgr = *pattern.count(1).get(0).get<EventMgr*>(0xC);
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x10), EventMgr__IsAliveEvt);

	// g_p_Item_piece ptr (not actual name)
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 08 8B 35 ? ? ? ? 85 F6 0F 84");
	g_p_Item_piece = *pattern.count(1).get(0).get<itemPiece**>(10);

	// Hook call to cSceSys::scheduler, so we can run code inside the main thread before other things update
	pattern = hook::pattern("74 ? B9 ? ? ? ? E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? E8");
	auto cSceSys__scheduler_thunk = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(17));
	ReadCall(cSceSys__scheduler_thunk.as_int(), cSceSys__scheduler);
	InjectHook(cSceSys__scheduler_thunk.as_int(), cSceSys__scheduler_Hook, PATCH_JUMP);

	// WeaponChange funcptr
	pattern = hook::pattern("6A 01 E8 ? ? ? ? 83 C4 04 E8 ? ? ? ? F6");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xA), WeaponChange);

	// Card related funcptrs
	pattern = hook::pattern("E8 ? ? ? ? 3C 01 75 ? 8B 4D ? 8B 55");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4::CardCheckDone);
	pattern = hook::pattern("E8 ? ? ? ? 6A ? E8 ? ? ? ? 83 C4 ? 8B 15 ? ? ? ? A1");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4::CardSave);
	pattern = hook::pattern("6A 00 E8 ? ? ? ? 83 C4 04 3C 01 75 ? 6A 17");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(2)).as_int(), bio4::CardLoad);

	// SndCall funcptr
	pattern = hook::pattern("05 94 00 00 00 50 6A 0C 6A 01 E8");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xA)).as_int(), bio4::SndCall);

	// SndStrReq_0 funcptr
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 5F B0 01 5E 8B");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4::SndStrReq_0);

	// QuakeExec ptr
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 14 8B E5 5D");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), bio4::QuakeExec);

	// joyFireOn ptr
	pattern = hook::pattern("E8 ? ? ? ? 85 C0 74 ? 8B 8E D8 07 00 00 8B 49 34 E8 ? ? ? ? 84 C0 0F ? ? ? ? ? 8B");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0)).as_int(), bio4::joyFireOn);

	// SubScreenOpen funcptr
	pattern = hook::pattern("55 8B EC A1 ? ? ? ? B9 ? ? ? ? 85 88");
	bio4::SubScreenOpen = (decltype(bio4::SubScreenOpen))pattern.count(1).get(0).get<uint32_t>(0);

	// SubScreenWk ptr
	pattern = hook::pattern("68 ? ? ? ? E8 ? ? ? ? 68 00 00 00 F0 E8");
	SubScreenWk = *pattern.count(1).get(0).get<SUB_SCREEN*>(1);

	// pzlPlayer::ptrPiece
	pattern = hook::pattern("8B 41 30 50 E8 ? ? ? ? C3");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(4), pzlPlayer__ptrPiece);

	// j_j_j_FadeSet (kept the werid j_j_j to make clear this is the simplified version of FadeSet that takes less params)
	pattern = hook::pattern("E8 ? ? ? ? 53 53 53 53 6A 04 53 E8 ? ? ? ? 83 C4 24 E9");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0), j_j_j_FadeSet);

	// cPlayer::subScrCheck
	pattern = hook::pattern("80 B9 FC 00 00 00 00 75 ? 8A 81 FD 00 00 00");
	cPlayer__subScrCheck = (cPlayer__subScrCheck_Fn)pattern.count(1).get(0).get<uint32_t>(0);

	// CamCtrl ptr
	pattern = hook::pattern("D9 EE B9 ? ? ? ? D9 9E A4 00 00 00 E8 ? ? ? ? D9 EE");
	CamCtrl = *pattern.count(1).get(0).get<CameraControl*>(3);

	// Sofdec ptr
	pattern = hook::pattern("B9 ? ? ? ? C6 86 1F 05 00 00 01");
	cSofdec = *pattern.count(1).get(0).get<uint32_t*>(1);

	// SatMgr/EatMgr ptrs
	pattern = hook::pattern("8D 8E B4 02 00 00 E8 ? ? ? ? 6A 00 56 B9");
	SatMgr = *pattern.count(1).get(0).get<cSatMgr*>(0xF);
	pattern = hook::pattern("6A 00 53 56 50 51 B9 ? ? ? ? E8");
	EatMgr = *pattern.count(1).get(0).get<cSatMgr*>(0x7);

	// RoomData ptrs
	pattern = hook::pattern("8B 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 85 C0");
	RoomData = *pattern.count(1).get(0).get<cRoomData*>(0x5);
	ReadCall(pattern.count(1).get(0).get<uint8_t>(9), cRoomData__getRoomSavePtr);

	// GX function ptrs
	pattern = hook::pattern("56 53 6A 05 6A 04 6A 01 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x8), bio4::GXSetBlendMode); // 0x956E60

	pattern = hook::pattern("53 E8 ? ? ? ? 6A 01 6A 07 53 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x1), bio4::GXSetCullMode); // 0x957130
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xB), bio4::GXSetZMode); // 0x957FE0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x11), bio4::GXSetNumTexGens); // 0x9575F0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x18), bio4::GXSetNumTevStages); // 0x957580
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x20), bio4::GXSetTevOp); // 0x957AC0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x32), bio4::GXSetTevOrder); // 0x957B30

	pattern = hook::pattern("83 C4 48 6A 01 E8 ? ? ? ? 6A 02 53");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x5), bio4::GXSetNumChans); // 0x9574A0

	pattern = hook::pattern("6A 01 6A 01 53 53 6A 04 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x8), bio4::GXSetChanCtrl); // 0x956F50
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xD), bio4::GXClearVtxDesc); // 0x9558C0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x16), bio4::GXSetVtxDesc); // 0x957F70
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x2C), bio4::GXSetVtxAttrFmt); // 0x957EF0

	pattern = hook::pattern("52 E8 ? ? ? ? 8D 45 ? 53 50 E8 ? ? ? ? 53 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xB), bio4::GXLoadPosMtxImm); // 0x956310
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x11), bio4::GXSetCurrentMtx); // 0x9571A0

	pattern = hook::pattern("6A 0C 53 68 90 00 00 00 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x8), bio4::GXBegin); // 0x955840

	pattern = hook::pattern("53 53 53 E8 ? ? ? ? 53 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x3), bio4::GXColor4u8); // 0x955B60
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x9), bio4::GXEnd); // 0x955E50

	pattern = hook::pattern("D9 04 08 D9 1C ? E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x6), bio4::GXPosition3f32); // 0x956A70

	pattern = hook::pattern("E8 ? ? ? ? 47 83 C4 ? 83 C6 ? 3B 3D ? ? ? ? 0F 82");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), bio4::GXShaderCall);

	pattern = hook::pattern("E8 ? ? ? ? 0F B7 4E ? 0F B7 56 ? 6A ? 6A ? 51");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), bio4::GXSetTexCopySrc);

	pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 6A 04 6A 00 50 E8");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), bio4::GXSetTexCopyDst);

	pattern = hook::pattern("E8 ? ? ? ? D9 05 ? ? ? ? D9 7D ? 6A ? 0F B7 45");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), bio4::GXCopyTex);

	pattern = hook::pattern("E8 ? ? ? ? 8B 4D ? 51 E8 ? ? ? ? 8B 4D ? 83 C4 ? 33 CD");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), bio4::GXTexCoord2f32);

	pattern = hook::pattern("E8 ? ? ? ? 8A 46 30 3C FD");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x0), bio4::CameraCurrentProjection); // 0x59F3A0

	pattern = hook::pattern("E8 ? ? ? ? 8D 4D BC 6A 01 51 E8 ? ? ? ? 8D 55 98 52 8D 45 B0 50 8D 4D A4 51 56 E8 ? ? ? ? 8B 4D FC 83 C4 34");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4::C_MTXOrtho);

	pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 4E 75 C1 8B 0D ? ? ? ? 8B 91");
	ReadCall(pattern.count(2).get(0).get<uint8_t>(0), bio4::SceSleep);

	// Store current game time that's being calculated inside GetGameTime
	pattern = hook::pattern("8B 55 ? 8B 45 ? 51 8B 4D ? 52 50 51 68");
	struct GetGameTime_hook
	{
		void operator()(injector::reg_pack& regs)
		{
			iCurPlayTime[0] = *(uint32_t*)(regs.ebp - 0xC); // Hours
			iCurPlayTime[1] = *(uint32_t*)(regs.ebp - 0x8); // Minutes
			iCurPlayTime[2] = *(uint32_t*)(regs.ebp - 0x4); // Seconds

			// Code we replaced
			regs.edx = *(uint32_t*)(regs.ebp - 0x4);
			regs.eax = *(uint32_t*)(regs.ebp - 0x8);
		}
	}; injector::MakeInline<GetGameTime_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	return true;
}
