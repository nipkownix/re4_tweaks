#pragma once
#include "UI_Utility.h"

enum AshleyState
{
	Default,
	Present,
	NotPresent
};

// Make sure to keep order of this in sync with flagCategoryInfo vector below
enum class CategoryInfoIdx
{
	DEBUG,
	STOP,
	STATUS,
	SYSTEM,
	ITEM_SET,
	SCENARIO,
	KEY_LOCK,
	DISP,
	EXTRA,
	CONFIG,
	ROOM_SAVE,
	ROOM
};

extern void PauseGame(bool setPaused); // cfgMenu.cpp
extern bool bPauseGameWhileInCfgMenu; // cfgMenu.cpp

extern bool ShowDebugTrgHint;

void Trainer_DrawDebugTrgHint();
void Trainer_ParseKeyCombos();
void Trainer_Init();
void Trainer_Update();
void Trainer_RenderUI(int columnCount);
void Trainer_ESP();

// Trainr_ItemMgr.cpp
void ItemMgr_Update();
void ItemMgr_Render();