#include <iostream>
#include "..\includes\stdafx.h"
#include "..\external\MinHook\MinHook.h"
#include "dllmain.h"
#include "Settings.h"
#include "settings_string.h"

Settings cfg;

void Settings::ReadSettings()
{
	CIniReader iniReader("");

	#ifdef VERBOSE
	std::cout << "Reading settings" << std::endl;
	#endif

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
	cfg.bIgnoreFPSWarning = iniReader.ReadBoolean("FRAME RATE", "IgnoreFPSWarning", false);
}

void Settings::WriteSettings()
{
	CIniReader iniReader("");

	#ifdef VERBOSE
	std::cout << "Writing settings to \"" << iniPath << "\"" << std::endl;
	#endif

	// Copy the default .ini to folder if one doesn't exist, just so we can keep comments and descriptions intact.
	const char* filename = iniPath.c_str();
	if (!std::filesystem::exists(filename)) {
		#ifdef VERBOSE
		std::cout << "ini file doesn't exist in folder. Creating new one." << std::endl;
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
