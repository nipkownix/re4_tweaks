#define NOMINMAX
#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "LAApatch.h"
#include "imgui\imgui.h"
#include "imgui\imgui_stdlib.h"
#include "MouseTurning.h"
#include "input.hpp"
#include "EndSceneHook.h"
#include "Patches.h"
#include "AudioTweaks.h"

bool bCfgMenuOpen;
bool NeedsToRestart;
bool bWaitingForHotkey;

ImVec2 PrevRightColumnPos;
ImVec2 PrevLeftColumnPos;

enum class MenuTab
{
	Display,
	Audio,
	Mouse,
	Keyboard,
	Controller,
	Framerate,
	Misc,
	Memory,
	Hotkeys,
	NumTabs,
};

MenuTab Tab = MenuTab::Display;

int MenuTipTimer = 500; // cfgMenu tooltip timer
int iCostumeComboLeon;
int iCostumeComboAshley;
int iCostumeComboAda;

std::string cfgMenuTitle = "re4_tweaks";

std::vector<uint32_t> cfgMenuCombo;

bool ParseConfigMenuKeyCombo(std::string_view in_combo)
{
	cfgMenuCombo.clear();
	cfgMenuCombo = ParseKeyCombo(in_combo);
	return cfgMenuCombo.size() > 0;
}

void cfgMenuBinding()
{
	if (_input->is_combo_pressed(&cfgMenuCombo) && !bWaitingForHotkey)
		bCfgMenuOpen = !bCfgMenuOpen;
}

// If we don't run these in another thread, we end up locking the rendering
void SetHotkeyComboThread(std::string* cfgHotkey)
{
	bWaitingForHotkey = true;

	_input->set_hotkey(cfgHotkey, true);

	bWaitingForHotkey = false;
	return;
}

void SetHotkeyThread(std::string* cfgHotkey)
{
	bWaitingForHotkey = true;

	_input->set_hotkey(cfgHotkey, false);

	bWaitingForHotkey = false;
	return;
}

// ImGui can't render special chars such as "�" without this. Meh.
std::string str_to_utf8(std::string const& str)
{
	int slength = (int)str.length() + 1;
	int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	std::wstring wide(len, L'\0');
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, &wide[0], len);

	slength = (int)wide.length() + 1;
	len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), slength, 0, 0, 0, 0);
	std::string utf8(len, '\0');
	WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), slength, &utf8[0], len, 0, 0);

	return utf8;
}

bool ImGui_TabButton(const char* btnID, const char* text, const ImVec4 &activeCol, 
	const ImVec4 &inactiveCol, MenuTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2 &size = ImVec2(0, 0))
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::PushStyleColor(ImGuiCol_Button, Tab == tabID ? activeCol : inactiveCol);
	bool ret = ImGui::Button(btnID, ImVec2(172, 31));
	ImGui::PopStyleColor();

	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 35.0f, p0.y + 10.0f), iconColor, icon, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 65.0f, p0.y + 8.0f), IM_COL32_WHITE, text, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));

	if (ret)
		Tab = tabID;

	return ret;
}

void ImGui_ItemBG(float RowSize, ImColor bgCol)
{
	// Works best when used inside a table

	ImDrawList * drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();
	float top = ImGui::GetCursorPosY();

	drawList->AddRectFilled(ImVec2(p0.x - 10, p0.y - 10), ImVec2(p0.x + 10 + ImGui::GetContentRegionAvail().x, p0.y + RowSize - top + 7), bgCol, 5.f);
}

void ImGui_ItemSeparator()
{
	// ImGui::Separator() doesn't work inside tables?

	ImDrawList * drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();

	drawList->AddLine(ImVec2(p0.x, p0.y), ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y), ImGui::GetColorU32(ImGuiCol_Separator));
}

void cfgMenuRender()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(940, 640), ImVec2(1280, 720));

	ImGui::Begin("cfgMenu", nullptr,  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
	{
		auto pos = ImGui::GetWindowPos();
		auto draw = ImGui::GetWindowDrawList();

		// Left side BG
		draw->AddRectFilled(ImVec2(pos.x + 0, pos.y + 0), ImVec2(pos.x + 201, pos.y + ImGui::GetWindowHeight()), ImColor(8, 8, 8, 230), 10.f, 5);

		// Separator
		draw->AddLine(ImVec2(pos.x + 15, pos.y + ImGui::GetWindowHeight() - 60), ImVec2(pos.x + 190, pos.y + ImGui::GetWindowHeight() - 60), ImColor(150, 10, 40));

		// Right side BG
		draw->AddRectFilled(ImVec2(pos.x + 200, pos.y + 0), ImVec2(pos.x + ImGui::GetWindowWidth(), pos.y + ImGui::GetWindowHeight()), ImColor(8, 8, 8, 250), 8.f, 10);

		// Separator
		draw->AddLine(ImVec2(pos.x + 210, pos.y + 70), ImVec2(pos.x + ImGui::GetItemRectSize().x - 15, pos.y + 70), ImColor(150, 10, 40));

		// Setup tabs
		{
			ImGui::BeginChild("left side", ImVec2(200, 0));

			ImColor icn_color = ImColor(230, 15, 95);

			ImVec4 active = ImVec4(150.0f / 255.0f, 10.0f / 255.0f, 40.0f / 255.0f, 255.0f / 255.0f);
			ImVec4 inactive = ImVec4(31.0f / 255.0f, 30.0f / 255.0f, 31.0f / 255.0f, 0.0f / 255.0f);

			// cfgMenu title
			ImGui::SetCursorPos(ImVec2(12, 25));

			ImGui::Text(cfgMenuTitle.data());

			ImGui::Dummy(ImVec2(30, 30));

			// Display
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##display", "Display", active, inactive, MenuTab::Display, ICON_FA_DESKTOP, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Audio
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##audio", "Audio", active, inactive, MenuTab::Audio, ICON_FA_HEADPHONES, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Mouse
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##mouse", "Mouse", active, inactive, MenuTab::Mouse, ICON_FA_LOCATION_ARROW, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Keyboard
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##keyboard", "Keyboard", active, inactive, MenuTab::Keyboard, ICON_FA_KEYBOARD, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Controller
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##controller", "Controller", active, inactive, MenuTab::Controller, ICON_FA_JOYSTICK, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Frame rate
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##framerate", "Frame rate", active, inactive, MenuTab::Framerate, ICON_FA_CHESS_CLOCK, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Misc
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##misc", "Misc", active, inactive, MenuTab::Misc, ICON_FA_COG, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Memory
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##memory", "Memory", active, inactive, MenuTab::Memory, ICON_FA_CALCULATOR, icn_color, IM_COL32_WHITE, ImVec2(172, 31));
			ImGui::Spacing();

			// Hotkeys
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##hotkeys", "Hotkeys", active, inactive, MenuTab::Hotkeys, ICON_FA_LAMBDA, icn_color, IM_COL32_WHITE, ImVec2(172, 31));

			ImGui::Dummy(ImVec2(0.0f, 12.0f));

			// Save/Load
			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);

			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			if (ImGui::Button(ICON_FA_ERASER" Discard", ImVec2(85, 35)))
			{
				cfg.ReadSettings();
				ImGui::GetIO().FontGlobalScale = cfg.fFontSize - 0.35f;
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_FA_CODE" Save", ImVec2(85, 35)))
			{
				// Parse key combos on save
				ParseConsoleKeyCombo(cfg.sConsoleKeyCombo);
				ParseToolMenuKeyCombo(cfg.sDebugMenuKeyCombo);
				ParseConfigMenuKeyCombo(cfg.sConfigMenuKeyCombo);
				ParseMouseTurnModifierCombo(cfg.sMouseTurnModifierKeyCombo);
				ParseJetSkiTrickCombo(cfg.sJetSkiTrickCombo);

				// Update console title
				con.TitleKeyCombo = cfg.sConsoleKeyCombo;

				cfg.fFontSize = ImGui::GetIO().FontGlobalScale + 0.35f;
				cfg.WriteSettings();
			}

			ImGui::EndChild();
		}

		ImGui::SameLine();

		// Setup top right
		{
			ImGui::BeginChild("top right", ImVec2(0, 71));

			if (ImGui::BeginTable("Display", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
			{
				ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 150.0f);
				ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

				ImGui::TableNextColumn();

				// Config menu font size
				if (ImGui::Button("-"))
				{
					if (ImGui::GetIO().FontGlobalScale > 0.65f)
						ImGui::GetIO().FontGlobalScale -= 0.05f;

					cfg.HasUnsavedChanges = true;
				}

				ImGui::SameLine();

				if (ImGui::Button("+"))
				{
					if (ImGui::GetIO().FontGlobalScale < 0.90f)
						ImGui::GetIO().FontGlobalScale += 0.05f;

					cfg.HasUnsavedChanges = true;
				}

				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Font Size");

				ImGui::TableNextColumn();

				// Tips
				if (cfg.HasUnsavedChanges)
				{
					const char *txt = "You have unsaved changes!";

					ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(txt).x - 35, ImGui::GetContentRegionAvail().y / 2));
					ImGui::SameLine();
					ImGui::BulletText(txt);
				}

				//ImGui::Dummy(ImVec2(0.0f, 10.0f));

				if (NeedsToRestart)
				{
					const char* txt = "Changes that have been made require the game to be restarted!";

					ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(txt).x - 35, ImGui::GetContentRegionAvail().y / 6));
					ImGui::SameLine();
					ImGui::BulletText(txt);
				}
			}

			ImGui::EndTable();
			ImGui::EndChild();
		}

		// Setup item view
		{
			ImGui::SetCursorPos(ImVec2(216, 80));

			ImGui::BeginChild("right side", ImVec2(0, 0));

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(16.f, 16.f));

			ImColor itmbgColor = ImColor(25, 20, 20, 166);

			if (Tab == MenuTab::Display)
			{
				if (ImGui::BeginTable("Display", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// FOVAdditional
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("FOVAdditional", &cfg.bEnableFOV))
						{
							cfg.HasUnsavedChanges = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Additional FOV value.");
						ImGui::TextWrapped("20 seems good for most cases.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("FOV Slider").x);
						ImGui::BeginDisabled(!cfg.bEnableFOV);
						ImGui::SliderFloat("FOV Slider", &cfg.fFOVAdditional, 0.0f, 50.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (!cfg.bEnableFOV)
							cfg.fFOVAdditional = 0.0f;

						// Store current pos, so we can use it as a BG height.
						// (ImGui really should have a way to simply get the current row/cell height of a table instead...
						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// FixUltraWideAspectRatio
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("FixUltraWideAspectRatio", &cfg.bFixUltraWideAspectRatio))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen. Only tested in 21:9.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// DisableVsync
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("DisableVsync", &cfg.bDisableVsync))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Force V-Sync to be disabled. For some reason the vanilla game doesn't provide a functional way to do this.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// FixDPIScale
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("FixDPIScale", &cfg.bFixDPIScale))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Forces game to run at normal 100%% DPI scaling, fixes resolution issues for players that have above 100%% DPI scaling set.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// FixDisplayMode
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("FixDisplayMode", &cfg.bFixDisplayMode))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows game to use non - 60Hz refresh rates in fullscreen, fixing the black screen issue people have when starting the game.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator();
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::Text("CustomRefreshRate");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(100);
						ImGui::BeginDisabled(!cfg.bFixDisplayMode);
						ImGui::InputInt("Hz", &cfg.iCustomRefreshRate);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (ImGui::IsItemEdited())
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui::Spacing();

						ImGui::TextWrapped("Determines a custom refresh rate for the game to use.");
						ImGui::TextWrapped("Requires FixDisplayMode to be enabled.");
						ImGui::TextWrapped("-1 will make it try to use the current refresh rate as reported by Windows.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// OverrideLaserColor
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("OverrideLaserColor", &cfg.bOverrideLaserColor);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Overrides the color of the laser sights.");
						ImGui::TextWrapped("Note: The game's vanilla config.ini contains a setting to change the");
						ImGui::TextWrapped("alpha/opacity of the laser, but since it doesn't work, we don't include it here.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::BeginDisabled(!cfg.bOverrideLaserColor);
						ImGui::ColorEdit4("Laser color picker", cfg.fLaserRGB, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
						ImGui::EndDisabled();

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// RestorePickupTransparency
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("RestorePickupTransparency", &cfg.bRestorePickupTransparency);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Restores transparency on the item pickup screeen.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// DisableBrokenFilter03
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableBrokenFilter03", &cfg.bDisableBrokenFilter03);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.");
						ImGui::TextWrapped("(if you're using the HD Project, you should disable this option)");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// FixBlurryImage
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixBlurryImage", &cfg.bFixBlurryImage);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes a problem related to a vertex buffer that caused the image to be slightly blurred, making the image much sharper and clearer.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// DisableFilmGrain
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableFilmGrain", &cfg.bDisableFilmGrain);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Disables the film grain overlay that is present in most sections of the game.");

						bgHeight = ImGui::GetCursorPos().y;
					}
					
					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// EnableGCBlur
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("EnableGCBlur", &cfg.bEnableGCBlur))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// EnableGCScopeBlur
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("EnableGCScopeBlur", &cfg.bEnableGCScopeBlur))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// WindowBorderless
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("WindowBorderless", &cfg.bWindowBorderless))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Whether to use a borderless-window when using windowed-mode.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator();
						ImGui::Spacing();

						ImGui::TextWrapped("Position to draw the game window when using windowed mode.");
						ImGui::TextWrapped("-1 will use the games default (usually places it at 0,0)");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::BeginDisabled(cfg.bRememberWindowPos);
						ImGui::PushItemWidth(150);
						ImGui::InputInt("X Pos", &cfg.iWindowPositionX);
						ImGui::InputInt("Y Pos", &cfg.iWindowPositionY);
						ImGui::PopItemWidth();
						ImGui::EndDisabled();

						ImGui::Dummy(ImVec2(10, 10));
						cfg.HasUnsavedChanges |= ImGui::Checkbox("RememberWindowPos", &cfg.bRememberWindowPos);
						ImGui::TextWrapped("Remember the last window position. This automatically updates the \"X Pos\" and \"Y Pos\" values above.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}
			
			if (Tab == MenuTab::Audio)
			{
				if (ImGui::BeginTable("Display", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// Audio volume
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::Spacing();
						ImGui::TextWrapped("Adjust the volume of the game");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Cutscene Volume will also affect volume of merchant screen dialogue, as that seems to be handled the same way as cutscenes.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Cutscene Volume").x);
						bool changed = ImGui::SliderInt("Master Volume", &cfg.iVolumeMaster, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Music Volume", &cfg.iVolumeBGM, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Effect Volume", &cfg.iVolumeSE, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Cutscene Volume", &cfg.iVolumeCutscene, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						ImGui::PopItemWidth();

						if (changed)
						{
							cfg.HasUnsavedChanges = true;
							AudioTweaks_UpdateVolume();
						}

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::EndTable();
				}
			}
						
			if (Tab == MenuTab::Mouse)
			{
				if (ImGui::BeginTable("Mouse", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// UseMouseTurning
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("UseMouseTurning", &cfg.bUseMouseTurning);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Makes it so the mouse turns the character instead of controlling the camera.");
						ImGui::TextWrapped("\"Modern\" aiming mode in the game's settings is recommended.");

						ImGui::Dummy(ImVec2(10, 20));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Sensitivity Slider").x);
						ImGui::BeginDisabled(!cfg.bUseMouseTurning);
						ImGui::SliderFloat("Sensitivity Slider", &cfg.fTurnSensitivity, 0.50f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// UseRawMouseInput
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("UseRawMouseInput", &cfg.bUseRawMouseInput);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Makes the game use Raw Input for aiming and turning (if MouseTurning is enabled).");
						ImGui::TextWrapped("Greatly improves mouse input by removing negative/positive accelerations that were being applied both by the game and by Direct Input.");
						ImGui::TextWrapped("This option automatically enables the game's \"Modern\" aiming mode, and is incompatible with the \"Classic\" mode.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// UnlockCameraFromAim
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("UnlockCameraFromAim", &cfg.bUnlockCameraFromAim);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("When using the \"Modern\" mouse setting, the game locks the camera position to the aiming position, making both move together.");
						ImGui::TextWrapped("Although this is the expected behavior in most games, some people might prefer to keep the original camera behavior while also having the benefits from \"Modern\" aiming.");
						ImGui::TextWrapped("Enabling this will also restore the horizontal aiming sway that was lost when the devs implemented \"Modern\" aiming.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// FixSniperZoom
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixSniperZoom", &cfg.bFixSniperZoom);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// FixSniperFocus
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("FixSniperFocus", &cfg.bFixSniperFocus))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("When zooming in and out with the sniper rifle, this option makes the \"focus\" animation look similar to how it looks like with a controller.");
						ImGui::TextWrapped("Requires EnableGCBlur to be enabled.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// FixRetryLoadMouseSelector
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixRetryLoadMouseSelector", &cfg.bFixRetryLoadMouseSelector);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Prevents the game from overriding your selection in the \"Retry / Load\" screen when moving the mouse before confirming an action.");
						ImGui::TextWrapped("This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Keyboard)
			{
				if (ImGui::BeginTable("Keyboard", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// FallbackToEnglishKeyIcons
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("FallbackToEnglishKeyIcons", &cfg.bFallbackToEnglishKeyIcons))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Game will turn keys invisible for certain unsupported keyboard languages, enabling this should make game use English keys for unsupported ones instead (requires game restart)");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Controller)
			{
				if (ImGui::BeginTable("Controller", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// OverrideControllerSensitivity
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("OverrideControllerSensitivity", &cfg.bOverrideControllerSensitivity);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Change the controller sensitivity. For some reason the vanilla game doesn't have an option to change it for controllers, only for the mouse.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Sensitivity Slider").x);
						ImGui::BeginDisabled(!cfg.bOverrideControllerSensitivity);
						ImGui::SliderFloat("Sensitivity Slider", &cfg.fControllerSensitivity, 0.50f, 4.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (!cfg.bOverrideControllerSensitivity)
							cfg.fControllerSensitivity = 1.0f;

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// RemoveExtraXinputDeadzone
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("RemoveExtraXinputDeadzone", &cfg.bRemoveExtraXinputDeadzone);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Removes unnecessary deadzones that were added for Xinput controllers.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// EnableDeadzoneOverride
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("EnableDeadzoneOverride", &cfg.bOverrideXinputDeadzone);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Change the Xinput controller deadzone.");
						ImGui::TextWrapped("(Unrelated to the previous option)");
						ImGui::TextWrapped("The game's default is 1, but that seems unnecessarily large, so we default to 0.4 instead.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Deadzone Slider").x);
						ImGui::BeginDisabled(!cfg.bOverrideXinputDeadzone);
						ImGui::SliderFloat("Deadzone Slider", &cfg.fXinputDeadzone, 0.0f, 3.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (ImGui::IsItemEdited())
						{
							cfg.HasUnsavedChanges = true;
							*g_XInputDeadzone_LS = (int)(cfg.fXinputDeadzone * 7849);
							*g_XInputDeadzone_RS = (int)(cfg.fXinputDeadzone * 8689);
						}

						if (!cfg.bOverrideXinputDeadzone)
							cfg.fXinputDeadzone = 1.0f;

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Framerate)
			{
				if (ImGui::BeginTable("Framerate", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// FixFallingItemsSpeed
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixFallingItemsSpeed", &cfg.bFixFallingItemsSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes the speed of falling items in 60 FPS, making them not fall at double speed.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// FixTurningSpeed
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixTurningSpeed", &cfg.bFixTurningSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes speed of backwards turning when using framerates other than 30FPS.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// FixQTE
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixQTE", &cfg.bFixQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs.");
						ImGui::TextWrapped("This fix makes QTEs that involve rapid button presses much more forgiving.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// FixAshleyBustPhysics
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixAshleyBustPhysics", &cfg.bFixAshleyBustPhysics);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes difference between 30/60FPS on physics applied to Ashley.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// EnableFastMath
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("EnableFastMath", &cfg.bEnableFastMath))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Replaces older math functions in the game with much more optimized equivalents.");
						ImGui::TextWrapped("Experimental, can hopefully improve framerate in some areas that had dips.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// PrecacheModels
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("PrecacheModels", &cfg.bPrecacheModels);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Forces game to fully cache all models in the level after loading in.");
						ImGui::TextWrapped("May help with framerate drops when viewing a model for the first time.");
						ImGui::TextWrapped("(not fully tested, could cause issues if level has many models to load!)");
						ImGui::TextWrapped("Changes take effect on next level load.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Misc)
			{
				if (ImGui::BeginTable("Misc", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// OverrideCostumes
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("OverrideCostumes", &cfg.bOverrideCostumes);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows overriding the costumes, making it possible to combine Normal/Special 1/Special 2 costumes.");
						ImGui::TextWrapped("May cause weird visuals in cutscenes.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::BeginDisabled(!cfg.bOverrideCostumes);
						ImGui::PushItemWidth(150);
						ImGui::Combo("Leon", &iCostumeComboLeon, sLeonCostumeNames, IM_ARRAYSIZE(sLeonCostumeNames));
						if (ImGui::IsItemEdited())
						{
							cfg.HasUnsavedChanges = true;
							cfg.CostumeOverride.Leon = (LeonCostumes)iCostumeComboLeon;
						}

						ImGui::Combo("Ashley", &iCostumeComboAshley, sAshleyCostumeNames, IM_ARRAYSIZE(sAshleyCostumeNames));
						if (ImGui::IsItemEdited())
						{
							cfg.HasUnsavedChanges = true;
							cfg.CostumeOverride.Ashley = (AshleyCostumes)iCostumeComboAshley;
						}

						ImGui::Combo("Ada", &iCostumeComboAda, sAdaCostumeNames, IM_ARRAYSIZE(sAdaCostumeNames));
						if (ImGui::IsItemEdited())
						{
							cfg.HasUnsavedChanges = true;
							cfg.CostumeOverride.Ada = (AdaCostumes)iCostumeComboAda;
						}
						ImGui::PopItemWidth();
						ImGui::EndDisabled();

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// AshleyJPCameraAngles
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("AshleyJPCameraAngles", &cfg.bAshleyJPCameraAngles);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Unlocks the JP-only classic camera angles during Ashley segment.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// ViolenceLevelOverride
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::Text("ViolenceLevelOverride");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows overriding the level of violence in the game.");
						ImGui::TextWrapped("Use -1 to leave as games default, 0 for low-violence mode (as used in JP/GER version), or 2 for full violence.");
						
						ImGui::Dummy(ImVec2(10, 10));
						ImGui::PushItemWidth(100);
						if (ImGui::InputInt("Violence Level Override", &cfg.iViolenceLevelOverride))
						{
							if (cfg.iViolenceLevelOverride < -1)
								cfg.iViolenceLevelOverride = -1;

							if (cfg.iViolenceLevelOverride > 2)
								cfg.iViolenceLevelOverride = 2;

							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true; // unfortunately required as game only reads pSys from savegame during first loading screen...
						}
						ImGui::PopItemWidth();

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// AllowSellingHandgunSilencer
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("AllowSellingHandgunSilencer", &cfg.bAllowSellingHandgunSilencer))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows selling the (normally unused) handgun silencer to the merchant.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// AllowMafiaLeonCutscenes
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("AllowMafiaLeonCutscenes", &cfg.bAllowMafiaLeonCutscenes))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows the game to display Leon's mafia outfit (\"Special 2\") on cutscenes.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// SilenceArmoredAshley
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("SilenceArmoredAshley", &cfg.bSilenceArmoredAshley);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Silence Ashley's armored outfit (\"Special 2\").");
						ImGui::TextWrapped("For those who also hate the constant \"Clank Clank Clank\".");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// AllowAshleySuplex
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("AllowAshleySuplex", &cfg.bAllowAshleySuplex);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows Ashley to Suplex enemies in very specific situations.");
						ImGui::TextWrapped("(previously was only possible in the initial NTSC GameCube ver., was patched out in all later ports.)");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// AllowMatildaQuickturn
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("AllowMatildaQuickturn", &cfg.bAllowMatildaQuickturn);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows quickturning character to camera direction when wielding Matilda.");
						ImGui::TextWrapped("(only effective if MouseTurning is disabled)");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// FixDitmanGlitch
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("FixDitmanGlitch", &cfg.bFixDitmanGlitch);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes the Ditman glitch, which would allow players to increase their walk speed.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// UseSprintToggle
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("UseSprintToggle", &cfg.bUseSprintToggle);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Changes sprint key to act like a toggle instead of needing to be held.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// DisableQTE
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableQTE", &cfg.bDisableQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Disables most of the QTEs, making them pass automatically.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// AutomaticMashingQTE
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("AutomaticMashingQTE", &cfg.bAutomaticMashingQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Unlike the previous option, this only automates the \"mashing\" QTEs, making them pass automatically. Prompts are still shown!");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// SkipIntroLogos
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						cfg.HasUnsavedChanges |= ImGui::Checkbox("SkipIntroLogos", &cfg.bSkipIntroLogos);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Whether to skip the Capcom etc intro logos when starting the game.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// EnableDebugMenu
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("EnableDebugMenu", &cfg.bEnableDebugMenu))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Enables the \"tool menu\" debug menu, present inside the game but unused, and adds a few custom menu entries (\"SAVE GAME\", \"DOF / BLUR MENU\", etc).");
						ImGui::TextWrapped("Can be opened with the LT+LS button combination (or CTRL+F3 on keyboard by default).");
						ImGui::TextWrapped("If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.");
						ImGui::TextWrapped("(may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Memory)
			{
				if (ImGui::BeginTable("Memory", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// AllowHighResolutionSFD
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("AllowHighResolutionSFD", &cfg.bAllowHighResolutionSFD))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.");
						ImGui::TextWrapped("Not tested beyond 1920x1080.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// RaiseVertexAlloc
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("RaiseVertexAlloc", &cfg.bRaiseVertexAlloc))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allocate more memory for some vertex buffers.");
						ImGui::TextWrapped("This prevents a crash that can happen when playing with a high FOV.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// RaiseInventoryAlloc
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						if (ImGui::Checkbox("RaiseInventoryAlloc", &cfg.bRaiseInventoryAlloc))
						{
							cfg.HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Hotkeys)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.51f, 0.00f, 0.14f, 1.00f));

				if (ImGui::BeginTable("HotkeysTop", 1, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui::TableNextColumn();

					static float bgHeight = 0;
					ImGui_ItemBG(bgHeight, itmbgColor);

					ImGui::TextWrapped("Key combinations for various re4_tweaks features.");
					ImGui::TextWrapped("Most keys can be combined (requiring multiple to be pressed at the same time). To combine, hold one key and press another at the same time.");
					ImGui::TextWrapped("(Press \"Save\" for changes to take effect.)");

					bgHeight = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				if (ImGui::BeginTable("Hotkeys", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					float column0_lastY = 0;
					float column1_lastY = 0;

					ImGui::TableNextColumn();

					// cfgMenu 
					{
						ImGui::TableSetColumnIndex(0); // First item on column0, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("Key combination to open the re4_tweaks config menu");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(1);
						if (ImGui::Button(str_to_utf8(cfg.sConfigMenuKeyCombo).c_str(), ImVec2(150, 0)))
						{
							cfg.HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sConfigMenuKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open config menu");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// Console
					{
						ImGui::TableSetColumnIndex(1); // First item on column1, no need to restore lastY

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("Key combination to open the re4_tweaks debug console (only in certain re4_tweaks builds)");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(2);
						if (ImGui::Button(str_to_utf8(cfg.sConsoleKeyCombo).c_str(), ImVec2(150, 0)))
						{
							cfg.HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sConsoleKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open console");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// Inv flip 
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("Key bindings for flipping items in the inventory screen when using keyboard and mouse.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Normally, you can only rotate them with the keyboard, not flip them. Flipping was possible in the old PC port and is possible using a controller.");
						ImGui::Spacing();
						ImGui::BulletText("Key combinations not supported");
						ImGui::Spacing();
						
						if (ImGui::BeginTable("inv flip", 2))
						{
							ImGui::TableNextColumn();
							ImGui::Spacing();

							// UP
							ImGui::TextWrapped("Flip UP");
							ImGui::PushID(3);
							if (ImGui::Button(str_to_utf8(cfg.sFlipItemUp).c_str(), ImVec2(140, 0)))
							{
								cfg.HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemUp, 0, NULL);
							}
							ImGui::PopID();

							ImGui::Dummy(ImVec2(10, 10));

							// DOWN
							ImGui::TextWrapped("Flip DOWN");
							ImGui::PushID(4);
							if (ImGui::Button(str_to_utf8(cfg.sFlipItemDown).c_str(), ImVec2(140, 0)))
							{
								cfg.HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemDown, 0, NULL);
							}
							ImGui::PopID();

							ImGui::TableNextColumn();

							// LEFT
							ImGui::TextWrapped("Flip LEFT");
							ImGui::PushID(5);
							if (ImGui::Button(str_to_utf8(cfg.sFlipItemLeft).c_str(), ImVec2(140, 0)))
							{
								cfg.HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemLeft, 0, NULL);
							}
							ImGui::PopID();

							ImGui::Dummy(ImVec2(10, 10));

							// RIGHT
							ImGui::TextWrapped("Flip RIGHT");
							ImGui::PushID(6);
							if (ImGui::Button(str_to_utf8(cfg.sFlipItemRight).c_str(), ImVec2(140, 0)))
							{
								cfg.HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemRight, 0, NULL);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// QTE 
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("Key bindings for QTE keys when playing with keyboard and mouse.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Unlike the \"official\" way of rebinding keys through usr_input.ini, this option also changes the on-screen prompt to properly match the selected key.");
						ImGui::BulletText("Key combinations not supported");
						ImGui::Spacing();

						if (ImGui::BeginTable("qtekey", 2))
						{
							ImGui::TableNextColumn();
							ImGui::Spacing();

							// QTE1
							ImGui::TextWrapped("QTE key 1");
							ImGui::PushID(7);
							if (ImGui::Button(str_to_utf8(cfg.sQTE_key_1).c_str(), ImVec2(140, 0)))
							{
								cfg.HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sQTE_key_1, 0, NULL);
							}
							ImGui::PopID();

							ImGui::TableNextColumn();

							// QTE2
							ImGui::TextWrapped("QTE key 2");
							ImGui::PushID(8);
							if (ImGui::Button(str_to_utf8(cfg.sQTE_key_2).c_str(), ImVec2(140, 0)))
							{
								cfg.HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sQTE_key_2, 0, NULL);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// Debug menu
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("Key combination to make the \"tool menu\" debug menu appear.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Requires EnableDebugMenu to be enabled.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(9);
						if (ImGui::Button(str_to_utf8(cfg.sDebugMenuKeyCombo).c_str(), ImVec2(150, 0)))
						{
							cfg.HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sDebugMenuKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open debug menu");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					// MouseTurningModifier
					{
						ImGui::TableSetColumnIndex(1);
						ImGui::SetCursorPosY(column1_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("MouseTurning camera modifier.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("When holding this key combination, MouseTurning is temporarily activated/deactivated.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(10);
						if (ImGui::Button(str_to_utf8(cfg.sMouseTurnModifierKeyCombo).c_str(), ImVec2(150, 0)))
						{
							cfg.HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sMouseTurnModifierKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("MouseTurning modifier");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column1_lastY = ImGui::GetCursorPos().y;

					// JetSkiTricks 
					{
						ImGui::TableSetColumnIndex(0);
						ImGui::SetCursorPosY(column0_lastY);

						static float bgHeight = 0;
						ImGui_ItemBG(bgHeight, itmbgColor);

						ImGui::TextWrapped("Key combination for jump tricks during Jet-ski sequence (normally RT+LT on controller)");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(11);
						if (ImGui::Button(str_to_utf8(cfg.sJetSkiTrickCombo).c_str(), ImVec2(150, 0)))
						{
							cfg.HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sJetSkiTrickCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Trick while riding Jet-ski");

						bgHeight = ImGui::GetCursorPos().y;
					}

					ImGui::Dummy(ImVec2(10, 25));
					column0_lastY = ImGui::GetCursorPos().y;

					ImGui::EndTable();
				}
				ImGui::PopStyleColor();
			}

			ImGui::PopStyleVar();

			ImGui::EndChild();
		}

		//
		ImGui::End();
	}
}

void ShowCfgMenuTip()
{
	if (!bCfgMenuOpen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(20, viewport->Pos.y + 20));
		ImGui::SetNextWindowBgAlpha(0.5);

		ImGui::Begin("cfgMenuTip", nullptr, ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

		std::string tooltip = str_to_utf8(std::string("re4_tweaks: Press ") + cfg.sConfigMenuKeyCombo + std::string(" to open the configuration menu"));

		ImGui::TextUnformatted(tooltip.data());

		ImGui::End();
	}
}
