#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Settings.h"
#include "settings_string.h"
#include "ToolMenu.h"
#include "MouseTurning.h"
#include "cfgMenu.h"

Settings cfg;

struct key_type {
	int vk;
	int dik;
};

std::unordered_map<std::string, key_type> key_map
{
	{ "ESCAPE",		{ 0x1B, 0x01 } },  // Esc
	{ "ESC",		{ 0x1B, 0x01 } },  // Esc
	{ "0",			{ 0x30, 0x0B } },  // 0
	{ "1",			{ 0x31, 0x02 } },  // 1
	{ "2",			{ 0x32, 0x03 } },  // 2
	{ "3",			{ 0x33, 0x04 } },  // 3
	{ "4",			{ 0x34, 0x05 } },  // 4
	{ "5",			{ 0x35, 0x06 } },  // 5
	{ "6",			{ 0x36, 0x07 } },  // 6
	{ "7",			{ 0x37, 0x08 } },  // 7
	{ "8",			{ 0x38, 0x09 } },  // 8
	{ "9",			{ 0x39, 0x0A } },  // 9
	{ "A",			{ 0x41, 0x1E } },  // A
	{ "B",			{ 0x42, 0x30 } },  // B
	{ "C",			{ 0x43, 0x2E } },  // C
	{ "D",			{ 0x44, 0x20 } },  // D
	{ "E",			{ 0x45, 0x12 } },  // E
	{ "F",			{ 0x46, 0x21 } },  // F
	{ "G",			{ 0x47, 0x22 } },  // G
	{ "H",			{ 0x48, 0x23 } },  // H
	{ "I",			{ 0x49, 0x17 } },  // I
	{ "J",			{ 0x4A, 0x24 } },  // J
	{ "K",			{ 0x4B, 0x25 } },  // K
	{ "L",			{ 0x4C, 0x26 } },  // L
	{ "M",			{ 0x4D, 0x32 } },  // M
	{ "N",			{ 0x4E, 0x31 } },  // N
	{ "O",			{ 0x4F, 0x18 } },  // O
	{ "P",			{ 0x50, 0x19 } },  // P
	{ "Q",			{ 0x51, 0x10 } },  // Q
	{ "R",			{ 0x52, 0x13 } },  // R
	{ "S",			{ 0x53, 0x1F } },  // S
	{ "T",			{ 0x54, 0x14 } },  // T
	{ "U",			{ 0x55, 0x16 } },  // U
	{ "V",			{ 0x56, 0x2F } },  // V
	{ "W",			{ 0x57, 0x11 } },  // W
	{ "X",			{ 0x58, 0x2D } },  // X
	{ "Y",			{ 0x59, 0x15 } },  // Y
	{ "Z",			{ 0x5A, 0x2C } },  // Z
	{ "BACK",		{ 0x08, 0x0E } },  // Backspace
	{ "BACKSPACE",  { 0x08, 0x0E } },  // Backspace
	{ "BKSP",		{ 0x08, 0x0E } },  // Backspace
	{ "ADD",		{ 0x6B, 0x4E } },  // Numpad +
	{ "NUM+",		{ 0x6B, 0x4E } },  // Numpad +
	{ "DECIMAL",	{ 0x6E, 0x53 } },  // Numpad .
	{ "NUM.",		{ 0x6E, 0x53 } },  // Numpad .
	{ "DIVIDE",		{ 0x6F, 0xB5 } },  // Numpad /
	{ "NUM/",		{ 0x6F, 0xB5 } },  // Numpad /
	{ "MULTIPLY",	{ 0x6A, 0x37 } },  // Numpad *
	{ "NUM*",		{ 0x6A, 0x37 } },  // Numpad *
	{ "SUBTRACT",	{ 0x6D, 0x4A } },  // Numpad -
	{ "NUM-",		{ 0x6D, 0x4A } },  // Numpad -
	{ "NUMPAD0",	{ 0x60, 0x52 } },  // Numpad 0
	{ "NUMPAD1",	{ 0x61, 0x4F } },  // Numpad 1
	{ "NUMPAD2",	{ 0x62, 0x50 } },  // Numpad 2
	{ "NUMPAD3",	{ 0x63, 0x51 } },  // Numpad 3
	{ "NUMPAD4",	{ 0x64, 0x4B } },  // Numpad 4
	{ "NUMPAD5",	{ 0x65, 0x4C } },  // Numpad 5
	{ "NUMPAD6",	{ 0x66, 0x4D } },  // Numpad 6
	{ "NUMPAD7",	{ 0x67, 0x47 } },  // Numpad 7
	{ "NUMPAD8",	{ 0x68, 0x48 } },  // Numpad 8
	{ "NUMPAD9",	{ 0x69, 0x49 } },  // Numpad 9
	{ "NUM0",		{ 0x60, 0x52 } },  // Numpad 0
	{ "NUM1",		{ 0x61, 0x4F } },  // Numpad 1
	{ "NUM2",		{ 0x62, 0x50 } },  // Numpad 2
	{ "NUM3",		{ 0x63, 0x51 } },  // Numpad 3
	{ "NUM4",		{ 0x64, 0x4B } },  // Numpad 4
	{ "NUM5",		{ 0x65, 0x4C } },  // Numpad 5
	{ "NUM6",		{ 0x66, 0x4D } },  // Numpad 6
	{ "NUM7",		{ 0x67, 0x47 } },  // Numpad 7
	{ "NUM8",		{ 0x68, 0x48 } },  // Numpad 8
	{ "NUM9",		{ 0x69, 0x49 } },  // Numpad 9
	{ "OEM_COMMA",  { 0xBC, 0x33 } },  // OEM_COMMA (< ,)
	{ "OEM_MINUS",  { 0xBD, 0x0C } },  // OEM_MINUS (_ -)
	{ "OEM_PERIOD", { 0xBE, 0x34 } },  // OEM_PERIOD (> .)
	{ "OEM_PLUS",	{ 0xBB, 0x0D } },  // OEM_PLUS (+ =)
	{ "<",			{ 0xBC, 0x33 } },  // OEM_COMMA (< ,)
	{ ",",			{ 0xBC, 0x33 } },  // OEM_COMMA (< ,)
	{ "_",			{ 0xBD, 0x0C } },  // OEM_MINUS (_ -)
	{ "-",			{ 0xBD, 0x0C } },  // OEM_MINUS (_ -)
	{ ">",			{ 0xBE, 0x34 } },  // OEM_PERIOD (> .)
	{ ".",			{ 0xBE, 0x34 } },  // OEM_PERIOD (> .)
	{ "+",			{ 0xBB, 0x0D } },  // OEM_PLUS (+ =)
	{ "=",			{ 0xBB, 0x0D } },  // OEM_PLUS (+ =)
	{ "RETURN",		{ 0x0D, 0x1C } },  // Enter
	{ "ENTER",		{ 0x0D, 0x1C } },  // Enter
	{ "RENTER",		{ 0x6C, 0x9C } },  // Right/Numpad Enter
	{ "SPACE",		{ 0x20, 0x39 } },  // Space
	{ "TAB",		{ 0x09, 0x0F } },  // Tab
	{ "APPS",		{ 0x5D, 0xDD } },  // Context Menu
	{ "CAPSLOCK",	{ 0x14, 0x3A } },  // Caps Lock
	{ "CONVERT",	{ 0x1C, 0x79 } },  // Convert
	{ "UP",			{ 0x26, 0xC8 } },  // Arrow Up
	{ "DOWN",		{ 0x28, 0xD0 } },  // Arrow Down
	{ "LEFT",		{ 0x25, 0xCB } },  // Arrow Left
	{ "RIGHT",		{ 0x27, 0xCD } },  // Arrow Right
	{ "F1",			{ 0x70, 0x3B } },  // F1
	{ "F2",			{ 0x71, 0x3C } },  // F2
	{ "F3",			{ 0x72, 0x3D } },  // F3
	{ "F4",			{ 0x73, 0x3E } },  // F4
	{ "F5",			{ 0x74, 0x3F } },  // F5
	{ "F6",			{ 0x75, 0x40 } },  // F6
	{ "F7",			{ 0x76, 0x41 } },  // F7
	{ "F8",			{ 0x77, 0x42 } },  // F8
	{ "F9",			{ 0x78, 0x43 } },  // F9
	{ "F10",		{ 0x79, 0x44 } },  // F10
	{ "F11",		{ 0x7A, 0x57 } },  // F11
	{ "F12",		{ 0x7B, 0x58 } },  // F12
	{ "F13",		{ 0x7C, 0x64 } },  // F13
	{ "F14",		{ 0x7D, 0x65 } },  // F14
	{ "F15",		{ 0x7E, 0x66 } },  // F15
	{ "HOME",		{ 0x24, 0xC7 } },  // Home
	{ "INSERT",		{ 0x2D, 0xD2 } },  // Insert
	{ "DELETE",		{ 0x2E, 0xD3 } },  // Delete
	{ "DEL",		{ 0x2E, 0xD3 } },  // Delete
	{ "END",		{ 0x23, 0xCF } },  // End
	{ "PAGEUP",		{ 0x21, 0xC9 } },  // Page Up
	{ "PAGEDOWN",	{ 0x22, 0xD1 } },  // Page Down
	{ "PGUP",		{ 0x21, 0xC9 } },  // Page Up
	{ "PGDOWN",		{ 0x22, 0xD1 } },  // Page Down
	{ "KANA",		{ 0x15, 0x70 } },  // Kana
	{ "KANJI",		{ 0x19, 0x94 } },  // Kanji
	{ "NONCONVERT",	{ 0x1D, 0x7B } },  // Non Convert
	{ "NUMLOCK",	{ 0x90, 0x45 } },  // Num Lock
	{ "PAUSE",		{ 0x13, 0xC5 } },  // Pause
	{ "SCROLL",		{ 0x91, 0x46 } },  // Scrol Lock
	{ "SLEEP",		{ 0x5F, 0xDF } },  // Sleep
	{ "PRINTSCR",	{ 0x2C, 0xB7 } },  // Print Screen

	// Control
	{ "CONTROL",	{ 0x11, 0x1D } },  // Ctrl
	{ "CTRL",		{ 0x11, 0x1D } },  // Ctrl
	{ "LCONTROL",	{ 0xA2, 0x1D } },  // Left Ctrl
	{ "LCTRL",		{ 0xA2, 0x1D } },  // Left Ctrl
	{ "RCONTROL",	{ 0xA3, 0x9D } },  // Right Ctrl
	{ "RCTRL",		{ 0xA3, 0x9D } },  // Right Ctrl

	// Alt
	{ "MENU",	{ 0x12, 0x38 } },  // Alt
	{ "LMENU",	{ 0xA4, 0x38 } },  // Left Alt
	{ "RMENU",	{ 0xA5, 0xB8 } },  // Right Alt
	{ "ALT",	{ 0x12, 0x38 } },  // Alt
	{ "LALT",	{ 0xA4, 0x38 } },  // Left Alt
	{ "RALT",	{ 0xA5, 0xB8 } },  // Right Alt
	{ "ALTGR",	{ 0xA5, 0xB8 } },  // Right Alt

	// Shift
	{ "SHIFT",	{ 0x10, 0x2A } },  // Shift
	{ "LSHIFT", { 0xA0, 0x2A } },  // Left Shift
	{ "RSHIFT", { 0xA1, 0x36 } },  // Right Shift
	
	// Winkey
	{ "LWIN",	{ 0x5B, 0xDB } },  // Left Win
	{ "RWIN",	{ 0x5C, 0xDC } },  // Right Win

	// VKs for localised keyboards
	{ ";",	{ VK_OEM_1, 0x27 } },
	{ ":",	{ VK_OEM_1, 0x27 } },
	{ "/",	{ VK_OEM_2, 0x35 } },
	{ "?",	{ VK_OEM_2, 0x35 } },
	{ "'",	{ VK_OEM_3, 0 } },  // UK keyboard
	{ "@",	{ VK_OEM_3, 0 } },  // UK keyboard
	{ "[",	{ VK_OEM_4, 0x1A } },
	{ "{",	{ VK_OEM_4, 0x1A } },
	{ "\\",	{ VK_OEM_5, 0x2B } },
	{ "|",	{ VK_OEM_5, 0x2B } },
	{ "]",	{ VK_OEM_6, 0x1B } },
	{ "}",	{ VK_OEM_6, 0x1B } },
	{ "#",	{ VK_OEM_7, 0 } },  // UK keyboard
	{ "\"", { VK_OEM_7, 0 } },  // UK keyboard
	{ "`",	{ VK_OEM_8, 0 } },  // UK keyboard

	// Mouse
	{ "LMOUSE", { VK_LBUTTON, 0 } },
	{ "RMOUSE", { VK_RBUTTON, 0 } },
	{ "MMOUSE", { VK_MBUTTON, 0 } },
	{ "LCLICK", { VK_LBUTTON, 0 } },
	{ "RCLICK", { VK_RBUTTON, 0 } },
	{ "MCLICK", { VK_MBUTTON, 0 } },
	{ "MOUSE1", { VK_LBUTTON, 0 } },
	{ "MOUSE2", { VK_RBUTTON, 0 } },
	{ "MOUSE3", { VK_MBUTTON, 0 } },
	{ "MOUSE4", { VK_XBUTTON1, 0 } },
	{ "MOUSE5", { VK_XBUTTON2, 0 } }
};

std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo)
{
	// Convert combo to uppercase to match Settings::key_map
	std::string combo(in_combo);
	std::transform(combo.begin(), combo.end(), combo.begin(),
		[](unsigned char c) { return std::toupper(c); });

	std::vector<uint32_t> new_combo;
	std::string cur_token;

	// Parse combo tokens into buttons bitfield (tokens seperated by any
	// non-alphabetical char, eg. +)
	for (size_t i = 0; i < combo.length(); i++)
	{
		char c = combo[i];

		if (!isalpha(c) && (c < 0x30 || c > 0x39) && c != '-')
		{
			// seperator, try parsing previous token

			if (cur_token.length())
			{
				uint32_t token_num = cfg.KeyMap(cur_token.c_str(), true);
				if (!token_num)
				{
					// parse failed...
					new_combo.clear();
					return new_combo;
				}
				new_combo.push_back(token_num);
			}

			cur_token.clear();
		}
		else
		{
			// Must be a character key, convert to upper and add to our cur_token
			cur_token += ::toupper(c);
		}
	}

	if (cur_token.length())
	{
		uint32_t token_num = cfg.KeyMap(cur_token.c_str(), true);
		if (!token_num)
		{
			// parse failed...
			new_combo.clear();
			return new_combo;
		}
		new_combo.push_back(token_num);
	}

	return new_combo;
}

int Settings::KeyMap(const char* key, bool get_vk)
{
	if (!key_map.count(key))
		return 0;

	if (get_vk)
		return key_map[key].vk;

	return key_map[key].dik;
}

void Settings::ReadSettings()
{
	CIniReader iniReader("");

	#ifdef VERBOSE
	con.AddLogChar("Reading settings");
	#endif

	cfg.HasUnsavedChanges = false;

	// DISPLAY
	cfg.fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", 0.0f);
	if (cfg.fFOVAdditional > 0.0f)
		cfg.bEnableFOV = true;
	else
		cfg.bEnableFOV = false;

	cfg.bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", true);
	cfg.bDisableVsync = iniReader.ReadBoolean("DISPLAY", "DisableVsync", false);
	cfg.bFixDisplayMode = iniReader.ReadBoolean("DISPLAY", "FixDisplayMode", true);
	cfg.iCustomRefreshRate = iniReader.ReadInteger("DISPLAY", "CustomRefreshRate", -1);
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

	// MISC
	cfg.sWrappedDllPath = iniReader.ReadString("MISC", "WrappedDLLPath", "");
	cfg.bAshleyJPCameraAngles = iniReader.ReadBoolean("MISC", "AshleyJPCameraAngles", false);
	cfg.bAllowSellingHandgunSilencer = iniReader.ReadBoolean("MISC", "AllowSellingHandgunSilencer", true);
	cfg.bAllowMafiaLeonCutscenes = iniReader.ReadBoolean("MISC", "AllowMafiaLeonCutscenes", true);
	cfg.bSilenceArmoredAshley = iniReader.ReadBoolean("MISC", "SilenceArmoredAshley", false);
	cfg.bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", false);
	cfg.bEnableDebugMenu = iniReader.ReadBoolean("MISC", "EnableDebugMenu", false);
	
	// CONTROLLER
	cfg.fControllerSensitivity = iniReader.ReadFloat("CONTROLLER", "ControllerSensitivity", 1.0f);
	if (cfg.fControllerSensitivity != 1.0f)
		cfg.bEnableControllerSens = true;
	else
		cfg.bEnableControllerSens = false;

	// MOUSE
	cfg.bUseMouseTurning = iniReader.ReadBoolean("MOUSE", "UseMouseTurning", true);
	cfg.fTurnSensitivity = iniReader.ReadFloat("MOUSE", "TurnSensitivity", 1.0f);
	cfg.bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", true);
	cfg.bFixSniperFocus = iniReader.ReadBoolean("MOUSE", "FixSniperFocus", true);
	cfg.bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", true);

	// KEYBOARD
	cfg.bFallbackToEnglishKeyIcons = iniReader.ReadBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", true);
	cfg.sFlipItemUp = iniReader.ReadString("KEYBOARD", "FlipItemUp", "HOME");
	cfg.sFlipItemDown = iniReader.ReadString("KEYBOARD", "FlipItemDown", "END");
	cfg.sFlipItemLeft = iniReader.ReadString("KEYBOARD", "FlipItemLeft", "INSERT");
	cfg.sFlipItemRight = iniReader.ReadString("KEYBOARD", "FlipItemRight", "PAGEUP");
	cfg.sQTE_key_1 = iniReader.ReadString("KEYBOARD", "QTE_key_1", "D");
	cfg.sQTE_key_2 = iniReader.ReadString("KEYBOARD", "QTE_key_2", "A");

	// FRAME RATE
	cfg.bFixFallingItemsSpeed = iniReader.ReadBoolean("FRAME RATE", "FixFallingItemsSpeed", true);
	cfg.bFixQTE = iniReader.ReadBoolean("FRAME RATE", "FixQTE", true);

	// MEMORY
	cfg.bAllowHighResolutionSFD = iniReader.ReadBoolean("MEMORY", "AllowHighResolutionSFD", true);
	cfg.bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", true);
	cfg.bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", true);

	// HOTKEYS
	cfg.sConfigMenuKeyCombo = iniReader.ReadString("HOTKEYS", "ConfigMenu", "F1");
	if (cfg.sConfigMenuKeyCombo.length())
		ParseConfigMenuKeyCombo(cfg.sConfigMenuKeyCombo);

	cfg.sConsoleKeyCombo = iniReader.ReadString("HOTKEYS", "Console", "F2");
	if (cfg.sConsoleKeyCombo.length())
	{
		ParseConsoleKeyCombo(cfg.sConsoleKeyCombo);

		// Update console title
		con.TitleKeyCombo = cfg.sConsoleKeyCombo;
	}

	cfg.sDebugMenuKeyCombo = iniReader.ReadString("HOTKEYS", "DebugMenu", "CTRL+F3");
	if (cfg.sDebugMenuKeyCombo.length())
		ParseToolMenuKeyCombo(cfg.sDebugMenuKeyCombo);

	cfg.sMouseTurnModifierKeyCombo = iniReader.ReadString("HOTKEYS", "MouseTurningModifier", "ALT");
	if (cfg.sMouseTurnModifierKeyCombo.length())
		ParseMouseTurnModifierCombo(cfg.sMouseTurnModifierKeyCombo);

	// FPS WARNING
	cfg.bIgnoreFPSWarning = iniReader.ReadBoolean("WARNING", "IgnoreFPSWarning", false);
	
	// IMGUI
	cfg.fFontSize = iniReader.ReadFloat("IMGUI", "FontSize", 1.0f);
	if (cfg.fFontSize < 1.0f)
		cfg.fFontSize = 1.0f;

	if (cfg.fFontSize > 1.3f)
		cfg.fFontSize = 1.3f;

	cfg.bDisableMenuTip = iniReader.ReadFloat("IMGUI", "DisableMenuTip", false);

	// LOG
	cfg.bVerboseLog = iniReader.ReadBoolean("LOG", "VerboseLog", true);
}

void Settings::WriteSettings()
{
	CIniReader iniReader("");

	std::string iniPath = rootPath + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".ini";

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
		iniFile << defaultSettings + 1; // +1 to skip the first new line
		iniFile.close();
	}

	// Try to remove read-only flag is it is set, for some reason.
	DWORD iniFile = GetFileAttributesA(iniPath.c_str());
	if (iniFile != INVALID_FILE_ATTRIBUTES) {
		bool isReadOnly = iniFile & FILE_ATTRIBUTE_READONLY;

		if (isReadOnly)
		{
			#ifdef VERBOSE
			con.AddLogChar("Read-only ini file detected. Attempting to remove flag.");
			#endif

			SetFileAttributesA(iniPath.c_str(), iniFile & ~FILE_ATTRIBUTE_READONLY);
		}
	}

	// DISPLAY
	iniReader.WriteFloat("DISPLAY", "FOVAdditional", cfg.fFOVAdditional);
	iniReader.WriteBoolean("DISPLAY", "FixUltraWideAspectRatio", cfg.bFixUltraWideAspectRatio);
	iniReader.WriteBoolean("DISPLAY", "DisableVsync", cfg.bDisableVsync);
	iniReader.WriteBoolean("DISPLAY", "FixDisplayMode", cfg.bFixDisplayMode);
	iniReader.WriteInteger("DISPLAY", "CustomRefreshRate", cfg.iCustomRefreshRate);
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

	// MISC
	iniReader.WriteBoolean("MISC", "AshleyJPCameraAngles", cfg.bAshleyJPCameraAngles);
	iniReader.WriteBoolean("MISC", "AllowSellingHandgunSilencer", cfg.bAllowSellingHandgunSilencer);
	iniReader.WriteBoolean("MISC", "AllowMafiaLeonCutscenes", cfg.bAllowMafiaLeonCutscenes);
	iniReader.WriteBoolean("MISC", "SilenceArmoredAshley", cfg.bSilenceArmoredAshley);
	iniReader.WriteBoolean("MISC", "SkipIntroLogos", cfg.bSkipIntroLogos);
	iniReader.WriteBoolean("MISC", "EnableDebugMenu", cfg.bEnableDebugMenu);

	// CONTROLLER
	iniReader.WriteFloat("CONTROLLER", "ControllerSensitivity", cfg.fControllerSensitivity);

	// MOUSE
	iniReader.WriteBoolean("MOUSE", "UseMouseTurning", cfg.bUseMouseTurning);
	iniReader.WriteFloat("MOUSE", "TurnSensitivity", cfg.fTurnSensitivity);
	iniReader.WriteBoolean("MOUSE", "FixSniperZoom", cfg.bFixSniperZoom);
	iniReader.WriteBoolean("MOUSE", "FixSniperFocus", cfg.bFixSniperFocus);
	iniReader.WriteBoolean("MOUSE", "FixRetryLoadMouseSelector", cfg.bFixRetryLoadMouseSelector);

	// KEYBOARD
	iniReader.WriteBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", cfg.bFallbackToEnglishKeyIcons);
	iniReader.WriteString("KEYBOARD", "FlipItemUp", " " + cfg.sFlipItemUp);
	iniReader.WriteString("KEYBOARD", "FlipItemDown", " " + cfg.sFlipItemDown);
	iniReader.WriteString("KEYBOARD", "FlipItemLeft", " " + cfg.sFlipItemLeft);
	iniReader.WriteString("KEYBOARD", "FlipItemRight", " " + cfg.sFlipItemRight);
	iniReader.WriteString("KEYBOARD", "QTE_key_1", " " + cfg.sQTE_key_1);
	iniReader.WriteString("KEYBOARD", "QTE_key_2", " " + cfg.sQTE_key_2);

	// FRAME RATE
	iniReader.WriteBoolean("FRAME RATE", "FixFallingItemsSpeed", cfg.bFixFallingItemsSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixQTE", cfg.bFixQTE);

	// MEMORY
	iniReader.WriteBoolean("MEMORY", "AllowHighResolutionSFD", cfg.bAllowHighResolutionSFD);
	iniReader.WriteBoolean("MEMORY", "RaiseVertexAlloc", cfg.bRaiseVertexAlloc);
	iniReader.WriteBoolean("MEMORY", "RaiseInventoryAlloc", cfg.bRaiseInventoryAlloc);

	// HOTKEYS
	iniReader.WriteString("HOTKEYS", "ConfigMenu", " " + cfg.sConfigMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "Console", " " + cfg.sConsoleKeyCombo);
	iniReader.WriteString("HOTKEYS", "DebugMenu", " " + cfg.sDebugMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "MouseTurningModifier", " " + cfg.sMouseTurnModifierKeyCombo);

	// IMGUI
	iniReader.WriteFloat("IMGUI", "FontSize", cfg.fFontSize);

	cfg.HasUnsavedChanges = false;
}
