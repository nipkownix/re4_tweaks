#pragma once
#include <d3d9.h>
#include <imgui.h>
#include "Settings.h"

extern HMODULE g_module_handle;
extern std::wstring wrapperName;
extern std::wstring rootPath;
extern std::wstring logPath;
extern HWND hWindow;

namespace re4t
{
	namespace init
	{
		// Init functions
		void AspectRatioTweaks();
		void AudioTweaks();
		void CameraTweaks();
		void CommandLine();
		void ControllerTweaks();
		void ConsoleWnd();
		void D3D9Hook();
		void DebugDisplay();
		void DisplayTweaks();
		void ExceptionHandler();
		void FrameRateFixes();
		void FilterXXFixes();
		void FrameRateFixes();
		bool Game();
		void HandleLimits();
		void HDProject();
		void KeyboardMouseTweaks();
		void MathReimpl();
		void Misc();
		void MouseTurning();
		void ModExpansion();
		void MultithreadFix();
		void QTEfixes();
		void Sofdec();
		void TitleMenu();
		void ToolMenu();
		void ToolMenuDebug();
		void WndProcHook();
	}
}

// Hotkey bindings and parsing
bool ParseConsoleKeyCombo(std::string_view in_combo);
bool ParseConfigMenuKeyCombo(std::string_view in_combo);
bool ParseMouseTurnModifierCombo(std::string_view in_combo);
bool ParseToolMenuKeyCombo(std::string_view in_combo);
bool ParseJetSkiTrickCombo(std::string_view in_combo);

// EndSceneHook
struct EndSceneHook
{
	void EndScene_hook(LPDIRECT3DDEVICE9 pDevice);

	std::chrono::high_resolution_clock::duration _last_frame_duration;
	std::chrono::high_resolution_clock::time_point _start_time;
	std::chrono::high_resolution_clock::time_point _last_present_time;

	float _cur_monitor_dpi = 1.0f;

	ImFont* ESP_font;

	ImGuiContext* _imgui_context = nullptr;
};

extern EndSceneHook esHook;
extern bool bRebuildFont;
bool ParseImGuiUIFocusCombo(std::string_view in_combo);

// cfgMenu
void cfgMenuRender();
void ShowCfgMenuTip();

extern bool bCfgMenuOpen;
extern bool bWaitingForHotkey;

extern float fLaserColorPicker[3];

extern int iGCBlurMode;

// LAApatch.cpp
void LAACheck();

// Controller vars
extern int* g_XInputDeadzone_LS;
extern int* g_XInputDeadzone_RS;

extern int8_t* AnalogRX_8;
extern int8_t* AnalogRY_9;

extern float* fAnalogLX;
extern float* fAnalogLY;
extern float* fAnalogRX;
extern float* fAnalogRY;

// Mouse vars
extern float* fMousePosX;
extern float* fMousePosY;
extern int32_t* MouseDeltaX;
extern int32_t* MouseDeltaY;

// Camera vars
extern float* fCameraPosX;
extern float* fCameraPosY;

extern float* wepPitch;