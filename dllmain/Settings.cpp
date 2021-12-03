#include <iostream>
#include "..\includes\stdafx.h"
#include "..\external\MinHook\MinHook.h"
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Settings.h"
#include "settings_string.h"

Settings cfg;

int Settings::KeyMap(char* key, const char* type)
{
	struct key_type {
		int vk;
		int dik;
	};

	std::map<std::string, key_type> key_map;

	key_map["ESCAPE"]       =   {0x1B, 0x01};    // Esc
    key_map["0"]            =   {0x30, 0x0B};    // 0
    key_map["1"]            =   {0x31, 0x02};    // 1
    key_map["2"]            =   {0x32, 0x03};    // 2
    key_map["3"]            =   {0x33, 0x04};    // 3
    key_map["4"]            =   {0x34, 0x05};    // 4
    key_map["5"]            =   {0x35, 0x06};    // 5
    key_map["6"]            =   {0x36, 0x07};    // 6
    key_map["7"]            =   {0x37, 0x08};    // 7
    key_map["8"]            =   {0x38, 0x09};    // 8
    key_map["9"]            =   {0x39, 0x0A};    // 9
    key_map["A"]            =   {0x41, 0x1E};    // A
    key_map["B"]            =   {0x42, 0x30};    // B
    key_map["C"]            =   {0x43, 0x2E};    // C
    key_map["D"]            =   {0x44, 0x20};    // D
    key_map["E"]            =   {0x45, 0x12};    // E
    key_map["F"]            =   {0x46, 0x21};    // F
    key_map["G"]            =   {0x47, 0x22};    // G
    key_map["H"]            =   {0x48, 0x23};    // H
    key_map["I"]            =   {0x49, 0x17};    // I
    key_map["J"]            =   {0x4A, 0x24};    // J
    key_map["K"]            =   {0x4B, 0x25};    // K
    key_map["L"]            =   {0x4C, 0x26};    // L
    key_map["M"]            =   {0x4D, 0x32};    // M
    key_map["N"]            =   {0x4E, 0x31};    // N
    key_map["O"]            =   {0x4F, 0x18};    // O
    key_map["P"]            =   {0x50, 0x19};    // P
    key_map["Q"]            =   {0x51, 0x10};    // Q
    key_map["R"]            =   {0x52, 0x13};    // R
    key_map["S"]            =   {0x53, 0x1F};    // S
    key_map["T"]            =   {0x54, 0x14};    // T
    key_map["U"]            =   {0x55, 0x16};    // U
    key_map["V"]            =   {0x56, 0x2F};    // V
    key_map["W"]            =   {0x57, 0x11};    // W
    key_map["X"]            =   {0x58, 0x2D};    // X
    key_map["Y"]            =   {0x59, 0x15};    // Y
    key_map["Z"]            =   {0x5A, 0x2C};    // Z
    key_map["BACK"]         =   {0x08, 0x0E};    // Backspace
    key_map["ADD"]          =   {0x6B, 0x4E};    // Numpad +
    key_map["DECIMAL"]      =   {0x6E, 0x53};    // Numpad .
    key_map["DIVIDE"]       =   {0x6F, 0xB5};    // Numpad /
    key_map["MULTIPLY"]     =   {0x6A, 0x37};    // Numpad *
    key_map["NUMPAD0"]      =   {0x60, 0x52};    // Numpad 0
    key_map["NUMPAD1"]      =   {0x61, 0x4F};    // Numpad 1
    key_map["NUMPAD2"]      =   {0x62, 0x50};    // Numpad 2
    key_map["NUMPAD3"]      =   {0x63, 0x51};    // Numpad 3
    key_map["NUMPAD4"]      =   {0x64, 0x4B};    // Numpad 4
    key_map["NUMPAD5"]      =   {0x65, 0x4C};    // Numpad 5
    key_map["NUMPAD6"]      =   {0x66, 0x4D};    // Numpad 6
    key_map["NUMPAD7"]      =   {0x67, 0x47};    // Numpad 7
    key_map["NUMPAD8"]      =   {0x68, 0x48};    // Numpad 8
    key_map["NUMPAD9"]      =   {0x69, 0x49};    // Numpad 9
    key_map["OEM_COMMA"]    =   {0xBC, 0x33};    // OEM_COMMA (< ,)
    key_map["OEM_MINUS"]    =   {0xBD, 0x0C};    // OEM_MINUS (_ -)
    key_map["OEM_PERIOD"]   =   {0xBE, 0x34};    // OEM_PERIOD (> .)
    key_map["OEM_PLUS"]     =   {0xBB, 0x0D};    // OEM_PLUS (+ =)
    key_map["RETURN"]       =   {0x0D, 0x1C};    // Enter
    key_map["SPACE"]        =   {0x20, 0x39};    // Space
    key_map["SUBTRACT"]     =   {0x6D, 0x4A};    // Num -
    key_map["TAB"]          =   {0x09, 0x0F};    // Tab
    key_map["APPS"]         =   {0x5D, 0xDD};    // Context Menu
    key_map["CAPSLOCK"]     =   {0x14, 0x3A};    // Caps Lock
    key_map["CONVERT"]      =   {0x1C, 0x79};    // Convert
    key_map["UP"]           =   {0x26, 0xC8};    // Arrow Up
    key_map["DOWN"]         =   {0x28, 0xD0};    // Arrow Down
    key_map["LEFT"]         =   {0x25, 0xCB};    // Arrow Left
    key_map["RIGHT"]        =   {0x27, 0xCD};    // Arrow Right
    key_map["F1"]           =   {0x70, 0x3B};    // F1
    key_map["F2"]           =   {0x71, 0x3C};    // F2
    key_map["F3"]           =   {0x72, 0x3D};    // F3
    key_map["F4"]           =   {0x73, 0x3E};    // F4
    key_map["F5"]           =   {0x74, 0x3F};    // F5
    key_map["F6"]           =   {0x75, 0x40};    // F6
    key_map["F7"]           =   {0x76, 0x41};    // F7
    key_map["F8"]           =   {0x77, 0x42};    // F8
    key_map["F9"]           =   {0x78, 0x43};    // F9
    key_map["F10"]          =   {0x79, 0x44};    // F10
    key_map["F11"]          =   {0x7A, 0x57};    // F11
    key_map["F12"]          =   {0x7B, 0x58};    // F12
    key_map["F13"]          =   {0x7C, 0x64};    // F13
    key_map["F14"]          =   {0x7D, 0x65};    // F14
    key_map["F15"]          =   {0x7E, 0x66};    // F15
    key_map["HOME"]         =   {0x24, 0xC7};    // Home
    key_map["INSERT"]       =   {0x2D, 0xD2};    // Insert
    key_map["DELETE"]       =   {0x2E, 0xD3};    // Delete
    key_map["END"]          =   {0x23, 0xCF};    // End
    key_map["PAGEUP"]       =   {0x21, 0xC9};    // Page Up
    key_map["PAGEDOWN"]     =   {0x22, 0xD1};    // Page Down
    key_map["KANA"]         =   {0x15, 0x70};    // Kana
    key_map["KANJI"]        =   {0x19, 0x94};    // Kanji
    key_map["LCONTROL"]     =   {0xA2, 0x1D};    // Left Ctrl
    key_map["LMENU"]        =   {0xA4, 0x38};    // Left Alt
    key_map["LSHIFT"]       =   {0xA0, 0x2A};    // Left Shift
    key_map["LWIN"]         =   {0x5B, 0xDB};    // Left Win
    key_map["NONCONVERT"]   =   {0x1D, 0x7B};    // Non Convert
    key_map["NUMLOCK"]      =   {0x90, 0x45};    // Num Lock
    key_map["PAUSE"]        =   {0x13, 0xC5};    // Pause
    key_map["RCONTROL"]     =   {0xA3, 0x9D};    // Right Ctrl
    key_map["RMENU"]        =   {0xA5, 0xB8};    // Right Alt
    key_map["RSHIFT"]       =   {0xA1, 0x36};    // Right Shift
    key_map["RWIN"]         =   {0x5C, 0xDC};    // Right Win
    key_map["SCROLL"]       =   {0x91, 0x46};    // Scrol Lock
    key_map["SLEEP"]        =   {0x5F, 0xDF};    // Sleep
    key_map["PRINTSCR"]     =   {0x2C, 0xB7};    // Print Screen

	if (type == "vk")
		return key_map[key].vk;
	else
		return key_map[key].dik;
}

void Settings::ReadSettings()
{
	CIniReader iniReader("");

	#ifdef VERBOSE
	con.AddLogChar("Reading settings");
	#endif

	cfg.HasUnsavedChanges = false;

	cfg.fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", 0.0f);
	if (cfg.fFOVAdditional > 0.0f)
		bEnableFOV = true;
	else
		bEnableFOV = false;

	cfg.bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", true);
	cfg.bFixVsyncToggle = iniReader.ReadBoolean("DISPLAY", "FixVsyncToggle", true);
	cfg.bRestorePickupTransparency = iniReader.ReadBoolean("DISPLAY", "RestorePickupTransparency", true);
	cfg.bDisableBrokenFilter03 = iniReader.ReadBoolean("DISPLAY", "DisableBrokenFilter03", true);
	cfg.bFixBlurryImage = iniReader.ReadBoolean("DISPLAY", "FixBlurryImage", true);
	cfg.bDisableFilmGrain = iniReader.ReadBoolean("DISPLAY", "DisableFilmGrain", true);
	cfg.bEnableGCBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCBlur", true);
	cfg.bEnableGCScopeBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCScopeBlur", true);
	cfg.bWindowBorderless = iniReader.ReadBoolean("DISPLAY", "WindowBorderless", false);
	cfg.iWindowPositionX = iniReader.ReadInteger("DISPLAY", "WindowPositionX", -1);
	cfg.iWindowPositionY = iniReader.ReadInteger("DISPLAY", "WindowPositionY", -1);
	cfg.bRememberWindowPos = iniReader.ReadBoolean("DISPLAY", "RememberWindowPos", false);
	cfg.b60fpsFixes = iniReader.ReadBoolean("MISC", "60fpsFixes", true);
	cfg.bFixQTE = iniReader.ReadBoolean("MISC", "FixQTE", true);
	cfg.bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", false);
	cfg.bEnableDebugMenu = iniReader.ReadBoolean("MISC", "EnableDebugMenu", false);
	cfg.bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", true);
	cfg.bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", true);
	cfg.flip_item_up = iniReader.ReadString("KEYBOARD", "flip_item_up", "HOME");
	cfg.flip_item_down = iniReader.ReadString("KEYBOARD", "flip_item_down", "END");
	cfg.flip_item_left = iniReader.ReadString("KEYBOARD", "flip_item_left", "INSERT");
	cfg.flip_item_right = iniReader.ReadString("KEYBOARD", "flip_item_right", "PAGEUP");
	cfg.QTE_key_1 = iniReader.ReadString("KEYBOARD", "QTE_key_1", "D");
	cfg.QTE_key_2 = iniReader.ReadString("KEYBOARD", "QTE_key_2", "A");
	cfg.bAllowHighResolutionSFD = iniReader.ReadBoolean("MOVIE", "AllowHighResolutionSFD", true);
	cfg.bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", true);
	cfg.bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", true);
	cfg.bUseMemcpy = iniReader.ReadBoolean("MEMORY", "UseMemcpy", true);
    cfg.bFixFPSSlowdown = iniReader.ReadBoolean("FRAME RATE", "FixSlowdown", false);
    cfg.bFixDisplayMode = iniReader.ReadBoolean("FRAME RATE", "FixDisplayMode", false);
	cfg.bIgnoreFPSWarning = iniReader.ReadBoolean("FRAME RATE", "IgnoreFPSWarning", false);
}

void Settings::WriteSettings()
{
	CIniReader iniReader("");

	#ifdef VERBOSE
	con.AddConcatLog("Writing settings to: ", iniPath.data());
	#endif

	// Copy the default .ini to folder if one doesn't exist, just so we can keep comments and descriptions intact.
	const char* filename = iniPath.c_str();
	if (!std::filesystem::exists(filename)) {
		#ifdef VERBOSE
		con.AddLogChar("ini file doesn't exist in folder. Creating new one.");
		#endif
		std::ofstream iniFile(iniPath);
		iniFile << defaultSettings;
		iniFile.close();
	}

	iniReader.WriteFloat("DISPLAY", "FOVAdditional", cfg.fFOVAdditional);
	iniReader.WriteBoolean("DISPLAY", "FixUltraWideAspectRatio", cfg.bFixUltraWideAspectRatio);
	iniReader.WriteBoolean("DISPLAY", "FixVsyncToggle", cfg.bFixVsyncToggle);
	iniReader.WriteBoolean("DISPLAY", "RestorePickupTransparency", cfg.bRestorePickupTransparency);
	iniReader.WriteBoolean("DISPLAY", "DisableBrokenFilter03", cfg.bDisableBrokenFilter03);
	iniReader.WriteBoolean("DISPLAY", "FixBlurryImage", cfg.bFixBlurryImage);
	iniReader.WriteBoolean("DISPLAY", "DisableFilmGrain", cfg.bDisableFilmGrain);
	iniReader.WriteBoolean("DISPLAY", "EnableGCBlur", cfg.bEnableGCBlur);
	iniReader.WriteBoolean("DISPLAY", "EnableGCScopeBlur", cfg.bEnableGCScopeBlur);
	iniReader.WriteBoolean("DISPLAY", "WindowBorderless", cfg.bWindowBorderless);
	iniReader.WriteInteger("DISPLAY", "WindowPositionX", cfg.iWindowPositionX);
	iniReader.WriteInteger("DISPLAY", "WindowPositionY", cfg.iWindowPositionY);
	iniReader.WriteBoolean("DISPLAY", "RememberWindowPos", cfg.bRememberWindowPos);
	iniReader.WriteBoolean("MISC", "FixQTE", cfg.bFixQTE);
	iniReader.WriteBoolean("MISC", "SkipIntroLogos", cfg.bSkipIntroLogos);
	iniReader.WriteBoolean("MISC", "EnableDebugMenu", cfg.bEnableDebugMenu);
	iniReader.WriteBoolean("MOUSE", "FixSniperZoom", cfg.bFixSniperZoom);
	iniReader.WriteBoolean("MOUSE", "FixRetryLoadMouseSelector", cfg.bFixRetryLoadMouseSelector);
	iniReader.WriteString("KEYBOARD", "flip_item_up", " " + cfg.flip_item_up);
	iniReader.WriteString("KEYBOARD", "flip_item_down", " " + cfg.flip_item_down);
	iniReader.WriteString("KEYBOARD", "flip_item_left", " " + cfg.flip_item_left);
	iniReader.WriteString("KEYBOARD", "flip_item_right", " " + cfg.flip_item_right);
	iniReader.WriteString("KEYBOARD", "QTE_key_1", " " + cfg.QTE_key_1);
	iniReader.WriteString("KEYBOARD", "QTE_key_2", " " + cfg.QTE_key_2);
	iniReader.WriteBoolean("MOVIE", "AllowHighResolutionSFD", cfg.bAllowHighResolutionSFD);
	iniReader.WriteBoolean("MEMORY", "RaiseVertexAlloc", cfg.bRaiseVertexAlloc);
	iniReader.WriteBoolean("MEMORY", "RaiseInventoryAlloc", cfg.bRaiseInventoryAlloc);
	iniReader.WriteBoolean("MEMORY", "UseMemcpy", cfg.bUseMemcpy);

	cfg.HasUnsavedChanges = false;
}
