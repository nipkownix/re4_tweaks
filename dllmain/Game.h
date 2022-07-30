#pragma once
#include <cstdint>
#include "GameFlags.h"

// From GC SDK?
typedef struct tagVec // PS2 seems to refer to these via the tag names
{
  float x;
  float y;
  float z;
} Vec, Point3d;
static_assert(sizeof(Vec) == 0xC, "sizeof(Vec)");

typedef struct tagQuaternion
{
  float x;
  float y;
  float z;
  float w;
} Quaternion;
static_assert(sizeof(Quaternion) == 0x10, "sizeof(Quaternion)");

typedef float Mtx[3][4];
typedef float Mtx44[4][4];
typedef float ROMtx[4][3];
static_assert(sizeof(Mtx) == 0x30, "sizeof(Mtx)");
static_assert(sizeof(Mtx44) == 0x40, "sizeof(Mtx44)");
static_assert(sizeof(ROMtx) == 0x30, "sizeof(ROMtx)");

struct GXTexObj
{
  uint32_t field_0;
  uint32_t field_4;
  uint32_t field_8;
  uint32_t field_C;
  uint32_t field_10;
  uint32_t field_14;
  uint32_t field_18;
  uint32_t field_1C;
};
static_assert(sizeof(GXTexObj) == 0x20, "sizeof(GXTexObj)"); // seems to be correct size, different to GC/Wii struct?

// Game structs
struct CAMERA
{
  Mtx mtx1_0;
  Mtx mtx2_30;
  float field_60;
  Mtx44 mtx3_64;
  Vec field_A4;
  Vec field_B0;
  float field_BC;
  float binoZoomLimitRelated_C0; // FOV?
  Vec field_C4;
  Vec field_D0;
  Vec field_DC;
  float field_E8;
  float field_EC;
  float field_F0;
  float field_F4;
};
static_assert(sizeof(CAMERA) == 0xF8, "sizeof(CAMERA)"); // TODO: find if this size is correct

enum class KEY_BTN : unsigned __int64
{
	KEY_FORWARD = 0x1,
	KEY_BACK = 0x2,
	KEY_RIGHT = 0x4,
	KEY_LEFT = 0x8,
	KEY_KAMAE = 0x10,
	KEY_RELOCKON = 0x20,
	KEY_RUN = 0x40,
	KEY_FIRE = 0x80,
	KEY_180TURN = 0x100,
	KEY_ASHLEY = 0x200,
	KEY_CK = 0x400,
	KEY_KAMAE_KNIFE = 0x800,
	KEY_ST = 0x1000,
	KEY_OPTION = 0x2000,
	KEY_C_U = 0x4000,
	KEY_C_D = 0x8000,
	KEY_Y = 0x10000,
	KEY_X = 0x20000,
	KEY_B = 0x40000,
	KEY_A = 0x80000,
	KEY_SSCRN = 0x100000,
	KEY_MAP = 0x200000,
	KEY_LB = 0x400000,
	KEY_RB = 0x800000,
	KEY_U = 0x1000000,
	KEY_D = 0x2000000,
	KEY_R = 0x4000000,
	KEY_L = 0x8000000,
	KEY_UDRL = 0xF000000,
	KEY_Z = 0x10000000,
	KEY_EV_CANCEL = 0x20000000,
	KEY_CANCEL = 0x40000000,
	KEY_OK = 0x80000000,
	KEY_LL = 0x100000000,
	KEY_LR = 0x200000000,
	KEY_LD = 0x400000000,
	KEY_LU = 0x800000000,
	KEY_C_R = 0x1000000000,
	KEY_C_L = 0x2000000000,
	KEY_PICK_UP = 0x4000000000,
	KEY_MAP_MENU = 0x8000000000,
	KEY_ACT_A = 0x10000000000,
	KEY_QUICK = 0x20000000000,
	KEY_ACT_B = 0x40000000000,
	KEY_MAP_ZOOM = 0x80000000000,
	KEY_PLUS = 0x100000000000,
	KEY_MINUS = 0x200000000000,
	KEY_1 = 0x400000000000,
	KEY_2 = 0x800000000000,
	KEY_KNIFE = 0x1000000000000,
	KEY_KNIFE2 = 0x2000000000000,
	KEY_CRANK = 0x4000000000000,
	KEY_SU = 0x10000000000000,
	KEY_SD = 0x20000000000000,
	KEY_SR = 0x40000000000000,
	KEY_SL = 0x80000000000000,
	KEY_SUDRL = 0xF0000000000000,
};

enum class InputDevices
{
	// 0 = Keyboard?
	// 1 = Mouse
	// 2 = Xinput Controller
	// 3 = Dinput Controller
	
	Keyboard,
	Mouse,
	XinputController,
	DinputController
};

enum class MouseAimingModes : std::uint8_t
{
	Classic,
	Modern
};

enum class keyConfigTypes : std::uint8_t
{
	TypeI,
	TypeII,
	TypeIII
};

enum class CharacterID : std::uint8_t
{
	Leon,
	Ashley,
	Ada,
	HUNK,
	Krauser,
	Wesker
};

enum class LeonCostumes
{
	Jacket,
	Normal,
	Vest,
	RPD,
	Mafia
};

enum class AshleyCostumes
{
	Normal,
	Popstar,
	Armor
};

enum class AdaCostumes
{
	RE2,
	Spy,
	RE2_d, // duplicated?
	Normal
};

enum EM_BE_FLAG : uint8_t
{
  EM_BE_FLAG_ALIVE = 0x1,
  EM_BE_FLAG_SET = 0x2,
  EM_BE_FLAG_DIE = 0x80,
};

struct EM_LIST
{
  EM_BE_FLAG be_flag_0;
  char id_1;
  char type_2;
  char set_3;
  uint32_t em_flag_4;
  int16_t hp_8;
  uint8_t gapA;
  char charaId_B;
  int16_t posX_C;
  int16_t posY_E;
  int16_t posZ_10;
  int16_t rotX_12;
  int16_t rotY_14;
  int16_t rotZ_16;
  int16_t roomId_18;
  int16_t guardRadius_1A;
  uint8_t gap1C[4];
};
static_assert(sizeof(EM_LIST) == 0x20, "sizeof(EM_LIST)"); // TODO: find if this size is correct

struct __declspec(align(4)) GLOBALS
{
  uint8_t field_0;
  uint8_t field_1;
  uint8_t field_2;
  uint8_t field_3;
  uint8_t flags_saveRelated_4;
  uint8_t saveRelated_5;
  uint8_t saveRelated_6;
  uint8_t saveRelated_7;
  uint32_t field_8;
  uint32_t field_C;
  void* ptr_romFont_10;
  uint8_t field_14;
  uint8_t field_15;
  uint8_t field_16;
  uint8_t wideMode_17;
  uint32_t field_18;
  uint32_t field_1C;
  uint8_t field_20;
  bool escflag_21; // pause game flag
  uint8_t field_22;
  uint8_t field_23;
  void* field_24;
  uint16_t nextRoomId_28;
  uint8_t field_2A;
  uint8_t field_2B;
  float field_2C;
  float field_30;
  float field_34;
  float field_38;
  void* ptr_field_3C;
  uint8_t* ptr_roomData_40;
  uint32_t ptr_wepData_44;
  uint32_t* ptr_coreData_48;
  uint32_t ptr_optionData_4C;
  void* ptr_playerData_50;
  uint32_t flags_SYSTEM_54;
  uint32_t flags_DISP0_58;
  uint32_t curGameTime_5C;
  uint32_t flags_DEBUG_60[4];
  float deltaTime_70;
  CAMERA cameraInstance_74;
  uint32_t field_16C;
  uint32_t flags_STOP_170;
  uint32_t flags_ROOM_174;
  uint32_t field_178;
  uint32_t field_17C;
  uint32_t field_180;
  float weightpaletteext0_184;
  float weightpaletteext0_188;
  float weightpaletteext0_18C;
  float weightpaletteext0_190;
  float weightpaletteext0_194;
  float weightpaletteext0_198;
  float weightpaletteext0_19C;
  float weightpaletteext0_1A0;
  float weightpaletteext0_1A4;
  float weightpaletteext0_1A8;
  float weightpaletteext0_1AC;
  float weightpaletteext0_1B0;
  uint8_t gap1B4[11868];
  GXTexObj texObjects_3010[248];
  uint32_t field_4F10;
  uint32_t field_4F14;
  uint32_t primBuffPtr_4F18;
  uint32_t primMaxCount_4F1C;
  uint32_t field_4F20;
  void* cameraCoreData_4F24;
  void* cameraRoomData_4F28;
  uint32_t field_4F2C;
  void* ptr_field_4F30;
  uint32_t field_4F34;
  uint32_t field_4F38;
  Vec field_4F3C;
  uint32_t field_4F48;
  uint8_t gap4F4C[32];
  uint32_t field_4F6C;
  uint32_t field_4F70;
  Vec field_4F74;
  uint8_t field_4F80;
  uint8_t field_4F81;
  uint16_t field_4F82;
  uint32_t field_4F84;
  uint32_t field_4F88;
  uint32_t field_4F8C;
  uint32_t field_globalSaveDataStartsHere_4F90;
  int field_4F94;
  uint32_t lucky_drop_4F98;
  uint16_t field_4F9C;
  uint16_t field_4F9E;
  uint16_t field_4FA0;
  uint8_t field_4FA2;
  uint8_t languageId_4FA3;
  uint32_t playtime_4FA4;
  uint32_t money_4FA8;
  uint16_t curRoomId_4FAC;
  uint8_t field_4FAE;
  uint8_t field_4FAF;
  uint16_t prevRoomId_4FB0;
  uint8_t field_4FB2;
  uint8_t field_4FB3;
  uint16_t playerHp_4FB4;
  uint16_t playerHpFull_4FB6;
  int16_t ashleyHp_4FB8;
  uint16_t ashleyHpFull_4FBA;
  uint32_t field_4FBC;
  uint8_t curWepid_4FC0;
  uint8_t field_4FC1;
  uint8_t field_4FC2;
  uint8_t field_4FC3;
  uint8_t field_4FC4;
  uint8_t field_4FC5;
  uint8_t field_4FC6;
  uint8_t field_4FC7;
  uint8_t curPlType_4FC8; // 0: Leon; 1: Ashley; 2: Ada; 3: HUNK; 4: Krauser; 5: Wesker
  uint8_t plCostume_4FC9; // Leon {0: Jacket, 1: Normal, 2: Vest, 3: RPD, 4: Mafia } || Ashley {0: Normal, 1: Popstar, 2: Armor} || Ada { 0:RE2, 1:Spy, 2:RE2 again?, 3:Normal } || HUNK { 0:Normal } | Krauser { 0:Normal } | Wesker { 0:Normal }
  uint8_t field_4FCA;
  uint8_t Costume_subchar_4FCB; // Ashley {0: Normal, 1: Popstar, 2: Armor}
  uint16_t field_4FCC;
  uint16_t field_4FCE;
  float field_4FD0;
  float field_4FD4;
  float field_4FD8;
  float field_4FDC;
  uint32_t gap4FE0[15];
  uint32_t flags_STATUS_501C[4];
  uint8_t gap502C[12];
  uint32_t flags_5038;
  uint8_t gap503C[80];
  uint32_t flags_508C;
  uint8_t gap5090[28];
  uint32_t flags_50AC;
  uint8_t gap50B0[44];
  uint32_t flags_50DC;
  uint8_t gap50E0[428];
  uint32_t flags_ITEM_SET_528C[1];
  uint8_t gap5290[60];
  uint32_t flags_SCENARIO_52CC[2];
  uint32_t field_52D4;
  uint32_t field_52D8;
  uint32_t field_52DC;
  uint32_t field_52E0;
  uint32_t field_52E4;
  uint32_t field_52E8;
  uint32_t flags_KEY_LOCK_52EC[5];
  uint8_t gap5300[12];
  uint32_t frameCounter_530C;
  uint32_t freeData_5310[64];
  EM_LIST emList_5410[256]; 
  uint8_t gap7410[4100];
  uint8_t field_8414;
  uint32_t field_8418[3];
  uint32_t field_8424;
  uint32_t field_8428;
  uint8_t gap842C[40];
  uint32_t field_8454;
  uint32_t field_8458;
  uint32_t field_845C;
  uint16_t chapterDead_8460;
  uint16_t totalDead_8462;
  uint32_t chapterKill_8464;
  uint32_t totalKill_8468;
  uint32_t chapterShot_846C;
  uint32_t totalShot_8470;
  uint32_t chapterHit_8474;
  uint32_t totalHit_8478;
  uint8_t curMode_847C; // Dynamic max 6 in pro 
  uint8_t field_847D;
  uint8_t field_847E;
  uint8_t field_847F;
  uint32_t field_8480;
  uint8_t gap8484[516];
  uint8_t field_8688;
  uint8_t field_8689;
  uint32_t field_868C;

  // seperate SystemSave struct follows - some things seem to access it via pG pointer though
  // TODO: maybe just include SYSTEM_SAVE here - need to check if anything refers to GLOBALS size anywhere first
  uint32_t flags_CONFIG_8690;
  uint32_t flags_EXTRA_8694;
  uint8_t languageId_8698;
  uint8_t field_8699;
  uint8_t field_869A;
  uint8_t field_869B;
  uint8_t soundMode_869C;
  uint8_t adasReportNo_869D;
};
static_assert(sizeof(GLOBALS) == 0x86A0, "sizeof(GLOBALS)"); // TODO: find if this size is correct

struct __declspec(align(4)) DAMAGE {
	uint8_t unk0[0x40];
	uint32_t knife40; // 200 
	uint32_t unk44;
	uint32_t unk48;
	uint32_t unk4C;
	uint32_t kick50; // 200
	// unknown 
};
static_assert(sizeof(DAMAGE) == 0x54, "sizeof(ATTACK_VALUE)");

struct SYSTEM_SAVE
{
  uint32_t flags_CONFIG_0;
  uint32_t flags_EXTRA_4;
  uint8_t languageId_8;
  uint8_t violenceLevel_9;
  uint8_t field_A;
  keyConfigTypes keyConfigType_B;
  uint8_t soundMode_C;
  uint8_t adasReportNo_D;
  uint8_t field_E;
  uint8_t field_F;
  uint32_t field_10;
  uint32_t field_14;
  uint32_t field_18;
  uint32_t field_1C;
  uint32_t field_20;
  uint32_t field_24;
  uint32_t field_28;
  uint8_t struct_2C[80];
  uint32_t field_7C;
  uint32_t field_80;
  uint32_t field_84;
  uint32_t field_88;
  uint32_t field_8C;
  uint32_t field_90;
  uint8_t gap94[12];
};
static_assert(sizeof(SYSTEM_SAVE) == 0xA0, "sizeof(SYSTEM_SAVE)"); // TODO: find if this size is correct

#pragma pack(push, 1)
struct MOTION_INFO
{
  int fcvDataPtr_0;
  uint32_t* animDataPtr_4;
  int16_t field_8;
  int field_A;
  int16_t field_E;
  int field_10;
  int16_t field_14;
  int field_16;
  int16_t field_1A;
  int field_1C;
  float frameCount_20;
  float frameCurrent_24;
  float frameCurrent_Backup_28;
  float framePrev_2C;
  uint8_t numAnimNodes_30;
  uint8_t unk_31[3];
  int animNodeAddrs_34;
  void* animDescPtr_38;
  uint16_t field_3C;
  uint16_t field_3E;
  uint16_t motionFlags_40;
  uint16_t field_42;
  uint32_t flags_44;
  Vec field_48;
  Vec field_54;
  Vec field_60;
  float field_6C;
  float field_70;
  float field_74;
  float field_78;
  float field_7C;
  float field_80;
  Vec field_84;
  Vec field_90;
  Vec field_9C;
  uint16_t* field_A8;
  int16_t field_AC;
  int16_t field_AE;
  int16_t field_B0;
  int16_t flags_B2;
  int field_B4;
  float cur_frame_B8;
  uint16_t numFrames_BC;
  uint8_t unk_BE[2];
  float speed_C0;
  uint8_t field_C4;
  uint8_t field_C5;
  uint8_t unk_C6[2];
  float field_C8;
  uint8_t* field_CC;
};
static_assert(sizeof(MOTION_INFO) == 0xD0, "sizeof(MOTION_INFO)");

struct cPlWep
{
  uint8_t unk_0[32];
  uint8_t field_20;
  uint8_t field_21;
  uint8_t unk_22[1];
  uint8_t field_23;
  uint8_t field_24;
  uint8_t unk_25[1];
  uint8_t flags_26;
  uint8_t unk_27[1];
  float field_28;
  float field_2C;
  float field_30;
  void* wep0_ptr_34;
  void* wep1_ptr_38;
  uint8_t unk_3C[4];
  uint8_t field_40;
  uint8_t unk_41[1];
  char field_42;
  uint8_t field_43;
  Vec field_44;
};
static_assert(sizeof(cPlWep) == 0x50, "sizeof(cPlWep)");

struct cPlayer // unsure if correct name!
{
  /* 0x000 */ uint8_t unk0[0x94];
  /* 0x094 */ float X;
  /* 0x098 */ float Y;
  /* 0x09C */ float Z;
  /* 0x0A0 */ float unkA0;
  /* 0x0A4 */ float rotation;
  /* 0x0A8 */ uint8_t unk_A8[0x1D8 - 0xA8];
  /* 0x1D8 */ MOTION_INFO MotInfo_1D8;
  /* 0x2A8 */ uint8_t unk_2A8[0x7D8 - 0x2A8];
  /* 0x7D8 */ cPlWep* plWep_7D8;
  /* goes to 7F0+ */
};
#pragma pack(pop)
static_assert(sizeof(cPlayer) == 0x7DC, "sizeof(cPlayer)"); // TODO: nowhere near the correct size!

struct DatTblEntry
{
  /* 0x00 */ char name_0[48];
  /* 0x30 */ uint32_t flags_30;
  /* 0x34 */ uint8_t* data_34;
  /* 0x38 */ void* unk_38;
};
static_assert(sizeof(DatTblEntry) == 0x3C, "sizeof(DatTblEntry)");

struct DatTbl
{
  /* 0x00 */ int count_0;
  /* 0x04 */ DatTblEntry* entries_4;
};
static_assert(sizeof(DatTbl) == 8, "sizeof(DatTbl)");

std::string GameVersion();
bool GameVersionIsDebug();
InputDevices LastUsedDevice();
void SetLastUsedDevice(InputDevices device);
int g_MOUSE_SENS();
MouseAimingModes GetMouseAimingMode();
void SetMouseAimingMode(MouseAimingModes newMode);
uint64_t Key_btn_on();
uint64_t Key_btn_trg();

GLOBALS* GlobalPtr();
DAMAGE* DamagePtr();
SYSTEM_SAVE* SystemSavePtr();
cPlayer* PlayerPtr();
cPlayer* AshleyPtr();
uint8_t* GameSavePtr();

// Length seems to always be 0xFFAA0 across all builds
#define GAMESAVE_LENGTH 0xFFAA0

// Original game funcs
extern bool(__cdecl* game_KeyOnCheck_0)(KEY_BTN a1);
extern void(__cdecl* game_C_MTXOrtho)(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far);
