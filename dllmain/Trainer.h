#pragma once

bool ParseNoclipToggleCombo(std::string_view in_combo);
void Trainer_Init();
void Trainer_Update();

extern bool bUseNumpadMovement;
extern bool bUseMouseWheelUPDOWN;
