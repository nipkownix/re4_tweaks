#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include "stdafx.h"
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Settings.h"
#include "settings_string.h"
#include "MouseTurning.h"
#include "cfgMenu.h"
#include "Patches.h"
#include "input.hpp"

Settings cfg;

const std::string sSettingOverridesPath = "re4_tweaks\\setting_overrides\\";
const std::string sHDProjectOverrideName = "HDProject.ini";

const char* sLeonCostumeNames[] = {"Jacket", "Normal", "Vest", "RPD", "Mafia"};
const char* sAshleyCostumeNames[] = {"Normal", "Popstar", "Armor"};
const char* sAdaCostumeNames[] = {"RE2", "Spy", "Normal"};

std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo)
{
	// Convert combo to uppercase to match Settings::key_map
	std::string combo(in_combo);
	std::transform(combo.begin(), combo.end(), combo.begin(),
		[](unsigned char c) { return std::toupper(c); });

	std::vector<uint32_t> new_combo;
	std::string cur_token;

	// Parse combo tokens into buttons bitfield (tokens seperated by "+")
	for (size_t i = 0; i < combo.length(); i++)
	{
		char c = combo[i];

		if (c == '+')
		{
			// seperator, try parsing previous token

			if (cur_token.length())
			{
				uint32_t token_num = _input->KeyMap_getVK(cur_token);
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
		// Get VK for the current token and push it into the vector
		uint32_t token_num = _input->KeyMap_getVK(cur_token);
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

void Settings::ReadSettings()
{
	// Read default settings file first
	std::string sDefaultIniPath = rootPath + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".ini";
	ReadSettings(sDefaultIniPath);

	// Try reading any setting override files
	auto override_path = rootPath + sSettingOverridesPath;

	if (!std::filesystem::exists(override_path))
		return;

	std::vector<std::filesystem::path> override_inis;
	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(override_path))
	{
		auto& path = dirEntry.path();

		std::string ext = path.extension().string();
		std::transform(ext.begin(), ext.end(), ext.begin(),
			[](unsigned char c) { return std::tolower(c); });

		if (ext != ".ini")
			continue;

		override_inis.push_back(path);
	}

	// Sort INI filenames alphabetically
	std::sort(override_inis.begin(), override_inis.end());

	// Process override INIs
	for (const auto& path : override_inis)
		ReadSettings(path.string());

	// Special case for HDProject settings, make sure it overrides all other INIs
	auto hdproject_path = override_path + sHDProjectOverrideName;
	if (std::filesystem::exists(hdproject_path))
		ReadSettings(hdproject_path);
}

void Settings::ReadSettings(std::string_view ini_path)
{
	CIniReader iniReader(ini_path);

	#ifdef VERBOSE
	con.AddLogChar("Reading settings from %s", ini_path.data());
	#endif

	cfg.HasUnsavedChanges = false;

	// DISPLAY
	cfg.fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", cfg.fFOVAdditional);
	if (cfg.fFOVAdditional > 0.0f)
		cfg.bEnableFOV = true;
	else
		cfg.bEnableFOV = false;

	cfg.bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", cfg.bFixUltraWideAspectRatio);
	cfg.bDisableVsync = iniReader.ReadBoolean("DISPLAY", "DisableVsync", cfg.bDisableVsync);
	cfg.bFixDPIScale = iniReader.ReadBoolean("DISPLAY", "FixDPIScale", cfg.bFixDPIScale);
	cfg.bFixDisplayMode = iniReader.ReadBoolean("DISPLAY", "FixDisplayMode", cfg.bFixDisplayMode);
	cfg.iCustomRefreshRate = iniReader.ReadInteger("DISPLAY", "CustomRefreshRate", cfg.iCustomRefreshRate);
	cfg.bOverrideLaserColor = iniReader.ReadBoolean("DISPLAY", "OverrideLaserColor", cfg.bOverrideLaserColor);
	cfg.bRainbowLaser = iniReader.ReadBoolean("DISPLAY", "RainbowLaser", cfg.bRainbowLaser);

	cfg.fLaserRGB[0] = iniReader.ReadFloat("DISPLAY", "LaserR", cfg.fLaserR) / 255.0f;
	cfg.fLaserRGB[1] = iniReader.ReadFloat("DISPLAY", "LaserG", cfg.fLaserG) / 255.0f;
	cfg.fLaserRGB[2] = iniReader.ReadFloat("DISPLAY", "LaserB", cfg.fLaserB) / 255.0f;

	cfg.bRestorePickupTransparency = iniReader.ReadBoolean("DISPLAY", "RestorePickupTransparency", cfg.bRestorePickupTransparency);
	cfg.bDisableBrokenFilter03 = iniReader.ReadBoolean("DISPLAY", "DisableBrokenFilter03", cfg.bDisableBrokenFilter03);
	cfg.bFixBlurryImage = iniReader.ReadBoolean("DISPLAY", "FixBlurryImage", cfg.bFixBlurryImage);
	cfg.bDisableFilmGrain = iniReader.ReadBoolean("DISPLAY", "DisableFilmGrain", cfg.bDisableFilmGrain);
	cfg.bEnableGCBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCBlur", cfg.bEnableGCBlur);
	cfg.bEnableGCScopeBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCScopeBlur", cfg.bEnableGCScopeBlur);
	cfg.bWindowBorderless = iniReader.ReadBoolean("DISPLAY", "WindowBorderless", cfg.bWindowBorderless);
	cfg.iWindowPositionX = iniReader.ReadInteger("DISPLAY", "WindowPositionX", cfg.iWindowPositionX);
	cfg.iWindowPositionY = iniReader.ReadInteger("DISPLAY", "WindowPositionY", cfg.iWindowPositionY);
	cfg.bRememberWindowPos = iniReader.ReadBoolean("DISPLAY", "RememberWindowPos", cfg.bRememberWindowPos);

	// AUDIO
	cfg.iVolumeMaster = iniReader.ReadInteger("AUDIO", "VolumeMaster", cfg.iVolumeMaster);
	cfg.iVolumeMaster = min(max(cfg.iVolumeMaster, 0), 100); // limit between 0 - 100

	cfg.iVolumeBGM = iniReader.ReadInteger("AUDIO", "VolumeBGM", cfg.iVolumeBGM);
	cfg.iVolumeBGM = min(max(cfg.iVolumeBGM, 0), 100); // limit between 0 - 100

	cfg.iVolumeSE = iniReader.ReadInteger("AUDIO", "VolumeSE", cfg.iVolumeSE);
	cfg.iVolumeSE = min(max(cfg.iVolumeSE, 0), 100); // limit between 0 - 100

	cfg.iVolumeCutscene = iniReader.ReadInteger("AUDIO", "VolumeCutscene", cfg.iVolumeCutscene);
	cfg.iVolumeCutscene = min(max(cfg.iVolumeCutscene, 0), 100); // limit between 0 - 100

	// MOUSE
	cfg.bUseMouseTurning = iniReader.ReadBoolean("MOUSE", "UseMouseTurning", cfg.bUseMouseTurning);
	cfg.fTurnSensitivity = iniReader.ReadFloat("MOUSE", "TurnSensitivity", cfg.fTurnSensitivity);
	cfg.bUseRawMouseInput = iniReader.ReadBoolean("MOUSE", "UseRawMouseInput", cfg.bUseRawMouseInput);
	cfg.bUnlockCameraFromAim = iniReader.ReadBoolean("MOUSE", "UnlockCameraFromAim", cfg.bUnlockCameraFromAim);
	cfg.bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", cfg.bFixSniperZoom);
	cfg.bFixSniperFocus = iniReader.ReadBoolean("MOUSE", "FixSniperFocus", cfg.bFixSniperFocus);
	cfg.bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", cfg.bFixRetryLoadMouseSelector);

	// KEYBOARD
	cfg.bFallbackToEnglishKeyIcons = iniReader.ReadBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", cfg.bFallbackToEnglishKeyIcons);

	// CONTROLLER
	cfg.bOverrideControllerSensitivity = iniReader.ReadBoolean("CONTROLLER", "OverrideControllerSensitivity", cfg.bOverrideControllerSensitivity);
	cfg.fControllerSensitivity = iniReader.ReadFloat("CONTROLLER", "ControllerSensitivity", cfg.fControllerSensitivity);
	cfg.fControllerSensitivity = fmin(fmax(cfg.fControllerSensitivity, 0.5f), 4.0f); // limit between 0.5 - 4.0

	cfg.bRemoveExtraXinputDeadzone = iniReader.ReadBoolean("CONTROLLER", "RemoveExtraXinputDeadzone", cfg.bRemoveExtraXinputDeadzone);

	cfg.bOverrideXinputDeadzone = iniReader.ReadBoolean("CONTROLLER", "OverrideXinputDeadzone", cfg.bOverrideXinputDeadzone);
	cfg.fXinputDeadzone = iniReader.ReadFloat("CONTROLLER", "XinputDeadzone", cfg.fXinputDeadzone);
	cfg.fXinputDeadzone = fmin(fmax(cfg.fXinputDeadzone, 0.0f), 3.5f); // limit between 0.0 - 3.5

	// FRAME RATE
	cfg.bFixFallingItemsSpeed = iniReader.ReadBoolean("FRAME RATE", "FixFallingItemsSpeed", cfg.bFixFallingItemsSpeed);
	cfg.bFixTurningSpeed = iniReader.ReadBoolean("FRAME RATE", "FixTurningSpeed", cfg.bFixTurningSpeed);
	cfg.bFixQTE = iniReader.ReadBoolean("FRAME RATE", "FixQTE", cfg.bFixQTE);
	cfg.bFixAshleyBustPhysics = iniReader.ReadBoolean("FRAME RATE", "FixAshleyBustPhysics", cfg.bFixAshleyBustPhysics);
	cfg.bEnableFastMath = iniReader.ReadBoolean("FRAME RATE", "EnableFastMath", cfg.bEnableFastMath);
	cfg.bPrecacheModels = iniReader.ReadBoolean("FRAME RATE", "PrecacheModels", cfg.bPrecacheModels);

	// MISC
	cfg.bOverrideCostumes = iniReader.ReadBoolean("MISC", "OverrideCostumes", cfg.bOverrideCostumes);

	std::string buf = iniReader.ReadString("MISC", "LeonCostume", "");
	if (!buf.empty())
	{
		if (buf == "Jacket") cfg.CostumeOverride.Leon = LeonCostumes::Jacket;
		if (buf == "Normal") cfg.CostumeOverride.Leon = LeonCostumes::Normal;
		if (buf == "Vest") cfg.CostumeOverride.Leon = LeonCostumes::Vest;
		if (buf == "RPD") cfg.CostumeOverride.Leon = LeonCostumes::RPD;
		if (buf == "Mafia") cfg.CostumeOverride.Leon = LeonCostumes::Mafia;
	}

	buf = iniReader.ReadString("MISC", "AshleyCostume", "");
	if (!buf.empty())
	{
		if (buf == "Normal") cfg.CostumeOverride.Ashley = AshleyCostumes::Normal;
		if (buf == "Popstar") cfg.CostumeOverride.Ashley = AshleyCostumes::Popstar;
		if (buf == "Armor") cfg.CostumeOverride.Ashley = AshleyCostumes::Armor;
	}

	buf = iniReader.ReadString("MISC", "AdaCostume", "");
	if (!buf.empty())
	{
		if (buf == "RE2") cfg.CostumeOverride.Ada = AdaCostumes::RE2;
		if (buf == "Spy") cfg.CostumeOverride.Ada = AdaCostumes::Spy;
		if (buf == "Normal") cfg.CostumeOverride.Ada = AdaCostumes::Normal;
	}

	iCostumeComboLeon = (int)cfg.CostumeOverride.Leon;
	iCostumeComboAshley = (int)cfg.CostumeOverride.Ashley;
	iCostumeComboAda = (int)cfg.CostumeOverride.Ada;

	cfg.bAshleyJPCameraAngles = iniReader.ReadBoolean("MISC", "AshleyJPCameraAngles", cfg.bAshleyJPCameraAngles);

	cfg.iViolenceLevelOverride = iniReader.ReadInteger("MISC", "ViolenceLevelOverride", cfg.iViolenceLevelOverride);
	cfg.iViolenceLevelOverride = min(max(cfg.iViolenceLevelOverride, -1), 2); // limit between -1 to 2

	cfg.bAllowSellingHandgunSilencer = iniReader.ReadBoolean("MISC", "AllowSellingHandgunSilencer", cfg.bAllowSellingHandgunSilencer);
	cfg.bAllowMafiaLeonCutscenes = iniReader.ReadBoolean("MISC", "AllowMafiaLeonCutscenes", cfg.bAllowMafiaLeonCutscenes);
	cfg.bSilenceArmoredAshley = iniReader.ReadBoolean("MISC", "SilenceArmoredAshley", cfg.bSilenceArmoredAshley);
	cfg.bAllowAshleySuplex = iniReader.ReadBoolean("MISC", "AllowAshleySuplex", cfg.bAllowAshleySuplex);
	cfg.bAllowMatildaQuickturn = iniReader.ReadBoolean("MISC", "AllowMatildaQuickturn", cfg.bAllowMatildaQuickturn);
	cfg.bFixDitmanGlitch = iniReader.ReadBoolean("MISC", "FixDitmanGlitch", cfg.bFixDitmanGlitch);
	cfg.bUseSprintToggle = iniReader.ReadBoolean("MISC", "UseSprintToggle", cfg.bUseSprintToggle);
	cfg.bDisableQTE = iniReader.ReadBoolean("MISC", "DisableQTE", cfg.bDisableQTE);
	cfg.bAutomaticMashingQTE = iniReader.ReadBoolean("MISC", "AutomaticMashingQTE", cfg.bAutomaticMashingQTE);
	cfg.bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", cfg.bSkipIntroLogos);
	cfg.bEnableDebugMenu = iniReader.ReadBoolean("MISC", "EnableDebugMenu", cfg.bEnableDebugMenu);

	// MEMORY
	cfg.bAllowHighResolutionSFD = iniReader.ReadBoolean("MEMORY", "AllowHighResolutionSFD", cfg.bAllowHighResolutionSFD);
	cfg.bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", cfg.bRaiseVertexAlloc);
	cfg.bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", cfg.bRaiseInventoryAlloc);

	// HOTKEYS
	cfg.sConfigMenuKeyCombo = iniReader.ReadString("HOTKEYS", "ConfigMenu", cfg.sConfigMenuKeyCombo);
	if (cfg.sConfigMenuKeyCombo.length())
		ParseConfigMenuKeyCombo(cfg.sConfigMenuKeyCombo);

	cfg.sConsoleKeyCombo = iniReader.ReadString("HOTKEYS", "Console", cfg.sConsoleKeyCombo);
	if (cfg.sConsoleKeyCombo.length())
	{
		ParseConsoleKeyCombo(cfg.sConsoleKeyCombo);

		// Update console title
		con.TitleKeyCombo = cfg.sConsoleKeyCombo;
	}

	cfg.sFlipItemUp = iniReader.ReadString("HOTKEYS", "FlipItemUp", cfg.sFlipItemUp);
	cfg.sFlipItemDown = iniReader.ReadString("HOTKEYS", "FlipItemDown", cfg.sFlipItemDown);
	cfg.sFlipItemLeft = iniReader.ReadString("HOTKEYS", "FlipItemLeft", cfg.sFlipItemLeft);
	cfg.sFlipItemRight = iniReader.ReadString("HOTKEYS", "FlipItemRight", cfg.sFlipItemRight);
	cfg.sQTE_key_1 = iniReader.ReadString("HOTKEYS", "QTE_key_1", cfg.sQTE_key_1);
	cfg.sQTE_key_2 = iniReader.ReadString("HOTKEYS", "QTE_key_2", cfg.sQTE_key_2);

	cfg.sDebugMenuKeyCombo = iniReader.ReadString("HOTKEYS", "DebugMenu", cfg.sDebugMenuKeyCombo);
	if (cfg.sDebugMenuKeyCombo.length())
		ParseToolMenuKeyCombo(cfg.sDebugMenuKeyCombo);

	cfg.sMouseTurnModifierKeyCombo = iniReader.ReadString("HOTKEYS", "MouseTurningModifier", cfg.sMouseTurnModifierKeyCombo);
	if (cfg.sMouseTurnModifierKeyCombo.length())
		ParseMouseTurnModifierCombo(cfg.sMouseTurnModifierKeyCombo);

	cfg.sJetSkiTrickCombo = iniReader.ReadString("HOTKEYS", "JetSkiTricks", cfg.sJetSkiTrickCombo);
	if (cfg.sJetSkiTrickCombo.length())
		ParseJetSkiTrickCombo(cfg.sJetSkiTrickCombo);

	// FPS WARNING
	cfg.bIgnoreFPSWarning = iniReader.ReadBoolean("WARNING", "IgnoreFPSWarning", cfg.bIgnoreFPSWarning);
	
	// IMGUI
	cfg.fFontSize = iniReader.ReadFloat("IMGUI", "FontSize", cfg.fFontSize);
	cfg.fFontSize = fmin(fmax(cfg.fFontSize, 1.0f), 1.25f); // limit between 1.0 - 1.25

	cfg.bDisableMenuTip = iniReader.ReadBoolean("IMGUI", "DisableMenuTip", cfg.bDisableMenuTip);

	// DEBUG
	cfg.bVerboseLog = iniReader.ReadBoolean("DEBUG", "VerboseLog", cfg.bVerboseLog);
	cfg.bNeverHideCursor = iniReader.ReadBoolean("DEBUG", "NeverHideCursor", cfg.bNeverHideCursor);
}

std::mutex settingsThreadRunningMutex;

DWORD WINAPI WriteSettingsThread(LPVOID lpParameter)
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // only allow single thread writing to INI at one time

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

			//Logging::Log() << __FUNCTION__ << " -> Read-only ini file detected. Attempting to remove flag.";

			SetFileAttributesA(iniPath.c_str(), iniFile & ~FILE_ATTRIBUTE_READONLY);
		}
	}

	// DISPLAY
	iniReader.WriteFloat("DISPLAY", "FOVAdditional", cfg.fFOVAdditional);
	iniReader.WriteBoolean("DISPLAY", "FixUltraWideAspectRatio", cfg.bFixUltraWideAspectRatio);
	iniReader.WriteBoolean("DISPLAY", "DisableVsync", cfg.bDisableVsync);
	iniReader.WriteBoolean("DISPLAY", "FixDPIScale", cfg.bFixDPIScale);
	iniReader.WriteBoolean("DISPLAY", "FixDisplayMode", cfg.bFixDisplayMode);
	iniReader.WriteInteger("DISPLAY", "CustomRefreshRate", cfg.iCustomRefreshRate);
	iniReader.WriteBoolean("DISPLAY", "OverrideLaserColor", cfg.bOverrideLaserColor);
	iniReader.WriteBoolean("DISPLAY", "RainbowLaser", cfg.bRainbowLaser);

	iniReader.WriteFloat("DISPLAY", "LaserR", cfg.fLaserRGB[0] * 255.0f);
	iniReader.WriteFloat("DISPLAY", "LaserG", cfg.fLaserRGB[1] * 255.0f);
	iniReader.WriteFloat("DISPLAY", "LaserB", cfg.fLaserRGB[2] * 255.0f);

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

	// AUDIO
	iniReader.WriteInteger("AUDIO", "VolumeMaster", cfg.iVolumeMaster);
	iniReader.WriteInteger("AUDIO", "VolumeBGM", cfg.iVolumeBGM);
	iniReader.WriteInteger("AUDIO", "VolumeSE", cfg.iVolumeSE);
	iniReader.WriteInteger("AUDIO", "VolumeCutscene", cfg.iVolumeCutscene);

	// MOUSE
	iniReader.WriteBoolean("MOUSE", "UseMouseTurning", cfg.bUseMouseTurning);
	iniReader.WriteFloat("MOUSE", "TurnSensitivity", cfg.fTurnSensitivity);
	iniReader.WriteBoolean("MOUSE", "UseRawMouseInput", cfg.bUseRawMouseInput);
	iniReader.WriteBoolean("MOUSE", "UnlockCameraFromAim", cfg.bUnlockCameraFromAim);
	iniReader.WriteBoolean("MOUSE", "FixSniperZoom", cfg.bFixSniperZoom);
	iniReader.WriteBoolean("MOUSE", "FixSniperFocus", cfg.bFixSniperFocus);
	iniReader.WriteBoolean("MOUSE", "FixRetryLoadMouseSelector", cfg.bFixRetryLoadMouseSelector);

	// KEYBOARD
	iniReader.WriteBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", cfg.bFallbackToEnglishKeyIcons);

	// CONTROLLER
	iniReader.WriteBoolean("CONTROLLER", "OverrideControllerSensitivity", cfg.bOverrideControllerSensitivity);
	iniReader.WriteFloat("CONTROLLER", "ControllerSensitivity", cfg.fControllerSensitivity);
	iniReader.WriteBoolean("CONTROLLER", "RemoveExtraXinputDeadzone", cfg.bRemoveExtraXinputDeadzone);
	iniReader.WriteBoolean("CONTROLLER", "OverrideXinputDeadzone", cfg.bOverrideXinputDeadzone);
	iniReader.WriteFloat("CONTROLLER", "XinputDeadzone", cfg.fXinputDeadzone);

	// FRAME RATE
	iniReader.WriteBoolean("FRAME RATE", "FixFallingItemsSpeed", cfg.bFixFallingItemsSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixTurningSpeed", cfg.bFixTurningSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixQTE", cfg.bFixQTE);
	iniReader.WriteBoolean("FRAME RATE", "FixAshleyBustPhysics", cfg.bFixAshleyBustPhysics);
	iniReader.WriteBoolean("FRAME RATE", "EnableFastMath", cfg.bEnableFastMath);
	iniReader.WriteBoolean("FRAME RATE", "PrecacheModels", cfg.bPrecacheModels);

	// MISC
	iniReader.WriteBoolean("MISC", "OverrideCostumes", cfg.bOverrideCostumes);
	iniReader.WriteString("MISC", "LeonCostume", " " + std::string(sLeonCostumeNames[(int)cfg.CostumeOverride.Leon]));
	iniReader.WriteString("MISC", "AshleyCostume", " " + std::string(sAshleyCostumeNames[(int)cfg.CostumeOverride.Ashley]));
	iniReader.WriteString("MISC", "AdaCostume", " " + std::string(sAdaCostumeNames[(int)cfg.CostumeOverride.Ada]));
	iniReader.WriteBoolean("MISC", "AshleyJPCameraAngles", cfg.bAshleyJPCameraAngles);
	iniReader.WriteInteger("MISC", "ViolenceLevelOverride", cfg.iViolenceLevelOverride);
	iniReader.WriteBoolean("MISC", "AllowSellingHandgunSilencer", cfg.bAllowSellingHandgunSilencer);
	iniReader.WriteBoolean("MISC", "AllowMafiaLeonCutscenes", cfg.bAllowMafiaLeonCutscenes);
	iniReader.WriteBoolean("MISC", "SilenceArmoredAshley", cfg.bSilenceArmoredAshley);
	iniReader.WriteBoolean("MISC", "AllowAshleySuplex", cfg.bAllowAshleySuplex);
	iniReader.WriteBoolean("MISC", "AllowMatildaQuickturn", cfg.bAllowMatildaQuickturn);
	iniReader.WriteBoolean("MISC", "FixDitmanGlitch", cfg.bFixDitmanGlitch);
	iniReader.WriteBoolean("MISC", "UseSprintToggle", cfg.bUseSprintToggle);
	iniReader.WriteBoolean("MISC", "DisableQTE", cfg.bDisableQTE);
	iniReader.WriteBoolean("MISC", "AutomaticMashingQTE", cfg.bAutomaticMashingQTE);
	iniReader.WriteBoolean("MISC", "SkipIntroLogos", cfg.bSkipIntroLogos);
	iniReader.WriteBoolean("MISC", "EnableDebugMenu", cfg.bEnableDebugMenu);

	// MEMORY
	iniReader.WriteBoolean("MEMORY", "AllowHighResolutionSFD", cfg.bAllowHighResolutionSFD);
	iniReader.WriteBoolean("MEMORY", "RaiseVertexAlloc", cfg.bRaiseVertexAlloc);
	iniReader.WriteBoolean("MEMORY", "RaiseInventoryAlloc", cfg.bRaiseInventoryAlloc);

	// HOTKEYS
	iniReader.WriteString("HOTKEYS", "ConfigMenu", " " + cfg.sConfigMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "Console", " " + cfg.sConsoleKeyCombo);
	iniReader.WriteString("HOTKEYS", "FlipItemUp", " " + cfg.sFlipItemUp);
	iniReader.WriteString("HOTKEYS", "FlipItemDown", " " + cfg.sFlipItemDown);
	iniReader.WriteString("HOTKEYS", "FlipItemLeft", " " + cfg.sFlipItemLeft);
	iniReader.WriteString("HOTKEYS", "FlipItemRight", " " + cfg.sFlipItemRight);
	iniReader.WriteString("HOTKEYS", "QTE_key_1", " " + cfg.sQTE_key_1);
	iniReader.WriteString("HOTKEYS", "QTE_key_2", " " + cfg.sQTE_key_2);
	iniReader.WriteString("HOTKEYS", "DebugMenu", " " + cfg.sDebugMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "MouseTurningModifier", " " + cfg.sMouseTurnModifierKeyCombo);
	iniReader.WriteString("HOTKEYS", "JetSkiTricks", " " + cfg.sJetSkiTrickCombo);

	// IMGUI
	iniReader.WriteFloat("IMGUI", "FontSize", cfg.fFontSize);

	cfg.HasUnsavedChanges = false;

	return 0;
}

void Settings::WriteSettings()
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // if thread is already running, wait for it to finish

	// Spawn a new thread to handle writing settings, as INI writing funcs that get used are pretty slow
	CreateThreadAutoClose(NULL, 0, WriteSettingsThread, NULL, 0, NULL);
}
