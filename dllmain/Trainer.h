#pragma once

enum AshleyState
{
	Default,
	Present,
	NotPresent
};

void Trainer_ParseKeyCombos();
void Trainer_Init();
void Trainer_Update();
void Trainer_RenderUI();
void Trainer_ESP();