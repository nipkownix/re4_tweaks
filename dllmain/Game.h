#pragma once
#include <cstdint>
#include "GameFlags.h"

#include "SDK/event.h"
#include "SDK/EtcModel.h"
#include "SDK/player.h"
#include "SDK/pad.h"
#include "SDK/global.h"
#include "SDK/snd.h"
#include "SDK/title.h"

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
bool GameVersionIsDebug();
InputDevices LastUsedDevice();
bool isController();
bool isKeyboardMouse();
int g_MOUSE_SENS();
MouseAimingModes GetMouseAimingMode();
void SetMouseAimingMode(MouseAimingModes newMode);
uint64_t Key_btn_on();
uint64_t Key_btn_trg();

GLOBAL_WK* GlobalPtr();
DAMAGE* DamagePtr();
SYSTEM_SAVE_WORK* SystemSavePtr();
cPlayer* PlayerPtr();
cPlayer* AshleyPtr();
uint8_t* GameSavePtr();
cEmMgr* EmMgrPtr();
TITLE_WORK* TitleWorkPtr();

void AreaJump(uint16_t roomNo, Vec& position, float rotation);

bool IsGanado(int id);
const char* GetEmListName(int emListNumber);

// Length seems to always be 0xFFAA0 across all builds
#define GAMESAVE_LENGTH 0xFFAA0

// Original game funcs
extern bool(__cdecl* game_KeyOnCheck_0)(KEY_BTN a1);
extern void(__cdecl* game_C_MTXOrtho)(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far);
