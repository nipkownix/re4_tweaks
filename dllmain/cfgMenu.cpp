#define NOMINMAX
#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Settings.h"
#include "imgui\imgui.h"
#include "imgui\imgui_stdlib.h"
#include "input.hpp"
#include "Patches.h"
#include <hashes.h>
#include "Utils.h"
#include "Trainer.h"

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
	Trainer,
	NumTabs,
};

enum class TrainerTab
{
	Patches,
	EmManager
};

MenuTab Tab = MenuTab::Display;
TrainerTab CurTrainerTab = TrainerTab::Patches;

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
	if (pInput->is_combo_pressed(&cfgMenuCombo) && !bWaitingForHotkey)
		bCfgMenuOpen = !bCfgMenuOpen;
}

// If we don't run these in another thread, we end up locking the rendering
void SetHotkeyComboThread(std::string* cfgHotkey)
{
	bWaitingForHotkey = true;

	pInput->set_hotkey(cfgHotkey, true);

	bWaitingForHotkey = false;
	return;
}

void SetHotkeyThread(std::string* cfgHotkey)
{
	bWaitingForHotkey = true;

	pInput->set_hotkey(cfgHotkey, false);

	bWaitingForHotkey = false;
	return;
}

bool ImGui_TabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, MenuTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size = ImVec2(0, 0))
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

bool ImGui_TrainerTabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, TrainerTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size = ImVec2(0, 0))
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::PushStyleColor(ImGuiCol_Button, CurTrainerTab == tabID ? activeCol : inactiveCol);
	bool ret = ImGui::Button(btnID, ImVec2(172, 31));
	ImGui::PopStyleColor();

	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 35.0f, p0.y + 10.0f), iconColor, icon, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 65.0f, p0.y + 8.0f), IM_COL32_WHITE, text, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));

	if (ret)
		CurTrainerTab = tabID;

	return ret;
}

void ImGui_ItemBG(float RowSize, ImColor bgCol)
{
	// Works best when used inside a table

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();
	float top = ImGui::GetCursorPosY();

	drawList->AddRectFilled(ImVec2(p0.x - 10, p0.y - 10), ImVec2(p0.x + 10 + ImGui::GetContentRegionAvail().x, p0.y + RowSize - top + 7), bgCol, 5.f);
}

void ImGui_ItemSeparator()
{
	// ImGui::Separator() doesn't work inside tables?

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();

	drawList->AddLine(ImVec2(p0.x, p0.y), ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y), ImGui::GetColorU32(ImGuiCol_Separator));
}

void ImGui_ItemSeparator2()
{
	// ImGui::Separator() doesn't work inside tables?

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();

	drawList->AddRectFilled(ImVec2(p0.x, p0.y), ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + 3), ImColor(150, 10, 40));
}

ImColor itmbgColor = ImColor(25, 20, 20, 166);
int itemIdx = -1;
float columnLastY[2] = { 0,0 };
void ImGui_ColumnInit()
{
	itemIdx = -1;
	columnLastY[0] = columnLastY[1] = 0;

	ImGui::TableNextColumn();
}

std::vector<float> bgHeightsPerTab[int(MenuTab::NumTabs)]; // item heights, per-tab
void ImGui_ColumnSwitch()
{
	auto& bgHeights = bgHeightsPerTab[int(Tab)];
	if (itemIdx >= 0)
	{
		// Store bgheight of previous item, so we can use it as a BG height.
		// (ImGui really should have a way to simply get the current row/cell height of a table instead...
		if (bgHeights.size() > size_t(itemIdx))
			bgHeights[itemIdx] = ImGui::GetCursorPos().y;
		else
			bgHeights.push_back(ImGui::GetCursorPos().y);

		ImGui::Dummy(ImVec2(10, 25));
		columnLastY[itemIdx % 2] = ImGui::GetCursorPos().y;
	}

	itemIdx++;
	ImGui::TableSetColumnIndex(itemIdx % 2);

	if (itemIdx > 1) // 0/1 are first item on column, no need to restore lastY
		ImGui::SetCursorPosY(columnLastY[itemIdx % 2]);

	float bgHeight = 0;
	if (bgHeights.size() > size_t(itemIdx))
		bgHeight = bgHeights[itemIdx];
	else
		bgHeight = 0;

	ImGui_ItemBG(bgHeight, itmbgColor);
}

void ImGui_ColumnFinish()
{
	if (itemIdx >= 0)
	{
		auto& bgHeights = bgHeightsPerTab[int(Tab)];

		// Store bgheight of previous item
		if (bgHeights.size() > size_t(itemIdx))
			bgHeights[itemIdx] = ImGui::GetCursorPos().y;
		else
			bgHeights.push_back(ImGui::GetCursorPos().y);
	}

	ImGui::Dummy(ImVec2(10, 25));
}

void cfgMenuRender()
{
	ImGui::SetNextWindowSizeConstraints(ImVec2(940, 640), ImVec2(1280, 720));

	ImGui::Begin("cfgMenu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
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
			ImGui::Spacing();

			// Trainer
			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			ImGui_TabButton("##trainer", "Trainer", active, inactive, MenuTab::Trainer, ICON_FA_SHOE_PRINTS, icn_color, IM_COL32_WHITE, ImVec2(172, 31));

			ImGui::Dummy(ImVec2(0.0f, 12.0f));

			// Save/Load
			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);

			ImGui::Dummy(ImVec2(0, 13)); ImGui::SameLine();
			if (ImGui::Button(ICON_FA_ERASER" Discard", ImVec2(85, 35)))
			{
				pConfig->ReadSettings();
				ImGui::GetIO().FontGlobalScale = pConfig->fFontSize - 0.35f;
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_FA_CODE" Save", ImVec2(85, 35)))
			{
				// Parse key combos on save
				ParseConsoleKeyCombo(pConfig->sConsoleKeyCombo);
				ParseToolMenuKeyCombo(pConfig->sDebugMenuKeyCombo);
				ParseConfigMenuKeyCombo(pConfig->sConfigMenuKeyCombo);
				ParseMouseTurnModifierCombo(pConfig->sMouseTurnModifierKeyCombo);
				ParseJetSkiTrickCombo(pConfig->sJetSkiTrickCombo);

				// Update console title
				con.TitleKeyCombo = pConfig->sConsoleKeyCombo;

				pConfig->fFontSize = ImGui::GetIO().FontGlobalScale + 0.35f;
				pConfig->WriteSettings();
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

					pConfig->HasUnsavedChanges = true;
				}

				ImGui::SameLine();

				if (ImGui::Button("+"))
				{
					if (ImGui::GetIO().FontGlobalScale < 0.90f)
						ImGui::GetIO().FontGlobalScale += 0.05f;

					pConfig->HasUnsavedChanges = true;
				}

				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Font Size");

				ImGui::TableNextColumn();

				// Tips
				if (pConfig->HasUnsavedChanges)
				{
					const char* txt = "You have unsaved changes!";

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

				ImGui::EndTable();
			}

			ImGui::EndChild();
		}

		// Setup item view
		{
			ImGui::SetCursorPos(ImVec2(216, 80));

			ImGui::BeginChild("right side", ImVec2(0, 0));

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(16.f, 16.f));

			if (Tab == MenuTab::Display)
			{
				if (ImGui::BeginTable("Display", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// FOVAdditional
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FOVAdditional", &pConfig->bEnableFOV))
						{
							pConfig->HasUnsavedChanges = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Additional FOV value.");
						ImGui::TextWrapped("20 seems good for most cases.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("FOV Slider").x);
						ImGui::BeginDisabled(!pConfig->bEnableFOV);
						ImGui::SliderFloat("FOV Slider", &pConfig->fFOVAdditional, 0.0f, 50.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (!pConfig->bEnableFOV)
							pConfig->fFOVAdditional = 0.0f;
					}

					// Aspect ratio tweaks
					{
						ImGui_ColumnSwitch();

						ImGui::Spacing();
						ImGui::TextWrapped("Aspect ratio tweaks");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						if (ImGui::Checkbox("UltraWideAspectSupport", &pConfig->bUltraWideAspectSupport))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}
						ImGui::TextWrapped("Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::BeginDisabled(!pConfig->bUltraWideAspectSupport);
						if (ImGui::Checkbox("SideAlignHUD", &pConfig->bSideAlignHUD))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}
						ImGui::TextWrapped("Moves the HUD to the right side of the screen. (Requires restart or game load to take effect!)");

						ImGui::Spacing();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("StretchFullscreenImages", &pConfig->bStretchFullscreenImages);
						ImGui::TextWrapped("Streches some images to fit the screen, such as the images shown when reading \"Files\".");

						ImGui::Spacing();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("StretchVideos", &pConfig->bStretchVideos);
						ImGui::TextWrapped("Streches pre-rendered videos to fit the screen.");
						ImGui::EndDisabled();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10));

						if (ImGui::Checkbox("Remove16by10BlackBars", &pConfig->bRemove16by10BlackBars))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}
						ImGui::TextWrapped("Removes top and bottom black bars that are present when playing in 16:10. Will crop a few pixels from each side of the screen.");
					}

					// DisableVsync
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("DisableVsync", &pConfig->bDisableVsync))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Force V-Sync to be disabled. For some reason the vanilla game doesn't provide a functional way to do this.");
					}

					// FixDPIScale
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FixDPIScale", &pConfig->bFixDPIScale))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Forces game to run at normal 100%% DPI scaling, fixes resolution issues for players that have above 100%% DPI scaling set.");
					}

					// FixDisplayMode
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FixDisplayMode", &pConfig->bFixDisplayMode))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows game to use non - 60Hz refresh rates in fullscreen, fixing the black screen issue people have when starting the game.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::Text("CustomRefreshRate");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(100);
						ImGui::BeginDisabled(!pConfig->bFixDisplayMode);
						ImGui::InputInt("Hz", &pConfig->iCustomRefreshRate);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (ImGui::IsItemEdited())
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui::Spacing();

						ImGui::TextWrapped("Determines a custom refresh rate for the game to use.");
						ImGui::TextWrapped("Requires FixDisplayMode to be enabled.");
						ImGui::TextWrapped("-1 will make it try to use the current refresh rate as reported by Windows.");
					}

					// OverrideLaserColor
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("OverrideLaserColor", &pConfig->bOverrideLaserColor);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Overrides the color of the laser sights.");
						ImGui::TextWrapped("Note: The game's vanilla config.ini contains a setting to change the");
						ImGui::TextWrapped("alpha/opacity of the laser, but since it doesn't work, we don't include it here.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::BeginDisabled(!pConfig->bOverrideLaserColor || pConfig->bRainbowLaser);
						ImGui::ColorEdit4("Laser color picker", pConfig->fLaserRGB, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
						ImGui::EndDisabled();

						ImGui::Spacing();

						ImGui::BeginDisabled(!pConfig->bOverrideLaserColor);
						pConfig->HasUnsavedChanges |= ImGui::Checkbox("RainbowLaser", &pConfig->bRainbowLaser);
						ImGui::EndDisabled();

						if (!pConfig->bOverrideLaserColor)
							pConfig->bRainbowLaser = false;
					}

					// RestorePickupTransparency
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("RestorePickupTransparency", &pConfig->bRestorePickupTransparency);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Restores transparency on the item pickup screeen.");
					}

					// DisableBrokenFilter03
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("DisableBrokenFilter03", &pConfig->bDisableBrokenFilter03);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.");
						ImGui::TextWrapped("(if you're using the HD Project, you should disable this option)");
					}

					// FixBlurryImage
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixBlurryImage", &pConfig->bFixBlurryImage);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes a problem related to a vertex buffer that caused the image to be slightly blurred, making the image much sharper and clearer.");
					}

					// DisableFilmGrain
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("DisableFilmGrain", &pConfig->bDisableFilmGrain);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Disables the film grain overlay that is present in most sections of the game.");
					}

					// EnableGCBlur
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableGCBlur", &pConfig->bEnableGCBlur))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.");
					}

					// EnableGCScopeBlur
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableGCScopeBlur", &pConfig->bEnableGCScopeBlur))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.");
					}

					// WindowBorderless
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("WindowBorderless", &pConfig->bWindowBorderless))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Whether to use a borderless-window when using windowed-mode.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Position to draw the game window when using windowed mode.");
						ImGui::TextWrapped("-1 will use the games default (usually places it at 0,0)");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::BeginDisabled(pConfig->bRememberWindowPos);
						ImGui::PushItemWidth(150);
						ImGui::InputInt("X Pos", &pConfig->iWindowPositionX);
						ImGui::InputInt("Y Pos", &pConfig->iWindowPositionY);
						ImGui::PopItemWidth();
						ImGui::EndDisabled();

						ImGui::Dummy(ImVec2(10, 10));
						pConfig->HasUnsavedChanges |= ImGui::Checkbox("RememberWindowPos", &pConfig->bRememberWindowPos);
						ImGui::TextWrapped("Remember the last window position. This automatically updates the \"X Pos\" and \"Y Pos\" values above.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Audio)
			{
				if (ImGui::BeginTable("Display", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// Audio volume
					{
						ImGui_ColumnSwitch();

						ImGui::Spacing();
						ImGui::TextWrapped("Adjust the volume of the game");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Cutscene Volume will also affect volume of merchant screen dialogue, as that seems to be handled the same way as cutscenes.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Cutscene Volume").x);
						bool changed = ImGui::SliderInt("Master Volume", &pConfig->iVolumeMaster, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Music Volume", &pConfig->iVolumeBGM, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Effect Volume", &pConfig->iVolumeSE, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Cutscene Volume", &pConfig->iVolumeCutscene, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						ImGui::PopItemWidth();

						if (changed)
						{
							pConfig->HasUnsavedChanges = true;
							AudioTweaks_UpdateVolume();
						}
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Mouse)
			{
				if (ImGui::BeginTable("Mouse", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// UseMouseTurning
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("UseMouseTurning", &pConfig->bUseMouseTurning);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Makes it so the mouse turns the character instead of controlling the camera.");
						ImGui::TextWrapped("\"Modern\" aiming mode in the game's settings is recommended.");

						ImGui::Dummy(ImVec2(10, 20));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Sensitivity Slider").x);
						ImGui::BeginDisabled(!pConfig->bUseMouseTurning);
						ImGui::SliderFloat("Sensitivity Slider", &pConfig->fTurnSensitivity, 0.50f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();
					}

					// UseRawMouseInput
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("UseRawMouseInput", &pConfig->bUseRawMouseInput);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Makes the game use Raw Input for aiming and turning (if MouseTurning is enabled).");
						ImGui::TextWrapped("Greatly improves mouse input by removing negative/positive accelerations that were being applied both by the game and by Direct Input.");
						ImGui::TextWrapped("This option automatically enables the game's \"Modern\" aiming mode, and is incompatible with the \"Classic\" mode.");
					}

					// DetachCameraFromAim
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("DetachCameraFromAim", &pConfig->bDetachCameraFromAim);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("When using the \"Modern\" mouse setting, the game locks the camera position to the aiming position, making both move together.");
						ImGui::TextWrapped("Although this is the expected behavior in most games, some people might prefer to keep the original camera behavior while also having the benefits from \"Modern\" aiming.");
						ImGui::TextWrapped("Enabling this will also restore the horizontal aiming sway that was lost when the devs implemented \"Modern\" aiming.");
					}

					// FixSniperZoom
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixSniperZoom", &pConfig->bFixSniperZoom);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.");
					}

					// FixSniperFocus
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FixSniperFocus", &pConfig->bFixSniperFocus))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("When zooming in and out with the sniper rifle, this option makes the \"focus\" animation look similar to how it looks like with a controller.");
						ImGui::TextWrapped("Requires EnableGCBlur to be enabled.");
					}

					// FixRetryLoadMouseSelector
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixRetryLoadMouseSelector", &pConfig->bFixRetryLoadMouseSelector);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Prevents the game from overriding your selection in the \"Retry / Load\" screen when moving the mouse before confirming an action.");
						ImGui::TextWrapped("This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Keyboard)
			{
				if (ImGui::BeginTable("Keyboard", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// FallbackToEnglishKeyIcons
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FallbackToEnglishKeyIcons", &pConfig->bFallbackToEnglishKeyIcons))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Game will turn keys invisible for certain unsupported keyboard languages, enabling this should make game use English keys for unsupported ones instead (requires game restart)");
					}

					// AllowReloadWithoutAiming_kbm
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowReloadWithoutAiming", &pConfig->bAllowReloadWithoutAiming_kbm))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Removes the need to be aiming the weapon before you can reload it.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10));

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("ReloadWithoutZoom", &pConfig->bReloadWithoutZoom_kbm);

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Don't zoom in when reloading without aiming.");
						ImGui::TextWrapped("Not usually recommended, since the zooming in and out masks some animation quirks when the reload ends.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Controller)
			{
				if (ImGui::BeginTable("Controller", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// OverrideControllerSensitivity
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("OverrideControllerSensitivity", &pConfig->bOverrideControllerSensitivity);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Change the controller sensitivity. For some reason the vanilla game doesn't have an option to change it for controllers, only for the mouse.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Sensitivity Slider").x);
						ImGui::BeginDisabled(!pConfig->bOverrideControllerSensitivity);
						ImGui::SliderFloat("Sensitivity Slider", &pConfig->fControllerSensitivity, 0.50f, 4.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (!pConfig->bOverrideControllerSensitivity)
							pConfig->fControllerSensitivity = 1.0f;
					}

					// RemoveExtraXinputDeadzone
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("RemoveExtraXinputDeadzone", &pConfig->bRemoveExtraXinputDeadzone);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Removes unnecessary deadzones that were added for Xinput controllers.");
					}

					// EnableDeadzoneOverride
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("EnableDeadzoneOverride", &pConfig->bOverrideXinputDeadzone);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Change the Xinput controller deadzone.");
						ImGui::TextWrapped("(Unrelated to the previous option)");
						ImGui::TextWrapped("The game's default is 1, but that seems unnecessarily large, so we default to 0.4 instead.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Deadzone Slider").x);
						ImGui::BeginDisabled(!pConfig->bOverrideXinputDeadzone);
						ImGui::SliderFloat("Deadzone Slider", &pConfig->fXinputDeadzone, 0.0f, 3.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (ImGui::IsItemEdited())
						{
							pConfig->HasUnsavedChanges = true;
							*g_XInputDeadzone_LS = (int)(pConfig->fXinputDeadzone * 7849);
							*g_XInputDeadzone_RS = (int)(pConfig->fXinputDeadzone * 8689);
						}

						if (!pConfig->bOverrideXinputDeadzone)
							pConfig->fXinputDeadzone = 1.0f;
					}

					// AllowReloadWithoutAiming_controller
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("AllowReloadWithoutAiming", &pConfig->bAllowReloadWithoutAiming_controller);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Removes the need to be aiming the weapon before you can reload it.");
						ImGui::TextWrapped("Warning: this will also change the reload button based on your current controller config type, since the original button is used for sprinting when not aiming.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("New reload button (Xinput):");
						ImGui::BulletText("Config Type I: B");
						ImGui::BulletText("Config Type II & III: X");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10));

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("ReloadWithoutZoom", &pConfig->bReloadWithoutZoom_controller);

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Don't zoom in when reloading without aiming.");
						ImGui::TextWrapped("Not usually recommended, since the zooming in and out masks some animation quirks when the reload ends.");

					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Framerate)
			{
				if (ImGui::BeginTable("Framerate", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// FixFallingItemsSpeed
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixFallingItemsSpeed", &pConfig->bFixFallingItemsSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes the speed of falling items in 60 FPS, making them not fall at double speed.");
					}

					// FixTurningSpeed
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixTurningSpeed", &pConfig->bFixTurningSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes speed of backwards turning when using framerates other than 30FPS.");
					}

					// FixQTE
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixQTE", &pConfig->bFixQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs.");
						ImGui::TextWrapped("This fix makes QTEs that involve rapid button presses much more forgiving.");
					}

					// FixAshleyBustPhysics
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixAshleyBustPhysics", &pConfig->bFixAshleyBustPhysics);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes difference between 30/60FPS on physics applied to Ashley.");
					}

					// EnableFastMath
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableFastMath", &pConfig->bEnableFastMath))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Replaces older math functions in the game with much more optimized equivalents.");
						ImGui::TextWrapped("Experimental, can hopefully improve framerate in some areas that had dips.");
					}

					// PrecacheModels
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("PrecacheModels", &pConfig->bPrecacheModels);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Forces game to fully cache all models in the level after loading in.");
						ImGui::TextWrapped("May help with framerate drops when viewing a model for the first time.");
						ImGui::TextWrapped("(not fully tested, could cause issues if level has many models to load!)");
						ImGui::TextWrapped("Changes take effect on next level load.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Misc)
			{
				if (ImGui::BeginTable("Misc", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// OverrideCostumes
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("OverrideCostumes", &pConfig->bOverrideCostumes);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows overriding the costumes, making it possible to combine Normal/Special 1/Special 2 costumes.");
						ImGui::TextWrapped("May cause weird visuals in cutscenes.");

						ImGui::Dummy(ImVec2(10, 10));

						ImGui::BeginDisabled(!pConfig->bOverrideCostumes);
						ImGui::PushItemWidth(150);
						ImGui::Combo("Leon", &iCostumeComboLeon, sLeonCostumeNames, IM_ARRAYSIZE(sLeonCostumeNames));
						if (ImGui::IsItemEdited())
						{
							pConfig->HasUnsavedChanges = true;
							pConfig->CostumeOverride.Leon = (LeonCostumes)iCostumeComboLeon;
						}

						ImGui::Combo("Ashley", &iCostumeComboAshley, sAshleyCostumeNames, IM_ARRAYSIZE(sAshleyCostumeNames));
						if (ImGui::IsItemEdited())
						{
							pConfig->HasUnsavedChanges = true;
							pConfig->CostumeOverride.Ashley = (AshleyCostumes)iCostumeComboAshley;
						}

						ImGui::Combo("Ada", &iCostumeComboAda, sAdaCostumeNames, IM_ARRAYSIZE(sAdaCostumeNames));
						if (ImGui::IsItemEdited())
						{
							pConfig->HasUnsavedChanges = true;

							// ID number 2 seems to be the exact same outfit as ID number 0, for some reason, so we increase the ID here to use the actual next costume
							if (iCostumeComboAda == 2)
								pConfig->CostumeOverride.Ada = (AdaCostumes)(iCostumeComboAda + 1);
							else
								pConfig->CostumeOverride.Ada = (AdaCostumes)iCostumeComboAda;
						}
						ImGui::PopItemWidth();
						ImGui::EndDisabled();
					}

					// AshleyJPCameraAngles
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("AshleyJPCameraAngles", &pConfig->bAshleyJPCameraAngles);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Unlocks the JP-only classic camera angles during Ashley segment.");
					}

					// ViolenceLevelOverride
					{
						ImGui_ColumnSwitch();

						ImGui::Text("ViolenceLevelOverride");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows overriding the level of violence in the game.");
						ImGui::TextWrapped("Use -1 to leave as games default, 0 for low-violence mode (as used in JP/GER version), or 2 for full violence.");

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::PushItemWidth(100);
						if (ImGui::InputInt("Violence Level Override", &pConfig->iViolenceLevelOverride))
						{
							if (pConfig->iViolenceLevelOverride < -1)
								pConfig->iViolenceLevelOverride = -1;

							if (pConfig->iViolenceLevelOverride > 2)
								pConfig->iViolenceLevelOverride = 2;

							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true; // unfortunately required as game only reads pSys from savegame during first loading screen...
						}
						ImGui::PopItemWidth();
					}

					// AllowSellingHandgunSilencer
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowSellingHandgunSilencer", &pConfig->bAllowSellingHandgunSilencer))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows selling the (normally unused) handgun silencer to the merchant.");
					}

					// AllowMafiaLeonCutscenes
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowMafiaLeonCutscenes", &pConfig->bAllowMafiaLeonCutscenes))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows the game to display Leon's mafia outfit (\"Special 2\") on cutscenes.");
					}

					// SilenceArmoredAshley
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("SilenceArmoredAshley", &pConfig->bSilenceArmoredAshley);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Silence Ashley's armored outfit (\"Special 2\").");
						ImGui::TextWrapped("For those who also hate the constant \"Clank Clank Clank\".");
					}

					// AllowAshleySuplex
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("AllowAshleySuplex", &pConfig->bAllowAshleySuplex);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows Ashley to Suplex enemies in very specific situations.");
						ImGui::TextWrapped("(previously was only possible in the initial NTSC GameCube ver., was patched out in all later ports.)");
					}

					// AllowMatildaQuickturn
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("AllowMatildaQuickturn", &pConfig->bAllowMatildaQuickturn);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allows quickturning character to camera direction when wielding Matilda.");
						ImGui::TextWrapped("(only effective if MouseTurning is disabled)");
					}

					// FixDitmanGlitch
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("FixDitmanGlitch", &pConfig->bFixDitmanGlitch);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Fixes the Ditman glitch, which would allow players to increase their walk speed.");
					}

					// UseSprintToggle
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("UseSprintToggle", &pConfig->bUseSprintToggle);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Changes sprint key to act like a toggle instead of needing to be held.");
					}

					// DisableQTE
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("DisableQTE", &pConfig->bDisableQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Disables most of the QTEs, making them pass automatically.");
					}

					// AutomaticMashingQTE
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("AutomaticMashingQTE", &pConfig->bAutomaticMashingQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Unlike the previous option, this only automates the \"mashing\" QTEs, making them pass automatically. Prompts are still shown!");
					}

					// SkipIntroLogos
					{
						ImGui_ColumnSwitch();

						pConfig->HasUnsavedChanges |= ImGui::Checkbox("SkipIntroLogos", &pConfig->bSkipIntroLogos);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Whether to skip the Capcom etc intro logos when starting the game.");
					}

					// EnableDebugMenu
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableDebugMenu", &pConfig->bEnableDebugMenu))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Enables the \"tool menu\" debug menu, present inside the game but unused, and adds a few custom menu entries (\"SAVE GAME\", \"DOF / BLUR MENU\", etc).");
						ImGui::TextWrapped("Can be opened with the LT+LS button combination (or CTRL+F3 on keyboard by default).");
						ImGui::TextWrapped("If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.");
						ImGui::TextWrapped("(may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Memory)
			{
				if (ImGui::BeginTable("Memory", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// AllowHighResolutionSFD
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowHighResolutionSFD", &pConfig->bAllowHighResolutionSFD))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.");
						ImGui::TextWrapped("Not tested beyond 1920x1080.");
					}

					// RaiseVertexAlloc
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("RaiseVertexAlloc", &pConfig->bRaiseVertexAlloc))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allocate more memory for some vertex buffers.");
						ImGui::TextWrapped("This prevents a crash that can happen when playing with a high FOV.");
					}

					// RaiseInventoryAlloc
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("RaiseInventoryAlloc", &pConfig->bRaiseInventoryAlloc))
						{
							pConfig->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.");
					}

					ImGui_ColumnFinish();
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
					ImGui_ColumnInit();

					// cfgMenu 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination to open the re4_tweaks config menu");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(1);
						if (ImGui::Button(StrToUTF8(pConfig->sConfigMenuKeyCombo).c_str(), ImVec2(150, 0)))
						{
							pConfig->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sConfigMenuKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open config menu");
					}

					// Console
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination to open the re4_tweaks debug console (only in certain re4_tweaks builds)");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(2);
						if (ImGui::Button(StrToUTF8(pConfig->sConsoleKeyCombo).c_str(), ImVec2(150, 0)))
						{
							pConfig->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sConsoleKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open console");
					}

					// Inv flip 
					{
						ImGui_ColumnSwitch();

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
							if (ImGui::Button(StrToUTF8(pConfig->sFlipItemUp).c_str(), ImVec2(140, 0)))
							{
								pConfig->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &pConfig->sFlipItemUp, 0, NULL);
							}
							ImGui::PopID();

							ImGui::Dummy(ImVec2(10, 10));

							// DOWN
							ImGui::TextWrapped("Flip DOWN");
							ImGui::PushID(4);
							if (ImGui::Button(StrToUTF8(pConfig->sFlipItemDown).c_str(), ImVec2(140, 0)))
							{
								pConfig->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &pConfig->sFlipItemDown, 0, NULL);
							}
							ImGui::PopID();

							ImGui::TableNextColumn();

							// LEFT
							ImGui::TextWrapped("Flip LEFT");
							ImGui::PushID(5);
							if (ImGui::Button(StrToUTF8(pConfig->sFlipItemLeft).c_str(), ImVec2(140, 0)))
							{
								pConfig->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &pConfig->sFlipItemLeft, 0, NULL);
							}
							ImGui::PopID();

							ImGui::Dummy(ImVec2(10, 10));

							// RIGHT
							ImGui::TextWrapped("Flip RIGHT");
							ImGui::PushID(6);
							if (ImGui::Button(StrToUTF8(pConfig->sFlipItemRight).c_str(), ImVec2(140, 0)))
							{
								pConfig->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &pConfig->sFlipItemRight, 0, NULL);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}
					}

					// QTE 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key bindings for QTE keys when playing with keyboard and mouse.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Unlike the \"official\" way of rebinding keys through usrpInput.ini, this option also changes the on-screen prompt to properly match the selected key.");
						ImGui::BulletText("Key combinations not supported");
						ImGui::Spacing();

						if (ImGui::BeginTable("qtekey", 2))
						{
							ImGui::TableNextColumn();
							ImGui::Spacing();

							// QTE1
							ImGui::TextWrapped("QTE key 1");
							ImGui::PushID(7);
							if (ImGui::Button(StrToUTF8(pConfig->sQTE_key_1).c_str(), ImVec2(140, 0)))
							{
								pConfig->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &pConfig->sQTE_key_1, 0, NULL);
							}
							ImGui::PopID();

							ImGui::TableNextColumn();

							// QTE2
							ImGui::TextWrapped("QTE key 2");
							ImGui::PushID(8);
							if (ImGui::Button(StrToUTF8(pConfig->sQTE_key_2).c_str(), ImVec2(140, 0)))
							{
								pConfig->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &pConfig->sQTE_key_2, 0, NULL);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}
					}

					// Debug menu
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination to make the \"tool menu\" debug menu appear.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("Requires EnableDebugMenu to be enabled.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(9);
						if (ImGui::Button(StrToUTF8(pConfig->sDebugMenuKeyCombo).c_str(), ImVec2(150, 0)))
						{
							pConfig->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sDebugMenuKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open debug menu");
					}

					// MouseTurningModifier
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("MouseTurning camera modifier.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::TextWrapped("When holding this key combination, MouseTurning is temporarily activated/deactivated.");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(10);
						if (ImGui::Button(StrToUTF8(pConfig->sMouseTurnModifierKeyCombo).c_str(), ImVec2(150, 0)))
						{
							pConfig->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sMouseTurnModifierKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("MouseTurning modifier");
					}

					// JetSkiTricks 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination for jump tricks during Jet-ski sequence (normally RT+LT on controller)");
						ImGui::Dummy(ImVec2(10, 10));

						ImGui::PushID(11);
						if (ImGui::Button(StrToUTF8(pConfig->sJetSkiTrickCombo).c_str(), ImVec2(150, 0)))
						{
							pConfig->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sJetSkiTrickCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Trick while riding Jet-ski");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
				ImGui::PopStyleColor();
			}

			if (Tab == MenuTab::Trainer)
			{
				ImColor icn_color = ImColor(230, 15, 95);

				ImVec4 active = ImVec4(150.0f / 255.0f, 10.0f / 255.0f, 40.0f / 255.0f, 255.0f / 255.0f);
				ImVec4 inactive = ImVec4(31.0f / 255.0f, 30.0f / 255.0f, 31.0f / 255.0f, 0.0f / 255.0f);

				// Patches
				ImGui_TrainerTabButton("##patches", "Patches", active, inactive, TrainerTab::Patches, ICON_FA_DESKTOP, icn_color, IM_COL32_WHITE, ImVec2(172, 31));

				// EmManager
				ImGui::SameLine();
				ImGui_TrainerTabButton("##emmgr", "Em Manager", active, inactive, TrainerTab::EmManager, ICON_FA_HEADPHONES, icn_color, IM_COL32_WHITE, ImVec2(172, 31));

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(0.0f, 12.0f));

				if (CurTrainerTab == TrainerTab::Patches)
				{

					if (ImGui::BeginTable("TrainerPatches", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
					{
						ImGui_ColumnInit();

						// Invincibility
						{
							ImGui_ColumnSwitch();
							bool invincibility = FlagIsSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
							if (ImGui::Checkbox("Invincibility", &invincibility))
								FlagSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2), invincibility);

							ImGui::TextWrapped("Prevents taking hits from enemies & automatically skips grabs.");
						}

						// Weak Enemies
						{
							ImGui_ColumnSwitch();

							bool weakEnemies = FlagIsSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK));
							if (ImGui::Checkbox("Weak Enemies", &weakEnemies))
								FlagSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK), weakEnemies);

							ImGui::TextWrapped("Makes most enemies die in 1 hit.");
						}

						// Inf Ammo
						{
							ImGui_ColumnSwitch();

							bool infAmmo = FlagIsSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
							if (ImGui::Checkbox("Infinite Ammo", &infAmmo))
								FlagSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET), infAmmo);

							ImGui::TextWrapped("Prevents game from removing bullets after firing.");
						}

						// Numpad Movement
						{
							ImGui_ColumnSwitch();

							bool DisablePlayerCollision = FlagIsSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
							if (ImGui::Checkbox("Disable Player Collision", &DisablePlayerCollision))
								FlagSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), DisablePlayerCollision);

							ImGui::TextWrapped("Disables collision (no-clip).");

							ImGui::Spacing();

							ImGui::BeginDisabled(!DisablePlayerCollision);
							ImGui::Checkbox("Numpad Movement", &bUseNumpadMovement);
							ImGui::TextWrapped("Allows noclip movement via numpad.");

							ImGui::Spacing();

							ImGui::Checkbox("Use Mouse Wheel", &bUseMouseWheelUPDOWN);
							ImGui::TextWrapped("Allows using the mouse wheel to go up and down.");
							ImGui::EndDisabled();
						}

						ImGui_ColumnFinish();
						ImGui::EndTable();
					}
				}

				if (CurTrainerTab == TrainerTab::EmManager)
				{
					static int emIdx = -1;
					if (ImGui::BeginTable("Trainer", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
					{
						ImGui_ColumnInit();

						static bool onlyShowValidEms = true;
						static bool onlyShowESLSpawned = false;
#ifdef _DEBUG
						static bool showEmPointers = true;
#else
						static bool showEmPointers = false;
#endif

						auto& emMgr = *EmMgrPtr();
						// cEm list
						{
							ImGui_ColumnSwitch();

							ImGui::Text("Em Count: %d | Max: %d", emMgr.count_valid(), emMgr.count());
							ImGui::SameLine();
							ImGui::Checkbox("Show addresses", &showEmPointers);

							if (ImGui::BeginListBox("", ImVec2(320, 420)))
							{
								int i = 0;
								for (auto& em : emMgr)
								{
									if (!onlyShowValidEms || em.IsValid())
									{
										if (!onlyShowESLSpawned || em.IsSpawnedByESL())
										{
											char tmpBuf[256];
											if (showEmPointers)
												sprintf(tmpBuf, "#%d:0x%x c%s (type %x) flags %x", i, (uint32_t)&em, cEmMgr::EmIdToName(em.ID_100).c_str(), int(em.type_101), int(em.be_flag_4));
											else
												sprintf(tmpBuf, "#%d c%s (type %x) flags %x", i, cEmMgr::EmIdToName(em.ID_100).c_str(), int(em.type_101), int(em.be_flag_4));
											const bool is_selected = (emIdx == i);
											if (ImGui::Selectable(tmpBuf, is_selected))
												emIdx = i;
											// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
											if (is_selected)
												ImGui::SetItemDefaultFocus();
										}
									}

									i++;
								}
								ImGui::EndListBox();
							}

							ImGui::Checkbox("Active", &onlyShowValidEms); ImGui::SameLine();
							ImGui::Checkbox("ESL-spawned", &onlyShowESLSpawned);
						}

						// cEm info
						{
							ImGui_ColumnSwitch();
							if (emIdx >= 0)
							{
								cEm* em = emMgr[emIdx];
								{
									if (showEmPointers)
										ImGui::Text("#%d:0x%x c%s (type %x)", emIdx, em, cEmMgr::EmIdToName(em->ID_100).c_str(), int(em->type_101));
									else
										ImGui::Text("#%d c%s (type %x)", emIdx, cEmMgr::EmIdToName(em->ID_100).c_str(), int(em->type_101));

									static Vec copyPosition = { 0 };
									if (ImGui::Button("Copy position"))
										copyPosition = em->position_94;

									ImGui::SameLine();

									static uint16_t AtariFlagBackup = 0;
									static bool AtariFlagBackupSet = false;
									if (AtariFlagBackupSet)
									{
										em->atariInfo_2B4.flags_1A = AtariFlagBackup;
										AtariFlagBackupSet = false;
									}
									if (ImGui::Button("Paste position"))
									{
										em->position_94 = copyPosition;
										em->oldPos_110 = copyPosition;

										// temporarily disable atariInfo collision, prevents colliding with map
										// TODO: some reason using em->move() isn't enough to bypass collision for some Em's
										// as workaround we let game render another frame and then restore flags above
										AtariFlagBackup = em->atariInfo_2B4.flags_1A;
										AtariFlagBackupSet = true;
										em->atariInfo_2B4.flags_1A = 0;

										em->matUpdate();
										em->move();
									}

									cPlayer* player = PlayerPtr();
									if (em != player)
									{
										if (ImGui::Button("Teleport player"))
										{
											player->position_94 = em->position_94;
											player->oldPos_110 = em->position_94;

											// temporarily disable atariInfo collision, prevents colliding with map
											uint16_t flagBackup = player->atariInfo_2B4.flags_1A;
											player->atariInfo_2B4.flags_1A = 0;

											player->matUpdate();
											player->move();

											player->atariInfo_2B4.flags_1A = flagBackup;
										}
										ImGui::SameLine();

										if (ImGui::Button("Move to player"))
										{
											em->position_94 = player->position_94;
											em->oldPos_110 = player->position_94;

											// temporarily disable atariInfo collision, prevents colliding with map
											AtariFlagBackup = em->atariInfo_2B4.flags_1A;
											AtariFlagBackupSet = true;
											em->atariInfo_2B4.flags_1A = 0;

											em->matUpdate();
											em->move();
										}
									}

									if (ImGui::BeginListBox("Flags"))
									{
										bool collisionEnabled = (em->atariInfo_2B4.flags_1A & 0x100) == 0x100;
										if (ImGui::Checkbox("MapCollision", &collisionEnabled))
										{
											if (collisionEnabled)
												em->atariInfo_2B4.flags_1A |= 0x100;
											else
												em->atariInfo_2B4.flags_1A &= ~0x100;
										}

										for (int i = 0; i < 32; i++)
										{
											bool bitSet = (em->be_flag_4 & (1 << i)) != 0;

											std::string lbl = cUnit::GetBeFlagName(i);
											if (ImGui::Checkbox(lbl.c_str(), &bitSet))
											{
												if (bitSet)
													em->be_flag_4 = em->be_flag_4 | (1 << i);
												else
													em->be_flag_4 = em->be_flag_4 & ~(1 << i);
											}
										}
										ImGui::EndListBox();
									}

									if (ImGui::InputFloat3("Position", (float*)&em->position_94, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
										em->matUpdate();

									if (ImGui::InputFloat3("Rotation", (float*)&em->rotation_A0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
										em->matUpdate();

									if (ImGui::InputFloat3("Scale", (float*)&em->scale_AC, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
									{
										if (IsGanado(em->ID_100))
										{
											// cEm10 holds a seperate scale value that it seems to grow/shrink the actual scale towards each frame
											// make sure we update that as well
											Vec* scale_bk_498 = (Vec*)((uint8_t*)em + 0x498);
											*scale_bk_498 = em->scale_AC;
										}
										em->matUpdate();
									}

									ImGui::InputFloat("MotInfo.Speed", &em->MotInfo_1D8.speed_C0, 0.1f);

									int hpCur = em->HpCur_324;
									if (ImGui::InputInt("HpCur", &hpCur, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
										em->HpCur_324 = hpCur;

									int hpMax = em->HpMax_326;
									if (ImGui::InputInt("HpMax", &hpMax, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
										em->HpMax_326 = hpMax;

									ImGui::Text("Routine: %02X %02X %02X %02X", em->r_no_0_FC, em->r_no_1_FD, em->r_no_2_FE, em->r_no_3_FF);
									ImGui::Text("Parts count: %d", em->PartCount());
									if (em->emListIndex_3A0 != 255)
										ImGui::Text("ESL: %s @ #%d (offset 0x%x)", getEmListName(GlobalPtr()->curEmListNumber_4FB3), int(em->emListIndex_3A0), int(em->emListIndex_3A0) * sizeof(EM_LIST));

									ImGui::Dummy(ImVec2(10, 25));

									// works, but unsure what to display atm
									/*
									static int partIdx = -1;
									if (ImGui::BeginListBox("Test"))
									{
										int i = 0;
										cParts* part = em->childParts_F4;
										while (part != nullptr)
										{
											sprintf(tmpBuf, "#%d", i);
											const bool is_selected = (partIdx == i);
											if (ImGui::Selectable(tmpBuf, is_selected))
												partIdx = i;
											// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
											if (is_selected)
												ImGui::SetItemDefaultFocus();

											i++;
											part = part->nextParts_F4;
										}
										ImGui::EndListBox();
									}*/

									ImGui_ItemSeparator();

									ImGui::Dummy(ImVec2(10, 25));

									ImGui::Text("Modification:");
									static float addPosition[3] = { 0 };
									ImGui::InputFloat3("PositionChange", addPosition, "%.3f");
									if (ImGui::Button("Apply"))
									{
										em->position_94.x += addPosition[0];
										em->position_94.y += addPosition[1];
										em->position_94.z += addPosition[2];
										em->matUpdate();
									}
								}
							}
						}


						ImGui_ColumnFinish();
						ImGui::EndTable();
					}
				}
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

		std::string tooltip = StrToUTF8(std::string("re4_tweaks: Press ") + pConfig->sConfigMenuKeyCombo + std::string(" to open the configuration menu"));

		ImGui::TextUnformatted(tooltip.data());

		ImGui::End();
	}
}