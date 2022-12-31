#pragma once

enum AshleyState
{
	Default,
	Present,
	NotPresent
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