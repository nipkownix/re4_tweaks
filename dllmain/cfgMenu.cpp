#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Settings.h"
#include "imgui.h"
#include "imgui\misc\cpp\imgui_stdlib.h"
#include "input.hpp"
#include "ConsoleWnd.h"
#include <FAhashes.h>
#include "Utils.h"
#include "UI_DebugWindows.h"
#include "UI_Utility.h"
#include "Trainer.h"
#include "AudioTweaks.h"
#include "../dxvk/src/config.h"

bool bCfgMenuOpen = false;
bool NeedsToRestart = false;
bool bWaitingForHotkey = false;
bool bPauseGameWhileInCfgMenu = true;

int iGCBlurMode = 0;
int iCostumeComboLeon;
int iCostumeComboAshley;
int iCostumeComboAda;

float fLaserColorPicker[3]{ 0.0f, 0.0f, 0.0f };

ImVec2 PrevRightColumnPos;
ImVec2 PrevLeftColumnPos;

int MenuTipTimer = 500; // cfgMenu tooltip timer

std::string cfgMenuTitle = "re4_tweaks";

std::vector<uint32_t> cfgMenuCombo;

void PauseGame(bool setPaused)
{
	static bool stopFlagsBackedUp = false;
	static int stopFlagsBackup = -1;

	// Disable pausing when not needed or when pausing would casue issues
	bool disable =
		!PlayerPtr() ||
		SubScreenWk->open_flag_2C != 0 ||
		OptionOpenFlag() ||
		GlobalPtr()->playerHpCur_4FB4 == 0 ||
		GlobalPtr()->subHpCur_4FB8 == 0;

	if (disable)
		return;

	if (setPaused && !stopFlagsBackedUp)
	{
		stopFlagsBackup = GlobalPtr()->flags_STOP_0_170[0];
		stopFlagsBackedUp = true;
		GlobalPtr()->flags_STOP_0_170[0] = 0xBFFFFF3F; // Same flags the game sets during its pause screen minus SPF_ID_SYSTEM 
	}
	else if (!setPaused && stopFlagsBackedUp)
	{
		GlobalPtr()->flags_STOP_0_170[0] = stopFlagsBackup;
		stopFlagsBackedUp = false;
	}
}

bool ParseConfigMenuKeyCombo(std::string_view in_combo)
{
	if (in_combo.empty())
		return false;

	cfgMenuCombo.clear();
	cfgMenuCombo = re4t::cfg->ParseKeyCombo(in_combo);

	pInput->register_hotkey({ []() {
		bCfgMenuOpen = !bCfgMenuOpen;

		// Pause game while cfgMenu is open
		PauseGame(bCfgMenuOpen && bPauseGameWhileInCfgMenu);
	}, &cfgMenuCombo });

	return cfgMenuCombo.size() > 0;
}

void cfgMenuRender()
{
	// Min/Max window sizes
	const float min_x = 940.0f * esHook._cur_monitor_dpi;
	const float min_y = 640.0f * esHook._cur_monitor_dpi;

	const float max_x = 1920.0f * esHook._cur_monitor_dpi;
	const float max_y = 1080.0f * esHook._cur_monitor_dpi;

	ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

	ImGui::Begin("cfgMenu", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar);
	{
		auto pos = ImGui::GetWindowPos();
		auto draw = ImGui::GetWindowDrawList();

		float leftside_size_x = 200; // Base
		leftside_size_x *= esHook._cur_monitor_dpi;

		float topright_size_y = 71; // Base
		topright_size_y *= re4t::cfg->fFontSizeScale;
		topright_size_y *= esHook._cur_monitor_dpi;

		// Left side BG
		draw->AddRectFilled(ImVec2(pos.x + 0, pos.y + 0), ImVec2(pos.x + leftside_size_x + 1, pos.y + ImGui::GetWindowHeight()), ImColor(8, 8, 8, 230), 10.f, 5);

		// Right side BG
		draw->AddRectFilled(ImVec2(pos.x + leftside_size_x, pos.y + 0), ImVec2(pos.x + ImGui::GetWindowWidth(), pos.y + ImGui::GetWindowHeight()), ImColor(8, 8, 8, 250), 8.f, 10);

		// Top right separator
		draw->AddLine(ImVec2(pos.x + leftside_size_x + 10, round(pos.y + topright_size_y)), ImVec2(pos.x + ImGui::GetItemRectSize().x - 15, round(pos.y + topright_size_y)), ImColor(150, 10, 40));

		// Setup tab buttons
		{
			ImGui::BeginChild("left side", ImVec2(leftside_size_x, 0));

			ImColor icn_color = ImColor(230, 15, 95);

			ImVec4 active = ImVec4(150.0f / 255.0f, 10.0f / 255.0f, 40.0f / 255.0f, 255.0f / 255.0f);
			ImVec4 inactive = ImVec4(31.0f / 255.0f, 30.0f / 255.0f, 31.0f / 255.0f, 0.0f / 255.0f);

			ImVec2 btn_size = ImVec2(leftside_size_x - 28, 31 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);

			// cfgMenu title
			ImGui::SetCursorPos(ImVec2(12, 25));

			ImGui::Text(cfgMenuTitle.data());

			ImGui::Dummy(ImVec2(30, 30 * esHook._cur_monitor_dpi));

			// Display
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##display", "Display", active, inactive, MenuTab::Display, ICON_FA_DESKTOP, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Audio
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##audio", "Audio", active, inactive, MenuTab::Audio, ICON_FA_HEADPHONES, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Mouse
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##mouse", "Mouse", active, inactive, MenuTab::Mouse, ICON_FA_LOCATION_ARROW, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Keyboard
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##keyboard", "Keyboard", active, inactive, MenuTab::Keyboard, ICON_FA_KEYBOARD, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Controller
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##controller", "Controller", active, inactive, MenuTab::Controller, ICON_FA_JOYSTICK, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Frame rate
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##framerate", "Frame rate", active, inactive, MenuTab::Framerate, ICON_FA_CHESS_CLOCK, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Misc
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##misc", "Misc", active, inactive, MenuTab::Misc, ICON_FA_COG, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Memory
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##memory", "Memory", active, inactive, MenuTab::Memory, ICON_FA_CALCULATOR, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			// Hotkeys
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			ImGui_TabButton("##hotkeys", "Hotkeys", active, inactive, MenuTab::Hotkeys, ICON_FA_LAMBDA, icn_color, IM_COL32_WHITE, btn_size);
			ImGui::Spacing();

			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			if (!re4t::cfg->bTrainerEnable)
			{
				if (ImGui_TabButton("##trainer", "Trainer", active, inactive, MenuTab::Trainer, ICON_FA_SHOE_PRINTS, ImColor(255, 255, 255, 60), ImColor(255, 255, 255, 60), btn_size))
					ImGui::OpenPopup("Trainer menu");

				// Always center this window when appearing
				ImVec2 center = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopupModal("Trainer menu", NULL, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::Text("Warning: this section contains cheats & patches that may ruin your game experience,\nor break things in the game.\n\nAre you sure you want to use the trainer menu?\n\n");
					ImGui::Separator();

					ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 265, ImGui::GetCursorPos().y));

					if (ImGui::Button("Yes", ImVec2(120, 0)))
					{
						re4t::cfg->bTrainerEnable = true;
						re4t::cfg->WriteSettings(true);
						ImGui::CloseCurrentPopup();
					}

					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();

					if (ImGui::Button("Cancel", ImVec2(120, 0)))
					{
						Tab = MenuTab::Display;
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

			}
			else
				ImGui_TabButton("##trainer", "Trainer", active, inactive, MenuTab::Trainer, ICON_FA_SHOE_PRINTS, icn_color, IM_COL32_WHITE, btn_size);

			ImGui::Dummy(ImVec2(0.0f, 12.0f * esHook._cur_monitor_dpi));

			// Save/Load
			float sl_btn_size_x = (leftside_size_x / 2) - 18;
			float sl_btn_size_y = 35.0f;
			sl_btn_size_y *= re4t::cfg->fFontSizeScale;
			sl_btn_size_y *= esHook._cur_monitor_dpi;

			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - 10 - sl_btn_size_x, ImGui::GetContentRegionAvail().y - 10 - sl_btn_size_y));

			// Separator
			draw->AddLine(ImVec2(ImGui::GetCursorScreenPos().x + 10, ImGui::GetCursorScreenPos().y - 10), ImVec2(ImGui::GetCursorScreenPos().x + leftside_size_x - 20, ImGui::GetCursorScreenPos().y - 10), ImColor(150, 10, 40));

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.51f, 0.00f, 0.14f, 0.00f));
			ImGui::Dummy(ImVec2(0, 13 * esHook._cur_monitor_dpi)); ImGui::SameLine();
			if (ImGui::Button(ICON_FA_ERASER" Discard", ImVec2(sl_btn_size_x, sl_btn_size_y)))
			{
				float oldSize = re4t::cfg->fFontSizeScale;

				re4t::cfg->ReadSettings();

				if (oldSize != re4t::cfg->fFontSizeScale)
					bRebuildFont = true;
			}

			ImGui::SameLine();

			if (ImGui::Button(ICON_FA_CODE" Save", ImVec2(sl_btn_size_x, sl_btn_size_y)))
			{
				// Parse key combos on save
				re4t::cfg->ParseHotkeys();

				// Update console title
				con.TitleKeyCombo = re4t::cfg->sConsoleKeyCombo;

				re4t::cfg->WriteSettings(false);
			}

			ImGui::PopStyleColor();

			ImGui::EndChild();
		}

		ImGui::SameLine();

		// Setup top right
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.51f, 0.00f, 0.14f, 0.00f));

			ImGui::BeginChild("top right", ImVec2(0, topright_size_y));

			// Config menu font size
			if (ImGui::Button("-"))
			{
				if (re4t::cfg->fFontSizeScale > 1.0f)
				{
					re4t::cfg->fFontSizeScale -= 0.05f;

					bRebuildFont = true;
					re4t::cfg->HasUnsavedChanges = true;
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("+"))
			{
				if (re4t::cfg->fFontSizeScale < 1.20f)
				{
					re4t::cfg->fFontSizeScale += 0.05f;
					bRebuildFont = true;
					re4t::cfg->HasUnsavedChanges = true;
				}
			}

			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Font Size");

			

			// Tips
			float tip_offset = 45.0f;
			tip_offset *= re4t::cfg->fFontSizeScale;
			tip_offset *= esHook._cur_monitor_dpi;

			if (re4t::cfg->HasUnsavedChanges)
			{
				ImGui::SameLine();

				const char* txt = "You have unsaved changes!";

				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(txt).x - tip_offset, ImGui::GetContentRegionAvail().y / 3.0f));
				ImGui::SameLine();
				ImGui::BulletText(txt);
			}

			if (ImGui::Checkbox("Pause game", &bPauseGameWhileInCfgMenu))
			{
				PauseGame(bCfgMenuOpen&& bPauseGameWhileInCfgMenu);
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Keep the game frozen while the config menu is open");

			if (NeedsToRestart)
			{
				ImGui::SameLine();

				const char* txt = "Changes that have been made require the game to be restarted!";

				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(txt).x - tip_offset, ImGui::GetContentRegionAvail().y));
				ImGui::SameLine();
				ImGui::BulletText(txt);
			}

			ImGui::EndChild();
		}

		// Setup item view
		{
			ImGui::SetCursorPos(ImVec2(leftside_size_x + 16, topright_size_y + 10));

			ImGui::BeginChild("right side", ImVec2(0, 0));

			ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(16.f, 16.f));

			int columnCount = 1 + (int)((ImGui::GetWindowWidth() / (716.0f * esHook._cur_monitor_dpi)) * 1.5f);

			ImColor itmbgColor = ImColor(25, 20, 20, 166);

			if (Tab == MenuTab::Display)
			{
				if (ImGui::BeginTable("Display", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// Vulkan
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("UseVulkanRenderer", &re4t::dxvk::cfg->bUseVulkanRenderer))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Enables the use of the DXVK-based vulkan renderer, which provides better performance on newer hardware.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::BeginDisabled(!re4t::dxvk::cfg->bUseVulkanRenderer);

						if (ImGui::Checkbox("ShowFPS", &re4t::dxvk::cfg->bShowFPS))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui::TextWrapped("Shows a frame rate counter on the top left of the screen.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						if (ImGui::Checkbox("ShowGPULoad", &re4t::dxvk::cfg->bShowGPULoad))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui::TextWrapped("Shows a estimated GPU load. May be inaccurate.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						if (ImGui::Checkbox("ShowDeviceInfo", &re4t::dxvk::cfg->bShowDeviceInfo))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui::TextWrapped("Shows the name of the GPU and the driver version.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						if (ImGui::Checkbox("DisableAsync", &re4t::dxvk::cfg->bDisableAsync))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui::TextWrapped("Disables asynchronous shader compilation. Not recommended.");

						ImGui::EndDisabled();
					}

					// FOVAdditional
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FOVAdditional", &re4t::cfg->bEnableFOV))
						{
							re4t::cfg->HasUnsavedChanges = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Additional FOV value.");
						ImGui::TextWrapped("20 seems good for most cases.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("FOV Slider").x);
						ImGui::BeginDisabled(!re4t::cfg->bEnableFOV);
						ImGui::SliderFloat("FOV Slider", &re4t::cfg->fFOVAdditional, 0.0f, 50.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (!re4t::cfg->bEnableFOV)
							re4t::cfg->fFOVAdditional = 0.0f;
					}

					// DisableVsync
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("DisableVsync", &re4t::cfg->bDisableVsync))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Force V-Sync to be disabled. For some reason the vanilla game doesn't provide a functional way to do this.");
					}

					// Aspect ratio tweaks
					{
						ImGui_ColumnSwitch();

						ImGui::Spacing();
						ImGui::TextWrapped("Aspect ratio tweaks");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("UltraWideAspectSupport", &re4t::cfg->bUltraWideAspectSupport);
						ImGui::TextWrapped("Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen.");
						ImGui::TextWrapped("(Change the resolution for this setting to take effect)");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::BeginDisabled(!re4t::cfg->bUltraWideAspectSupport);
						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("SideAlignHUD", &re4t::cfg->bSideAlignHUD);
						ImGui::TextWrapped("Moves the HUD to the right side of the screen.");

						ImGui::Spacing();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("StretchFullscreenImages", &re4t::cfg->bStretchFullscreenImages);
						ImGui::TextWrapped("Streches some images to fit the screen, such as the images shown when reading \"Files\".");

						ImGui::Spacing();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("StretchVideos", &re4t::cfg->bStretchVideos);
						ImGui::TextWrapped("Streches pre-rendered videos to fit the screen.");
						ImGui::EndDisabled();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("Remove16by10BlackBars", &re4t::cfg->bRemove16by10BlackBars);
						ImGui::TextWrapped("Removes top and bottom black bars that are present when playing in 16:10. Will crop a few pixels from each side of the screen.");
						ImGui::TextWrapped("(Change the resolution for this setting to take effect)");
					}

					// FixDPIScale
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FixDPIScale", &re4t::cfg->bFixDPIScale))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Forces game to run at normal 100%% DPI scaling, fixes resolution issues for players that have above 100%% DPI scaling set.");
					}

					// FixDisplayMode
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FixDisplayMode", &re4t::cfg->bFixDisplayMode))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows game to use non - 60Hz refresh rates in fullscreen, fixing the black screen issue people have when starting the game.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::Text("CustomRefreshRate");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(100 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
						ImGui::BeginDisabled(!re4t::cfg->bFixDisplayMode);
						ImGui::InputInt("Hz", &re4t::cfg->iCustomRefreshRate);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (ImGui::IsItemEdited())
						{
							re4t::cfg->HasUnsavedChanges = true;
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

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("OverrideLaserColor", &re4t::cfg->bOverrideLaserColor);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Overrides the color of the laser sights.");
						ImGui::TextWrapped("Note: The game's vanilla config.ini contains a setting to change the");
						ImGui::TextWrapped("alpha/opacity of the laser, but since it doesn't work, we don't include it here.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::BeginDisabled(!re4t::cfg->bOverrideLaserColor || re4t::cfg->bRainbowLaser);
						ImGui::ColorEdit4("Laser color picker", fLaserColorPicker, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);
						if (ImGui::IsItemEdited())
						{
							re4t::cfg->iLaserR = (int)(fLaserColorPicker[0] * 255.0f);
							re4t::cfg->iLaserG = (int)(fLaserColorPicker[1] * 255.0f);
							re4t::cfg->iLaserB = (int)(fLaserColorPicker[2] * 255.0f);
						}
						ImGui::EndDisabled();

						ImGui::Spacing();

						ImGui::BeginDisabled(!re4t::cfg->bOverrideLaserColor);
						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RainbowLaser", &re4t::cfg->bRainbowLaser);
						ImGui::EndDisabled();

						if (!re4t::cfg->bOverrideLaserColor)
							re4t::cfg->bRainbowLaser = false;
					}

					// RestorePickupTransparency
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RestorePickupTransparency", &re4t::cfg->bRestorePickupTransparency);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Restores transparency on the item pickup screeen.");
					}

					// DisableBrokenFilter03
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("DisableBrokenFilter03", &re4t::cfg->bDisableBrokenFilter03);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.");
						ImGui::TextWrapped("(if you're using the HD Project, you should disable this option)");
					}

					// FixBlurryImage
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixBlurryImage", &re4t::cfg->bFixBlurryImage);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes a problem related to a vertex buffer that caused the image to be slightly blurred, making the image much sharper and clearer.");
					}

					// DisableFilmGrain
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("DisableFilmGrain", &re4t::cfg->bDisableFilmGrain);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Disables the film grain overlay that is present in most sections of the game.");
					}

					// FixWaterScaling
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("ImproveWater", &re4t::cfg->bImproveWater);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10));
						ImGui::TextWrapped("Improves some of the water effects, fixing the \"jelly-like\" water that is present in some areas.");
						ImGui::TextWrapped("(if you're using the HD Project, you should disable this option)");
					}

					// EnableGCBlur
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableGCBlur", &re4t::cfg->bEnableGCBlur))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						if (ImGui::RadioButton("Enhanced", &iGCBlurMode, 0))
							re4t::cfg->bUseEnhancedGCBlur = true;
						ImGui::Indent(29.0);
						ImGui::TextWrapped("Slightly improved implementation to look better on modern high-definition displays.");
						ImGui::Unindent(29.0);

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						if (ImGui::RadioButton("Classic", &iGCBlurMode, 1))
							re4t::cfg->bUseEnhancedGCBlur = false;
						ImGui::Indent(29.0);
						ImGui::TextWrapped("The original GC implementation.");
						ImGui::Unindent(29.0);

					}

					// EnableGCScopeBlur
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableGCScopeBlur", &re4t::cfg->bEnableGCScopeBlur))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.");
					}

					// WindowBorderless
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("WindowBorderless", &re4t::cfg->bWindowBorderless))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Whether to use a borderless-window when using windowed-mode.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Position to draw the game window when using windowed mode.");
						ImGui::TextWrapped("-1 will use the games default (usually places it at 0,0)");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::BeginDisabled(re4t::cfg->bRememberWindowPos);
						ImGui::PushItemWidth(150 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
						ImGui::InputInt("X Pos", &re4t::cfg->iWindowPositionX);
						ImGui::InputInt("Y Pos", &re4t::cfg->iWindowPositionY);
						ImGui::PopItemWidth();
						ImGui::EndDisabled();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RememberWindowPos", &re4t::cfg->bRememberWindowPos);
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

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Cutscene Volume will also affect volume of merchant screen dialogue, as that seems to be handled the same way as cutscenes.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Cutscene Volume").x);
						bool changed = ImGui::SliderInt("Master Volume", &re4t::cfg->iVolumeMaster, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Music Volume", &re4t::cfg->iVolumeBGM, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Effect Volume", &re4t::cfg->iVolumeSE, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						changed |= ImGui::SliderInt("Cutscene Volume", &re4t::cfg->iVolumeCutscene, 0, 100, "%d", ImGuiSliderFlags_AlwaysClamp);
						ImGui::PopItemWidth();

						if (changed)
						{
							re4t::cfg->HasUnsavedChanges = true;
							re4t::AudioTweaks::UpdateVolume();
						}
					}

					// GC sound effects
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RestoreGCSoundEffects", &re4t::cfg->bRestoreGCSoundEffects);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Restores certain sound effects that were changed from the original GC release.");
						ImGui::TextWrapped("(currently only changes sound of the knife)");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}
						
			if (Tab == MenuTab::Mouse)
			{
				if (ImGui::BeginTable("Mouse", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// CameraImprovements
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("CameraImprovements", &re4t::cfg->bCameraImprovements);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Provides a small set of camera tweaks:");

						ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Aim weapons in the direction the camera is pointing to");
						ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Stop camera from being randomly reset");
						ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Allow full 360 degree movement");
						ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Smoother and more responsive movement");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::BeginDisabled(!re4t::cfg->bCameraImprovements);
						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("ResetCameraAfterUsingWeapons", &re4t::cfg->bResetCameraAfterUsingWeapons);
						ImGui::TextWrapped("Center the camera after using weapons.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("ResetCameraAfterUsingKnife", &re4t::cfg->bResetCameraAfterUsingKnife);
						ImGui::TextWrapped("Center the camera after using the knife.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("ResetCameraWhenRunning", &re4t::cfg->bResetCameraWhenRunning);
						ImGui::TextWrapped("Center the camera when the run key is pressed.");
						ImGui::TextWrapped("Only used if MouseTurning isn't enabled.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Camera sensitivity").x);
						ImGui::SliderFloat("Camera sensitivity", &re4t::cfg->fCameraSensitivity, 0.50f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();
					}

					// UseMouseTurning
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("UseMouseTurning", &re4t::cfg->bUseMouseTurning);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Provides alternative ways to turn the character using the mouse.");
						ImGui::TextWrapped("\"Modern\" aiming mode in the game's settings is recommended.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::RadioButton("Type A", &re4t::cfg->iMouseTurnType, MouseTurnTypes::TypeA);
						ImGui::Indent(29.0);
						ImGui::TextWrapped("The character's rotation is influenced by the camera's position, similar to Resident Evil 6.");
						ImGui::TextWrapped("Using CameraImprovements is recommended.");
						ImGui::Unindent(29.0);

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::RadioButton("Type B", &re4t::cfg->iMouseTurnType, MouseTurnTypes::TypeB);
						ImGui::Indent(29.0);
						ImGui::TextWrapped("The character's rotation is directly changed by the mouse, similar to Resident Evil 5.");
						ImGui::Unindent(29.0);

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Type B sensitivity").x - 60);
						ImGui::BeginDisabled(!(re4t::cfg->bUseMouseTurning && (re4t::cfg->iMouseTurnType == MouseTurnTypes::TypeB)));
						ImGui::Indent(29.0);
						ImGui::SliderFloat("Type B sensitivity", &re4t::cfg->fTurnTypeBSensitivity, 0.50f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();
						ImGui::Unindent(29.0);
					}

					// UseRawMouseInput
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("UseRawMouseInput", &re4t::cfg->bUseRawMouseInput);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Makes the game use Raw Input for aiming and turning (if MouseTurning is enabled).");
						ImGui::TextWrapped("Greatly improves mouse input by removing negative/positive accelerations that were being applied both by the game and by Direct Input.");
						ImGui::TextWrapped("This option automatically enables the game's \"Modern\" aiming mode, and is incompatible with the \"Classic\" mode.");
					}

					// DetachCameraFromAim
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("DetachCameraFromAim", &re4t::cfg->bDetachCameraFromAim);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("When using the \"Modern\" mouse setting, the game locks the camera position to the aiming position, making both move together.");
						ImGui::TextWrapped("Although this is the expected behavior in most games, some people might prefer to keep the original camera behavior while also having the benefits from \"Modern\" aiming.");
						ImGui::TextWrapped("Enabling this will also restore the horizontal aiming sway that was lost when the devs implemented \"Modern\" aiming.");
					}

					// FixSniperZoom
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixSniperZoom", &re4t::cfg->bFixSniperZoom);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.");
					}

					// FixSniperFocus
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FixSniperFocus", &re4t::cfg->bFixSniperFocus))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("When zooming in and out with the sniper rifle, this option makes the \"focus\" animation look similar to how it looks like with a controller.");
						ImGui::TextWrapped("Requires EnableGCBlur to be enabled.");
					}

					// FixRetryLoadMouseSelector
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixRetryLoadMouseSelector", &re4t::cfg->bFixRetryLoadMouseSelector);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Prevents the game from overriding your selection in the \"Retry / Load\" screen when moving the mouse before confirming an action.");
						ImGui::TextWrapped("This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Keyboard)
			{
				if (ImGui::BeginTable("Keyboard", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// FallbackToEnglishKeyIcons
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("FallbackToEnglishKeyIcons", &re4t::cfg->bFallbackToEnglishKeyIcons))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Game will turn keys invisible for certain unsupported keyboard languages, enabling this should make game use English keys for unsupported ones instead (requires game restart)");
					}

					// AllowReloadWithoutAiming_kbm
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowReloadWithoutAiming", &re4t::cfg->bAllowReloadWithoutAiming_kbm))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Removes the need to be aiming the weapon before you can reload it.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("ReloadWithoutZoom", &re4t::cfg->bReloadWithoutZoom_kbm);

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Don't zoom in when reloading without aiming.");
						ImGui::TextWrapped("Not usually recomended, since the zooming in and out masks some animation quirks when the reload ends.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Controller)
			{
				if (ImGui::BeginTable("Controller", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// OverrideControllerSensitivity
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("OverrideControllerSensitivity", &re4t::cfg->bOverrideControllerSensitivity);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Change the controller sensitivity. For some reason the vanilla game doesn't have an option to change it for controllers, only for the mouse.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Sensitivity Slider").x);
						ImGui::BeginDisabled(!re4t::cfg->bOverrideControllerSensitivity);
						ImGui::SliderFloat("Sensitivity Slider", &re4t::cfg->fControllerSensitivity, 0.50f, 4.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (!re4t::cfg->bOverrideControllerSensitivity)
							re4t::cfg->fControllerSensitivity = 1.0f;
					}

					// RemoveExtraXinputDeadzone
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RemoveExtraXinputDeadzone", &re4t::cfg->bRemoveExtraXinputDeadzone);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Removes unnecessary deadzones that were added for Xinput controllers.");
					}

					// EnableDeadzoneOverride
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("EnableDeadzoneOverride", &re4t::cfg->bOverrideXinputDeadzone);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Change the Xinput controller deadzone.");
						ImGui::TextWrapped("(Unrelated to the previous option)");
						ImGui::TextWrapped("The game's default is 1, but that seems unnecessarily large, so we default to 0.4 instead.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Deadzone Slider").x);
						ImGui::BeginDisabled(!re4t::cfg->bOverrideXinputDeadzone);
						ImGui::SliderFloat("Deadzone Slider", &re4t::cfg->fXinputDeadzone, 0.0f, 3.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
						ImGui::EndDisabled();
						ImGui::PopItemWidth();

						if (ImGui::IsItemEdited())
							re4t::cfg->HasUnsavedChanges = true;

						if (!re4t::cfg->bOverrideXinputDeadzone)
							re4t::cfg->fXinputDeadzone = 1.0f;
					}

					// AllowReloadWithoutAiming_controller
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("AllowReloadWithoutAiming", &re4t::cfg->bAllowReloadWithoutAiming_controller);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Removes the need to be aiming the weapon before you can reload it.");
						ImGui::TextWrapped("Warning: this will also change the reload button based on your current controller config type, since the original button is used for sprinting when not aiming.");
						
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("New reload button (Xinput):");
						ImGui::BulletText("Config Type I: B");
						ImGui::BulletText("Config Type II & III: X");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui_ItemSeparator2();
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("ReloadWithoutZoom", &re4t::cfg->bReloadWithoutZoom_controller);

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Don't zoom in when reloading without aiming.");
						ImGui::TextWrapped("Not usually recomended, since the zooming in and out masks some animation quirks when the reload ends.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Framerate)
			{
				if (ImGui::BeginTable("Framerate", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// FixFallingItemsSpeed
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixFallingItemsSpeed", &re4t::cfg->bFixFallingItemsSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes the speed of falling items when using framerates other than 30FPS, making them not fall at double speed.");
					}

					// FixCompartmentsOpeningSpeed
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixCompartmentsOpeningSpeed", &re4t::cfg->bFixCompartmentsOpeningSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes the speed of opening compartments such as drawers, cabinets, chests, boxes, shelves, etc.");
					}

					// FixMovingGeometrySpeed
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixMovingGeometrySpeed", &re4t::cfg->bFixMovingGeometrySpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes the speed of moving geometry such as: doors, passages, gates, ceilings, etc.");
					}

					// FixTurningSpeed
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixTurningSpeed", &re4t::cfg->bFixTurningSpeed);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes speed of backwards turning when using framerates other than 30FPS.");
					}

					// FixQTE
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixQTE", &re4t::cfg->bFixQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs.");
						ImGui::TextWrapped("This fix makes QTEs that involve rapid button presses much more forgiving.");
					}

					// FixAshleyBustPhysics
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixAshleyBustPhysics", &re4t::cfg->bFixAshleyBustPhysics);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes difference between 30/60FPS on physics applied to Ashley.");
					}

					// EnableFastMath
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableFastMath", &re4t::cfg->bEnableFastMath))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Replaces older math functions in the game with much more optimized equivalents.");
						ImGui::TextWrapped("Experimental, can hopefully improve framerate in some areas that had dips.");
					}

					// ReplaceFramelimiter
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("ReplaceFramelimiter", &re4t::cfg->bReplaceFramelimiter))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Replaces the games 60/30FPS framelimiter with our own version, which reduces CPU usage quite a lot.");
						ImGui::TextWrapped("(experimental, not known if the new framelimiter performs the same as the old one yet)");
					}

					// MultithreadFix
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("MultithreadFix", &re4t::cfg->bMultithreadFix))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Enables fixes for multithreaded DirectX9, giving a slight performance boost");
						ImGui::TextWrapped("Experimental, in case you have issues with the game crashing/freezing, please try disabling this setting and playing again.");
					}

					// PrecacheModels
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("PrecacheModels", &re4t::cfg->bPrecacheModels);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
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
				if (ImGui::BeginTable("Misc", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// OverrideCostumes
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("OverrideCostumes", &re4t::cfg->bOverrideCostumes);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows overriding the costumes, making it possible to combine Normal/Special 1/Special 2 costumes.");
						ImGui::TextWrapped("May cause weird visuals in cutscenes.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::BeginDisabled(!re4t::cfg->bOverrideCostumes);
						ImGui::PushItemWidth(150 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
						ImGui::Combo("Leon", &iCostumeComboLeon, sLeonCostumeNames, IM_ARRAYSIZE(sLeonCostumeNames));
						if (ImGui::IsItemEdited())
						{
							re4t::cfg->HasUnsavedChanges = true;
							re4t::cfg->CostumeOverride.Leon = (LeonCostume)iCostumeComboLeon;
						}

						ImGui::Combo("Ashley", &iCostumeComboAshley, sAshleyCostumeNames, IM_ARRAYSIZE(sAshleyCostumeNames));
						if (ImGui::IsItemEdited())
						{
							re4t::cfg->HasUnsavedChanges = true;
							re4t::cfg->CostumeOverride.Ashley = (AshleyCostume)iCostumeComboAshley;
						}

						ImGui::Combo("Ada", &iCostumeComboAda, sAdaCostumeNames, IM_ARRAYSIZE(sAdaCostumeNames));
						if (ImGui::IsItemEdited())
						{
							re4t::cfg->HasUnsavedChanges = true;

							// ID number 2 seems to be the exact same outfit as ID number 0, for some reason, so we increase the ID here to use the actual next costume
							if (iCostumeComboAda == 2)
								re4t::cfg->CostumeOverride.Ada = (AdaCostume)(iCostumeComboAda + 1);
							else
								re4t::cfg->CostumeOverride.Ada = (AdaCostume)iCostumeComboAda;
						}
						ImGui::PopItemWidth();
						ImGui::EndDisabled();
					}

					// EnableNTSCMode
					{
						ImGui_ColumnSwitch();

						if(ImGui::Checkbox("EnableNTSCMode", &re4t::cfg->bEnableNTSCMode))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Unlocks minor difficulty boosts previously exclusive to the North American console versions of RE4.");
						ImGui::TextWrapped("Higher starting adaptive difficulty, more difficult Ada missions, and a more difficult Mercenaries village stage.");
						ImGui::TextWrapped("Bottle caps require 3000 points in the shooting gallery, and Easy difficulty is removed from the title menu.");
					}

					// AshleyJPCameraAngles
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("AshleyJPCameraAngles", &re4t::cfg->bAshleyJPCameraAngles);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Unlocks the JP-only classic camera angles during Ashley segment.");
					}

					// RestoreDemoVideos
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RestoreDemoVideos", &re4t::cfg->bRestoreDemoVideos);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Restores the playback of demo videos that play when the game is idle at the main menu or the \"press any key\" screen for about 20 seconds.");
						ImGui::TextWrapped("By default, the PC port only includes a single demo video (demo0.sfd/demo0eng.sfd).");
						ImGui::TextWrapped("However, if present in the BIO4/movie folder, re4_tweaks also supports playing the original GameCube videos.");
						ImGui::TextWrapped("Supported demo videos:");
						ImGui::BulletText("demo0_gc.sfd");
						ImGui::BulletText("demo1_gc.sfd");
						ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("demo0.sfd/demo0eng.sfd (included in the vanilla game, and also present in the Wii version as \"demo2.sfd\")");
					}

					// ViolenceLevelOverride
					{
						ImGui_ColumnSwitch();

						ImGui::Text("ViolenceLevelOverride");

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows overriding the level of violence in the game.");
						ImGui::TextWrapped("Use -1 to leave as games default, 0 for low-violence mode (as used in JP/GER version), or 2 for full violence.");
						
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::PushItemWidth(100 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
						if (ImGui::InputInt("Violence Level Override", &re4t::cfg->iViolenceLevelOverride))
						{
							if (re4t::cfg->iViolenceLevelOverride < -1)
								re4t::cfg->iViolenceLevelOverride = -1;

							if (re4t::cfg->iViolenceLevelOverride > 2)
								re4t::cfg->iViolenceLevelOverride = 2;

							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true; // unfortunately required as game only reads pSys from savegame during first loading screen...
						}
						ImGui::PopItemWidth();
					}

					// RifleScreenShake
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("RifleScreenShake", &re4t::cfg->bRifleScreenShake))
						{
							re4t::cfg->HasUnsavedChanges = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Adds a screen shake effect when firing a rifle.");
					}

					// AllowSellingHandgunSilencer
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowSellingHandgunSilencer", &re4t::cfg->bAllowSellingHandgunSilencer))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows selling the (normally unused) handgun silencer to the merchant.");
					}

					// RestoreAnalogTitleScroll
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("RestoreAnalogTitleScroll", &re4t::cfg->bRestoreAnalogTitleScroll);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Restores the ability to manipulate the background scroll of the post-new game title menu with the right analog stick or mouse movement.");
						ImGui::TextWrapped("For keyboard and mouse: Move the mouse while holding CTRL.");
					}

					// AllowMafiaLeonCutscenes
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowMafiaLeonCutscenes", &re4t::cfg->bAllowMafiaLeonCutscenes))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows the game to display Leon's mafia outfit (\"Special 2\") on cutscenes.");
					}

					// SilenceArmoredAshley
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("SilenceArmoredAshley", &re4t::cfg->bSilenceArmoredAshley);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Silence Ashley's armored outfit (\"Special 2\").");
						ImGui::TextWrapped("For those who also hate the constant \"Clank Clank Clank\".");
					}

					// AllowAshleySuplex
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("AllowAshleySuplex", &re4t::cfg->bAllowAshleySuplex);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows Ashley to Suplex enemies in very specific situations.");
						ImGui::TextWrapped("(previously was only possible in the initial NTSC GameCube ver., was patched out in all later ports.)");
					}

					// AllowMatildaQuickturn
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("AllowMatildaQuickturn", &re4t::cfg->bAllowMatildaQuickturn);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allows quickturning character to camera direction when wielding Matilda.");
						ImGui::TextWrapped("(only effective if MouseTurning is disabled)");
					}

					// FixDitmanGlitch
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("FixDitmanGlitch", &re4t::cfg->bFixDitmanGlitch);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Fixes the Ditman glitch, which would allow players to increase their walk speed.");
					}

					// UseSprintToggle
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("UseSprintToggle", &re4t::cfg->bUseSprintToggle);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Changes sprint key to act like a toggle instead of needing to be held.");
					}

					// DisableQTE
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("DisableQTE", &re4t::cfg->bDisableQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Disables most of the QTEs, making them pass automatically.");
					}

					// AutomaticMashingQTE
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("AutomaticMashingQTE", &re4t::cfg->bAutomaticMashingQTE);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Unlike the previous option, this only automates the \"mashing\" QTEs, making them pass automatically. Prompts are still shown!");
					}

					// SkipIntroLogos
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("SkipIntroLogos", &re4t::cfg->bSkipIntroLogos);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Whether to skip the Capcom etc intro logos when starting the game.");

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::BeginDisabled(!re4t::cfg->bSkipIntroLogos);
						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("SkipMenuFades", &re4t::cfg->bSkipMenuFades);
						ImGui::EndDisabled();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Reduces the length of menu fades/messages (eg. 'Save/Load successful!') and skips the initial \"Resident Evil 4\" logo / \"PRESS ANY KEY\" screen.");
						ImGui::TextWrapped("(will only take effect when SkipIntroLogos is also set to true)");
					}

					// LimitMatildaBurst
					{
						ImGui_ColumnSwitch();

						re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("LimitMatildaBurst", &re4t::cfg->bLimitMatildaBurst);

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Limit the Matilda to one three round burst per trigger pull.");
					}

					// EnableDebugMenu
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("EnableDebugMenu", &re4t::cfg->bEnableDebugMenu))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Enables the \"tool menu\" debug menu, present inside the game but unused, and adds a few custom menu entries (\"SAVE GAME\", \"DOF / BLUR MENU\", etc).");
						ImGui::TextWrapped("Can be opened with the LT+LS button combination (or CTRL+F3 on keyboard by default).");
						ImGui::TextWrapped("If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.");
						ImGui::TextWrapped("(may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)");
					}

					// ShowGameOutput
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("ShowGameOutput", &re4t::cfg->bShowGameOutput))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Displays the game's original logs/debug output into a console window.");					
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Memory)
			{
				if (ImGui::BeginTable("Memory", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// AllowHighResolutionSFD
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("AllowHighResolutionSFD", &re4t::cfg->bAllowHighResolutionSFD))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.");
						ImGui::TextWrapped("Not tested beyond 1920x1080.");
					}

					// RaiseVertexAlloc
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("RaiseVertexAlloc", &re4t::cfg->bRaiseVertexAlloc))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allocate more memory for some vertex buffers.");
						ImGui::TextWrapped("This prevents a crash that can happen when playing with a high FOV.");
					}

					// RaiseInventoryAlloc
					{
						ImGui_ColumnSwitch();

						if (ImGui::Checkbox("RaiseInventoryAlloc", &re4t::cfg->bRaiseInventoryAlloc))
						{
							re4t::cfg->HasUnsavedChanges = true;
							NeedsToRestart = true;
						}

						ImGui_ItemSeparator();

						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
						ImGui::TextWrapped("Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Hotkeys)
			{
				float btn_size_x = 150 * esHook._cur_monitor_dpi;

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

				if (ImGui::BeginTable("Hotkeys", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
				{
					ImGui_ColumnInit();

					// cfgMenu 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination to open the re4_tweaks config menu");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushID(1);
						if (ImGui::Button(re4t::cfg->sConfigMenuKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
						{
							re4t::cfg->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyComboThread, &re4t::cfg->sConfigMenuKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open config menu");
					}

					// Console
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination to open the re4_tweaks debug console");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushID(2);
						if (ImGui::Button(re4t::cfg->sConsoleKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
						{
							re4t::cfg->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyComboThread, &re4t::cfg->sConsoleKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open console");
					}

					// Inv flip 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key bindings for flipping items in the inventory screen when using keyboard and mouse.");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

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
							if (ImGui::Button(re4t::cfg->sFlipItemUp.c_str(), ImVec2(btn_size_x, 0)))
							{
								re4t::cfg->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyThread, &re4t::cfg->sFlipItemUp, 0, NULL);
							}
							ImGui::PopID();

							ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

							// DOWN
							ImGui::TextWrapped("Flip DOWN");
							ImGui::PushID(4);
							if (ImGui::Button(re4t::cfg->sFlipItemDown.c_str(), ImVec2(btn_size_x, 0)))
							{
								re4t::cfg->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyThread, &re4t::cfg->sFlipItemDown, 0, NULL);
							}
							ImGui::PopID();

							ImGui::TableNextColumn();

							// LEFT
							ImGui::TextWrapped("Flip LEFT");
							ImGui::PushID(5);
							if (ImGui::Button(re4t::cfg->sFlipItemLeft.c_str(), ImVec2(btn_size_x, 0)))
							{
								re4t::cfg->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyThread, &re4t::cfg->sFlipItemLeft, 0, NULL);
							}
							ImGui::PopID();

							ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

							// RIGHT
							ImGui::TextWrapped("Flip RIGHT");
							ImGui::PushID(6);
							if (ImGui::Button(re4t::cfg->sFlipItemRight.c_str(), ImVec2(btn_size_x, 0)))
							{
								re4t::cfg->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyThread, &re4t::cfg->sFlipItemRight, 0, NULL);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}
					}

					// QTE 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key bindings for QTE keys when playing with keyboard and mouse.");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

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
							if (ImGui::Button(re4t::cfg->sQTE_key_1.c_str(), ImVec2(btn_size_x, 0)))
							{
								re4t::cfg->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyThread, &re4t::cfg->sQTE_key_1, 0, NULL);
							}
							ImGui::PopID();

							ImGui::TableNextColumn();

							// QTE2
							ImGui::TextWrapped("QTE key 2");
							ImGui::PushID(8);
							if (ImGui::Button(re4t::cfg->sQTE_key_2.c_str(), ImVec2(btn_size_x, 0)))
							{
								re4t::cfg->HasUnsavedChanges = true;
								CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyThread, &re4t::cfg->sQTE_key_2, 0, NULL);
							}
							ImGui::PopID();

							ImGui::EndTable();
						}
					}

					// Debug menu
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination to make the \"tool menu\" debug menu appear.");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("Requires EnableDebugMenu to be enabled.");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushID(9);
						if (ImGui::Button(re4t::cfg->sDebugMenuKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
						{
							re4t::cfg->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyComboThread, &re4t::cfg->sDebugMenuKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Open debug menu");
					}

					// MouseTurningModifier
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("MouseTurning camera modifier.");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::TextWrapped("When holding this key combination, MouseTurning is temporarily activated/deactivated.");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushID(10);
						if (ImGui::Button(re4t::cfg->sMouseTurnModifierKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
						{
							re4t::cfg->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyComboThread, &re4t::cfg->sMouseTurnModifierKeyCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("MouseTurning modifier");
					}

					// JetSkiTricks 
					{
						ImGui_ColumnSwitch();

						ImGui::TextWrapped("Key combination for jump tricks during Jet-ski sequence (normally RT+LT on controller)");
						ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

						ImGui::PushID(11);
						if (ImGui::Button(re4t::cfg->sJetSkiTrickCombo.c_str(), ImVec2(btn_size_x, 0)))
						{
							re4t::cfg->HasUnsavedChanges = true;
							CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&ImGui_SetHotkeyComboThread, &re4t::cfg->sJetSkiTrickCombo, 0, NULL);
						}
						ImGui::PopID();

						ImGui::SameLine();

						ImGui::TextWrapped("Trick while riding Jet-ski");
					}

					ImGui_ColumnFinish();
					ImGui::EndTable();
				}
			}

			if (Tab == MenuTab::Trainer)
			{
				Trainer_RenderUI(columnCount);
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

		std::string tooltip = std::string("re4_tweaks: Press ") + re4t::cfg->sConfigMenuKeyCombo.c_str() + std::string(" to open the configuration menu");

		ImGui::TextUnformatted(tooltip.data());

		ImGui::End();
	}
}
