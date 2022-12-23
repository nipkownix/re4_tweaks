#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "input.hpp"
#include "Utils.h"
#include "Trainer.h"
#include "../dxvk/src/config.h"

const std::wstring sSettingOverridesPath = L"re4_tweaks\\setting_overrides\\";
const std::wstring sHDProjectOverrideName = L"HDProject.ini";

const char* sLeonCostumeNames[] = {"Jacket", "Normal", "Vest", "RPD", "Mafia"};
const char* sAshleyCostumeNames[] = {"Normal", "Popstar", "Armor"};
const char* sAdaCostumeNames[] = {"RE2", "Spy", "Normal"};

const char* sGameDifficultyNames[] = { "Unk0", "Amateur", "Unk2", "Easy", "Unk4", "Normal", "Professional" };

std::vector<uint32_t> re4t_cfg::ParseKeyCombo(std::string_view in_combo)
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
				uint32_t token_num = pInput->vk_from_key_name(cur_token);
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
		uint32_t token_num = pInput->vk_from_key_name(cur_token);
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

void re4t_cfg::ReadSettings()
{
	// Read default settings file first
	std::wstring sDefaultIniPath = rootPath + wrapperName + L".ini";
	ReadSettings(sDefaultIniPath);

	// Try reading in trainer.ini settings
	std::wstring sTrainerIniPath = rootPath + L"re4_tweaks\\trainer.ini";
	if (std::filesystem::exists(sTrainerIniPath))
		ReadSettings(sTrainerIniPath);

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
		ReadSettings(path.wstring());

	// Special case for HDProject settings, make sure it overrides all other INIs
	auto hdproject_path = override_path + sHDProjectOverrideName;
	if (std::filesystem::exists(hdproject_path))
	{
		re4t::cfg->bIsUsingHDProject = true;
		ReadSettings(hdproject_path);
	}
}

void re4t_cfg::ParseHotkeys()
{
	pInput->clear_hotkeys();

	ParseConfigMenuKeyCombo(re4t::cfg->sConfigMenuKeyCombo);
	ParseConsoleKeyCombo(re4t::cfg->sConsoleKeyCombo);
	ParseToolMenuKeyCombo(re4t::cfg->sDebugMenuKeyCombo);
	ParseMouseTurnModifierCombo(re4t::cfg->sMouseTurnModifierKeyCombo);
	ParseJetSkiTrickCombo(re4t::cfg->sJetSkiTrickCombo);
	ParseImGuiUIFocusCombo(re4t::cfg->sTrainerFocusUIKeyCombo);

	Trainer_ParseKeyCombos();
}

void re4t_cfg::ReadSettings(std::wstring ini_path)
{
	CIniReader iniReader(WstrToStr(ini_path));

	#ifdef VERBOSE
	con.log("Reading settings from: %s", WstrToStr(ini_path).data());
	#endif

	spd::log()->info("Reading settings from: \"{}\"", WstrToStr(ini_path).data());

	re4t::cfg->HasUnsavedChanges = false;
	
	// VULKAN
	re4t::dxvk::cfg->bUseVulkanRenderer = iniReader.ReadBoolean("VULKAN", "UseVulkanRenderer", re4t::dxvk::cfg->bUseVulkanRenderer);
	re4t::dxvk::cfg->bShowFPS = iniReader.ReadBoolean("VULKAN", "ShowFPS", re4t::dxvk::cfg->bShowFPS);
	re4t::dxvk::cfg->bShowGPULoad = iniReader.ReadBoolean("VULKAN", "ShowGPULoad", re4t::dxvk::cfg->bShowGPULoad);
	re4t::dxvk::cfg->bShowDeviceInfo = iniReader.ReadBoolean("VULKAN", "ShowDeviceInfo", re4t::dxvk::cfg->bShowDeviceInfo);
	re4t::dxvk::cfg->bDisableAsync = iniReader.ReadBoolean("VULKAN", "DisableAsync", re4t::dxvk::cfg->bDisableAsync);

	re4t::dxvk::cfg->DXVK_HUD = iniReader.ReadString("VULKAN", "DXVK_HUD", re4t::dxvk::cfg->DXVK_HUD);
	re4t::dxvk::cfg->DXVK_FILTER_DEVICE_NAME = iniReader.ReadString("VULKAN", "DXVK_FILTER_DEVICE_NAME", re4t::dxvk::cfg->DXVK_FILTER_DEVICE_NAME);

	// DISPLAY
	re4t::cfg->fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", re4t::cfg->fFOVAdditional);
	if (re4t::cfg->fFOVAdditional > 0.0f)
		re4t::cfg->bEnableFOV = true;
	else
		re4t::cfg->bEnableFOV = false;

	re4t::cfg->bDisableVsync = iniReader.ReadBoolean("DISPLAY", "DisableVsync", re4t::cfg->bDisableVsync);

	re4t::cfg->bUltraWideAspectSupport = iniReader.ReadBoolean("DISPLAY", "UltraWideAspectSupport", re4t::cfg->bUltraWideAspectSupport);
	re4t::cfg->bSideAlignHUD = iniReader.ReadBoolean("DISPLAY", "SideAlignHUD", re4t::cfg->bSideAlignHUD);
	re4t::cfg->bStretchFullscreenImages = iniReader.ReadBoolean("DISPLAY", "StretchFullscreenImages", re4t::cfg->bStretchFullscreenImages);
	re4t::cfg->bStretchVideos = iniReader.ReadBoolean("DISPLAY", "StretchVideos", re4t::cfg->bStretchVideos);
	re4t::cfg->bRemove16by10BlackBars = iniReader.ReadBoolean("DISPLAY", "Remove16by10BlackBars", re4t::cfg->bRemove16by10BlackBars);

	re4t::cfg->bFixDPIScale = iniReader.ReadBoolean("DISPLAY", "FixDPIScale", re4t::cfg->bFixDPIScale);
	re4t::cfg->bFixDisplayMode = iniReader.ReadBoolean("DISPLAY", "FixDisplayMode", re4t::cfg->bFixDisplayMode);
	re4t::cfg->iCustomRefreshRate = iniReader.ReadInteger("DISPLAY", "CustomRefreshRate", re4t::cfg->iCustomRefreshRate);
	re4t::cfg->bOverrideLaserColor = iniReader.ReadBoolean("DISPLAY", "OverrideLaserColor", re4t::cfg->bOverrideLaserColor);
	re4t::cfg->bRainbowLaser = iniReader.ReadBoolean("DISPLAY", "RainbowLaser", re4t::cfg->bRainbowLaser);

	re4t::cfg->iLaserR = iniReader.ReadInteger("DISPLAY", "LaserR", re4t::cfg->iLaserR);
	re4t::cfg->iLaserG = iniReader.ReadInteger("DISPLAY", "LaserG", re4t::cfg->iLaserG);
	re4t::cfg->iLaserB = iniReader.ReadInteger("DISPLAY", "LaserB", re4t::cfg->iLaserB);
	fLaserColorPicker[0] = re4t::cfg->iLaserR / 255.0f;
	fLaserColorPicker[1] = re4t::cfg->iLaserG / 255.0f;
	fLaserColorPicker[2] = re4t::cfg->iLaserB / 255.0f;

	re4t::cfg->bRestorePickupTransparency = iniReader.ReadBoolean("DISPLAY", "RestorePickupTransparency", re4t::cfg->bRestorePickupTransparency);
	re4t::cfg->bDisableBrokenFilter03 = iniReader.ReadBoolean("DISPLAY", "DisableBrokenFilter03", re4t::cfg->bDisableBrokenFilter03);
	re4t::cfg->bFixBlurryImage = iniReader.ReadBoolean("DISPLAY", "FixBlurryImage", re4t::cfg->bFixBlurryImage);
	re4t::cfg->bDisableFilmGrain = iniReader.ReadBoolean("DISPLAY", "DisableFilmGrain", re4t::cfg->bDisableFilmGrain);

	re4t::cfg->bImproveWater = iniReader.ReadBoolean("DISPLAY", "ImproveWater", re4t::cfg->bImproveWater);
	if (re4t::cfg->bIsUsingHDProject)
	{
		re4t::cfg->bImproveWater = false; // Should be inside HDProject.ini instead, but we'll keep it here untill a new version of the HD Project is released
	}

	re4t::cfg->bEnableGCBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCBlur", re4t::cfg->bEnableGCBlur);

	std::string GCBlurTypeStr = iniReader.ReadString("DISPLAY", "GCBlurType", "");
	if (!GCBlurTypeStr.empty())
	{
		if (GCBlurTypeStr == std::string("Enhanced"))
		{
			re4t::cfg->bUseEnhancedGCBlur = true;
			iGCBlurMode = 0;
		}
		else if (GCBlurTypeStr == std::string("Classic"))
		{
			re4t::cfg->bUseEnhancedGCBlur = false;
			iGCBlurMode = 1;
		}
	}

	re4t::cfg->bEnableGCScopeBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCScopeBlur", re4t::cfg->bEnableGCScopeBlur);
	re4t::cfg->bWindowBorderless = iniReader.ReadBoolean("DISPLAY", "WindowBorderless", re4t::cfg->bWindowBorderless);
	re4t::cfg->iWindowPositionX = iniReader.ReadInteger("DISPLAY", "WindowPositionX", re4t::cfg->iWindowPositionX);
	re4t::cfg->iWindowPositionY = iniReader.ReadInteger("DISPLAY", "WindowPositionY", re4t::cfg->iWindowPositionY);
	re4t::cfg->bRememberWindowPos = iniReader.ReadBoolean("DISPLAY", "RememberWindowPos", re4t::cfg->bRememberWindowPos);

	// AUDIO
	re4t::cfg->iVolumeMaster = iniReader.ReadInteger("AUDIO", "VolumeMaster", re4t::cfg->iVolumeMaster);
	re4t::cfg->iVolumeMaster = std::min(std::max(re4t::cfg->iVolumeMaster, 0), 100); // limit between 0 - 100

	re4t::cfg->iVolumeBGM = iniReader.ReadInteger("AUDIO", "VolumeBGM", re4t::cfg->iVolumeBGM);
	re4t::cfg->iVolumeBGM = std::min(std::max(re4t::cfg->iVolumeBGM, 0), 100); // limit between 0 - 100

	re4t::cfg->iVolumeSE = iniReader.ReadInteger("AUDIO", "VolumeSE", re4t::cfg->iVolumeSE);
	re4t::cfg->iVolumeSE = std::min(std::max(re4t::cfg->iVolumeSE, 0), 100); // limit between 0 - 100

	re4t::cfg->iVolumeCutscene = iniReader.ReadInteger("AUDIO", "VolumeCutscene", re4t::cfg->iVolumeCutscene);
	re4t::cfg->iVolumeCutscene = std::min(std::max(re4t::cfg->iVolumeCutscene, 0), 100); // limit between 0 - 100

	re4t::cfg->bRestoreGCSoundEffects = iniReader.ReadBoolean("AUDIO", "RestoreGCSoundEffects", re4t::cfg->bRestoreGCSoundEffects);

	// MOUSE
	re4t::cfg->bCameraImprovements = iniReader.ReadBoolean("MOUSE", "CameraImprovements", re4t::cfg->bCameraImprovements);
	re4t::cfg->bResetCameraAfterUsingWeapons = iniReader.ReadBoolean("MOUSE", "ResetCameraAfterUsingWeapons", re4t::cfg->bResetCameraAfterUsingWeapons);
	re4t::cfg->bResetCameraAfterUsingKnife = iniReader.ReadBoolean("MOUSE", "ResetCameraAfterUsingKnife", re4t::cfg->bResetCameraAfterUsingKnife);
	re4t::cfg->bResetCameraWhenRunning = iniReader.ReadBoolean("MOUSE", "ResetCameraWhenRunning", re4t::cfg->bResetCameraWhenRunning);
	re4t::cfg->fCameraSensitivity = iniReader.ReadFloat("MOUSE", "CameraSensitivity", re4t::cfg->fCameraSensitivity);
	re4t::cfg->fCameraSensitivity = fmin(fmax(re4t::cfg->fCameraSensitivity, 0.5f), 2.0f); // limit between 0.5 - 2.0
	re4t::cfg->bUseMouseTurning = iniReader.ReadBoolean("MOUSE", "UseMouseTurning", re4t::cfg->bUseMouseTurning);
	
	std::string MouseTurnTypeStr = iniReader.ReadString("MOUSE", "MouseTurnType", "");
	if (!MouseTurnTypeStr.empty())
	{
		if (MouseTurnTypeStr == std::string("TypeA"))
			re4t::cfg->iMouseTurnType = MouseTurnTypes::TypeA;
		else if (MouseTurnTypeStr == std::string("TypeB"))
			re4t::cfg->iMouseTurnType = MouseTurnTypes::TypeB;
	}

	re4t::cfg->fTurnTypeBSensitivity = iniReader.ReadFloat("MOUSE", "TurnTypeBSensitivity", re4t::cfg->fTurnTypeBSensitivity);
	re4t::cfg->fTurnTypeBSensitivity = fmin(fmax(re4t::cfg->fTurnTypeBSensitivity, 0.5f), 2.0f); // limit between 0.5 - 2.0
	re4t::cfg->bUseRawMouseInput = iniReader.ReadBoolean("MOUSE", "UseRawMouseInput", re4t::cfg->bUseRawMouseInput);
	re4t::cfg->bDetachCameraFromAim = iniReader.ReadBoolean("MOUSE", "DetachCameraFromAim", re4t::cfg->bDetachCameraFromAim);
	re4t::cfg->bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", re4t::cfg->bFixSniperZoom);
	re4t::cfg->bFixSniperFocus = iniReader.ReadBoolean("MOUSE", "FixSniperFocus", re4t::cfg->bFixSniperFocus);
	re4t::cfg->bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", re4t::cfg->bFixRetryLoadMouseSelector);

	// KEYBOARD
	re4t::cfg->bFallbackToEnglishKeyIcons = iniReader.ReadBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", re4t::cfg->bFallbackToEnglishKeyIcons);
	re4t::cfg->bAllowReloadWithoutAiming_kbm = iniReader.ReadBoolean("KEYBOARD", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_kbm);
	re4t::cfg->bReloadWithoutZoom_kbm = iniReader.ReadBoolean("KEYBOARD", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_kbm);

	// CONTROLLER
	re4t::cfg->bOverrideControllerSensitivity = iniReader.ReadBoolean("CONTROLLER", "OverrideControllerSensitivity", re4t::cfg->bOverrideControllerSensitivity);
	re4t::cfg->fControllerSensitivity = iniReader.ReadFloat("CONTROLLER", "ControllerSensitivity", re4t::cfg->fControllerSensitivity);
	re4t::cfg->fControllerSensitivity = fmin(fmax(re4t::cfg->fControllerSensitivity, 0.5f), 4.0f); // limit between 0.5 - 4.0
	re4t::cfg->bRemoveExtraXinputDeadzone = iniReader.ReadBoolean("CONTROLLER", "RemoveExtraXinputDeadzone", re4t::cfg->bRemoveExtraXinputDeadzone);
	re4t::cfg->bOverrideXinputDeadzone = iniReader.ReadBoolean("CONTROLLER", "OverrideXinputDeadzone", re4t::cfg->bOverrideXinputDeadzone);
	re4t::cfg->fXinputDeadzone = iniReader.ReadFloat("CONTROLLER", "XinputDeadzone", re4t::cfg->fXinputDeadzone);
	re4t::cfg->fXinputDeadzone = fmin(fmax(re4t::cfg->fXinputDeadzone, 0.0f), 3.5f); // limit between 0.0 - 3.5
	re4t::cfg->bAllowReloadWithoutAiming_controller = iniReader.ReadBoolean("CONTROLLER", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_controller);
	re4t::cfg->bReloadWithoutZoom_controller = iniReader.ReadBoolean("CONTROLLER", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_controller);

	// FRAME RATE
	re4t::cfg->bFixFallingItemsSpeed = iniReader.ReadBoolean("FRAME RATE", "FixFallingItemsSpeed", re4t::cfg->bFixFallingItemsSpeed);
	re4t::cfg->bFixCompartmentsOpeningSpeed = iniReader.ReadBoolean("FRAME RATE", "FixCompartmentsOpeningSpeed", re4t::cfg->bFixCompartmentsOpeningSpeed);
	re4t::cfg->bFixMovingGeometrySpeed = iniReader.ReadBoolean("FRAME RATE", "FixMovingGeometrySpeed", re4t::cfg->bFixMovingGeometrySpeed);
	re4t::cfg->bFixTurningSpeed = iniReader.ReadBoolean("FRAME RATE", "FixTurningSpeed", re4t::cfg->bFixTurningSpeed);
	re4t::cfg->bFixQTE = iniReader.ReadBoolean("FRAME RATE", "FixQTE", re4t::cfg->bFixQTE);
	re4t::cfg->bFixAshleyBustPhysics = iniReader.ReadBoolean("FRAME RATE", "FixAshleyBustPhysics", re4t::cfg->bFixAshleyBustPhysics);
	re4t::cfg->bEnableFastMath = iniReader.ReadBoolean("FRAME RATE", "EnableFastMath", re4t::cfg->bEnableFastMath);
	re4t::cfg->bReplaceFramelimiter = iniReader.ReadBoolean("FRAME RATE", "ReplaceFramelimiter", re4t::cfg->bReplaceFramelimiter);
	re4t::cfg->bMultithreadFix = iniReader.ReadBoolean("FRAME RATE", "MultithreadFix", re4t::cfg->bMultithreadFix);
	re4t::cfg->bPrecacheModels = iniReader.ReadBoolean("FRAME RATE", "PrecacheModels", re4t::cfg->bPrecacheModels);

	// MISC
	re4t::cfg->bNeverCheckForUpdates = iniReader.ReadBoolean("MISC", "NeverCheckForUpdates", re4t::cfg->bNeverCheckForUpdates);

	re4t::cfg->bOverrideCostumes = iniReader.ReadBoolean("MISC", "OverrideCostumes", re4t::cfg->bOverrideCostumes);
	std::string buf = iniReader.ReadString("MISC", "LeonCostume", "");
	if (!buf.empty())
	{
		if (buf == "Jacket") re4t::cfg->CostumeOverride.Leon = LeonCostume::Jacket;
		if (buf == "Normal") re4t::cfg->CostumeOverride.Leon = LeonCostume::Normal;
		if (buf == "Vest") re4t::cfg->CostumeOverride.Leon = LeonCostume::Vest;
		if (buf == "RPD") re4t::cfg->CostumeOverride.Leon = LeonCostume::RPD;
		if (buf == "Mafia") re4t::cfg->CostumeOverride.Leon = LeonCostume::Mafia;

		iCostumeComboLeon = (int)re4t::cfg->CostumeOverride.Leon;
	}

	buf = iniReader.ReadString("MISC", "AshleyCostume", "");
	if (!buf.empty())
	{
		if (buf == "Normal") re4t::cfg->CostumeOverride.Ashley = AshleyCostume::Normal;
		if (buf == "Popstar") re4t::cfg->CostumeOverride.Ashley = AshleyCostume::Popstar;
		if (buf == "Armor") re4t::cfg->CostumeOverride.Ashley = AshleyCostume::Armor;

		iCostumeComboAshley = (int)re4t::cfg->CostumeOverride.Ashley;
	}

	buf = iniReader.ReadString("MISC", "AdaCostume", "");
	if (!buf.empty())
	{
		if (buf == "RE2") re4t::cfg->CostumeOverride.Ada = AdaCostume::RE2;
		if (buf == "Spy") re4t::cfg->CostumeOverride.Ada = AdaCostume::Spy;
		if (buf == "Normal") re4t::cfg->CostumeOverride.Ada = AdaCostume::Normal;

		iCostumeComboAda = (int)re4t::cfg->CostumeOverride.Ada;

		// Normal is id 3, but we're lying to ImGui by pretending Normal is id 2 instead.
		if (re4t::cfg->CostumeOverride.Ada == AdaCostume::Normal)
			iCostumeComboAda--;
	}

	re4t::cfg->bOverrideDifficulty = iniReader.ReadBoolean("MISC", "OverrideDifficulty", re4t::cfg->bOverrideDifficulty);
	buf = iniReader.ReadString("MISC", "NewDifficulty", "");
	if (!buf.empty())
	{
		if (buf == "Amateur") re4t::cfg->NewDifficulty = GameDifficulty::VeryEasy;
		if (buf == "Easy") re4t::cfg->NewDifficulty = GameDifficulty::Easy;
		if (buf == "Normal") re4t::cfg->NewDifficulty = GameDifficulty::Medium;
		if (buf == "Professional") re4t::cfg->NewDifficulty = GameDifficulty::Pro;
	}

	re4t::cfg->bEnableNTSCMode = iniReader.ReadBoolean("MISC", "EnableNTSCMode", re4t::cfg->bEnableNTSCMode);
	re4t::cfg->bAshleyJPCameraAngles = iniReader.ReadBoolean("MISC", "AshleyJPCameraAngles", re4t::cfg->bAshleyJPCameraAngles);
	re4t::cfg->bRestoreDemoVideos = iniReader.ReadBoolean("MISC", "RestoreDemoVideos", re4t::cfg->bRestoreDemoVideos);
	re4t::cfg->bRestoreAnalogTitleScroll = iniReader.ReadBoolean("MISC", "RestoreAnalogTitleScroll", re4t::cfg->bRestoreAnalogTitleScroll);
	re4t::cfg->iViolenceLevelOverride = iniReader.ReadInteger("MISC", "ViolenceLevelOverride", re4t::cfg->iViolenceLevelOverride);
	re4t::cfg->iViolenceLevelOverride = std::min(std::max(re4t::cfg->iViolenceLevelOverride, -1), 2); // limit between -1 to 2
	re4t::cfg->bRifleScreenShake = iniReader.ReadBoolean("MISC", "RifleScreenShake", re4t::cfg->bRifleScreenShake);
	re4t::cfg->bAllowSellingHandgunSilencer = iniReader.ReadBoolean("MISC", "AllowSellingHandgunSilencer", re4t::cfg->bAllowSellingHandgunSilencer);
	re4t::cfg->bAllowMafiaLeonCutscenes = iniReader.ReadBoolean("MISC", "AllowMafiaLeonCutscenes", re4t::cfg->bAllowMafiaLeonCutscenes);
	re4t::cfg->bSilenceArmoredAshley = iniReader.ReadBoolean("MISC", "SilenceArmoredAshley", re4t::cfg->bSilenceArmoredAshley);
	re4t::cfg->bAllowAshleySuplex = iniReader.ReadBoolean("MISC", "AllowAshleySuplex", re4t::cfg->bAllowAshleySuplex);
	re4t::cfg->bAllowMatildaQuickturn = iniReader.ReadBoolean("MISC", "AllowMatildaQuickturn", re4t::cfg->bAllowMatildaQuickturn);
	re4t::cfg->bFixDitmanGlitch = iniReader.ReadBoolean("MISC", "FixDitmanGlitch", re4t::cfg->bFixDitmanGlitch);
	re4t::cfg->bUseSprintToggle = iniReader.ReadBoolean("MISC", "UseSprintToggle", re4t::cfg->bUseSprintToggle);
	re4t::cfg->bDisableQTE = iniReader.ReadBoolean("MISC", "DisableQTE", re4t::cfg->bDisableQTE);
	re4t::cfg->bAutomaticMashingQTE = iniReader.ReadBoolean("MISC", "AutomaticMashingQTE", re4t::cfg->bAutomaticMashingQTE);
	re4t::cfg->bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", re4t::cfg->bSkipIntroLogos);
	re4t::cfg->bSkipMenuFades = iniReader.ReadBoolean("MISC", "SkipMenuFades", re4t::cfg->bSkipMenuFades);
	re4t::cfg->bEnableDebugMenu = iniReader.ReadBoolean("MISC", "EnableDebugMenu", re4t::cfg->bEnableDebugMenu);
	re4t::cfg->bShowGameOutput = iniReader.ReadBoolean("MISC", "ShowGameOutput", re4t::cfg->bShowGameOutput);
	re4t::cfg->bEnableModExpansion = iniReader.ReadBoolean("MISC", "EnableModExpansion", re4t::cfg->bEnableModExpansion);
	re4t::cfg->bForceETSApplyScale = iniReader.ReadBoolean("MISC", "ForceETSApplyScale", re4t::cfg->bForceETSApplyScale);
	re4t::cfg->bLimitMatildaBurst = iniReader.ReadBoolean("MISC", "LimitMatildaBurst", re4t::cfg->bLimitMatildaBurst);
	re4t::cfg->bSeparateWaysDifficultyMenu = iniReader.ReadBoolean("MISC", "SeparateWaysDifficultyMenu", re4t::cfg->bSeparateWaysDifficultyMenu);
	re4t::cfg->bAlwaysShowOriginalTitleBackground = iniReader.ReadBoolean("MISC", "AlwaysShowOriginalTitleBackground", re4t::cfg->bAlwaysShowOriginalTitleBackground);

	// MEMORY
	re4t::cfg->bAllowHighResolutionSFD = iniReader.ReadBoolean("MEMORY", "AllowHighResolutionSFD", re4t::cfg->bAllowHighResolutionSFD);
	re4t::cfg->bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", re4t::cfg->bRaiseVertexAlloc);
	re4t::cfg->bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", re4t::cfg->bRaiseInventoryAlloc);

	// HOTKEYS
	re4t::cfg->sConfigMenuKeyCombo = StrToUpper(iniReader.ReadString("HOTKEYS", "ConfigMenu", re4t::cfg->sConfigMenuKeyCombo));
	re4t::cfg->sConsoleKeyCombo = StrToUpper(iniReader.ReadString("HOTKEYS", "Console", re4t::cfg->sConsoleKeyCombo));
	re4t::cfg->sFlipItemUp = StrToUpper(iniReader.ReadString("HOTKEYS", "FlipItemUp", re4t::cfg->sFlipItemUp));
	re4t::cfg->sFlipItemDown = StrToUpper(iniReader.ReadString("HOTKEYS", "FlipItemDown", re4t::cfg->sFlipItemDown));
	re4t::cfg->sFlipItemLeft = StrToUpper(iniReader.ReadString("HOTKEYS", "FlipItemLeft", re4t::cfg->sFlipItemLeft));
	re4t::cfg->sFlipItemRight = StrToUpper(iniReader.ReadString("HOTKEYS", "FlipItemRight", re4t::cfg->sFlipItemRight));

	re4t::cfg->sQTE_key_1 = StrToUpper(iniReader.ReadString("HOTKEYS", "QTE_key_1", re4t::cfg->sQTE_key_1));
	re4t::cfg->sQTE_key_2 = StrToUpper(iniReader.ReadString("HOTKEYS", "QTE_key_2", re4t::cfg->sQTE_key_2));
	
	// Check if the QTE bindings are valid for the current keyboard layout.
	// Try to reset them using VK Hex Codes if they aren't.
	if (pInput->vk_from_key_name(re4t::cfg->sQTE_key_1) == 0)
		re4t::cfg->sQTE_key_1 = pInput->key_name_from_vk(0x44); // Latin D

	if (pInput->vk_from_key_name(re4t::cfg->sQTE_key_2) == 0)
		re4t::cfg->sQTE_key_2 = pInput->key_name_from_vk(0x41); // Latin A

	re4t::cfg->sDebugMenuKeyCombo = StrToUpper(iniReader.ReadString("HOTKEYS", "DebugMenu", re4t::cfg->sDebugMenuKeyCombo));
	re4t::cfg->sMouseTurnModifierKeyCombo = StrToUpper(iniReader.ReadString("HOTKEYS", "MouseTurningModifier", re4t::cfg->sMouseTurnModifierKeyCombo));
	re4t::cfg->sJetSkiTrickCombo = StrToUpper(iniReader.ReadString("HOTKEYS", "JetSkiTricks", re4t::cfg->sJetSkiTrickCombo));

	// TRAINER
	re4t::cfg->bTrainerEnable = iniReader.ReadBoolean("TRAINER", "Enable", re4t::cfg->bTrainerEnable);
	re4t::cfg->bTrainerPlayerSpeedOverride = iniReader.ReadBoolean("TRAINER", "EnablePlayerSpeedOverride", re4t::cfg->bTrainerPlayerSpeedOverride);
	re4t::cfg->fTrainerPlayerSpeedOverride = iniReader.ReadFloat("TRAINER", "PlayerSpeedOverride", re4t::cfg->fTrainerPlayerSpeedOverride);
	re4t::cfg->bTrainerUseNumpadMovement = iniReader.ReadBoolean("TRAINER", "UseNumpadMovement", re4t::cfg->bTrainerUseNumpadMovement);
	re4t::cfg->bTrainerUseMouseWheelUpDown = iniReader.ReadBoolean("TRAINER", "UseMouseWheelUpDown", re4t::cfg->bTrainerUseMouseWheelUpDown);
	re4t::cfg->fTrainerNumMoveSpeed = iniReader.ReadFloat("TRAINER", "NumpadMovementSpeed", re4t::cfg->fTrainerNumMoveSpeed);
	re4t::cfg->fTrainerNumMoveSpeed = fmin(fmax(re4t::cfg->fTrainerNumMoveSpeed, 0.1f), 10.0f); // limit between 0.1 - 10
	re4t::cfg->bTrainerEnableFreeCam = iniReader.ReadBoolean("TRAINER", "EnableFreeCamera", re4t::cfg->bTrainerEnableFreeCam);
	re4t::cfg->fTrainerFreeCamSpeed = iniReader.ReadFloat("TRAINER", "FreeCamSpeed", re4t::cfg->fTrainerFreeCamSpeed);
	re4t::cfg->fTrainerFreeCamSpeed = fmin(fmax(re4t::cfg->fTrainerFreeCamSpeed, 0.1f), 10.0f); // limit between 0.1 - 10
	re4t::cfg->bTrainerEnemyHPMultiplier = iniReader.ReadBoolean("TRAINER", "EnableEnemyHPMultiplier", re4t::cfg->bTrainerEnemyHPMultiplier);
	re4t::cfg->fTrainerEnemyHPMultiplier = iniReader.ReadFloat("TRAINER", "EnemyHPMultiplier", re4t::cfg->fTrainerEnemyHPMultiplier);
	re4t::cfg->fTrainerEnemyHPMultiplier = fmin(fmax(re4t::cfg->fTrainerEnemyHPMultiplier, 0.1f), 15.0f); // limit between 0.1 - 15
	re4t::cfg->bTrainerRandomHPMultiplier = iniReader.ReadBoolean("TRAINER", "UseRandomHPMultiplier", re4t::cfg->bTrainerRandomHPMultiplier);
	re4t::cfg->fTrainerRandomHPMultiMin = iniReader.ReadFloat("TRAINER", "RandomHPMultiplierMin", re4t::cfg->fTrainerRandomHPMultiMin);
	re4t::cfg->fTrainerRandomHPMultiMin = fmin(fmax(re4t::cfg->fTrainerRandomHPMultiMin, 0.1f), 14.0f); // limit between 0.1 - 14
	re4t::cfg->fTrainerRandomHPMultiMax = iniReader.ReadFloat("TRAINER", "RandomHPMultiplierMax", re4t::cfg->fTrainerRandomHPMultiMax);
	re4t::cfg->fTrainerRandomHPMultiMax = fmin(fmax(re4t::cfg->fTrainerRandomHPMultiMax, fTrainerRandomHPMultiMin), 15.0f); // limit between fTrainerRandomHPMultiMin - 15
	re4t::cfg->bTrainerDisableEnemySpawn = iniReader.ReadBoolean("TRAINER", "DisableEnemySpawn", re4t::cfg->bTrainerDisableEnemySpawn);
	re4t::cfg->bTrainerDeadBodiesNeverDisappear = iniReader.ReadBoolean("TRAINER", "DeadBodiesNeverDisappear", re4t::cfg->bTrainerDeadBodiesNeverDisappear);
	re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh = iniReader.ReadBoolean("TRAINER", "AllowEnterDoorsWithoutAshley", re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh);
	re4t::cfg->bTrainerEnableDebugTrg = iniReader.ReadBoolean("TRAINER", "EnableDebugTrg", re4t::cfg->bTrainerEnableDebugTrg);
	re4t::cfg->bTrainerShowDebugTrgHintText = iniReader.ReadBoolean("TRAINER", "ShowDebugTrgHintText", re4t::cfg->bTrainerShowDebugTrgHintText);

	// ESP
	re4t::cfg->bShowESP = iniReader.ReadBoolean("ESP", "ShowESP", re4t::cfg->bShowESP);
	re4t::cfg->bEspShowInfoOnTop = iniReader.ReadBoolean("ESP", "ShowInfoOnTop", re4t::cfg->bEspShowInfoOnTop);
	re4t::cfg->bEspOnlyShowEnemies = iniReader.ReadBoolean("ESP", "OnlyShowEnemies", re4t::cfg->bEspOnlyShowEnemies);
	re4t::cfg->bEspOnlyShowValidEms = iniReader.ReadBoolean("ESP", "OnlyShowValidEms", re4t::cfg->bEspOnlyShowValidEms);
	re4t::cfg->bEspOnlyShowESLSpawned = iniReader.ReadBoolean("ESP", "OnlyShowESLSpawned", re4t::cfg->bEspOnlyShowESLSpawned);
	re4t::cfg->bEspOnlyShowAlive = iniReader.ReadBoolean("ESP", "OnlyShowAlive", re4t::cfg->bEspOnlyShowAlive);
	re4t::cfg->fEspMaxEmDistance = iniReader.ReadFloat("ESP", "MaxEmDistance", re4t::cfg->fEspMaxEmDistance);
	re4t::cfg->bEspOnlyShowClosestEms = iniReader.ReadBoolean("ESP", "OnlyShowClosestEms", re4t::cfg->bEspOnlyShowClosestEms);
	re4t::cfg->iEspClosestEmsAmount = iniReader.ReadInteger("ESP", "ClosestEmsAmount", re4t::cfg->iEspClosestEmsAmount);
	re4t::cfg->bEspDrawLines = iniReader.ReadBoolean("ESP", "DrawLines", re4t::cfg->bEspDrawLines);

	buf = iniReader.ReadString("ESP", "EmNameMode", "");
	if (!buf.empty())
	{
		if (buf == "DontShow") re4t::cfg->iEspEmNameMode = 0;
		if (buf == "Normal") re4t::cfg->iEspEmNameMode = 1;
		if (buf == "Simplified") re4t::cfg->iEspEmNameMode = 2;
	}

	buf = iniReader.ReadString("ESP", "EmHPMode", "");
	if (!buf.empty())
	{
		if (buf == "DontShow") re4t::cfg->iEspEmHPMode = 0;
		if (buf == "Bar") re4t::cfg->iEspEmHPMode = 1;
		if (buf == "Text") re4t::cfg->iEspEmHPMode = 2;
	}

	re4t::cfg->bEspDrawDebugInfo = iniReader.ReadBoolean("ESP", "DrawDebugInfo", re4t::cfg->bEspDrawDebugInfo);

	// SIDEINFO
	re4t::cfg->bShowSideInfo = iniReader.ReadBoolean("SIDEINFO", "ShowSideInfo", re4t::cfg->bShowSideInfo);
	re4t::cfg->bSideShowEmCount = iniReader.ReadBoolean("SIDEINFO", "ShowEmCount", re4t::cfg->bSideShowEmCount);
	re4t::cfg->bSideShowEmList = iniReader.ReadBoolean("SIDEINFO", "ShowEmList", re4t::cfg->bSideShowEmList);
	re4t::cfg->bSideOnlyShowESLSpawned = iniReader.ReadBoolean("SIDEINFO", "OnlyShowESLSpawned", re4t::cfg->bSideOnlyShowESLSpawned);
	re4t::cfg->bSideShowSimpleNames = iniReader.ReadBoolean("SIDEINFO", "ShowSimpleNames", re4t::cfg->bSideShowSimpleNames);
	re4t::cfg->iSideClosestEmsAmount = iniReader.ReadInteger("SIDEINFO", "ClosestEmsAmount", re4t::cfg->iSideClosestEmsAmount);
	re4t::cfg->fSideMaxEmDistance = iniReader.ReadFloat("SIDEINFO", "MaxEmDistance", re4t::cfg->fSideMaxEmDistance);

	buf = iniReader.ReadString("SIDEINFO", "EmHPMode", "");
	if (!buf.empty())
	{
		if (buf == "DontShow") re4t::cfg->iSideEmHPMode = 0;
		if (buf == "Bar") re4t::cfg->iSideEmHPMode = 1;
		if (buf == "Text") re4t::cfg->iSideEmHPMode = 2;
	}

	// TRAINER HOTKEYS
	re4t::cfg->sTrainerFocusUIKeyCombo = iniReader.ReadString("TRAINER_HOTKEYS", "FocusUI", re4t::cfg->sTrainerFocusUIKeyCombo);
	re4t::cfg->sTrainerNoclipKeyCombo = iniReader.ReadString("TRAINER_HOTKEYS", "NoclipToggle", re4t::cfg->sTrainerNoclipKeyCombo);
	re4t::cfg->sTrainerFreeCamKeyCombo = iniReader.ReadString("TRAINER_HOTKEYS", "FreeCamToggle", re4t::cfg->sTrainerFreeCamKeyCombo);
	re4t::cfg->sTrainerSpeedOverrideKeyCombo = iniReader.ReadString("TRAINER_HOTKEYS", "SpeedOverrideToggle", re4t::cfg->sTrainerSpeedOverrideKeyCombo);
	re4t::cfg->sTrainerMoveAshToPlayerKeyCombo = iniReader.ReadString("TRAINER_HOTKEYS", "MoveAshleyToPlayer", re4t::cfg->sTrainerMoveAshToPlayerKeyCombo);
	re4t::cfg->sTrainerDebugTrgKeyCombo = iniReader.ReadString("TRAINER_HOTKEYS", "DebugTrg", re4t::cfg->sTrainerDebugTrgKeyCombo);

	// WEAPON HOTKEYS
	re4t::cfg->bWeaponHotkeysEnable = iniReader.ReadBoolean("WEAPON_HOTKEYS", "Enable", re4t::cfg->bWeaponHotkeysEnable);
	re4t::cfg->sWeaponHotkeys[0] = iniReader.ReadString("WEAPON_HOTKEYS", "WeaponHotkeySlot1", re4t::cfg->sWeaponHotkeys[0]);
	re4t::cfg->sWeaponHotkeys[1] = iniReader.ReadString("WEAPON_HOTKEYS", "WeaponHotkeySlot2", re4t::cfg->sWeaponHotkeys[1]);
	re4t::cfg->sWeaponHotkeys[2] = iniReader.ReadString("WEAPON_HOTKEYS", "WeaponHotkeySlot3", re4t::cfg->sWeaponHotkeys[2]);
	re4t::cfg->sWeaponHotkeys[3] = iniReader.ReadString("WEAPON_HOTKEYS", "WeaponHotkeySlot4", re4t::cfg->sWeaponHotkeys[3]);
	re4t::cfg->sWeaponHotkeys[4] = iniReader.ReadString("WEAPON_HOTKEYS", "WeaponHotkeySlot5", re4t::cfg->sWeaponHotkeys[4]);
	re4t::cfg->sLastWeaponHotkey = iniReader.ReadString("WEAPON_HOTKEYS", "LastWeaponHotkey", re4t::cfg->sLastWeaponHotkey);
	re4t::cfg->iWeaponHotkeyWepIds[0] = iniReader.ReadInteger("WEAPON_HOTKEYS", "WeaponIdSlot1", re4t::cfg->iWeaponHotkeyWepIds[0]);
	re4t::cfg->iWeaponHotkeyWepIds[1] = iniReader.ReadInteger("WEAPON_HOTKEYS", "WeaponIdSlot2", re4t::cfg->iWeaponHotkeyWepIds[1]);
	re4t::cfg->iWeaponHotkeyWepIds[2] = iniReader.ReadInteger("WEAPON_HOTKEYS", "WeaponIdSlot3", re4t::cfg->iWeaponHotkeyWepIds[2]);
	re4t::cfg->iWeaponHotkeyWepIds[3] = iniReader.ReadInteger("WEAPON_HOTKEYS", "WeaponIdSlot4", re4t::cfg->iWeaponHotkeyWepIds[3]);
	re4t::cfg->iWeaponHotkeyWepIds[4] = iniReader.ReadInteger("WEAPON_HOTKEYS", "WeaponIdSlot5", re4t::cfg->iWeaponHotkeyWepIds[4]);
	auto readIntVect = [&iniReader](std::string section, std::string key, std::string& default_value)
	{
		std::vector<int> ret;

		default_value = iniReader.ReadString(section, key, default_value);

		std::stringstream ss(default_value);

		for (int i; ss >> i;) {
			ret.push_back(i);
			int peeked = ss.peek();
			if (peeked == ',' || peeked == ' ')
				ss.ignore();
		}

		return ret;
	};
	re4t::cfg->iWeaponHotkeyCycle[0] = readIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot1", re4t::cfg->iWeaponHotkeyCycleString[0]);
	re4t::cfg->iWeaponHotkeyCycle[1] = readIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot2", re4t::cfg->iWeaponHotkeyCycleString[1]);
	re4t::cfg->iWeaponHotkeyCycle[2] = readIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot3", re4t::cfg->iWeaponHotkeyCycleString[2]);
	re4t::cfg->iWeaponHotkeyCycle[3] = readIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot4", re4t::cfg->iWeaponHotkeyCycleString[3]);
	re4t::cfg->iWeaponHotkeyCycle[4] = readIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot5", re4t::cfg->iWeaponHotkeyCycleString[4]);

	// Parse all hotkeys
	ParseHotkeys();

	// FPS WARNING
	re4t::cfg->bIgnoreFPSWarning = iniReader.ReadBoolean("WARNING", "IgnoreFPSWarning", re4t::cfg->bIgnoreFPSWarning);
	
	// IMGUI
	re4t::cfg->fFontSizeScale = iniReader.ReadFloat("IMGUI", "FontSizeScale", re4t::cfg->fFontSizeScale);
	re4t::cfg->fFontSizeScale = fmin(fmax(re4t::cfg->fFontSizeScale, 1.0f), 1.25f); // limit between 1.0 - 1.25

	re4t::cfg->bEnableDPIScale = iniReader.ReadBoolean("IMGUI", "EnableDPIScale", re4t::cfg->bEnableDPIScale);
	re4t::cfg->bDisableMenuTip = iniReader.ReadBoolean("IMGUI", "DisableMenuTip", re4t::cfg->bDisableMenuTip);

	// DEBUG
	re4t::cfg->bVerboseLog = iniReader.ReadBoolean("DEBUG", "VerboseLog", re4t::cfg->bVerboseLog);
	re4t::cfg->bNeverHideCursor = iniReader.ReadBoolean("DEBUG", "NeverHideCursor", re4t::cfg->bNeverHideCursor);
	re4t::cfg->bUseDynamicFrametime = iniReader.ReadBoolean("DEBUG", "UseDynamicFrametime", re4t::cfg->bUseDynamicFrametime);
	re4t::cfg->bDisableFramelimiting = iniReader.ReadBoolean("DEBUG", "DisableFramelimiting", re4t::cfg->bDisableFramelimiting);

	if (iniReader.ReadBoolean("DEBUG", "TweaksDevMode", TweaksDevMode))
		TweaksDevMode = true; // let the INI enable it if it's disabled, but not disable it
}

std::mutex settingsThreadRunningMutex;

void WriteSettings(std::wstring iniPath, bool trainerIni)
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // only allow single thread writing to INI at one time

	CIniReader iniReader(WstrToStr(iniPath));

	#ifdef VERBOSE
	con.log("Writing settings to: %s", WstrToStr(iniPath));
	#endif

	// Copy the default .ini to folder if one doesn't exist, just so we can keep comments and descriptions intact.
	if (!std::filesystem::exists(iniPath)) {
		#ifdef VERBOSE
		con.log("ini file doesn't exist in folder. Creating new one.");
		#endif

		std::filesystem::create_directory(std::filesystem::path(iniPath).parent_path()); // Create the dir if it doesn't exist

		const auto copyOptions = std::filesystem::copy_options::overwrite_existing;

		if (!trainerIni)
		{
			if (std::filesystem::exists(rootPath + L"re4_tweaks\\default_settings\\settings.ini"))
				std::filesystem::copy(rootPath + L"re4_tweaks\\default_settings\\settings.ini", iniPath, copyOptions);
		}
		else
		{
			if (std::filesystem::exists(rootPath + L"re4_tweaks\\default_settings\\trainer_settings.ini"))
				std::filesystem::copy(rootPath + L"re4_tweaks\\default_settings\\trainer_settings.ini", iniPath, copyOptions);
		}
	}

	// Try to remove read-only flag is it is set, for some reason.
	DWORD iniFile = GetFileAttributesW(iniPath.data());
	if (iniFile != INVALID_FILE_ATTRIBUTES) {
		bool isReadOnly = iniFile & FILE_ATTRIBUTE_READONLY;

		if (isReadOnly)
		{
			#ifdef VERBOSE
			con.log("Read-only ini file detected. Attempting to remove flag");
			#endif

			spd::log()->info("{} -> Read-only ini file detected. Attempting to remove flag", __FUNCTION__);

			SetFileAttributesW(iniPath.c_str(), iniFile & ~FILE_ATTRIBUTE_READONLY);
		}
	}

	if (trainerIni)
	{
		// trainer.ini-only settings
		iniReader = CIniReader(WstrToStr(iniPath));

		// TRAINER
		iniReader.WriteBoolean("TRAINER", "Enable", re4t::cfg->bTrainerEnable);
		iniReader.WriteBoolean("TRAINER", "EnablePlayerSpeedOverride", re4t::cfg->bTrainerPlayerSpeedOverride);
		iniReader.WriteFloat("TRAINER", "PlayerSpeedOverride", re4t::cfg->fTrainerPlayerSpeedOverride);
		iniReader.WriteBoolean("TRAINER", "UseNumpadMovement", re4t::cfg->bTrainerUseNumpadMovement);
		iniReader.WriteBoolean("TRAINER", "UseMouseWheelUpDown", re4t::cfg->bTrainerUseMouseWheelUpDown);
		iniReader.WriteFloat("TRAINER", "NumpadMovementSpeed", re4t::cfg->fTrainerNumMoveSpeed);
		iniReader.WriteBoolean("TRAINER", "EnableFreeCamera", re4t::cfg->bTrainerEnableFreeCam);
		iniReader.WriteFloat("TRAINER", "FreeCamSpeed", re4t::cfg->fTrainerFreeCamSpeed);
		iniReader.WriteBoolean("TRAINER", "EnableEnemyHPMultiplier", re4t::cfg->bTrainerEnemyHPMultiplier);
		iniReader.WriteFloat("TRAINER", "EnemyHPMultiplier", re4t::cfg->fTrainerEnemyHPMultiplier);
		iniReader.WriteBoolean("TRAINER", "UseRandomHPMultiplier", re4t::cfg->bTrainerRandomHPMultiplier);
		iniReader.WriteFloat("TRAINER", "RandomHPMultiplierMin", re4t::cfg->fTrainerRandomHPMultiMin);
		iniReader.WriteFloat("TRAINER", "RandomHPMultiplierMax", re4t::cfg->fTrainerRandomHPMultiMax);
		iniReader.WriteBoolean("TRAINER", "DisableEnemySpawn", re4t::cfg->bTrainerDisableEnemySpawn);
		iniReader.WriteBoolean("TRAINER", "DeadBodiesNeverDisappear", re4t::cfg->bTrainerDeadBodiesNeverDisappear);
		iniReader.WriteBoolean("TRAINER", "AllowEnterDoorsWithoutAshley", re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh);
		iniReader.WriteBoolean("TRAINER", "EnableDebugTrg", re4t::cfg->bTrainerEnableDebugTrg);
		iniReader.WriteBoolean("TRAINER", "ShowDebugTrgHintText", re4t::cfg->bTrainerShowDebugTrgHintText);

		// ESP
		iniReader.WriteBoolean("ESP", "ShowESP", re4t::cfg->bShowESP);
		iniReader.WriteBoolean("ESP", "ShowInfoOnTop", re4t::cfg->bEspShowInfoOnTop);
		iniReader.WriteBoolean("ESP", "OnlyShowEnemies", re4t::cfg->bEspOnlyShowEnemies);
		iniReader.WriteBoolean("ESP", "OnlyShowValidEms", re4t::cfg->bEspOnlyShowValidEms);
		iniReader.WriteBoolean("ESP", "OnlyShowESLSpawned", re4t::cfg->bEspOnlyShowESLSpawned);
		iniReader.WriteBoolean("ESP", "OnlyShowAlive", re4t::cfg->bEspOnlyShowAlive);
		iniReader.WriteFloat("ESP", "MaxEmDistance", re4t::cfg->fEspMaxEmDistance);
		iniReader.WriteBoolean("ESP", "OnlyShowClosestEms", re4t::cfg->bEspOnlyShowClosestEms);
		iniReader.WriteInteger("ESP", "ClosestEmsAmount", re4t::cfg->iEspClosestEmsAmount);
		iniReader.WriteBoolean("ESP", "DrawLines", re4t::cfg->bEspDrawLines);

		std::string buf;
		switch (re4t::cfg->iEspEmNameMode) {
		case 0:
			buf = "DontShow";
			break;
		case 1:
			buf = "Normal";
			break;
		case 2:
			buf = "Simplified";
			break;
		} iniReader.WriteString("ESP", "EmNameMode", " " + buf);

		switch (re4t::cfg->iEspEmHPMode) {
		case 0:
			buf = "DontShow";
			break;
		case 1:
			buf = "Bar";
			break;
		case 2:
			buf = "Text";
			break;
		} iniReader.WriteString("ESP", "EmHPMode", " " + buf);

		iniReader.WriteBoolean("ESP", "DrawDebugInfo", re4t::cfg->bEspDrawDebugInfo);

		// SIDEINFO
		iniReader.WriteBoolean("SIDEINFO", "ShowSideInfo", re4t::cfg->bShowSideInfo);
		iniReader.WriteBoolean("SIDEINFO", "ShowEmCount", re4t::cfg->bSideShowEmCount);
		iniReader.WriteBoolean("SIDEINFO", "ShowEmList", re4t::cfg->bSideShowEmList);
		iniReader.WriteBoolean("SIDEINFO", "OnlyShowESLSpawned", re4t::cfg->bSideOnlyShowESLSpawned);
		iniReader.WriteBoolean("SIDEINFO", "ShowSimpleNames", re4t::cfg->bSideShowSimpleNames);
		iniReader.WriteInteger("SIDEINFO", "ClosestEmsAmount", re4t::cfg->iSideClosestEmsAmount);
		iniReader.WriteFloat("SIDEINFO", "MaxEmDistance", re4t::cfg->fSideMaxEmDistance);

		switch (re4t::cfg->iSideEmHPMode) {
		case 0:
			buf = "DontShow";
			break;
		case 1:
			buf = "Bar";
			break;
		case 2:
			buf = "Text";
			break;
		} iniReader.WriteString("SIDEINFO", "EmHPMode", " " + buf);

		// TRAINER_HOTKEYS
		iniReader.WriteString("TRAINER_HOTKEYS", "FocusUI", " " + re4t::cfg->sTrainerFocusUIKeyCombo);
		iniReader.WriteString("TRAINER_HOTKEYS", "NoclipToggle", " " + re4t::cfg->sTrainerNoclipKeyCombo);
		iniReader.WriteString("TRAINER_HOTKEYS", "FreeCamToggle", " " + re4t::cfg->sTrainerFreeCamKeyCombo);
		iniReader.WriteString("TRAINER_HOTKEYS", "SpeedOverrideToggle", " " + re4t::cfg->sTrainerSpeedOverrideKeyCombo);
		iniReader.WriteString("TRAINER_HOTKEYS", "MoveAshleyToPlayer", " " + re4t::cfg->sTrainerMoveAshToPlayerKeyCombo);
		iniReader.WriteString("TRAINER_HOTKEYS", "DebugTrg", " " + re4t::cfg->sTrainerDebugTrgKeyCombo);

		// WEAPON HOTKEYS
		iniReader.WriteBoolean("WEAPON_HOTKEYS", "Enable", re4t::cfg->bWeaponHotkeysEnable);
		iniReader.WriteString("WEAPON_HOTKEYS", "WeaponHotkeySlot1", " " + re4t::cfg->sWeaponHotkeys[0]);
		iniReader.WriteString("WEAPON_HOTKEYS", "WeaponHotkeySlot2", " " + re4t::cfg->sWeaponHotkeys[1]);
		iniReader.WriteString("WEAPON_HOTKEYS", "WeaponHotkeySlot3", " " + re4t::cfg->sWeaponHotkeys[2]);
		iniReader.WriteString("WEAPON_HOTKEYS", "WeaponHotkeySlot4", " " + re4t::cfg->sWeaponHotkeys[3]);
		iniReader.WriteString("WEAPON_HOTKEYS", "WeaponHotkeySlot5", " " + re4t::cfg->sWeaponHotkeys[4]);
		iniReader.WriteInteger("WEAPON_HOTKEYS", "WeaponIdSlot1", re4t::cfg->iWeaponHotkeyWepIds[0]);
		iniReader.WriteInteger("WEAPON_HOTKEYS", "WeaponIdSlot2", re4t::cfg->iWeaponHotkeyWepIds[1]);
		iniReader.WriteInteger("WEAPON_HOTKEYS", "WeaponIdSlot3", re4t::cfg->iWeaponHotkeyWepIds[2]);
		iniReader.WriteInteger("WEAPON_HOTKEYS", "WeaponIdSlot4", re4t::cfg->iWeaponHotkeyWepIds[3]);
		iniReader.WriteInteger("WEAPON_HOTKEYS", "WeaponIdSlot5", re4t::cfg->iWeaponHotkeyWepIds[4]);

		auto writeIntVect = [&iniReader](std::string section, std::string key, std::vector<int>& vect) {
			std::string val = "";
			for (int num : vect)
				val += std::to_string(num) + ", ";
			if (!val.empty())
				val = val.substr(0, val.size() - 2);
			iniReader.WriteString(section, key, " " + val);
		};

		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot1", re4t::cfg->iWeaponHotkeyCycle[0]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot2", re4t::cfg->iWeaponHotkeyCycle[1]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot3", re4t::cfg->iWeaponHotkeyCycle[2]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot4", re4t::cfg->iWeaponHotkeyCycle[3]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot5", re4t::cfg->iWeaponHotkeyCycle[4]);

		iniReader.WriteString("WEAPON_HOTKEYS", "LastWeaponHotkey", " " + re4t::cfg->sLastWeaponHotkey);

		return;
	}

	// VULKAN
	iniReader.WriteBoolean("VULKAN", "UseVulkanRenderer", re4t::dxvk::cfg->bUseVulkanRenderer);
	iniReader.WriteBoolean("VULKAN", "ShowFPS", re4t::dxvk::cfg->bShowFPS);
	iniReader.WriteBoolean("VULKAN", "ShowGPULoad", re4t::dxvk::cfg->bShowGPULoad);
	iniReader.WriteBoolean("VULKAN", "ShowDeviceInfo", re4t::dxvk::cfg->bShowDeviceInfo);
	iniReader.WriteBoolean("VULKAN", "DisableAsync", re4t::dxvk::cfg->bDisableAsync);

	// DISPLAY
	iniReader.WriteFloat("DISPLAY", "FOVAdditional", re4t::cfg->fFOVAdditional);
	iniReader.WriteBoolean("DISPLAY", "DisableVsync", re4t::cfg->bDisableVsync);

	iniReader.WriteBoolean("DISPLAY", "UltraWideAspectSupport", re4t::cfg->bUltraWideAspectSupport);
	iniReader.WriteBoolean("DISPLAY", "SideAlignHUD", re4t::cfg->bSideAlignHUD);
	iniReader.WriteBoolean("DISPLAY", "StretchFullscreenImages", re4t::cfg->bStretchFullscreenImages);
	iniReader.WriteBoolean("DISPLAY", "StretchVideos", re4t::cfg->bStretchVideos);
	iniReader.WriteBoolean("DISPLAY", "Remove16by10BlackBars", re4t::cfg->bRemove16by10BlackBars);
	iniReader.WriteBoolean("DISPLAY", "FixDPIScale", re4t::cfg->bFixDPIScale);
	iniReader.WriteBoolean("DISPLAY", "FixDisplayMode", re4t::cfg->bFixDisplayMode);
	iniReader.WriteInteger("DISPLAY", "CustomRefreshRate", re4t::cfg->iCustomRefreshRate);
	iniReader.WriteBoolean("DISPLAY", "OverrideLaserColor", re4t::cfg->bOverrideLaserColor);
	iniReader.WriteBoolean("DISPLAY", "RainbowLaser", re4t::cfg->bRainbowLaser);

	iniReader.WriteInteger("DISPLAY", "LaserR", re4t::cfg->iLaserR);
	iniReader.WriteInteger("DISPLAY", "LaserG", re4t::cfg->iLaserG);
	iniReader.WriteInteger("DISPLAY", "LaserB", re4t::cfg->iLaserB);

	iniReader.WriteBoolean("DISPLAY", "RestorePickupTransparency", re4t::cfg->bRestorePickupTransparency);
	iniReader.WriteBoolean("DISPLAY", "DisableBrokenFilter03", re4t::cfg->bDisableBrokenFilter03);
	iniReader.WriteBoolean("DISPLAY", "FixBlurryImage", re4t::cfg->bFixBlurryImage);
	iniReader.WriteBoolean("DISPLAY", "DisableFilmGrain", re4t::cfg->bDisableFilmGrain);
	iniReader.WriteBoolean("DISPLAY", "ImproveWater", re4t::cfg->bImproveWater);
	iniReader.WriteBoolean("DISPLAY", "EnableGCBlur", re4t::cfg->bEnableGCBlur);

	if (re4t::cfg->bUseEnhancedGCBlur)
		iniReader.WriteString("DISPLAY", "GCBlurType", "Enhanced");
	else
		iniReader.WriteString("DISPLAY", "GCBlurType", "Classic");

	iniReader.WriteBoolean("DISPLAY", "EnableGCScopeBlur", re4t::cfg->bEnableGCScopeBlur);
	iniReader.WriteBoolean("DISPLAY", "WindowBorderless", re4t::cfg->bWindowBorderless);
	iniReader.WriteInteger("DISPLAY", "WindowPositionX", re4t::cfg->iWindowPositionX);
	iniReader.WriteInteger("DISPLAY", "WindowPositionY", re4t::cfg->iWindowPositionY);
	iniReader.WriteBoolean("DISPLAY", "RememberWindowPos", re4t::cfg->bRememberWindowPos);

	// AUDIO
	iniReader.WriteInteger("AUDIO", "VolumeMaster", re4t::cfg->iVolumeMaster);
	iniReader.WriteInteger("AUDIO", "VolumeBGM", re4t::cfg->iVolumeBGM);
	iniReader.WriteInteger("AUDIO", "VolumeSE", re4t::cfg->iVolumeSE);
	iniReader.WriteInteger("AUDIO", "VolumeCutscene", re4t::cfg->iVolumeCutscene);
	iniReader.WriteBoolean("AUDIO", "RestoreGCSoundEffects", re4t::cfg->bRestoreGCSoundEffects);

	// MOUSE
	iniReader.WriteBoolean("MOUSE", "CameraImprovements", re4t::cfg->bCameraImprovements);
	iniReader.WriteBoolean("MOUSE", "ResetCameraAfterUsingWeapons", re4t::cfg->bResetCameraAfterUsingWeapons);
	iniReader.WriteBoolean("MOUSE", "ResetCameraAfterUsingKnife", re4t::cfg->bResetCameraAfterUsingKnife);
	iniReader.WriteBoolean("MOUSE", "ResetCameraWhenRunning", re4t::cfg->bResetCameraWhenRunning);
	iniReader.WriteFloat("MOUSE", "CameraSensitivity", re4t::cfg->fCameraSensitivity);
	iniReader.WriteBoolean("MOUSE", "UseMouseTurning", re4t::cfg->bUseMouseTurning);

	if (re4t::cfg->iMouseTurnType == MouseTurnTypes::TypeA)
		iniReader.WriteString("MOUSE", "MouseTurnType", "TypeA");
	else
		iniReader.WriteString("MOUSE", "MouseTurnType", "TypeB");

	iniReader.WriteFloat("MOUSE", "TurnTypeBSensitivity", re4t::cfg->fTurnTypeBSensitivity);
	iniReader.WriteBoolean("MOUSE", "UseRawMouseInput", re4t::cfg->bUseRawMouseInput);
	iniReader.WriteBoolean("MOUSE", "DetachCameraFromAim", re4t::cfg->bDetachCameraFromAim);
	iniReader.WriteBoolean("MOUSE", "FixSniperZoom", re4t::cfg->bFixSniperZoom);
	iniReader.WriteBoolean("MOUSE", "FixSniperFocus", re4t::cfg->bFixSniperFocus);
	iniReader.WriteBoolean("MOUSE", "FixRetryLoadMouseSelector", re4t::cfg->bFixRetryLoadMouseSelector);

	// KEYBOARD
	iniReader.WriteBoolean("KEYBOARD", "FallbackToEnglishKeyIcons", re4t::cfg->bFallbackToEnglishKeyIcons);
	iniReader.WriteBoolean("KEYBOARD", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_kbm);
	iniReader.WriteBoolean("KEYBOARD", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_kbm);

	// CONTROLLER
	iniReader.WriteBoolean("CONTROLLER", "OverrideControllerSensitivity", re4t::cfg->bOverrideControllerSensitivity);
	iniReader.WriteFloat("CONTROLLER", "ControllerSensitivity", re4t::cfg->fControllerSensitivity);
	iniReader.WriteBoolean("CONTROLLER", "RemoveExtraXinputDeadzone", re4t::cfg->bRemoveExtraXinputDeadzone);
	iniReader.WriteBoolean("CONTROLLER", "OverrideXinputDeadzone", re4t::cfg->bOverrideXinputDeadzone);
	iniReader.WriteFloat("CONTROLLER", "XinputDeadzone", re4t::cfg->fXinputDeadzone);
	iniReader.WriteBoolean("CONTROLLER", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_controller);
	iniReader.WriteBoolean("CONTROLLER", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_controller);

	// FRAME RATE
	iniReader.WriteBoolean("FRAME RATE", "FixFallingItemsSpeed", re4t::cfg->bFixFallingItemsSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixCompartmentsOpeningSpeed", re4t::cfg->bFixCompartmentsOpeningSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixMovingGeometrySpeed", re4t::cfg->bFixMovingGeometrySpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixTurningSpeed", re4t::cfg->bFixTurningSpeed);
	iniReader.WriteBoolean("FRAME RATE", "FixQTE", re4t::cfg->bFixQTE);
	iniReader.WriteBoolean("FRAME RATE", "FixAshleyBustPhysics", re4t::cfg->bFixAshleyBustPhysics);
	iniReader.WriteBoolean("FRAME RATE", "EnableFastMath", re4t::cfg->bEnableFastMath);
	iniReader.WriteBoolean("FRAME RATE", "ReplaceFramelimiter", re4t::cfg->bReplaceFramelimiter);
	iniReader.WriteBoolean("FRAME RATE", "MultithreadFix", re4t::cfg->bMultithreadFix);
	iniReader.WriteBoolean("FRAME RATE", "PrecacheModels", re4t::cfg->bPrecacheModels);

	// MISC
	iniReader.WriteBoolean("MISC", "NeverCheckForUpdates", re4t::cfg->bNeverCheckForUpdates);
	iniReader.WriteBoolean("MISC", "OverrideCostumes", re4t::cfg->bOverrideCostumes);
	iniReader.WriteString("MISC", "LeonCostume", " " + std::string(sLeonCostumeNames[iCostumeComboLeon]));
	iniReader.WriteString("MISC", "AshleyCostume", " " + std::string(sAshleyCostumeNames[iCostumeComboAshley]));
	iniReader.WriteString("MISC", "AdaCostume", " " + std::string(sAdaCostumeNames[iCostumeComboAda]));
	iniReader.WriteBoolean("MISC", "OverrideDifficulty", re4t::cfg->bOverrideDifficulty);
	iniReader.WriteString("MISC", "NewDifficulty", " " + std::string(sGameDifficultyNames[int(re4t::cfg->NewDifficulty)]));
	iniReader.WriteBoolean("MISC", "EnableNTSCMode", re4t::cfg->bEnableNTSCMode);
	iniReader.WriteBoolean("MISC", "AshleyJPCameraAngles", re4t::cfg->bAshleyJPCameraAngles);
	iniReader.WriteBoolean("MISC", "RestoreDemoVideos", re4t::cfg->bRestoreDemoVideos);
	iniReader.WriteBoolean("MISC", "RestoreAnalogTitleScroll", re4t::cfg->bRestoreAnalogTitleScroll);
	iniReader.WriteInteger("MISC", "ViolenceLevelOverride", re4t::cfg->iViolenceLevelOverride);
	iniReader.WriteBoolean("MISC", "RifleScreenShake", re4t::cfg->bRifleScreenShake);
	iniReader.WriteBoolean("MISC", "AllowSellingHandgunSilencer", re4t::cfg->bAllowSellingHandgunSilencer);
	iniReader.WriteBoolean("MISC", "AllowMafiaLeonCutscenes", re4t::cfg->bAllowMafiaLeonCutscenes);
	iniReader.WriteBoolean("MISC", "SilenceArmoredAshley", re4t::cfg->bSilenceArmoredAshley);
	iniReader.WriteBoolean("MISC", "AllowAshleySuplex", re4t::cfg->bAllowAshleySuplex);
	iniReader.WriteBoolean("MISC", "AllowMatildaQuickturn", re4t::cfg->bAllowMatildaQuickturn);
	iniReader.WriteBoolean("MISC", "FixDitmanGlitch", re4t::cfg->bFixDitmanGlitch);
	iniReader.WriteBoolean("MISC", "UseSprintToggle", re4t::cfg->bUseSprintToggle);
	iniReader.WriteBoolean("MISC", "DisableQTE", re4t::cfg->bDisableQTE);
	iniReader.WriteBoolean("MISC", "AutomaticMashingQTE", re4t::cfg->bAutomaticMashingQTE);
	iniReader.WriteBoolean("MISC", "SkipIntroLogos", re4t::cfg->bSkipIntroLogos);
	iniReader.WriteBoolean("MISC", "SkipMenuFades", re4t::cfg->bSkipMenuFades);
	iniReader.WriteBoolean("MISC", "LimitMatildaBurst", re4t::cfg->bLimitMatildaBurst);
	iniReader.WriteBoolean("MISC", "SeparateWaysDifficultyMenu", re4t::cfg->bSeparateWaysDifficultyMenu);
	iniReader.WriteBoolean("MISC", "AlwaysShowOriginalTitleBackground", re4t::cfg->bAlwaysShowOriginalTitleBackground);
	iniReader.WriteBoolean("MISC", "EnableDebugMenu", re4t::cfg->bEnableDebugMenu);
	iniReader.WriteBoolean("MISC", "ShowGameOutput", re4t::cfg->bShowGameOutput);
	// Not writing EnableModExpansion / ForceETSApplyScale back to users INI in case those were enabled by a mod override INI (which the user might want to remove later)
	// We don't have any UI options for those anyway, so pointless for us to write it back

	// MEMORY
	iniReader.WriteBoolean("MEMORY", "AllowHighResolutionSFD", re4t::cfg->bAllowHighResolutionSFD);
	iniReader.WriteBoolean("MEMORY", "RaiseVertexAlloc", re4t::cfg->bRaiseVertexAlloc);
	iniReader.WriteBoolean("MEMORY", "RaiseInventoryAlloc", re4t::cfg->bRaiseInventoryAlloc);

	// HOTKEYS
	iniReader.WriteString("HOTKEYS", "ConfigMenu", " " + re4t::cfg->sConfigMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "Console", " " + re4t::cfg->sConsoleKeyCombo);
	iniReader.WriteString("HOTKEYS", "FlipItemUp", " " + re4t::cfg->sFlipItemUp);
	iniReader.WriteString("HOTKEYS", "FlipItemDown", " " + re4t::cfg->sFlipItemDown);
	iniReader.WriteString("HOTKEYS", "FlipItemLeft", " " + re4t::cfg->sFlipItemLeft);
	iniReader.WriteString("HOTKEYS", "FlipItemRight", " " + re4t::cfg->sFlipItemRight);
	iniReader.WriteString("HOTKEYS", "QTE_key_1", " " + re4t::cfg->sQTE_key_1);
	iniReader.WriteString("HOTKEYS", "QTE_key_2", " " + re4t::cfg->sQTE_key_2);
	iniReader.WriteString("HOTKEYS", "DebugMenu", " " + re4t::cfg->sDebugMenuKeyCombo);
	iniReader.WriteString("HOTKEYS", "MouseTurningModifier", " " + re4t::cfg->sMouseTurnModifierKeyCombo);
	iniReader.WriteString("HOTKEYS", "JetSkiTricks", " " + re4t::cfg->sJetSkiTrickCombo);

	// IMGUI
	iniReader.WriteFloat("IMGUI", "FontSizeScale", re4t::cfg->fFontSizeScale);
}

DWORD WINAPI WriteSettingsThread(LPVOID lpParameter)
{
	std::wstring iniPathMain = rootPath + wrapperName + L".ini";
	std::wstring iniPathTrainer = rootPath + L"\\re4_tweaks\\trainer.ini";
	WriteSettings(iniPathMain, false);
	WriteSettings(iniPathTrainer, true);

	re4t::cfg->HasUnsavedChanges = false;

	return 0;
}

void re4t_cfg::WriteSettings()
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // if thread is already running, wait for it to finish

	// Spawn a new thread to handle writing settings, as INI writing funcs that get used are pretty slow
	CreateThreadAutoClose(NULL, 0, WriteSettingsThread, NULL, 0, NULL);
}

void re4t_cfg::LogSettings()
{
	spd::log()->info("+--------------------------------+-----------------+");
	spd::log()->info("| Setting                        | Value           |");
	spd::log()->info("+--------------------------------+-----------------+");

	// DISPLAY
	spd::log()->info("+ DISPLAY------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "FOVAdditional", re4t::cfg->fFOVAdditional);
	spd::log()->info("| {:<30} | {:>15} |", "DisableVsync", re4t::cfg->bDisableVsync ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "UltraWideAspectSupport", re4t::cfg->bUltraWideAspectSupport ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SideAlignHUD", re4t::cfg->bSideAlignHUD ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "StretchFullscreenImages", re4t::cfg->bStretchFullscreenImages ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "StretchVideos", re4t::cfg->bStretchVideos ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "Remove16by10BlackBars", re4t::cfg->bRemove16by10BlackBars ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixDPIScale", re4t::cfg->bFixDPIScale ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixDisplayMode", re4t::cfg->bFixDisplayMode ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "CustomRefreshRate", re4t::cfg->iCustomRefreshRate);
	spd::log()->info("| {:<30} | {:>15} |", "OverrideLaserColor", re4t::cfg->bOverrideLaserColor ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RainbowLaser", re4t::cfg->bRainbowLaser ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "LaserR", re4t::cfg->iLaserR);
	spd::log()->info("| {:<30} | {:>15} |", "LaserG", re4t::cfg->iLaserG);
	spd::log()->info("| {:<30} | {:>15} |", "LaserB", re4t::cfg->iLaserB);
	spd::log()->info("| {:<30} | {:>15} |", "RestorePickupTransparency", re4t::cfg->bRestorePickupTransparency ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DisableBrokenFilter03", re4t::cfg->bDisableBrokenFilter03 ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixBlurryImage", re4t::cfg->bFixBlurryImage ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DisableFilmGrain", re4t::cfg->bDisableFilmGrain ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ImproveWater", re4t::cfg->bImproveWater ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableGCBlur", re4t::cfg->bEnableGCBlur ? "true" : "false");
	
	if (re4t::cfg->bUseEnhancedGCBlur)
		spd::log()->info("| {:<30} | {:>15} |", "GCBlurType", "Enhanced");
	else
		spd::log()->info("| {:<30} | {:>15} |", "GCBlurType", "Classic");

	spd::log()->info("| {:<30} | {:>15} |", "EnableGCScopeBlur", re4t::cfg->bEnableGCScopeBlur ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "WindowBorderless", re4t::cfg->bWindowBorderless ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "WindowPositionX", re4t::cfg->iWindowPositionX);
	spd::log()->info("| {:<30} | {:>15} |", "WindowPositionY", re4t::cfg->iWindowPositionY);
	spd::log()->info("| {:<30} | {:>15} |", "RememberWindowPos", re4t::cfg->bRememberWindowPos ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// AUDIO
	spd::log()->info("+ AUDIO--------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "VolumeMaster", re4t::cfg->iVolumeMaster);
	spd::log()->info("| {:<30} | {:>15} |", "VolumeBGM", re4t::cfg->iVolumeBGM);
	spd::log()->info("| {:<30} | {:>15} |", "VolumeSE", re4t::cfg->iVolumeSE);
	spd::log()->info("| {:<30} | {:>15} |", "VolumeCutscene", re4t::cfg->iVolumeCutscene);
	spd::log()->info("| {:<30} | {:>15} |", "RestoreGCSoundEffects", re4t::cfg->bRestoreGCSoundEffects ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// MOUSE
	spd::log()->info("+ MOUSE--------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "CameraImprovements", re4t::cfg->bCameraImprovements ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ResetCameraAfterUsingWeapons", re4t::cfg->bResetCameraAfterUsingWeapons ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ResetCameraAfterUsingKnife", re4t::cfg->bResetCameraAfterUsingKnife ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ResetCameraWhenRunning", re4t::cfg->bResetCameraWhenRunning ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "CameraSensitivity", re4t::cfg->fCameraSensitivity);
	spd::log()->info("| {:<30} | {:>15} |", "UseMouseTurning", re4t::cfg->bUseMouseTurning ? "true" : "false");

	if (re4t::cfg->iMouseTurnType == MouseTurnTypes::TypeA)
		spd::log()->info("| {:<30} | {:>15} |", "MouseTurnType", "TypeA");
	else
		spd::log()->info("| {:<30} | {:>15} |", "MouseTurnType", "TypeB");

	spd::log()->info("| {:<30} | {:>15} |", "TurnTypeBSensitivity", re4t::cfg->fTurnTypeBSensitivity);
	spd::log()->info("| {:<30} | {:>15} |", "UseRawMouseInput", re4t::cfg->bUseRawMouseInput ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DetachCameraFromAim", re4t::cfg->bDetachCameraFromAim ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixSniperZoom", re4t::cfg->bFixSniperZoom ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixSniperFocus", re4t::cfg->bFixSniperFocus ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixRetryLoadMouseSelector", re4t::cfg->bFixRetryLoadMouseSelector ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// KEYBOARD
	spd::log()->info("+ KEYBOARD-----------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "FallbackToEnglishKeyIcons", re4t::cfg->bFallbackToEnglishKeyIcons ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_kbm ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_kbm ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// CONTROLLER
	spd::log()->info("+ CONTROLLER---------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "OverrideControllerSensitivity", re4t::cfg->bOverrideControllerSensitivity ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ControllerSensitivity", re4t::cfg->fControllerSensitivity);
	spd::log()->info("| {:<30} | {:>15} |", "RemoveExtraXinputDeadzone", re4t::cfg->bRemoveExtraXinputDeadzone ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OverrideXinputDeadzone", re4t::cfg->bOverrideXinputDeadzone ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "XinputDeadzone", re4t::cfg->fXinputDeadzone);
	spd::log()->info("| {:<30} | {:>15} |", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_controller ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_controller ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// FRAME RATE
	spd::log()->info("+ FRAME RATE---------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "FixFallingItemsSpeed", re4t::cfg->bFixFallingItemsSpeed ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixCompartmentsOpeningSpeed", re4t::cfg->bFixCompartmentsOpeningSpeed ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixMovingGeometrySpeed", re4t::cfg->bFixMovingGeometrySpeed ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixTurningSpeed", re4t::cfg->bFixTurningSpeed ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixQTE", re4t::cfg->bFixQTE ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixAshleyBustPhysics", re4t::cfg->bFixAshleyBustPhysics ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableFastMath", re4t::cfg->bEnableFastMath ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ReplaceFramelimiter", re4t::cfg->bReplaceFramelimiter ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "MultithreadFix", re4t::cfg->bMultithreadFix ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "PrecacheModels", re4t::cfg->bPrecacheModels ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// MISC
	spd::log()->info("+ MISC---------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "WrappedDllPath", re4t::cfg->sWrappedDllPath.data());
	spd::log()->info("| {:<30} | {:>15} |", "NeverCheckForUpdates", re4t::cfg->bNeverCheckForUpdates ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OverrideCostumes", re4t::cfg->bOverrideCostumes ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "LeonCostume", sLeonCostumeNames[iCostumeComboLeon]);
	spd::log()->info("| {:<30} | {:>15} |", "AshleyCostume", sAshleyCostumeNames[iCostumeComboAshley]);
	spd::log()->info("| {:<30} | {:>15} |", "AdaCostume", sAdaCostumeNames[iCostumeComboAda]);
	spd::log()->info("| {:<30} | {:>15} |", "OverrideDifficulty", re4t::cfg->bOverrideDifficulty ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "NewDifficulty", sGameDifficultyNames[int(re4t::cfg->NewDifficulty)]);
	spd::log()->info("| {:<30} | {:>15} |", "AshleyJPCameraAngles", re4t::cfg->bAshleyJPCameraAngles ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RestoreDemoVideos", re4t::cfg->bRestoreDemoVideos ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RestoreAnalogTitleScroll", re4t::cfg->bRestoreAnalogTitleScroll ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ViolenceLevelOverride", re4t::cfg->iViolenceLevelOverride);
	spd::log()->info("| {:<30} | {:>15} |", "RifleScreenShake", re4t::cfg->bRifleScreenShake ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowSellingHandgunSilencer", re4t::cfg->bAllowSellingHandgunSilencer ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowMafiaLeonCutscenes", re4t::cfg->bAllowMafiaLeonCutscenes ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SilenceArmoredAshley", re4t::cfg->bSilenceArmoredAshley ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowAshleySuplex", re4t::cfg->bAllowAshleySuplex ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowMatildaQuickturn", re4t::cfg->bAllowMatildaQuickturn ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixDitmanGlitch", re4t::cfg->bFixDitmanGlitch ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "UseSprintToggle", re4t::cfg->bUseSprintToggle ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DisableQTE", re4t::cfg->bDisableQTE ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AutomaticMashingQTE", re4t::cfg->bAutomaticMashingQTE ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SkipIntroLogos", re4t::cfg->bSkipIntroLogos ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SkipMenuFades", re4t::cfg->bSkipMenuFades ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableDebugMenu", re4t::cfg->bEnableDebugMenu ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowGameOutput", re4t::cfg->bShowGameOutput ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableModExpansion", re4t::cfg->bEnableModExpansion ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ForceETSApplyScale", re4t::cfg->bForceETSApplyScale ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableNTSCMode", re4t::cfg->bEnableNTSCMode ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "LimitMatildaBurst", re4t::cfg->bLimitMatildaBurst ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SeparateWaysDifficultyMenu", re4t::cfg->bSeparateWaysDifficultyMenu ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AlwaysShowOriginalTitleBackground", re4t::cfg->bAlwaysShowOriginalTitleBackground ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// MEMORY
	spd::log()->info("+ MEMORY-------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "AllowHighResolutionSFD", re4t::cfg->bAllowHighResolutionSFD ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RaiseVertexAlloc", re4t::cfg->bRaiseVertexAlloc ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RaiseInventoryAlloc", re4t::cfg->bRaiseInventoryAlloc ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// HOTKEYS
	spd::log()->info("+ HOTKEYS------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "ConfigMenu", re4t::cfg->sConfigMenuKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "Console", re4t::cfg->sConsoleKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "FlipItemUp", re4t::cfg->sFlipItemUp.data());
	spd::log()->info("| {:<30} | {:>15} |", "FlipItemDown", re4t::cfg->sFlipItemDown.data());
	spd::log()->info("| {:<30} | {:>15} |", "FlipItemLeft", re4t::cfg->sFlipItemLeft.data());
	spd::log()->info("| {:<30} | {:>15} |", "FlipItemRight", re4t::cfg->sFlipItemRight.data());
	spd::log()->info("| {:<30} | {:>15} |", "QTE_key_1", re4t::cfg->sQTE_key_1.data());
	spd::log()->info("| {:<30} | {:>15} |", "QTE_key_2", re4t::cfg->sQTE_key_2.data());
	spd::log()->info("| {:<30} | {:>15} |", "DebugMenu", re4t::cfg->sDebugMenuKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "MouseTurningModifier", re4t::cfg->sMouseTurnModifierKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "JetSkiTricks", re4t::cfg->sJetSkiTrickCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "FocusUI", re4t::cfg->sTrainerFocusUIKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "NoclipToggle", re4t::cfg->sTrainerNoclipKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "SpeedOverrideToggle", re4t::cfg->sTrainerSpeedOverrideKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "MoveAshleyToPlayer", re4t::cfg->sTrainerMoveAshToPlayerKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "DebugTrg", re4t::cfg->sTrainerDebugTrgKeyCombo.data());
	spd::log()->info("+--------------------------------+-----------------+");

	// FPS WARNING
	spd::log()->info("+ WARNING------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "IgnoreFPSWarning", re4t::cfg->bIgnoreFPSWarning ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// IMGUI
	spd::log()->info("+ IMGUI--------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "FontSizeScale", re4t::cfg->fFontSizeScale);
	spd::log()->info("| {:<30} | {:>15} |", "DisableMenuTip", re4t::cfg->bDisableMenuTip ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// DEBUG
	spd::log()->info("+ DEBUG--------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "VerboseLog", re4t::cfg->bVerboseLog ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "NeverHideCursor", re4t::cfg->bNeverHideCursor ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "UseDynamicFrametime", re4t::cfg->bUseDynamicFrametime ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DisableFramelimiting", re4t::cfg->bDisableFramelimiting ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// HDPROJECT
	spd::log()->info("+ HDPROJECT--------------------------+-------------+");
	spd::log()->info("| {:<30} | {:>15} |", "IsUsingHDProject", bIsUsingHDProject ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");
}