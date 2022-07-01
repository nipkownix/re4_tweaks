#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include "stdafx.h"
#include "Settings.h"
#include "settings_string.h"
#include "Patches.h"
#include "input.hpp"

std::shared_ptr<class Config> pConfig = std::make_shared<Config>();

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
				uint32_t token_num = pInput->KeyMap_getVK(cur_token);
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
		uint32_t token_num = pInput->KeyMap_getVK(cur_token);
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

void Config::ReadSettings()
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

void Config::ReadSettings(std::string_view ini_path)
{
	CIniReader iniReader(ini_path);

	#ifdef VERBOSE
	con.AddLogChar("Reading settings from %s", ini_path.data());
	#endif

	pConfig->HasUnsavedChanges = false;

	// DISPLAY
	pConfig->fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", pConfig->fFOVAdditional);
	if (pConfig->fFOVAdditional > 0.0f)
		pConfig->bEnableFOV = true;
	else
		pConfig->bEnableFOV = false;

	pConfig->bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", pConfig->bFixUltraWideAspectRatio);
	pConfig->bDisableVsync = iniReader.ReadBoolean("DISPLAY", "DisableVsync", pConfig->bDisableVsync);
	pConfig->bFixDPIScale = iniReader.ReadBoolean("DISPLAY", "FixDPIScale", pConfig->bFixDPIScale);
	pConfig->bFixDisplayMode = iniReader.ReadBoolean("DISPLAY", "FixDisplayMode", pConfig->bFixDisplayMode);
	pConfig->iCustomRefreshRate = iniReader.ReadInteger("DISPLAY", "CustomRefreshRate", pConfig->iCustomRefreshRate);
	pConfig->bOverrideLaserColor = iniReader.ReadBoolean("DISPLAY", "OverrideLaserColor", pConfig->bOverrideLaserColor);
	pConfig->bRainbowLaser = iniReader.ReadBoolean("DISPLAY", "RainbowLaser", pConfig->bRainbowLaser);

	pConfig->fLaserRGB[0] = iniReader.ReadFloat("DISPLAY", "LaserR", pConfig->fLaserRGB[0]) / 255.0f;
	pConfig->fLaserRGB[1] = iniReader.ReadFloat("DISPLAY", "LaserG", pConfig->fLaserRGB[1]) / 255.0f;
	pConfig->fLaserRGB[2] = iniReader.ReadFloat("DISPLAY", "LaserB", pConfig->fLaserRGB[2]) / 255.0f;

	pConfig->bRestorePickupTransparency = iniReader.ReadBoolean("DISPLAY", "RestorePickupTransparency", pConfig->bRestorePickupTransparency);
	pConfig->bDisableBrokenFilter03 = iniReader.ReadBoolean("DISPLAY", "DisableBrokenFilter03", pConfig->bDisableBrokenFilter03);
	pConfig->bFixBlurryImage = iniReader.ReadBoolean("DISPLAY", "FixBlurryImage", pConfig->bFixBlurryImage);
	pConfig->bDisableFilmGrain = iniReader.ReadBoolean("DISPLAY", "DisableFilmGrain", pConfig->bDisableFilmGrain);
	pConfig->bEnableGCBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCBlur", pConfig->bEnableGCBlur);
	pConfig->bEnableGCScopeBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCScopeBlur", pConfig->bEnableGCScopeBlur);
	pConfig->bWindowBorderless = iniReader.ReadBoolean("DISPLAY", "WindowBorderless", pConfig->bWindowBorderless);
	pConfig->iWindowPositionX = iniReader.ReadInteger("DISPLAY", "WindowPositionX", pConfig->iWindowPositionX);
	pConfig->iWindowPositionY = iniReader.ReadInteger("DISPLAY", "WindowPositionY", pConfig->iWindowPositionY);
	pConfig->bRememberWindowPos = iniReader.ReadBoolean("DISPLAY", "RememberWindowPos", pConfig->bRememberWindowPos);

	// AUDIO
	pConfig->iVolumeMaster = iniReader.ReadInteger("AUDIO", "VolumeMaster", pConfig->iVolumeMaster);
	pConfig->iVolumeMaster = min(max(pConfig->iVolumeMaster, 0), 100); // limit between 0 - 100

	pConfig->iVolumeBGM = iniReader.ReadInteger("AUDIO", "VolumeBGM", pConfig->iVolumeBGM);
	pConfig->iVolumeBGM = min(max(pConfig->iVolumeBGM, 0), 100); // limit between 0 - 100

	pConfig->iVolumeSE = iniReader.ReadInteger("AUDIO", "VolumeSE", pConfig->iVolumeSE);
	pConfig->iVolumeSE = min(max(pConfig->iVolumeSE, 0), 100); // limit between 0 - 100

	pConfig->iVolumeCutscene = iniReader.ReadInteger("AUDIO", "VolumeCutscene", pConfig->iVolumeCutscene);
	pConfig->iVolumeCutscene = min(max(pConfig->iVolumeCutscene, 0), 100); // limit between 0 - 100

	// MOUSE
	pConfig->bUseMouseTurning = iniReader.ReadBoolean("MOUSE", "UseMouseTurning", pConfig->bUseMouseTurning);
	pConfig->fTurnSensitivity = iniReader.ReadFloat("MOUSE", "TurnSensitivity", pConfig->fTurnSensitivity);
	pConfig->bUseRawMouseInput = iniReader.ReadBoolean("MOUSE", "UseRawMouseInput", pConfig->bUseRawMouseInput);
	pConfig->bDetachCameraFromAim = iniReader.ReadBoolean("MOUSE", "DetachCameraFromAim", pConfig->bDetachCameraFromAim);
	pConfig->bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", pConfig->bFixSniperZoom);
	pConfig->bFixSniperFocus = iniReader.ReadBoolean("MOUSE", "FixSniperFocus", pConfig->bFixSniperFocus);
	pConfig->bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", pConfig->bFixRetryLoadMouseSelector);

	// KEYBOARD
	pConfig->bFallbackToEnglishKeyIcons = iniReader.ReadBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", pConfig->bFallbackToEnglishKeyIcons);

	// CONTROLLER
	pConfig->bOverrideControllerSensitivity = iniReader.ReadBoolean("CONTROLLER", "OverrideControllerSensitivity", pConfig->bOverrideControllerSensitivity);
	pConfig->fControllerSensitivity = iniReader.ReadFloat("CONTROLLER", "ControllerSensitivity", pConfig->fControllerSensitivity);
	pConfig->fControllerSensitivity = fmin(fmax(pConfig->fControllerSensitivity, 0.5f), 4.0f); // limit between 0.5 - 4.0

	pConfig->bRemoveExtraXinputDeadzone = iniReader.ReadBoolean("CONTROLLER", "RemoveExtraXinputDeadzone", pConfig->bRemoveExtraXinputDeadzone);

	pConfig->bOverrideXinputDeadzone = iniReader.ReadBoolean("CONTROLLER", "OverrideXinputDeadzone", pConfig->bOverrideXinputDeadzone);
	pConfig->fXinputDeadzone = iniReader.ReadFloat("CONTROLLER", "XinputDeadzone", pConfig->fXinputDeadzone);
	pConfig->fXinputDeadzone = fmin(fmax(pConfig->fXinputDeadzone, 0.0f), 3.5f); // limit between 0.0 - 3.5

	// FRAME RATE
	pConfig->bFixFallingItemsSpeed = iniReader.ReadBoolean("FRAME RATE", "FixFallingItemsSpeed", pConfig->bFixFallingItemsSpeed);
	pConfig->bFixTurningSpeed = iniReader.ReadBoolean("FRAME RATE", "FixTurningSpeed", pConfig->bFixTurningSpeed);
	pConfig->bFixQTE = iniReader.ReadBoolean("FRAME RATE", "FixQTE", pConfig->bFixQTE);
	pConfig->bFixAshleyBustPhysics = iniReader.ReadBoolean("FRAME RATE", "FixAshleyBustPhysics", pConfig->bFixAshleyBustPhysics);
	pConfig->bEnableFastMath = iniReader.ReadBoolean("FRAME RATE", "EnableFastMath", pConfig->bEnableFastMath);
	pConfig->bPrecacheModels = iniReader.ReadBoolean("FRAME RATE", "PrecacheModels", pConfig->bPrecacheModels);

	// MISC
	pConfig->bOverrideCostumes = iniReader.ReadBoolean("MISC", "OverrideCostumes", pConfig->bOverrideCostumes);

	std::string buf = iniReader.ReadString("MISC", "LeonCostume", "");
	if (!buf.empty())
	{
		if (buf == "Jacket") pConfig->CostumeOverride.Leon = LeonCostumes::Jacket;
		if (buf == "Normal") pConfig->CostumeOverride.Leon = LeonCostumes::Normal;
		if (buf == "Vest") pConfig->CostumeOverride.Leon = LeonCostumes::Vest;
		if (buf == "RPD") pConfig->CostumeOverride.Leon = LeonCostumes::RPD;
		if (buf == "Mafia") pConfig->CostumeOverride.Leon = LeonCostumes::Mafia;
	}

	buf = iniReader.ReadString("MISC", "AshleyCostume", "");
	if (!buf.empty())
	{
		if (buf == "Normal") pConfig->CostumeOverride.Ashley = AshleyCostumes::Normal;
		if (buf == "Popstar") pConfig->CostumeOverride.Ashley = AshleyCostumes::Popstar;
		if (buf == "Armor") pConfig->CostumeOverride.Ashley = AshleyCostumes::Armor;
	}

	buf = iniReader.ReadString("MISC", "AdaCostume", "");
	if (!buf.empty())
	{
		if (buf == "RE2") pConfig->CostumeOverride.Ada = AdaCostumes::RE2;
		if (buf == "Spy") pConfig->CostumeOverride.Ada = AdaCostumes::Spy;
		if (buf == "Normal") pConfig->CostumeOverride.Ada = AdaCostumes::Normal;
	}

	iCostumeComboLeon = (int)pConfig->CostumeOverride.Leon;
	iCostumeComboAshley = (int)pConfig->CostumeOverride.Ashley;
	iCostumeComboAda = (int)pConfig->CostumeOverride.Ada;

	pConfig->bAshleyJPCameraAngles = iniReader.ReadBoolean("MISC", "AshleyJPCameraAngles", pConfig->bAshleyJPCameraAngles);

	pConfig->iViolenceLevelOverride = iniReader.ReadInteger("MISC", "ViolenceLevelOverride", pConfig->iViolenceLevelOverride);
	pConfig->iViolenceLevelOverride = min(max(pConfig->iViolenceLevelOverride, -1), 2); // limit between -1 to 2

	pConfig->bAllowSellingHandgunSilencer = iniReader.ReadBoolean("MISC", "AllowSellingHandgunSilencer", pConfig->bAllowSellingHandgunSilencer);
	pConfig->bAllowMafiaLeonCutscenes = iniReader.ReadBoolean("MISC", "AllowMafiaLeonCutscenes", pConfig->bAllowMafiaLeonCutscenes);
	pConfig->bSilenceArmoredAshley = iniReader.ReadBoolean("MISC", "SilenceArmoredAshley", pConfig->bSilenceArmoredAshley);
	pConfig->bAllowAshleySuplex = iniReader.ReadBoolean("MISC", "AllowAshleySuplex", pConfig->bAllowAshleySuplex);
	pConfig->bAllowMatildaQuickturn = iniReader.ReadBoolean("MISC", "AllowMatildaQuickturn", pConfig->bAllowMatildaQuickturn);
	pConfig->bFixDitmanGlitch = iniReader.ReadBoolean("MISC", "FixDitmanGlitch", pConfig->bFixDitmanGlitch);
	pConfig->bUseSprintToggle = iniReader.ReadBoolean("MISC", "UseSprintToggle", pConfig->bUseSprintToggle);
	pConfig->bDisableQTE = iniReader.ReadBoolean("MISC", "DisableQTE", pConfig->bDisableQTE);
	pConfig->bAutomaticMashingQTE = iniReader.ReadBoolean("MISC", "AutomaticMashingQTE", pConfig->bAutomaticMashingQTE);
	pConfig->bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", pConfig->bSkipIntroLogos);
	pConfig->bEnableDebugMenu = iniReader.ReadBoolean("MISC", "EnableDebugMenu", pConfig->bEnableDebugMenu);

	// MEMORY
	pConfig->bAllowHighResolutionSFD = iniReader.ReadBoolean("MEMORY", "AllowHighResolutionSFD", pConfig->bAllowHighResolutionSFD);
	pConfig->bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", pConfig->bRaiseVertexAlloc);
	pConfig->bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", pConfig->bRaiseInventoryAlloc);

	// HOTKEYS
	pConfig->sConfigMenuKeyCombo = iniReader.ReadString("HOTKEYS", "ConfigMenu", pConfig->sConfigMenuKeyCombo);
	if (pConfig->sConfigMenuKeyCombo.length())
		ParseConfigMenuKeyCombo(pConfig->sConfigMenuKeyCombo);

	pConfig->sConsoleKeyCombo = iniReader.ReadString("HOTKEYS", "Console", pConfig->sConsoleKeyCombo);
	if (pConfig->sConsoleKeyCombo.length())
	{
		ParseConsoleKeyCombo(pConfig->sConsoleKeyCombo);

		// Update console title
		con.TitleKeyCombo = pConfig->sConsoleKeyCombo;
	}

	pConfig->sFlipItemUp = iniReader.ReadString("HOTKEYS", "FlipItemUp", pConfig->sFlipItemUp);
	pConfig->sFlipItemDown = iniReader.ReadString("HOTKEYS", "FlipItemDown", pConfig->sFlipItemDown);
	pConfig->sFlipItemLeft = iniReader.ReadString("HOTKEYS", "FlipItemLeft", pConfig->sFlipItemLeft);
	pConfig->sFlipItemRight = iniReader.ReadString("HOTKEYS", "FlipItemRight", pConfig->sFlipItemRight);
	pConfig->sQTE_key_1 = iniReader.ReadString("HOTKEYS", "QTE_key_1", pConfig->sQTE_key_1);
	pConfig->sQTE_key_2 = iniReader.ReadString("HOTKEYS", "QTE_key_2", pConfig->sQTE_key_2);

	pConfig->sDebugMenuKeyCombo = iniReader.ReadString("HOTKEYS", "DebugMenu", pConfig->sDebugMenuKeyCombo);
	if (pConfig->sDebugMenuKeyCombo.length())
		ParseToolMenuKeyCombo(pConfig->sDebugMenuKeyCombo);

	pConfig->sMouseTurnModifierKeyCombo = iniReader.ReadString("HOTKEYS", "MouseTurningModifier", pConfig->sMouseTurnModifierKeyCombo);
	if (pConfig->sMouseTurnModifierKeyCombo.length())
		ParseMouseTurnModifierCombo(pConfig->sMouseTurnModifierKeyCombo);

	pConfig->sJetSkiTrickCombo = iniReader.ReadString("HOTKEYS", "JetSkiTricks", pConfig->sJetSkiTrickCombo);
	if (pConfig->sJetSkiTrickCombo.length())
		ParseJetSkiTrickCombo(pConfig->sJetSkiTrickCombo);

	// FPS WARNING
	pConfig->bIgnoreFPSWarning = iniReader.ReadBoolean("WARNING", "IgnoreFPSWarning", pConfig->bIgnoreFPSWarning);
	
	// IMGUI
	pConfig->fFontSize = iniReader.ReadFloat("IMGUI", "FontSize", pConfig->fFontSize);
	pConfig->fFontSize = fmin(fmax(pConfig->fFontSize, 1.0f), 1.25f); // limit between 1.0 - 1.25

	pConfig->bDisableMenuTip = iniReader.ReadBoolean("IMGUI", "DisableMenuTip", pConfig->bDisableMenuTip);

	// DEBUG
	pConfig->bVerboseLog = iniReader.ReadBoolean("DEBUG", "VerboseLog", pConfig->bVerboseLog);
	pConfig->bNeverHideCursor = iniReader.ReadBoolean("DEBUG", "NeverHideCursor", pConfig->bNeverHideCursor);
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
	iniReader.WriteFloat("DISPLAY", "FOVAdditional", pConfig->fFOVAdditional);
	iniReader.WriteBoolean("DISPLAY", "FixUltraWideAspectRatio", pConfig->bFixUltraWideAspectRatio);
	iniReader.WriteBoolean("DISPLAY", "DisableVsync", pConfig->bDisableVsync);
	iniReader.WriteBoolean("DISPLAY", "FixDPIScale", pConfig->bFixDPIScale);
	iniReader.WriteBoolean("DISPLAY", "FixDisplayMode", pConfig->bFixDisplayMode);
	iniReader.WriteInteger("DISPLAY", "CustomRefreshRate", pConfig->iCustomRefreshRate);
	iniReader.WriteBoolean("DISPLAY", "OverrideLaserColor", pConfig->bOverrideLaserColor);
	iniReader.WriteBoolean("DISPLAY", "RainbowLaser", pConfig->bRainbowLaser);

	iniReader.WriteFloat("DISPLAY", "LaserR", pConfig->fLaserRGB[0] * 255.0f);
	iniReader.WriteFloat("DISPLAY", "LaserG", pConfig->fLaserRGB[1] * 255.0f);
	iniReader.WriteFloat("DISPLAY", "LaserB", pConfig->fLaserRGB[2] * 255.0f);

	iniReader.WriteBoolean("DISPLAY", "RestorePickupTransparency", pConfig->bRestorePickupTransparency);
	iniReader.WriteBoolean("DISPLAY", "DisableBrokenFilter03", pConfig->bDisableBrokenFilter03);
	iniReader.WriteBoolean("DISPLAY", "FixBlurryImage", pConfig->bFixBlurryImage);
	iniReader.WriteBoolean("DISPLAY", "DisableFilmGrain", pConfig->bDisableFilmGrain);
	iniReader.WriteBoolean("DISPLAY", "EnableGCBlur", pConfig->bEnableGCBlur);
	iniReader.WriteBoolean("DISPLAY", "EnableGCScopeBlur", pConfig->bEnableGCScopeBlur);
	iniReader.WriteBoolean("DISPLAY", "WindowBorderless", pConfig->bWindowBorderless);
	iniReader.WriteInteger("DISPLAY", "WindowPositionX", pConfig->iWindowPositionX);
	iniReader.WriteInteger("DISPLAY", "WindowPositionY", pConfig->iWindowPositionY);
	iniReader.WriteBoolean("DISPLAY", "RememberWindowPos", pConfig->bRememberWindowPos);

	// AUDIO
	iniReader.WriteInteger("AUDIO", "VolumeMaster", pConfig->iVolumeMaster);
	iniReader.WriteInteger("AUDIO", "VolumeBGM", pConfig->iVolumeBGM);
	iniReader.WriteInteger("AUDIO", "VolumeSE", pConfig->iVolumeSE);
	iniReader.WriteInteger("AUDIO", "VolumeCutscene", pConfig->iVolumeCutscene);

	// MOUSE
	iniReader.WriteBoolean("MOUSE", "UseMouseTurning", pConfig->bUseMouseTurning);
	iniReader.WriteFloat("MOUSE", "TurnSensitivity", pConfig->fTurnSensitivity);
	iniReader.WriteBoolean("MOUSE", "UseRawMouseInput", pConfig->bUseRawMouseInput);
	iniReader.WriteBoolean("MOUSE", "DetachCameraFromAim", pConfig->bDetachCameraFromAim);
	iniReader.WriteBoolean("MOUSE", "FixSniperZoom", pConfig->bFixSniperZoom);
	iniReader.WriteBoolean("MOUSE", "FixSniperFocus", pConfig->bFixSniperFocus);
	iniReader.WriteBoolean("MOUSE", "FixRetryLoadMouseSelector", pConfig->bFixRetryLoadMouseSelector);

	// KEYBOARD
	iniReader.WriteBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", pConfig->bFallbackToEnglishKeyIcons);

	// CONTROLLER
	iniReader.WriteBoolean("CONTROLLER", "OverrideControllerSensitivity", pConfig->bOverrideControllerSensitivity);
	iniReader.WriteFloat("CONTROLLER", "ControllerSensitivity", pConfig->fControllerSensitivity);
	iniReader.WriteBoolean("CONTROLLER", "RemoveExtraXinputDeadzone", pConfig->bRemoveExtraXinputDeadzone);
	iniReader.WriteBoolean("CONTROLLER", "OverrideXinputDeadzone", pConfig->bOverrideXinputDeadzone);
	iniReader.WriteFloat("CONTROLLER", "XinputDeadzone", pConfig->fXinputDeadzone);

	// FRAME RATE
	iniReader.WriteBoolean("FRAME RATE", "FixFallingItemsSpeed", pConfig->bFixFallingItemsSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixTurningSpeed", pConfig->bFixTurningSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixQTE", pConfig->bFixQTE);
	iniReader.WriteBoolean("FRAME RATE", "FixAshleyBustPhysics", pConfig->bFixAshleyBustPhysics);
	iniReader.WriteBoolean("FRAME RATE", "EnableFastMath", pConfig->bEnableFastMath);
	iniReader.WriteBoolean("FRAME RATE", "PrecacheModels", pConfig->bPrecacheModels);

	// MISC
	iniReader.WriteBoolean("MISC", "OverrideCostumes", pConfig->bOverrideCostumes);
	iniReader.WriteString("MISC", "LeonCostume", " " + std::string(sLeonCostumeNames[(int)pConfig->CostumeOverride.Leon]));
	iniReader.WriteString("MISC", "AshleyCostume", " " + std::string(sAshleyCostumeNames[(int)pConfig->CostumeOverride.Ashley]));
	iniReader.WriteString("MISC", "AdaCostume", " " + std::string(sAdaCostumeNames[(int)pConfig->CostumeOverride.Ada]));
	iniReader.WriteBoolean("MISC", "AshleyJPCameraAngles", pConfig->bAshleyJPCameraAngles);
	iniReader.WriteInteger("MISC", "ViolenceLevelOverride", pConfig->iViolenceLevelOverride);
	iniReader.WriteBoolean("MISC", "AllowSellingHandgunSilencer", pConfig->bAllowSellingHandgunSilencer);
	iniReader.WriteBoolean("MISC", "AllowMafiaLeonCutscenes", pConfig->bAllowMafiaLeonCutscenes);
	iniReader.WriteBoolean("MISC", "SilenceArmoredAshley", pConfig->bSilenceArmoredAshley);
	iniReader.WriteBoolean("MISC", "AllowAshleySuplex", pConfig->bAllowAshleySuplex);
	iniReader.WriteBoolean("MISC", "AllowMatildaQuickturn", pConfig->bAllowMatildaQuickturn);
	iniReader.WriteBoolean("MISC", "FixDitmanGlitch", pConfig->bFixDitmanGlitch);
	iniReader.WriteBoolean("MISC", "UseSprintToggle", pConfig->bUseSprintToggle);
	iniReader.WriteBoolean("MISC", "DisableQTE", pConfig->bDisableQTE);
	iniReader.WriteBoolean("MISC", "AutomaticMashingQTE", pConfig->bAutomaticMashingQTE);
	iniReader.WriteBoolean("MISC", "SkipIntroLogos", pConfig->bSkipIntroLogos);
	iniReader.WriteBoolean("MISC", "EnableDebugMenu", pConfig->bEnableDebugMenu);

	// MEMORY
	iniReader.WriteBoolean("MEMORY", "AllowHighResolutionSFD", pConfig->bAllowHighResolutionSFD);
	iniReader.WriteBoolean("MEMORY", "RaiseVertexAlloc", pConfig->bRaiseVertexAlloc);
	iniReader.WriteBoolean("MEMORY", "RaiseInventoryAlloc", pConfig->bRaiseInventoryAlloc);

	// HOTKEYS
	iniReader.WriteString("HOTKEYS", "ConfigMenu", " " + pConfig->sConfigMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "Console", " " + pConfig->sConsoleKeyCombo);
	iniReader.WriteString("HOTKEYS", "FlipItemUp", " " + pConfig->sFlipItemUp);
	iniReader.WriteString("HOTKEYS", "FlipItemDown", " " + pConfig->sFlipItemDown);
	iniReader.WriteString("HOTKEYS", "FlipItemLeft", " " + pConfig->sFlipItemLeft);
	iniReader.WriteString("HOTKEYS", "FlipItemRight", " " + pConfig->sFlipItemRight);
	iniReader.WriteString("HOTKEYS", "QTE_key_1", " " + pConfig->sQTE_key_1);
	iniReader.WriteString("HOTKEYS", "QTE_key_2", " " + pConfig->sQTE_key_2);
	iniReader.WriteString("HOTKEYS", "DebugMenu", " " + pConfig->sDebugMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "MouseTurningModifier", " " + pConfig->sMouseTurnModifierKeyCombo);
	iniReader.WriteString("HOTKEYS", "JetSkiTricks", " " + pConfig->sJetSkiTrickCombo);

	// IMGUI
	iniReader.WriteFloat("IMGUI", "FontSize", pConfig->fFontSize);

	pConfig->HasUnsavedChanges = false;

	return 0;
}

void Config::WriteSettings()
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // if thread is already running, wait for it to finish

	// Spawn a new thread to handle writing settings, as INI writing funcs that get used are pretty slow
	CreateThreadAutoClose(NULL, 0, WriteSettingsThread, NULL, 0, NULL);
}
