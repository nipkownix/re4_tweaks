#pragma once

// Init functions
void Init_60fpsFixes();
void Init_ControllerTweaks();
void Init_D3D9Hook();
void Init_DisplayTweaks();
void Init_ExceptionHandler();
void Init_FilterXXFixes();
bool Init_Game();
void Init_HandleLimits();
void Init_Input();
void Init_KeyboardMouseTweaks();
void Init_Logging();
void Init_MathReimpl();
void Init_Misc();
void Init_QTEfixes();
void Init_sofdec();
void Init_ToolMenu();
void Init_ToolMenuDebug();
void Init_UltraWideFix();
void Init_WndProcHook();

// Hotkey bindings and parsing
void ConsoleBinding();
void cfgMenuBinding();
bool ParseConsoleKeyCombo(std::string_view in_combo);
bool ParseConfigMenuKeyCombo(std::string_view in_combo);
bool ParseMouseTurnModifierCombo(std::string_view in_combo);
bool ParseToolMenuKeyCombo(std::string_view in_combo);
bool ParseJetSkiTrickCombo(std::string_view in_combo);

// Deadzone pointers
extern int* g_XInputDeadzone_LS;
extern int* g_XInputDeadzone_RS;