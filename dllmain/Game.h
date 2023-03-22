#pragma once
#include <cstdint>
#include "GameFlags.h"

#include "SDK/atari.h"
#include "SDK/event.h"
#include "SDK/EtcModel.h"
#include "SDK/em10.h"
#include "SDK/player.h"
#include "SDK/pad.h"
#include "SDK/fade.h"
#include "SDK/global.h"
#include "SDK/snd.h"
#include "SDK/ID.h"
#include "SDK/cockpit.h"
#include "SDK/message.h"
#include "SDK/title.h"
#include "SDK/light.h"
#include "SDK/item.h"
#include "SDK/puzzle.h"
#include "SDK/roomdata.h"
#include "SDK/sscrn.h"
#include "SDK/merchant.h"
#include "SDK/card.h"
#include "SDK/cam_ctrl.h"
#include "SDK/event.h"
#include "SDK/GX.h"
#include "SDK/gc_math.h"

struct INIConfig
{
	int resolutionWidth;
	int resolutionHeight;
	int resolutionRefreshRate;
	int vsync;
	int fullscreen;
	int adapter;
	int shadowQuality;
	int motionblurEnabled;
	int ppEnabled;
	int useHDTexture;
	int antialiasing;
	int anisotropy;
	int variableframerate;
	int subtitle;
	int laserR;
	int laserG;
	int laserB;
	int laserA;
};

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

extern SND_CTRL* Snd_ctrl_work;

std::string GameVersion();
extern int iCurPlayTime[3];
bool GameVersionIsDebug();
int GetGameVariableFrameRate();
void SetGameVariableFrameRate(int newRate);
INIConfig* g_INIConfig();
int CurrentFrameRate();
InputDevices LastUsedDevice();
bool isController();
bool isKeyboardMouse();
int g_MOUSE_SENS();
MouseAimingModes GetMouseAimingMode();
void SetMouseAimingMode(MouseAimingModes newMode);
uint64_t Key_btn_on();
uint64_t Key_btn_trg();
JOY* JoyPtr();

GLOBAL_WK* GlobalPtr();
DAMAGE* DamagePtr();
SYSTEM_SAVE_WORK* SystemSavePtr();
cPlayer* PlayerPtr();
cPlayer* AshleyPtr();
uint8_t* GameSavePtr();
cEmMgr* EmMgrPtr();
TITLE_WORK* TitleWorkPtr();
FADE_WORK* FadeWorkPtr(FADE_NO no);
extern double* fGPUUsagePtr;
extern double* fCPUUsagePtr;

void Game_ScheduleInMainThread(std::function<void()> function);
bool AreaJump(uint16_t roomNo, Vec& position, float rotation);

void Game_SetCameraSmoothness(float scale);
float Game_GetCameraSmoothness();
void InventoryItemAdd(ITEM_ID id, uint32_t count, bool always_show_inv_ui, bool handle_attache_case);

bool IsGanado(int id);
bool IsEnemy(int id);
bool OptionOpenFlag();
const char* GetEmListName(int emListNumber);
const char* GetEmListEnumName(int emListNumber);

// Length seems to always be 0xFFAA0 across all builds
#define GAMESAVE_LENGTH 0xFFAA0

// Original game funcs
namespace bio4
{
	extern void(__cdecl* C_MTXOrtho)(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far);
	extern uint8_t(__cdecl* WeaponId2MaxLevel)(ITEM_ID item_id, int type);
	extern uint8_t(__cdecl* WeaponId2WeaponNo)(ITEM_ID item_id);

	extern void(__cdecl* SceSleep)(uint32_t ctr);

	namespace g_D3D {
		extern uint32_t* RefreshRate;
		extern uint32_t* Width_1;
		extern uint32_t* Height_1;
		extern bool* Fullscreen;
	}

	extern void(__cdecl* D3D_SetupResolution)(int Width, int Height);
	extern void(__cdecl* ScreenReSize)(int Width, int Height);
}