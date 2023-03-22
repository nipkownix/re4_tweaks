#include <iostream>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Settings.h"
#include "AutoUpdater.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include "imgui\misc\cpp\imgui_stdlib.h"
#include <faprolight.hpp>
#include <FAhashes.h>
#include <IBMPlexSansJP_M.hpp>
#include "input.hpp"
#include "resource.h"
#include "UI_DebugWindows.h"
#include "Trainer.h"

EndSceneHook esHook;

std::vector<UI_Window*> DebugWindows; // Always-on-top debug windows, input only provided when cfgMenu is active
std::vector<UI_Window*> NewWindows; // Keep newly created windows in a seperate list, so they can be added after we've rendered current list

bool didInit = false;
bool bRebuildFont = false;
bool bImGuiUIFocus = false;

std::vector<uint32_t> KeyImGuiUIFocus;
bool ParseImGuiUIFocusCombo(std::string_view in_combo)
{
	if (in_combo.empty())
		return false;

	KeyImGuiUIFocus.clear();
	KeyImGuiUIFocus = re4t::cfg->ParseKeyCombo(in_combo);

	pInput->register_hotkey({ []() {
		bImGuiUIFocus = !bImGuiUIFocus;
	}, &KeyImGuiUIFocus });

	for (auto& window : DebugWindows)
		window->UpdateWindowTitle();

	return KeyImGuiUIFocus.size() > 0;
}

extern std::string cfgMenuTitle; // cfgMenu.cpp

void UI_NewEmManager(int selectedEmIndex)
{
	static int id = 0;
	std::string windowTitle = "Em Manager " + std::to_string(id++);
	NewWindows.push_back(new UI_EmManager(windowTitle, selectedEmIndex));

	bImGuiUIFocus = true;
}

void UI_NewGlobalsViewer()
{
	static int id = 0;
	std::string windowTitle = "Globals " + std::to_string(id++);
	NewWindows.push_back(new UI_Globals(windowTitle));

	bImGuiUIFocus = true;
}

void UI_NewIDInspector()
{
	static int id = 0;
	std::string windowTitle = "ID Inspector " + std::to_string(id++);
	NewWindows.push_back(new UI_IDInspector(windowTitle));

	bImGuiUIFocus = false;
}

void UI_NewAreaJump()
{
	static int id = 0;
	std::string windowTitle = "AreaJump " + std::to_string(id++);
	NewWindows.push_back(new UI_AreaJump(windowTitle));

	bImGuiUIFocus = true;
}

void UI_NewFilterTool()
{
	static int id = 0;
	std::string windowTitle = "FilterTool " + std::to_string(id++);
	NewWindows.push_back(new UI_FilterTool(windowTitle));

	bImGuiUIFocus = true;
}


void ApplyImGuiTheme()
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowBorderSize = 0.0f;
	style->WindowTitleAlign = ImVec2(0.5, 0.5);
	style->Alpha = 1.0f;
	style->WindowRounding = 1.0f;
	style->FramePadding = ImVec2(4, 3);
	style->WindowPadding = ImVec2(8, 8);
	style->ItemInnerSpacing = ImVec2(4, 4);
	style->ItemSpacing = ImVec2(8, 4);
	style->FrameRounding = 5.0f;
	style->ScrollbarSize = 10.f;
	style->ScrollbarRounding = 12.f;

	ImVec4* colors = ImGui::GetStyle().Colors;

	/*
	bool bAlternativeColors = true;
	if (bAlternativeColors)
	{
		// Not sure if this looks good enough. Meh.
		colors[ImGuiCol_Text] = ImVec4(0.86f, 0.93f, 0.89f, 0.78f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.86f, 0.93f, 0.89f, 0.28f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.13f, 0.14f, 0.17f, 0.94f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.14f, 0.15f, 0.19f, 0.98f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.90f);
		colors[ImGuiCol_Border] = ImVec4(0.54f, 0.48f, 0.25f, 0.16f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.46f, 0.20f, 0.30f, 0.78f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.46f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.23f, 0.20f, 0.27f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.50f, 0.08f, 0.26f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.22f, 0.27f, 0.75f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.20f, 0.22f, 0.27f, 0.47f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.22f, 0.27f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.09f, 0.15f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.46f, 0.20f, 0.30f, 0.78f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.46f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.90f, 0.17f, 0.34f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.59f, 0.16f, 0.26f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.83f, 0.10f, 0.18f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.59f, 0.16f, 0.26f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.20f, 0.30f, 0.86f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.50f, 0.04f, 0.23f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.46f, 0.20f, 0.30f, 0.76f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.46f, 0.20f, 0.30f, 0.86f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.08f, 0.26f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.47f, 0.77f, 0.83f, 0.04f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.46f, 0.20f, 0.30f, 0.78f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.45f, 0.20f, 0.30f, 0.76f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.72f, 0.09f, 0.35f, 0.76f);
		colors[ImGuiCol_TabActive] = ImVec4(0.59f, 0.16f, 0.26f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.46f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.86f, 0.93f, 0.89f, 0.63f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.46f, 0.20f, 0.30f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.46f, 0.20f, 0.30f, 0.43f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}
	else
	*/
	{
		colors[ImGuiCol_Button] = ImVec4(0.51f, 0.00f, 0.14f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.44f, 0.13f, 0.23f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.74f, 0.05f, 0.13f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.90f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.98f);
		colors[ImGuiCol_Border] = ImVec4(0.07f, 0.07f, 0.11f, 0.50f);
		colors[ImGuiCol_FrameBg] = ImColor(25, 25, 33, 255);
		colors[ImGuiCol_FrameBgActive] = ImColor(25, 25, 33, 255);
		colors[ImGuiCol_FrameBgHovered] = ImColor(25, 25, 33, 255);
		colors[ImGuiCol_Header] = ImVec4(0.52f, 0.05f, 0.12f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.68f, 0.08f, 0.23f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.80f, 0.00f, 0.10f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.05f, 0.03f, 0.04f, 0.94f);
		colors[ImGuiCol_Border] = ImVec4(0.17f, 0.17f, 0.20f, 0.20f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.13f, 0.23f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.13f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.78f, 0.15f, 0.28f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.96f, 0.00f, 0.20f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.96f, 0.00f, 0.20f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.76f, 0.46f, 0.53f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.96f, 0.00f, 0.20f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.96f, 0.00f, 0.20f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.96f, 0.00f, 0.20f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.31f, 0.45f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.96f, 0.00f, 0.20f, 0.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.96f, 0.00f, 0.20f, 0.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.96f, 0.00f, 0.20f, 0.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.49f, 0.15f, 0.22f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.41f, 0.53f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.96f, 0.00f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.95f, 0.41f, 0.53f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.00f, 0.13f, 0.32f, 1.00f);
	}
}

ImGuiKey VKToImGuiKey(int VK)
{
	switch (VK)
	{
	case VK_TAB: return ImGuiKey_Tab;
	case VK_LEFT: return ImGuiKey_LeftArrow;
	case VK_RIGHT: return ImGuiKey_RightArrow;
	case VK_UP: return ImGuiKey_UpArrow;
	case VK_DOWN: return ImGuiKey_DownArrow;
	case VK_PRIOR: return ImGuiKey_PageUp;
	case VK_NEXT: return ImGuiKey_PageDown;
	case VK_HOME: return ImGuiKey_Home;
	case VK_END: return ImGuiKey_End;
	case VK_INSERT: return ImGuiKey_Insert;
	case VK_DELETE: return ImGuiKey_Delete;
	case VK_BACK: return ImGuiKey_Backspace;
	case VK_SPACE: return ImGuiKey_Space;
	case VK_RETURN: return ImGuiKey_Enter;
	case VK_ESCAPE: return ImGuiKey_Escape;
	case VK_OEM_7: return ImGuiKey_Apostrophe;
	case VK_OEM_COMMA: return ImGuiKey_Comma;
	case VK_OEM_MINUS: return ImGuiKey_Minus;
	case VK_OEM_PERIOD: return ImGuiKey_Period;
	case VK_OEM_2: return ImGuiKey_Slash;
	case VK_OEM_1: return ImGuiKey_Semicolon;
	case VK_OEM_PLUS: return ImGuiKey_Equal;
	case VK_OEM_4: return ImGuiKey_LeftBracket;
	case VK_OEM_5: return ImGuiKey_Backslash;
	case VK_OEM_6: return ImGuiKey_RightBracket;
	case VK_OEM_3: return ImGuiKey_GraveAccent;
	case VK_CAPITAL: return ImGuiKey_CapsLock;
	case VK_SCROLL: return ImGuiKey_ScrollLock;
	case VK_NUMLOCK: return ImGuiKey_NumLock;
	case VK_SNAPSHOT: return ImGuiKey_PrintScreen;
	case VK_PAUSE: return ImGuiKey_Pause;
	case VK_NUMPAD0: return ImGuiKey_Keypad0;
	case VK_NUMPAD1: return ImGuiKey_Keypad1;
	case VK_NUMPAD2: return ImGuiKey_Keypad2;
	case VK_NUMPAD3: return ImGuiKey_Keypad3;
	case VK_NUMPAD4: return ImGuiKey_Keypad4;
	case VK_NUMPAD5: return ImGuiKey_Keypad5;
	case VK_NUMPAD6: return ImGuiKey_Keypad6;
	case VK_NUMPAD7: return ImGuiKey_Keypad7;
	case VK_NUMPAD8: return ImGuiKey_Keypad8;
	case VK_NUMPAD9: return ImGuiKey_Keypad9;
	case VK_DECIMAL: return ImGuiKey_KeypadDecimal;
	case VK_DIVIDE: return ImGuiKey_KeypadDivide;
	case VK_MULTIPLY: return ImGuiKey_KeypadMultiply;
	case VK_SUBTRACT: return ImGuiKey_KeypadSubtract;
	case VK_ADD: return ImGuiKey_KeypadAdd;
	case VK_LSHIFT: return ImGuiKey_LeftShift;
	case VK_LCONTROL: return ImGuiKey_LeftCtrl;
	case VK_LMENU: return ImGuiKey_LeftAlt;
	case VK_LWIN: return ImGuiKey_LeftSuper;
	case VK_RSHIFT: return ImGuiKey_RightShift;
	case VK_RCONTROL: return ImGuiKey_RightCtrl;
	case VK_RMENU: return ImGuiKey_RightAlt;
	case VK_RWIN: return ImGuiKey_RightSuper;
	case VK_APPS: return ImGuiKey_Menu;
	case '0': return ImGuiKey_0;
	case '1': return ImGuiKey_1;
	case '2': return ImGuiKey_2;
	case '3': return ImGuiKey_3;
	case '4': return ImGuiKey_4;
	case '5': return ImGuiKey_5;
	case '6': return ImGuiKey_6;
	case '7': return ImGuiKey_7;
	case '8': return ImGuiKey_8;
	case '9': return ImGuiKey_9;
	case 'A': return ImGuiKey_A;
	case 'B': return ImGuiKey_B;
	case 'C': return ImGuiKey_C;
	case 'D': return ImGuiKey_D;
	case 'E': return ImGuiKey_E;
	case 'F': return ImGuiKey_F;
	case 'G': return ImGuiKey_G;
	case 'H': return ImGuiKey_H;
	case 'I': return ImGuiKey_I;
	case 'J': return ImGuiKey_J;
	case 'K': return ImGuiKey_K;
	case 'L': return ImGuiKey_L;
	case 'M': return ImGuiKey_M;
	case 'N': return ImGuiKey_N;
	case 'O': return ImGuiKey_O;
	case 'P': return ImGuiKey_P;
	case 'Q': return ImGuiKey_Q;
	case 'R': return ImGuiKey_R;
	case 'S': return ImGuiKey_S;
	case 'T': return ImGuiKey_T;
	case 'U': return ImGuiKey_U;
	case 'V': return ImGuiKey_V;
	case 'W': return ImGuiKey_W;
	case 'X': return ImGuiKey_X;
	case 'Y': return ImGuiKey_Y;
	case 'Z': return ImGuiKey_Z;
	case VK_F1: return ImGuiKey_F1;
	case VK_F2: return ImGuiKey_F2;
	case VK_F3: return ImGuiKey_F3;
	case VK_F4: return ImGuiKey_F4;
	case VK_F5: return ImGuiKey_F5;
	case VK_F6: return ImGuiKey_F6;
	case VK_F7: return ImGuiKey_F7;
	case VK_F8: return ImGuiKey_F8;
	case VK_F9: return ImGuiKey_F9;
	case VK_F10: return ImGuiKey_F10;
	case VK_F11: return ImGuiKey_F11;
	case VK_F12: return ImGuiKey_F12;
	default: return ImGuiKey_None;
	}
}

void ImGuipInputUpdate()
{
	ImGuiIO& io = ImGui::GetIO();

	// Update delta time
	io.DeltaTime = esHook._last_frame_duration.count() * 1e-9f;

	// Update mouse pos
	io.AddMousePosEvent((float)pInput->mouse_position_x(), (float)pInput->mouse_position_y());

	// Add wheel delta to the current absolute mouse wheel position
	io.MouseWheel += pInput->mouse_wheel_delta();

	// Update all the button states
	io.AddKeyEvent(ImGuiKey_ModAlt, pInput->is_key_down(0x12)); // VK_MENU
	io.AddKeyEvent(ImGuiKey_ModCtrl, pInput->is_key_down(0x11)); // VK_CONTROL
	io.AddKeyEvent(ImGuiKey_ModShift, pInput->is_key_down(0x10)); // VK_SHIFT

	io.AddMouseButtonEvent(0, pInput->is_mouse_button_down(0));
	io.AddMouseButtonEvent(1, pInput->is_mouse_button_down(1));
	io.AddMouseButtonEvent(2, pInput->is_mouse_button_down(2));
	io.AddMouseButtonEvent(3, pInput->is_mouse_button_down(3));
	io.AddMouseButtonEvent(4, pInput->is_mouse_button_down(4));

	for (unsigned int i = 0; i < 256; i++)
		io.AddKeyEvent(VKToImGuiKey(i), pInput->is_key_down(i));

	for (wchar_t c : pInput->text_input())
		io.AddInputCharacter(c);
}

bool ImGuiShouldAcceptInput()
{
	if (DebugWindows.size() < 1 && !bConsoleOpen)
		bImGuiUIFocus = false;

	return (bCfgMenuOpen || bImGuiUIFocus || (updt.UpdateStatus != UpdateStatus::Finished));
}

void BuildFontAtlas()
{
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->Clear();

	float fFontSize = 18.1f; // Base size
	fFontSize *= re4t::cfg->fFontSizeScale;
	fFontSize *= esHook._cur_monitor_dpi;

	// Add IBM Plex Sans JP Medium for text
	static const ImWchar ranges[] = { 0x20, 0xFFFF, 0 };

	ImFontConfig ImCustomFont;
	ImCustomFont.FontDataOwnedByAtlas = false;

	io.Fonts->AddFontFromMemoryCompressedTTF(IBMPlexSansJP_M_compressed_data, IBMPlexSansJP_M_compressed_size, fFontSize, &ImCustomFont, ranges);

	// Add FontAwesome for icons
	static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	ImFontConfig ImCustomIcons;
	ImCustomIcons.MergeMode = true;
	ImCustomIcons.PixelSnapH = true;
	ImCustomIcons.GlyphOffset.y = 2;
	ImCustomIcons.FontDataOwnedByAtlas = false;

	io.Fonts->AddFontFromMemoryTTF(&FAprolight, sizeof FAprolight, fFontSize, &ImCustomIcons, icon_ranges);

	// Fixed-size copy for ESP
	esHook.ESP_font = io.Fonts->AddFontFromMemoryCompressedTTF(IBMPlexSansJP_M_compressed_data, IBMPlexSansJP_M_compressed_size, 18.1f, &ImCustomFont, ranges);

	// Build atlas
	io.Fonts->Build();
}

void Init_ImGui(LPDIRECT3DDEVICE9 pDevice)
{
	didInit = true;
	esHook._start_time = std::chrono::high_resolution_clock::now();
	esHook._last_present_time = std::chrono::high_resolution_clock::now();
	esHook._last_frame_duration = std::chrono::milliseconds(1);

	esHook._imgui_context = ImGui::CreateContext();

	if (re4t::cfg->bEnableDPIScale)
		esHook._cur_monitor_dpi = ImGui_ImplWin32_GetDpiScaleForHwnd(hWindow);

	ImGuiIO& io = ImGui::GetIO();

	static const std::string path = WstrToStr(rootPath) + "re4_tweaks/imgui.ini";
	io.IniFilename = path.c_str();

	BuildFontAtlas();

	ApplyImGuiTheme();

	ImGui_ImplWin32_Init(hWindow);
	ImGui_ImplDX9_Init(pDevice);

	// Update console window title
	con.TitleKeyCombo = re4t::cfg->sConsoleKeyCombo;

	// Update cfgMenu window title
	cfgMenuTitle.append(" v");
	cfgMenuTitle.append(APP_VERSION);

	if (TweaksDevMode)
	{
		UI_NewGlobalsViewer();
		bImGuiUIFocus = false;
	}
}

// Add our new code right before the game calls EndScene
void EndSceneHook::EndScene_hook(LPDIRECT3DDEVICE9 pDevice)
{
	// Init ImGui
	if (!didInit)
		Init_ImGui(pDevice);

	// Rebuild font if needed
	if (bRebuildFont)
	{
		BuildFontAtlas();
		ImGui_ImplDX9_InvalidateDeviceObjects();
		bRebuildFont = false;
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	// Update ImGui input for this frame
	if (ImGuiShouldAcceptInput())
		ImGuipInputUpdate();

	ImGui::GetIO().SetAppAcceptingEvents(ImGuiShouldAcceptInput());

	ImGui::NewFrame();

	Trainer_Update();

	// Show cfgMenu tip
	if (!re4t::cfg->bDisableMenuTip && ((esHook._last_present_time - esHook._start_time) < std::chrono::seconds(10)))
		ShowCfgMenuTip();

	if (ShowDebugTrgHint)
	{
		if (re4t::cfg->bTrainerShowDebugTrgHintText)
			Trainer_DrawDebugTrgHint();
		ShowDebugTrgHint = false; // always reset ShowDebugTrgHint regardless of ShowHintText value, otherwise it could appear next time user enables option...
	}

	// Show the console window
	bool verbose = false;
	#ifdef VERBOSE
	verbose = true;
	#endif

	if ((verbose || re4t::cfg->bShowGameOutput) && bConsoleOpen)
	{
		con.Render();
	}

	// Show the cfgMenu
	if (bCfgMenuOpen)
	{
		//ImGui::ShowDemoWindow();
		cfgMenuRender();
	}

	// Render any active always-on-top debug windows
	auto it = DebugWindows.begin();
	while (it != DebugWindows.end())
	{
		auto* window = *it;
		bool remainVisible = window->Render(true);
		if (!remainVisible)
		{
			delete window;
			it = DebugWindows.erase(it);
		}
		else
			++it;
	}

	// Now we've finished rendering, add any newly created windows to our main list
	if (NewWindows.size())
	{
		auto newWindows = NewWindows; // make copy of NewWindows vect before processing, in case any new windows modify NewWindows vector...
		NewWindows.clear();
		for (auto window : newWindows)
		{
			if(window->Init())
				DebugWindows.push_back(window);
		}
	}

	// Draw trainer ESP
	Trainer_ESP();

	// Show update dialog if needed
	if (updt.UpdateStatus != UpdateStatus::Finished)
	{
		if ((esHook._last_present_time - esHook._start_time) > std::chrono::seconds(3))
			updt.RenderUI();
	}

	// Show cursor if needed
	ImGui::GetIO().MouseDrawCursor = ImGuiShouldAcceptInput();

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	pInput->block_mouse_input(ImGuiShouldAcceptInput(), false);
	pInput->block_keyboard_input(ImGuiShouldAcceptInput());

	// Update _last_frame_duration and _last_present_time
	const auto current_time = std::chrono::high_resolution_clock::now();
	esHook._last_frame_duration = current_time - esHook._last_present_time;
	esHook._last_present_time = current_time;

	// Reset mouse wheel delta and _text_input
	pInput->imgui_next_frame();

	return;
}
