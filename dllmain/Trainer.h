#pragma once

bool ParseNoclipToggleCombo(std::string_view in_combo);
void Trainer_Init();
void Trainer_Update();
void Trainer_RenderUI();

extern bool bUseNumpadMovement;
extern bool bUseMouseWheelUPDOWN;
