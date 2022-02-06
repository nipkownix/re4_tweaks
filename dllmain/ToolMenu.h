#pragma once

void Init_ToolMenu();
void Init_ToolMenuDebug();
void ToolMenuBinding(UINT uMsg, WPARAM wParam);
bool ParseToolMenuKeyCombo(std::string_view in_combo);