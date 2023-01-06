#pragma once 
#include "Game.h"

enum class MenuTab
{
	Display,
	Audio,
	Mouse,
	Keyboard,
	Controller,
	Framerate,
	Gameplay,
	Misc,
	Memory,
	Hotkeys,
	Trainer,
	NumTabs,
};
extern MenuTab Tab;

enum class TrainerTab
{
	Patches,
	Overrides,
	Hotkeys,
	FlagEdit,
	EmMgr,
	ESP,
	DebugTools,
	ItemMgr,
	NumTabs
};
extern TrainerTab CurTrainerTab; // UI_Utility.cpp

extern ImGuiTextFilter OptionsFilter; // UI_Utility.cpp

bool ImGuiShouldAcceptInput(); // EndSceneHook.cpp

void ImGui_ItemSeparator();
void ImGui_ItemSeparator2();

void ImGui_ColumnInit();
void ImGui_ColumnSwitch();
void ImGui_ColumnFinish();

void ImGui_BeginBackground();
void ImGui_EndBackground(float extraMargin = 10.0f, float rounding = 5.0f, bool fillX = true, ImColor bgCol = ImColor(25, 20, 20, 166));

void ImGui_SetHotkeyComboThread(std::string* cfgHotkey);
void ImGui_SetHotkeyThread(std::string* cfgHotkey);

bool ImGui_ButtonSameLine(const char* label, bool samelinecheck = true, 
	float offset = 0.0f, const ImVec2 size = ImVec2(0, 0));

bool ImGui_TabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, MenuTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size = ImVec2(0, 0));
bool ImGui_TrainerTabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, TrainerTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size = ImVec2(0, 0), const bool samelinecheck = true);

bool ImGui_BufferingBar(const char* label, float value, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
bool ImGui_Spinner(const char* label, float radius, int thickness, const ImU32& color);
