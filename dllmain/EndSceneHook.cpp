#include <iostream>
#include "..\includes\stdafx.h"
#include <d3d9.h>
#include "WndProcHook.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "LAApatch.h"
#include "ToolMenu.h"
#include "..\external\imgui\imgui.h"
#include "..\external\imgui\imgui_internal.h"
#include "..\external\imgui\imgui_impl_win32.h"
#include "..\external\imgui\imgui_impl_dx9.h"
#include "..\external\imgui\imgui_stdlib.h"
#include <faprolight.hpp>
#include "MouseTurning.h"
#include "input.hpp"
#include "EndSceneHook.h"
#include "cfgMenu.h"

EndSceneHook esHook;

uintptr_t* ptrD3D9Device;

void ApplyImGuiTheme()
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

void ImGui_InputUpdate()
{
	ImGuiIO& io = ImGui::GetIO();

	// Update delta time
	io.DeltaTime = esHook._last_frame_duration.count() * 1e-9f;

	// Update mouse pos
	io.MousePos.x = static_cast<float>(_input->mouse_position_x());
	io.MousePos.y = static_cast<float>(_input->mouse_position_y());

	// Add wheel delta to the current absolute mouse wheel position
	io.MouseWheel += _input->mouse_wheel_delta();

	// Update all the button states
	io.KeyAlt = _input->is_key_down(0x12); // VK_MENU
	io.KeyCtrl = _input->is_key_down(0x11); // VK_CONTROL
	io.KeyShift = _input->is_key_down(0x10); // VK_SHIFT
	for (unsigned int i = 0; i < 256; i++)
		io.KeysDown[i] = _input->is_key_down(i);
	for (unsigned int i = 0; i < 5; i++)
		io.MouseDown[i] = _input->is_mouse_button_down(i);
	for (wchar_t c : _input->text_input())
		io.AddInputCharacter(c);
}

// Add our new code right before the game calls EndScene
HRESULT APIENTRY EndScene_hook(LPDIRECT3DDEVICE9 pDevice)
{
	static bool didInit = false;

	// Init ImGui
	if (!didInit)
	{
		didInit = true;

		esHook._start_time = std::chrono::high_resolution_clock::now();
		esHook._last_present_time = std::chrono::high_resolution_clock::now();
		esHook._last_frame_duration = std::chrono::milliseconds(1);

		esHook._imgui_context = ImGui::CreateContext();
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

		ApplyImGuiTheme();

		ImGui_ImplWin32_Init(hWindow);
		ImGui_ImplDX9_Init(pDevice);

		// Check if the exe is LAA
		if (!laa.GameIsLargeAddressAware())
		{
			laa.LAA_State = LAADialogState::Showing; // Show LAA patch prompt
		}
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	// Update ImGui input for this frame
	ImGui_InputUpdate();

	ImGui::NewFrame();

	// Check if the configuration menu binding has been pressed
	cfgMenuBinding();

	// Check if the console binding has been pressed
	ConsoleBinding();

	// Show cfgMenu tip
	if (!cfg.bDisableMenuTip && ((esHook._last_present_time - esHook._start_time) < std::chrono::seconds(10)))
		ShowCfgMenuTip();

	#ifdef VERBOSE
	// Show the console if in verbose
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
		cfgMenuRender();
	}

	if (!bCfgMenuOpen && (laa.LAA_State == LAADialogState::NotShowing))
		ImGui::GetIO().MouseDrawCursor = false;

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	_input->block_mouse_input(bCfgMenuOpen);
	_input->block_keyboard_input(bCfgMenuOpen);

	// Update _last_frame_duration and _last_present_time
	const auto current_time = std::chrono::high_resolution_clock::now();
	esHook._last_frame_duration = current_time - esHook._last_present_time;
	esHook._last_present_time = current_time;

	// Reset input status
	_input->next_frame();

	return S_OK;
}

void Init_EndSceneHook()
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
}