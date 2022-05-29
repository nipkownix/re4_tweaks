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

bool bCfgMenuOpen;
bool NeedsToRestart;

int MenuTipTimer = 500; // cfgMenu tooltip timer
int Tab = 1;
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
	if (_input->is_combo_pressed(&cfgMenuCombo))
		bCfgMenuOpen = !bCfgMenuOpen;
}

void cfgMenuRender()
{
	ImGui::SetNextWindowSize(ImVec2(910, 570), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(910, (570 * (ImGui::GetIO().FontGlobalScale + 0.35f))));

	if (ImGui::Begin(cfgMenuTitle.data(), 0))
	{
		// Left side
		static int selected = 0;
		{
			ImGui::BeginChild("left side", ImVec2(230, 0));

			static ImVec4 active = ImVec4(41.0f / 255.0f, 40.0f / 255.0f, 41.0f / 255.0f, 255.0f / 255.0f);
			static ImVec4 inactive = ImVec4(31.0f / 255.0f, 30.0f / 255.0f, 31.0f / 255.0f, 255.0f / 255.0f);

			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 1 ? active : inactive);
			if (ImGui::Button(ICON_FA_DESKTOP " Display", ImVec2(230 - 15, 41)))
				Tab = 1;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 2 ? active : inactive);
			if (ImGui::Button(ICON_FA_LOCATION_ARROW " Mouse", ImVec2(230 - 15, 41)))
				Tab = 2;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 3 ? active : inactive);
			if (ImGui::Button(ICON_FA_KEYBOARD " Keyboard", ImVec2(230 - 15, 41)))
				Tab = 3;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 4 ? active : inactive);
			if (ImGui::Button(ICON_FA_JOYSTICK " Controller", ImVec2(230 - 15, 41)))
				Tab = 4;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 5 ? active : inactive);
			if (ImGui::Button(ICON_FA_CHESS_CLOCK " Frame rate", ImVec2(230 - 15, 41)))
				Tab = 5;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 6 ? active : inactive);
			if (ImGui::Button(ICON_FA_COG " Misc", ImVec2(230 - 15, 41)))
				Tab = 6;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 7 ? active : inactive);
			if (ImGui::Button(ICON_FA_CALCULATOR " Memory", ImVec2(230 - 15, 41)))
				Tab = 7;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 8 ? active : inactive);
			if (ImGui::Button(ICON_FA_LAMBDA " Hotkeys", ImVec2(230 - 15, 41)))
				Tab = 8;

			ImGui::PopStyleColor(8);

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
			if (Tab == 1)
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
					ImGui::TextWrapped("Additional FOV value. 20 seems good for most cases.");

					ImGui::TableNextColumn();
					ImGui::Dummy(ImVec2(0.0f, 15.0f));
					ImGui::PushItemWidth(170);
					if (!cfg.bEnableFOV)
						cfg.fFOVAdditional = 0.0f;
					ImGui::SliderFloat("FOV Slider", &cfg.fFOVAdditional, 0.0f, 50.0f, "%.0f");
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

			// Mouse tab
			if (Tab == 2)
			{
				if (ImGui::BeginTable("FOVTable", 2, TableFlags))
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
					ImGui::Dummy(ImVec2(0.0f, 20.0f));
					ImGui::PushItemWidth(170);

					ImGui::SliderFloat("Sensitivity Slider", &cfg.fTurnSensitivity, 0.50f, 2.0f, "%.2f");
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
			if (Tab == 3)
			{
				// Inv flip key bindings
				ImGui::TextWrapped("Key bindings for flipping items in the inventory screen when using keyboard and mouse.");
				ImGui::TextWrapped("Normally, you can only rotate them with the keyboard, not flip them. Flipping was possible in the old PC port and is possible using a controller.");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Flip UP", &cfg.sFlipItemUp, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("Flip DOWN", &cfg.sFlipItemDown, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("Flip LEFT", &cfg.sFlipItemLeft, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("Flip RIGHT", &cfg.sFlipItemRight, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// QTE key bindings
				ImGui::TextWrapped("Key bindings for QTE keys when playing with keyboard and mouse.");
				ImGui::TextWrapped("Unlike the \"official\" way of rebinding keys through usr_input.ini, this option also changes the on-screen prompt to properly match the selected key.");

				ImGui::PushItemWidth(100);
				ImGui::InputText("QTE key 1", &cfg.sQTE_key_1, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("QTE key 2", &cfg.sQTE_key_2, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::PopItemWidth();

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// English key icons
				if (ImGui::Checkbox("FallbackToEnglishKeyIcons", &cfg.bFallbackToEnglishKeyIcons))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Game will turn keys invisible for certain unsupported keyboard languages, enabling this should make game use English keys for unsupported ones instead (requires game restart)");
			}

			// Controller tab
			if (Tab == 4)
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
					ImGui::PushItemWidth(170);
					if (!cfg.bEnableControllerSens)
						cfg.fControllerSensitivity = 1.0f;
					ImGui::SliderFloat("Sensitivity Slider", &cfg.fControllerSensitivity, 0.50f, 4.0f, "%.2f");
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

					ImGui::TextWrapped("Change the Xinput controller deadzone.");
					ImGui::TextWrapped("The game's default is 1, but that seems unnecessarily large, so we default to 0.4 instead.");

					ImGui::TableNextColumn();
					ImGui::Dummy(ImVec2(0.0f, 12.0f));
					ImGui::PushItemWidth(170);
					ImGui::SliderFloat("Deadzone Slider", &cfg.fXinputDeadzone, 0.0f, 3.5f, "%.2f");
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
			if (Tab == 5)
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
			}

			// Misc tab
			if (Tab == 6)
			{
				// OverrideCostumes
				cfg.HasUnsavedChanges |= ImGui::Checkbox("OverrideCostumes", &cfg.bOverrideCostumes);
				ImGui::TextWrapped("Allows overriding the costumes, making it possible to combine Normal/Special 1/Special 2 costumes.");

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
			if (Tab == 7)
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
			if (Tab == 8)
			{
				ImGui::TextWrapped("All keys can be combined (requiring multiple to be pressed at the same time) by using + symbol between key names.");
				ImGui::TextWrapped("(Press \"Save\" for changes to take effect.)");
				ImGui::TextWrapped("(see top of Settings.cpp file for possible key names to use)");

				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				ImGui::Dummy(ImVec2(0.0f, 5.0f));

				// cfgMenu key binding
				ImGui::TextWrapped("Key combination to open the re4_tweaks config menu");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Open config menu", &cfg.sConfigMenuKeyCombo, ImGuiInputTextFlags_CharsUppercase);
				ImGui::PopItemWidth();

				if (ImGui::IsItemEdited())
					cfg.HasUnsavedChanges = true;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// Console key binding
				ImGui::TextWrapped("Key combination to open the re4_tweaks debug console (only in certain re4_tweaks builds)");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Open console", &cfg.sConsoleKeyCombo, ImGuiInputTextFlags_CharsUppercase);
				ImGui::PopItemWidth();

				if (ImGui::IsItemEdited())
					cfg.HasUnsavedChanges = true;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// Debug menu keyboard bindings
				ImGui::TextWrapped("Key-combination to make the \"tool menu\" debug menu appear.");
				ImGui::TextWrapped("Requires EnableDebugMenu to be enabled.");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Open debug menu", &cfg.sDebugMenuKeyCombo, ImGuiInputTextFlags_CharsUppercase);
				ImGui::PopItemWidth();

				if (ImGui::IsItemEdited())
					cfg.HasUnsavedChanges = true;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// MouseTurningModifier bindings
				ImGui::TextWrapped("MouseTurning camera modifier. When holding this key combination, MouseTurning is temporarily activated/deactivated.");

				ImGui::PushItemWidth(100);
				ImGui::InputText("MouseTurning modifier", &cfg.sMouseTurnModifierKeyCombo, ImGuiInputTextFlags_CharsUppercase);
				ImGui::PopItemWidth();

				if (ImGui::IsItemEdited())
					cfg.HasUnsavedChanges = true;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// JetSkiTricks bindings
				ImGui::TextWrapped("Key combo for jump tricks during Jet-ski sequence (normally RT+LT on controller)");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Trick while riding Jet-ski", &cfg.sJetSkiTrickCombo, ImGuiInputTextFlags_CharsUppercase);
				ImGui::PopItemWidth();

				if (ImGui::IsItemEdited())
					cfg.HasUnsavedChanges = true;
			}

			ImGui::EndChild();
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();
}

void ShowCfgMenuTip()
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
