#include <iostream>
#include "..\includes\stdafx.h"
#include <d3d9.h>
#include "WndProcHook.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "LAApatch.h"
#include "tool_menu.h"
#include "..\external\imgui\imgui.h"
#include "..\external\imgui\imgui_impl_win32.h"
#include "..\external\imgui\imgui_impl_dx9.h"
#include <hashes.h>
#include <sffont.hpp>
#include <faprolight.hpp>

uintptr_t ptrInputProcess;
uintptr_t* ptrD3D9Device;

bool NeedsToRestart;

int Tab = 1;
void MenuRender()
{
	ImGui::SetNextWindowSize(ImVec2(900, 470), ImGuiCond_FirstUseEver);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(900, (470 * (ImGui::GetIO().FontGlobalScale + 0.35f))));
	if (ImGui::Begin("re4_tweaks - Configuration", 0))
	{
		// Left side
		static int selected = 0;
		{
			ImGui::BeginChild("left side", ImVec2(230, 0));

			static ImVec4 active = ImVec4(41 / 255.0, 40 / 255.0, 41 / 255.0, 255 / 255.0);
			static ImVec4 inactive = ImVec4(31 / 255.0, 30 / 255.0, 31 / 255.0, 255 / 255.0);

			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 1 ? active : inactive);
			if (ImGui::Button(ICON_FA_DESKTOP " Display", ImVec2(230 - 15, 41)))
				Tab = 1;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 2 ? active : inactive);
			if (ImGui::Button(ICON_FA_COG " Misc", ImVec2(230 - 15, 41)))
				Tab = 2;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 3 ? active : inactive);
			if (ImGui::Button(ICON_FA_LOCATION_ARROW " Mouse", ImVec2(230 - 15, 41)))
				Tab = 3;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 4 ? active : inactive);
			if (ImGui::Button(ICON_FA_KEYBOARD " Keyboard", ImVec2(230 - 15, 41)))
				Tab = 4;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 5 ? active : inactive);
			if (ImGui::Button(ICON_FA_VIDEO " Movie", ImVec2(230 - 15, 41)))
				Tab = 5;

			ImGui::Spacing();
			ImGui::PushStyleColor(ImGuiCol_Button, Tab == 6 ? active : inactive);
			if (ImGui::Button(ICON_FA_CALCULATOR " Memory", ImVec2(230 - 15, 41)))
				Tab = 6;

			ImGui::PopStyleColor(6);

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

				// FixVsyncToggle
				if (ImGui::Checkbox("FixVsyncToggle", &cfg.bFixVsyncToggle))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Makes it so that the vsync option found in the game's config.ini actually works.");

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

				// RestorePickupTransparency
				cfg.HasUnsavedChanges |= ImGui::Checkbox("RestorePickupTransparency", &cfg.bRestorePickupTransparency);
				ImGui::TextWrapped("Restores transparency on the item pickup screeen.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// DisableBrokenFilter03
				cfg.HasUnsavedChanges |= ImGui::Checkbox("DisableBrokenFilter03", &cfg.bDisableBrokenFilter03);
				ImGui::TextWrapped("This filter was originally meant to add an extra glow effect on certain fire sources, but it was broken when the game was ported to the Xbox 360, making the entire image have an orange tint overlay applied to it.");

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

			// Misc tab
			if (Tab == 2)
			{
				// 60FPSfixes
				cfg.HasUnsavedChanges |= ImGui::Checkbox("60fpsFixes", &cfg.b60fpsFixes);
				ImGui::TextWrapped("Fixes animation inconsistencies between 30 and 60 FPS that were not addressed by the developers.");

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

				// AshleyJPCameraAngles
				cfg.HasUnsavedChanges |= ImGui::Checkbox("AshleyJPCameraAngles", &cfg.bAshleyJPCameraAngles);
				ImGui::TextWrapped("Unlocks the JP-only classic camera angles during Ashley segment.");

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
				ImGui::TextWrapped("Can be opened with the LT+LS button combination (or CTRL+F3 on keyboard).");
				ImGui::TextWrapped("If enabled on the 1.0.6 debug build it'll apply some fixes to the existing debug menu, fixing AREA JUMP etc, but won't add our custom entries due to lack of space.");
				ImGui::TextWrapped("(may have a rare chance to cause a heap corruption crash when loading a save, but if the game loads fine then there shouldn't be any chance of crashing)");
			
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				// Debug menu keyboard bindings
				ImGui::TextWrapped("Keyboard key-combination to make the \"tool menu\" debug menu appear");
				ImGui::TextWrapped("All keys can be combined (requiring multiple to be pressed at the same time) by using + symbol between key names");
				ImGui::TextWrapped("(see top of Settings.cpp file for possible key names to use)");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Open debug menu", cfg.sDebugMenuKeyCombo.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				ImGui::PopItemWidth;

				if (ImGui::IsItemEdited())
				{
					cfg.HasUnsavedChanges = true;
					ParseToolMenuKeyCombo(cfg.sDebugMenuKeyCombo);
				}
			}

			// Mouse tab
			if (Tab == 3)
			{
				if (ImGui::BeginTable("FOVTable", 2, TableFlags))
				{
					ImGui::TableSetupColumn("Col1", ImGuiTableColumnFlags_WidthFixed, 320.0f);
					ImGui::TableSetupColumn("Col2", ImGuiTableColumnFlags_WidthStretch, 320.0f);

					ImGui::TableNextColumn();
					if (ImGui::Checkbox("UseMouseTurning", &cfg.bUseMouseTurning))
					{
						cfg.HasUnsavedChanges = true;
					}
					ImGui::TextWrapped("Makes it so the mouse turns the character instead of controlling the camera.");
					ImGui::TextWrapped("\"Modern\" aiming mode in the game's settings is recommended.");

					ImGui::TableNextColumn();
					ImGui::Dummy(ImVec2(0.0f, 15.0f));
					ImGui::PushItemWidth(170);

					ImGui::SliderFloat("Sensitivity Slider", &cfg.fTurnSensitivity, 0.10f, 2.0f, "%.2f");
					ImGui::PopItemWidth();

					ImGui::EndTable();
				}

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixSniperZoom
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixSniperZoom", &cfg.bFixSniperZoom);
				ImGui::TextWrapped("Prevents the camera from being randomly displaced after you zoom with a sniper rifle when using keyboard and mouse.");

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// FixRetryLoadMouseSelector
				cfg.HasUnsavedChanges |= ImGui::Checkbox("FixRetryLoadMouseSelector", &cfg.bFixRetryLoadMouseSelector);
				ImGui::TextWrapped("Prevents the game from overriding your selection in the \"Retry / Load\" screen when moving the mouse before confirming an action.");
				ImGui::TextWrapped("This bug usually causes people to return to the main menu by mistake, when they actually wanted to just restart from the last checkpoint.");
			}

			// Keyboard tab
			if (Tab == 4)
			{
				// Inv flip key bindings
				ImGui::TextWrapped("Key bindings for flipping items in the inventory screen when using keyboard and mouse.");
				ImGui::TextWrapped("Normally, you can only rotate them with the keyboard, not flip them. Flipping was possible in the old PC port and is possible using a controller.");

				ImGui::PushItemWidth(100);
				ImGui::InputText("Flip UP", cfg.flip_item_up.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("Flip DOWN", cfg.flip_item_down.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("Flip LEFT", cfg.flip_item_left.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("Flip RIGHT", cfg.flip_item_right.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::PopItemWidth;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// QTE key bindings
				ImGui::TextWrapped("Key bindings for QTE keys when playing with keyboard and mouse.");
				ImGui::TextWrapped("Unlike the \"official\" way of rebinding keys through usr_input.ini, this option also changes the on-screen prompt to properly match the selected key.");

				ImGui::PushItemWidth(100);
				ImGui::InputText("QTE key 1", cfg.QTE_key_1.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::InputText("QTE key 2", cfg.QTE_key_2.data(), 64, ImGuiInputTextFlags_CharsUppercase);
				cfg.HasUnsavedChanges |= ImGui::IsItemEdited();
				ImGui::PopItemWidth;

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();
			}

			// Movie tab
			if (Tab == 5)
			{
				// AllowHighResolutionSFD
				if (ImGui::Checkbox("AllowHighResolutionSFD", &cfg.bAllowHighResolutionSFD))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Allocate more memory for SFD movie files, and properly scale its resolution display above 512x336.");
				ImGui::TextWrapped("Not tested beyond 1920x1080.");
			}

			// Memory tab
			if (Tab == 6)
			{
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

				ImGui::Spacing();
				ImGui::Separator();
				ImGui::Spacing();

				// UseMemcpy
				if (ImGui::Checkbox("UseMemcpy", &cfg.bUseMemcpy))
				{
					cfg.HasUnsavedChanges = true;
					NeedsToRestart = true;
				}
				ImGui::TextWrapped("Makes the game use the memcpy function instead of MemorySwap, possibly resulting in some slight performance improvement.");
			}

			ImGui::EndChild();
		}
	}
	ImGui::PopStyleVar();
	ImGui::End();
}

void ApplyMenuTheme()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowBorderSize = 0;
	style->WindowTitleAlign = ImVec2(0.5, 0.5);

	style->FramePadding = ImVec2(8, 6);

	style->Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.14f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.14f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.14f, 0.15f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.53f, 0.07f, 0.10f, 0.63f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(1.00f, 0.16f, 0.21f, 0.82f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.16f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.99f, 0.23f, 0.28f, 1.00f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.00f, 0.06f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	style->Colors[ImGuiCol_SeparatorActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 0.16f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.99f, 0.23f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 0.16f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_Tab] = ImVec4(1.00f, 0.16f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_TabHovered] = ImVec4(0.99f, 0.23f, 0.27f, 1.00f);
	style->Colors[ImGuiCol_TabActive] = ImVec4(1.00f, 0.16f, 0.21f, 1.00f);
	style->Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
	style->Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	style->Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	style->Colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	style->Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	style->Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

// Add our new code right before the game calls EndScene
HRESULT APIENTRY EndScene_hook(LPDIRECT3DDEVICE9 pDevice)
{
	static bool didInit = false;

	if (!didInit)
	{
		didInit = true;

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		io.IniFilename = nullptr;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF, // Basic Latin + Latin Supplement
			0x2000, 0x206F, // General Punctuation
			0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
			0x31F0, 0x31FF, // Katakana Phonetic Extensions
			0xFF00, 0xFFEF, // Half-width characters
			0x4e00, 0x9FAF, // CJK Ideograms
			0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
			0x2DE0, 0x2DFF, // Cyrillic Extended-A
			0xA640, 0xA69F, // Cyrillic Extended-B
			0,
		};

		ImFontConfig ImCustomFont;
		ImCustomFont.FontDataOwnedByAtlas = false;

		io.Fonts->AddFontFromMemoryCompressedTTF(sfpro_compressed_data, sfpro_compressed_size, 24, &ImCustomFont, ranges);

		static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

		ImFontConfig ImCustomIcons;
		ImCustomIcons.MergeMode = true;
		ImCustomIcons.PixelSnapH = true;

		io.Fonts->AddFontFromMemoryTTF(&faprolight, sizeof faprolight, 24, &ImCustomIcons, icon_ranges);

		io.FontGlobalScale = cfg.fFontSize - 0.35f;

		ApplyMenuTheme();

		ImGui_ImplWin32_Init(hWindow);
		ImGui_ImplDX9_Init(pDevice);

		if (!laa.GameIsLargeAddressAware())
		{
			laa.LAA_State = LAADialogState::Showing;
		}
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	#ifdef VERBOSE
	con.ShowConsoleOutput();
	#endif 

	// Calls the LAA window if needed
	if (laa.LAA_State != LAADialogState::NotShowing)
	{
		// Make cursor visible
		ImGui::GetIO().MouseDrawCursor = true;

		laa.LAARender();
	}

	// Calls the actual menu function
	if (bCfgMenuOpen)
	{
		// Make cursor visible
		ImGui::GetIO().MouseDrawCursor = true;

		//ImGui::ShowDemoWindow();
		MenuRender();
	}
	
	if (!bCfgMenuOpen && (laa.LAA_State == LAADialogState::NotShowing))
		ImGui::GetIO().MouseDrawCursor = false;

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return S_OK;
}

void Init_cfgMenu()
{
	auto pattern = hook::pattern("A1 ? ? ? ? 8B 08 8B 91 ? ? ? ? 50 FF D2 C6 05 ? ? ? ? ? A1");
	ptrD3D9Device = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct EndScene_HookStruct
	{
		void operator()(injector::reg_pack& regs)
		{
			auto pDevice = *(LPDIRECT3DDEVICE9*)(ptrD3D9Device);

			regs.eax = (uint32_t)pDevice;

			EndScene_hook(pDevice);
		}
	}; injector::MakeInline<EndScene_HookStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	
	// Reset hook 1
	pattern = hook::pattern("8B 08 8B 51 ? 68 ? ? ? ? 50 FF D2 85 C0 79 ? 89 35 ? ? ? ? 5E");
	struct Reset_HookStruct
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ecx = *(int32_t*)(regs.eax);
			regs.edx = *(int32_t*)(regs.ecx + 0x40);

			ImGui_ImplDX9_InvalidateDeviceObjects();
		}
	}; injector::MakeInline<Reset_HookStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Reset hook 2
	pattern = hook::pattern("8B 08 8B 51 ? 68 ? ? ? ? 50 FF D2 85 C0 79 ? 89 35 ? ? ? ? EB");
	injector::MakeInline<Reset_HookStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Input process hook
	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? 6A ? E8 ? ? ? ? 83 C4 ? E8");
	ptrInputProcess = injector::GetBranchDestination(pattern.get_first(0)).as_int();
	struct InputProcess
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!bCfgMenuOpen)
				_asm {call ptrInputProcess}
		}
	}; injector::MakeInline<InputProcess>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
}