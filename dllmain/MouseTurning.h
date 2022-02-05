#pragma once

void Init_MouseTurning();
void isMouseTurnEnabled(UINT uMsg, WPARAM wParam);
bool ParseMouseTurnModifierCombo(std::string_view in_combo);