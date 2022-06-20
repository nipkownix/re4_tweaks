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

int MenuTipTimer = 500; // cfgMenu tooltip timer
MenuTab Tab = MenuTab::Display;
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

void cfgMenuRender()
{
	ImGui::SetNextWindowSize(ImVec2(960, 630), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(960, (630 * (ImGui::GetIO().FontGlobalScale + 0.35f))));

	if (ImGui::Begin(cfgMenuTitle.data(), 0))
	{
		// Left side
		static int selected = 0;
		{
			ImGui::BeginChild("left side", ImVec2(230, 0));

			static ImVec4 active = ImVec4(41.0f / 255.0f, 40.0f / 255.0f, 41.0f / 255.0f, 255.0f / 255.0f);
			static ImVec4 inactive = ImVec4(31.0f / 255.0f, 30.0f / 255.0f, 31.0f / 255.0f, 255.0f / 255.0f);

			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Display ? active : inactive);
			if (ImGui::Button(ICON_FA_DESKTOP " Display", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Display;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Audio ? active : inactive);
			if (ImGui::Button(ICON_FA_HEADPHONES " Audio", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Audio;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Mouse ? active : inactive);
			if (ImGui::Button(ICON_FA_LOCATION_ARROW " Mouse", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Mouse;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Keyboard ? active : inactive);
			if (ImGui::Button(ICON_FA_KEYBOARD " Keyboard", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Keyboard;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Controller ? active : inactive);
			if (ImGui::Button(ICON_FA_JOYSTICK " Controller", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Controller;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Framerate ? active : inactive);
			if (ImGui::Button(ICON_FA_CHESS_CLOCK " Frame rate", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Framerate;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Misc ? active : inactive);
			if (ImGui::Button(ICON_FA_COG " Misc", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Misc;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Memory ? active : inactive);
			if (ImGui::Button(ICON_FA_CALCULATOR " Memory", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Memory;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == MenuTab::Hotkeys ? active : inactive);
			if (ImGui::Button(ICON_FA_LAMBDA " Hotkeys", ImVec2(230 - 15, 41)))
				Tab = MenuTab::Hotkeys;

			ImGui::PopStyleColor(int(MenuTab::NumTabs));

			ImGui::Dummy(ImVec2(0.0f, 12.0f));

			if (cfg.HasUnsavedChanges)
			{
				ImGui::Bullet();
				ImGui::TextWrapped("You have unsaved changes!");
			}

			ImGui::Dummy(ImVec2(0.0f, 12.0f));

			if (NeedsToRestart)
			{
				ImGui::Bullet();
				ImGui::TextWrapped("Changes that have been made require the game to be restarted!");
			}

			ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 40);

			if (ImGui::Button(ICON_FA_ERASER" Discard", ImVec2(104, 35)))
			{
				cfg.ReadSettings();
				ImGui::GetIO().FontGlobalScale = cfg.fFontSize - 0.35f;
			}
				
			ImGui::SameLine();

			if (ImGui::Button(ICON_FA_CODE" Save", ImVec2(104, 35)))
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

		// Right
		{
			ImGui::BeginChild("item view", ImVec2(0, 0));

			ImGui::Spacing();

			static ImGuiTableFlags TableFlags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_ContextMenuInBody;

			// Display tab
			if (Tab == MenuTab::Display)
			{
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
					if (ImGui::GetIO().FontGlobalScale < 0.95f)
						ImGui::GetIO().FontGlobalScale += 0.05f;

					cfg.HasUnsavedChanges = true;
				}

				ImGui::SameLine();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Font Size");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FOVAdditional
				if (ImGui::BeginTable("FOVTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 300.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();
					if (ImGui::Checkbox("FOVAdditional", &cfg.bEnableFOV))
					{
						cfg.HasUnsavedChanges = true;
					}
					ImGui::TextWrapped("Additional FOV value.");
					ImGui::TextWrapped("20 seems good for most cases.");

					ImGui::TableNextColumn();

					ImGui::Dummy(ImVec2(0.0f, 15.0f));

					ImGui::PushItemWidth(ImGui::GetWindowSize().x - 480);

					if (!cfg.bEnableFOV)
						cfg.fFOVAdditional = 0.0f;

					ImGui::BeginDisabled(!cfg.bEnableFOV);

					ImGui::SliderFloat("FOV Slider", &cfg.fFOVAdditional, 0.0f, 50.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);

					ImGui::EndDisabled();
					ImGui::PopItemWidth();

					ImGui::EndTable();
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixUltraWideAspectRatio
				if (ImGui::Checkbox("FixUltraWideAspectRatio", &cfg.bFixUltraWideAspectRatio))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Fixes the incorrect aspect ratio when playing in ultrawide resolutions, preventing the image from being cut off and the HUD appearing off-screen. Only tested in 21:9.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// DisableVsync
				if (ImGui::Checkbox("DisableVsync", &cfg.bDisableVsync))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Force V-Sync to be disabled. For some reason the vanilla game doesn't provide a functional way to do this.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixDPIScale
				if (ImGui::Checkbox("FixDPIScale", &cfg.bFixDPIScale))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Forces game to run at normal 100%% DPI scaling, fixes resolution issues for players that have above 100%% DPI scaling set.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixDisplayMode
				if (ImGui::Checkbox("FixDisplayMode", &cfg.bFixDisplayMode))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allows game to use non - 60Hz refresh rates in fullscreen, fixing the black screen issue people have when starting the game.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::TextWrapped("Determines a custom refresh rate for the game to use.");
				ImGui::TextWrapped("Requires FixDisplayMode to be enabled.");
				ImGui::TextWrapped("-1 will make it try to use the current refresh rate as reported by Windows.");

				ImGui::PushItemWidth(150);
				ImGui::InputInt("Hz", &cfg.iCustomRefreshRate);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// OverrideLaserColor
				if (ImGui::BeginTable("LaserTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 480.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();
					cfg.HasUnsavedChanges |= ImGui::Checkbox("OverrideLaserColor", &cfg.bOverrideLaserColor);
					ImGui::TextWrapped("Overrides the color of the laser sights.");
					ImGui::TextWrapped("Note: The game's vanilla config.ini contains a setting to change the");
					ImGui::TextWrapped("alpha/opacity of the laser, but since it doesn't work, we don't include it here.");

					ImGui::TableNextColumn();

					ImGui::Dummy(ImVec2(0.0f, 25.0f));

					ImGui::BeginDisabled(!cfg.bOverrideLaserColor);

					ImGui::ColorEdit4("Laser color picker", cfg.fLaserRGB, ImGuiColorEditFlags_PickerHueWheel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs);

					ImGui::EndDisabled();

					ImGui::EndTable();
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// RestorePickupTransparency
				cfg.HasUnsavedChanges |= ImGui::Checkbox("RestorePickupTransparency", &cfg.bRestorePickupTransparency);
				ImGui::TextWrapped("Restores transparency on the item pickup screeen.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// DisableBrokenFilter03
				cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableBrokenFilter03", &cfg.bDisableBrokenFilter03);
				ImGui::TextWrapped("This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.");
				ImGui::TextWrapped("(if you're using the HD Project, you should disable this option)");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixBlurryImage
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixBlurryImage", &cfg.bFixBlurryImage);
				ImGui::TextWrapped("Fixes a problem related to a vertex buffer that caused the image to be slightly blurred, making the image much sharper and clearer.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// DisableFilmGrain
				cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableFilmGrain", &cfg.bDisableFilmGrain);
				ImGui::TextWrapped("Disables the film grain overlay that is present in most sections of the game.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// EnableGCBlur
				if (ImGui::Checkbox("EnableGCBlur", &cfg.bEnableGCBlur))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Restores DoF blurring from the GC version, which was removed/unimplemented in later ports.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// EnableGCScopeBlur
				if (ImGui::Checkbox("EnableGCScopeBlur", &cfg.bEnableGCScopeBlur))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Restores outer-scope blurring when using a scope, which was removed/unimplemented in later ports.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// WindowBorderless
				if (ImGui::Checkbox("WindowBorderless", &cfg.bWindowBorderless))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Whether to use a borderless-window when using windowed-mode.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Text("Position to draw the game window when using windowed mode.");
				ImGui::Text("-1 will use the games default (usually places it at 0,0)");

				ImGui::PushItemWidth(150);
				ImGui::InputInt("X Pos", &cfg.iWindowPositionX);
				ImGui::InputInt("Y Pos", &cfg.iWindowPositionY);
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::TextWrapped("Remember the last window position. This automatically updates the \"X Pos\" and \"Y Pos\" values above.");
				cfg.HasUnsavedChanges |= ImGui::Checkbox("RememberWindowPos", &cfg.bRememberWindowPos);
			}

			// Audio tab
			if (Tab == MenuTab::Audio)
			{
				if (ImGui::BeginTable("AudioVolTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 340.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();

					ImGui::Dummy(ImVec2(0.0f, 32.0f));

					ImGui::TextWrapped("Adjust the volume of the game.");
					ImGui::Spacing();
					ImGui::TextWrapped("Cutscene Volume will also affect volume of merchant screen dialogue, as that seems to be handled the same way as cutscenes.");

					ImGui::TableNextColumn();

					ImGui::Dummy(ImVec2(0.0f, 12.0f));

					ImGui::PushItemWidth(ImGui::GetWindowSize().x - 520);

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

					ImGui::EndTable();
				}
			}

			// Mouse tab
			if (Tab == MenuTab::Mouse)
			{
				if (ImGui::BeginTable("MSensTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 340.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();
					if (ImGui::Checkbox("UseMouseTurning", &cfg.bUseMouseTurning))
					{
						cfg.HasUnsavedChanges = true;
					}
					ImGui::TextWrapped("Makes it so the mouse turns the character instead of controlling the camera.");
					ImGui::TextWrapped("\"Modern\" aiming mode in the game's settings is recommended.");

					ImGui::TableNextColumn();
					ImGui::Dummy(ImVec2(0.0f, 30.0f));

					ImGui::PushItemWidth(ImGui::GetWindowSize().x - 520);

					ImGui::BeginDisabled(!cfg.bUseMouseTurning);

					ImGui::SliderFloat("Sensitivity Slider", &cfg.fTurnSensitivity, 0.50f, 2.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

					ImGui::EndDisabled();

					ImGui::PopItemWidth();

					ImGui::EndTable();
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// UseRawMouseInput
				cfg.HasUnsavedChanges |= ImGui::Checkbox("UseRawMouseInput", &cfg.bUseRawMouseInput);
				ImGui::TextWrapped("Makes the game use Raw Input for aiming and turning (if MouseTurning is enabled).");
				ImGui::TextWrapped("Greatly improves mouse input by removing negative/positive accelerations that were being applied both by the game and by Direct Input.");
				ImGui::TextWrapped("This option automatically enables the game's \"Modern\" aiming mode, and is incompatible with the \"Classic\" mode.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// UnlockCameraFromAim
				cfg.HasUnsavedChanges |= ImGui::Checkbox("UnlockCameraFromAim", &cfg.bUnlockCameraFromAim);
				ImGui::TextWrapped("When using the \"Modern\" mouse setting, the game locks the camera position to the aiming position, making both move together.");
				ImGui::TextWrapped("Although this is the expected behavior in most games, some people might prefer to keep the original camera behavior while also having the benefits from \"Modern\" aiming.");
				ImGui::TextWrapped("Enabling this will also restore the horizontal aiming sway that was lost when the devs implemented \"Modern\" aiming.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixSniperZoom
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixSniperZoom", &cfg.bFixSniperZoom);
				ImGui::TextWrapped("Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixSniperFocus
				if (ImGui::Checkbox("FixSniperFocus", &cfg.bFixSniperFocus))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}

				ImGui::TextWrapped("When zooming in and out with the sniper rifle, this option makes the \"focus\" animation look similar to how it looks like with a controller.");
				ImGui::TextWrapped("Requires EnableGCBlur to be enabled.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixRetryLoadMouseSelector
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixRetryLoadMouseSelector", &cfg.bFixRetryLoadMouseSelector);
				ImGui::TextWrapped("Prevents the game from overriding your selection in the \"Retry / Load\" screen when moving the mouse before confirming an action.");
				ImGui::TextWrapped("This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.");
			}

			// Keyboard tab
			if (Tab == MenuTab::Keyboard)
			{
				// English key icons
				if (ImGui::Checkbox("FallbackToEnglishKeyIcons", &cfg.bFallbackToEnglishKeyIcons))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Game will turn keys invisible for certain unsupported keyboard languages, enabling this should make game use English keys for unsupported ones instead (requires game restart)");
			}

			// Controller tab
			if (Tab == MenuTab::Controller)
			{
				// ControllerSensitivity
				if (ImGui::BeginTable("CSensTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 340.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();
					if (ImGui::Checkbox("Controller Sensitivity", &cfg.bEnableControllerSens))
					{
						cfg.HasUnsavedChanges = true;
					}
					ImGui::TextWrapped("Change the controller sensitivity. For some reason the vanilla game doesn't have an option to change it for controllers, only for the mouse.");

					ImGui::TableNextColumn();
					ImGui::Dummy(ImVec2(0.0f, 20.0f));

					ImGui::PushItemWidth(ImGui::GetWindowSize().x - 520);

					if (!cfg.bEnableControllerSens)
						cfg.fControllerSensitivity = 1.0f;

					ImGui::BeginDisabled(!cfg.bEnableControllerSens);

					ImGui::SliderFloat("Sensitivity Slider", &cfg.fControllerSensitivity, 0.50f, 4.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

					ImGui::EndDisabled();

					ImGui::PopItemWidth();

					ImGui::EndTable();
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// RemoveExtraXinputDeadzone
				cfg.HasUnsavedChanges |= ImGui::Checkbox("RemoveExtraXinputDeadzone", &cfg.bRemoveExtraXinputDeadzone);
				ImGui::TextWrapped("Removes unnecessary deadzones that were added for Xinput controllers.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// XinputDeadzone
				if (ImGui::BeginTable("CDzoneTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 340.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();

					if (ImGui::Checkbox("Override Xinput Deadzone", &cfg.bEnableDeadzoneOverride))
					{
						cfg.HasUnsavedChanges = true;
					}
					ImGui::TextWrapped("Change the Xinput controller deadzone.");
					ImGui::TextWrapped("(Unrelated to the previous option)");
					ImGui::TextWrapped("The game's default is 1, but that seems unnecessarily large, so we default to 0.4 instead.");

					ImGui::TableNextColumn();
					ImGui::Dummy(ImVec2(0.0f, 12.0f));

					ImGui::PushItemWidth(ImGui::GetWindowSize().x - 520);

					if (!cfg.bEnableDeadzoneOverride)
						cfg.fXinputDeadzone = 1.0f;

					ImGui::BeginDisabled(!cfg.bEnableDeadzoneOverride);

					ImGui::SliderFloat("Deadzone Slider", &cfg.fXinputDeadzone, 0.0f, 3.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

					ImGui::EndDisabled();

					ImGui::PopItemWidth();

					if (ImGui::IsItemEdited())
					{
						cfg.HasUnsavedChanges = true;
						*g_XInputDeadzone_LS = (int)(cfg.fXinputDeadzone * 7849);
						*g_XInputDeadzone_RS = (int)(cfg.fXinputDeadzone * 8689);
					}

					ImGui::EndTable();
				}

			}

			// Frame rate tab
			if (Tab == MenuTab::Framerate)
			{
				// FixFallingItemsSpeed
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixFallingItemsSpeed", &cfg.bFixFallingItemsSpeed);
				ImGui::TextWrapped("Fixes the speed of falling items in 60 FPS, making them not fall at double speed.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixTurningSpeed
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixTurningSpeed", &cfg.bFixTurningSpeed);
				ImGui::TextWrapped("Fixes speed of backwards turning when using framerates other than 30FPS.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixQTE
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixQTE", &cfg.bFixQTE);
				ImGui::TextWrapped("When running in 60 FPS, some QTEs require extremely fast button presses to work. This gets even worse in Professional difficulty, making it seem almost impossible to survive the minecart and the statue bridge QTEs.");
				ImGui::TextWrapped("This fix makes QTEs that involve rapid button presses much more forgiving.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AshleyBustPhysicsFix
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixAshleyBustPhysics", &cfg.bFixAshleyBustPhysics);
				ImGui::TextWrapped("Fixes difference between 30/60FPS on physics applied to Ashley.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// EnableFastMath
				if (ImGui::Checkbox("EnableFastMath", &cfg.bEnableFastMath))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Replaces older math functions in the game with much more optimized equivalents.");
				ImGui::TextWrapped("Experimental, can hopefully improve framerate in some areas that had dips.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// PrecacheModels
				cfg.HasUnsavedChanges |= ImGui::Checkbox("PrecacheModels", &cfg.bPrecacheModels);
				ImGui::TextWrapped("Forces game to fully cache all models in the level after loading in.");
				ImGui::TextWrapped("May help with framerate drops when viewing a model for the first time.");
				ImGui::TextWrapped("(not fully tested, could cause issues if level has many models to load!)");
				ImGui::TextWrapped("Changes take effect on next level load.");
			}

			// Misc tab
			if (Tab == MenuTab::Misc)
			{
				// OverrideCostumes
				cfg.HasUnsavedChanges |= ImGui::Checkbox("OverrideCostumes", &cfg.bOverrideCostumes);
				ImGui::TextWrapped("Allows overriding the costumes, making it possible to combine Normal/Special 1/Special 2 costumes.");
				ImGui::TextWrapped("May cause weird visuals in cutscenes.");

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

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AshleyJPCameraAngles
				cfg.HasUnsavedChanges |= ImGui::Checkbox("AshleyJPCameraAngles", &cfg.bAshleyJPCameraAngles);
				ImGui::TextWrapped("Unlocks the JP-only classic camera angles during Ashley segment.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// ViolenceLevelOverride
				ImGui::PushItemWidth(150);
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
				ImGui::TextWrapped("Allows overriding the level of violence in the game.");
				ImGui::TextWrapped("Use -1 to leave as games default, 0 for low-violence mode (as used in JP/GER version), or 2 for full violence.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AllowSellingHandgunSilencer
				if (ImGui::Checkbox("AllowSellingHandgunSilencer", &cfg.bAllowSellingHandgunSilencer))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allows selling the (normally unused) handgun silencer to the merchant.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AllowMafiaLeonCutscenes
				if (ImGui::Checkbox("AllowMafiaLeonCutscenes", &cfg.bAllowMafiaLeonCutscenes))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allows the game to display Leon's mafia outfit (\"Special 2\") on cutscenes.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// SilenceArmoredAshley
				cfg.HasUnsavedChanges |= ImGui::Checkbox("SilenceArmoredAshley", &cfg.bSilenceArmoredAshley);
				ImGui::TextWrapped("Silence Ashley's armored outfit (\"Special 2\").");
				ImGui::TextWrapped("For those who also hate the constant \"Clank Clank Clank\".");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AllowAshleySuplex
				cfg.HasUnsavedChanges |= ImGui::Checkbox("AllowAshleySuplex", &cfg.bAllowAshleySuplex);
				ImGui::TextWrapped("Allows Ashley to Suplex enemies in very specific situations.");
				ImGui::TextWrapped("(previously was only possible in the initial NTSC GameCube ver., was patched out in all later ports.)");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AllowMatildaQuickturn
				cfg.HasUnsavedChanges |= ImGui::Checkbox("AllowMatildaQuickturn", &cfg.bAllowMatildaQuickturn);
				ImGui::TextWrapped("Allows quickturning character to camera direction when wielding Matilda.");
				ImGui::TextWrapped("(only effective if MouseTurning is disabled)");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixDitmanGlitch
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixDitmanGlitch", &cfg.bFixDitmanGlitch);
				ImGui::TextWrapped("Fixes the Ditman glitch, which would allow players to increase their walk speed.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// UseSprintToggle
				cfg.HasUnsavedChanges |= ImGui::Checkbox("UseSprintToggle", &cfg.bUseSprintToggle);
				ImGui::TextWrapped("Changes sprint key to act like a toggle instead of needing to be held.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// DisableQTE
				cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableQTE", &cfg.bDisableQTE);
				ImGui::TextWrapped("Disables most of the QTEs, making them pass automatically.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// AutomaticMashingQTE
				cfg.HasUnsavedChanges |= ImGui::Checkbox("AutomaticMashingQTE", &cfg.bAutomaticMashingQTE);
				ImGui::TextWrapped("Unlike the previous option, this only automates the \"mashing\" QTEs, making them pass automatically. Prompts are still shown!");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// SkipIntroLogos
				cfg.HasUnsavedChanges |= ImGui::Checkbox("SkipIntroLogos", &cfg.bSkipIntroLogos);
				ImGui::TextWrapped("Whether to skip the Capcom etc intro logos when starting the game.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// EnableDebugMenu
				if (ImGui::Checkbox("EnableDebugMenu", &cfg.bEnableDebugMenu))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Enables the \"tool menu\" debug menu, present inside the game but unused, and adds a few custom menu entries (\"SAVE GAME\", \"DOF / BLUR MENU\", etc).");
				ImGui::TextWrapped("Can be opened with the LT+LS button combination (or CTRL+F3 on keyboard by default).");
				ImGui::TextWrapped("If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.");
				ImGui::TextWrapped("(may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)");
			}

			// Memory tab
			if (Tab == MenuTab::Memory)
			{
				// AllowHighResolutionSFD
				if (ImGui::Checkbox("AllowHighResolutionSFD", &cfg.bAllowHighResolutionSFD))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.");
				ImGui::TextWrapped("Not tested beyond 1920x1080.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// RaiseVertexAlloc
				if (ImGui::Checkbox("RaiseVertexAlloc", &cfg.bRaiseVertexAlloc))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allocate more memory for some vertex buffers.");
				ImGui::TextWrapped("This prevents a crash that can happen when playing with a high FOV.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// RaiseInventoryAlloc
				if (ImGui::Checkbox("RaiseInventoryAlloc", &cfg.bRaiseInventoryAlloc))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allocate more memory for the inventory screen, preventing crashes with high-poly models inside ss_pzzl.dat.");
			}

			// Hotkeys tab
			if (Tab == MenuTab::Hotkeys)
			{
				ImGui::TextWrapped("Key combinations for various re4_tweaks features.");
				ImGui::TextWrapped("Most keys can be combined (requiring multiple to be pressed at the same time). To combine, hold one key and press another at the same time.");
				ImGui::TextWrapped("(Press \"Save\" for changes to take effect.)");

				ImGui::Dummy(ImVec2(0.0f, 10.0f));

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// cfgMenu key binding
				ImGui::TextWrapped("Key combination to open the re4_tweaks config menu");
				ImGui::Spacing();

				ImGui::PushID(1);
				if (ImGui::Button(cfg.sConfigMenuKeyCombo.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sConfigMenuKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Open config menu", &cfg.sConfigMenuKeyCombo, ImGuiInputTextFlags_CharsUppercase);

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// Console key binding
				ImGui::TextWrapped("Key combination to open the re4_tweaks debug console (only in certain re4_tweaks builds)");
				ImGui::Spacing();

				ImGui::PushID(2);
				if (ImGui::Button(cfg.sConsoleKeyCombo.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sConsoleKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Open console");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// Inv flip key bindings
				ImGui::TextWrapped("Key bindings for flipping items in the inventory screen when using keyboard and mouse.");
				ImGui::TextWrapped("Normally, you can only rotate them with the keyboard, not flip them. Flipping was possible in the old PC port and is possible using a controller.");
				ImGui::BulletText("Key combinations not supported");
				ImGui::Spacing();

				ImGui::Columns(2, "inv flip");

				// UP
				ImGui::PushID(3);
				if (ImGui::Button(cfg.sFlipItemUp.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemUp, 0, NULL);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextWrapped("Flip UP");

				// DOWN
				ImGui::PushID(4);
				if (ImGui::Button(cfg.sFlipItemDown.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemDown, 0, NULL);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextWrapped("Flip DOWN");

				ImGui::NextColumn();

				// LEFT
				ImGui::PushID(5);
				if (ImGui::Button(cfg.sFlipItemLeft.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemLeft, 0, NULL);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextWrapped("Flip LEFT");

				// RIGHT
				ImGui::PushID(6);
				if (ImGui::Button(cfg.sFlipItemRight.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sFlipItemRight, 0, NULL);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextWrapped("Flip RIGHT");

				ImGui::Columns();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// QTE key bindings
				ImGui::TextWrapped("Key bindings for QTE keys when playing with keyboard and mouse.");
				ImGui::TextWrapped("Unlike the \"official\" way of rebinding keys through usr_input.ini, this option also changes the on-screen prompt to properly match the selected key.");
				ImGui::BulletText("Key combinations not supported");
				ImGui::Spacing();

				ImGui::Columns(2, "qte");

				// QTE1
				ImGui::PushID(7);
				if (ImGui::Button(cfg.sQTE_key_1.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sQTE_key_1, 0, NULL);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextWrapped("QTE key 1");

				ImGui::NextColumn();

				// QTE2
				ImGui::PushID(8);
				if (ImGui::Button(cfg.sQTE_key_2.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyThread, &cfg.sQTE_key_2, 0, NULL);
				}
				ImGui::PopID();
				ImGui::SameLine();
				ImGui::TextWrapped("QTE key 2");

				ImGui::Columns();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// Debug menu keyboard bindings
				ImGui::TextWrapped("Key combination to make the \"tool menu\" debug menu appear.");
				ImGui::TextWrapped("Requires EnableDebugMenu to be enabled.");
				ImGui::Spacing();

				ImGui::PushID(9);
				if (ImGui::Button(cfg.sDebugMenuKeyCombo.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sDebugMenuKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Open debug menu");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// MouseTurningModifier bindings
				ImGui::TextWrapped("MouseTurning camera modifier. When holding this key combination, MouseTurning is temporarily activated/deactivated.");
				ImGui::Spacing();

				ImGui::PushID(10);
				if (ImGui::Button(cfg.sMouseTurnModifierKeyCombo.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sMouseTurnModifierKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("MouseTurning modifier");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// JetSkiTricks bindings
				ImGui::TextWrapped("Key combination for jump tricks during Jet-ski sequence (normally RT+LT on controller)");
				ImGui::Spacing();

				ImGui::PushID(11);
				if (ImGui::Button(cfg.sJetSkiTrickCombo.c_str(), ImVec2(150, 0)))
				{
					cfg.HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &cfg.sJetSkiTrickCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Trick while riding Jet-ski");
			}

			ImGui::EndChild();
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();
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

		std::string tooltip = std::string("re4_tweaks: Press ") + cfg.sConfigMenuKeyCombo + std::string(" to open the configuration menu");

		ImGui::TextUnformatted(tooltip.data());

		ImGui::End();
	}
}
