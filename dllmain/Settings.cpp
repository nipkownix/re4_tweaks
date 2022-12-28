#include <algorithm>
#include <filesystem>
#include <iostream>
#include <mutex>
#include "dllmain.h"
#include <iniReader.h>
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
const char* sAdaCostumeNames[] = {"Resident Evil 2", "Spy", "RE2_d" , "Normal"};

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
	iniReader ini(WstrToStr(ini_path));

	#ifdef VERBOSE
	con.log("Reading settings from: %s", WstrToStr(ini_path).data());
	#endif

	spd::log()->info("Reading settings from: \"{}\"", WstrToStr(ini_path).data());

	re4t::cfg->HasUnsavedChanges = false;
	
	// VULKAN
	re4t::dxvk::cfg->bUseVulkanRenderer = ini.getBool("VULKAN", "UseVulkanRenderer", re4t::dxvk::cfg->bUseVulkanRenderer);
	re4t::dxvk::cfg->bShowFPS = ini.getBool("VULKAN", "ShowFPS", re4t::dxvk::cfg->bShowFPS);
	re4t::dxvk::cfg->bShowGPULoad = ini.getBool("VULKAN", "ShowGPULoad", re4t::dxvk::cfg->bShowGPULoad);
	re4t::dxvk::cfg->bShowDeviceInfo = ini.getBool("VULKAN", "ShowDeviceInfo", re4t::dxvk::cfg->bShowDeviceInfo);
	re4t::dxvk::cfg->bDisableAsync = ini.getBool("VULKAN", "DisableAsync", re4t::dxvk::cfg->bDisableAsync);

	re4t::dxvk::cfg->DXVK_HUD = ini.getString("VULKAN", "DXVK_HUD", re4t::dxvk::cfg->DXVK_HUD);
	re4t::dxvk::cfg->DXVK_FILTER_DEVICE_NAME = ini.getString("VULKAN", "DXVK_FILTER_DEVICE_NAME", re4t::dxvk::cfg->DXVK_FILTER_DEVICE_NAME);

	// DISPLAY
	re4t::cfg->fFOVAdditional = ini.getFloat("DISPLAY", "FOVAdditional", re4t::cfg->fFOVAdditional);
	if (re4t::cfg->fFOVAdditional > 0.0f)
		re4t::cfg->bEnableFOV = true;
	else
		re4t::cfg->bEnableFOV = false;

	re4t::cfg->bDisableVsync = ini.getBool("DISPLAY", "DisableVsync", re4t::cfg->bDisableVsync);

	re4t::cfg->bUltraWideAspectSupport = ini.getBool("DISPLAY", "UltraWideAspectSupport", re4t::cfg->bUltraWideAspectSupport);
	re4t::cfg->bSideAlignHUD = ini.getBool("DISPLAY", "SideAlignHUD", re4t::cfg->bSideAlignHUD);
	re4t::cfg->bStretchFullscreenImages = ini.getBool("DISPLAY", "StretchFullscreenImages", re4t::cfg->bStretchFullscreenImages);
	re4t::cfg->bStretchVideos = ini.getBool("DISPLAY", "StretchVideos", re4t::cfg->bStretchVideos);
	re4t::cfg->bRemove16by10BlackBars = ini.getBool("DISPLAY", "Remove16by10BlackBars", re4t::cfg->bRemove16by10BlackBars);

	re4t::cfg->bFixDPIScale = ini.getBool("DISPLAY", "FixDPIScale", re4t::cfg->bFixDPIScale);
	re4t::cfg->bFixDisplayMode = ini.getBool("DISPLAY", "FixDisplayMode", re4t::cfg->bFixDisplayMode);
	re4t::cfg->iCustomRefreshRate = ini.getInt("DISPLAY", "CustomRefreshRate", re4t::cfg->iCustomRefreshRate);
	re4t::cfg->bOverrideLaserColor = ini.getBool("DISPLAY", "OverrideLaserColor", re4t::cfg->bOverrideLaserColor);
	re4t::cfg->bRainbowLaser = ini.getBool("DISPLAY", "RainbowLaser", re4t::cfg->bRainbowLaser);

	re4t::cfg->iLaserR = ini.getInt("DISPLAY", "LaserR", re4t::cfg->iLaserR);
	re4t::cfg->iLaserG = ini.getInt("DISPLAY", "LaserG", re4t::cfg->iLaserG);
	re4t::cfg->iLaserB = ini.getInt("DISPLAY", "LaserB", re4t::cfg->iLaserB);
	fLaserColorPicker[0] = re4t::cfg->iLaserR / 255.0f;
	fLaserColorPicker[1] = re4t::cfg->iLaserG / 255.0f;
	fLaserColorPicker[2] = re4t::cfg->iLaserB / 255.0f;

	re4t::cfg->bRestorePickupTransparency = ini.getBool("DISPLAY", "RestorePickupTransparency", re4t::cfg->bRestorePickupTransparency);
	re4t::cfg->bDisableBrokenFilter03 = ini.getBool("DISPLAY", "DisableBrokenFilter03", re4t::cfg->bDisableBrokenFilter03);
	re4t::cfg->bFixBlurryImage = ini.getBool("DISPLAY", "FixBlurryImage", re4t::cfg->bFixBlurryImage);
	re4t::cfg->bDisableFilmGrain = ini.getBool("DISPLAY", "DisableFilmGrain", re4t::cfg->bDisableFilmGrain);

	re4t::cfg->bImproveWater = ini.getBool("DISPLAY", "ImproveWater", re4t::cfg->bImproveWater);
	if (re4t::cfg->bIsUsingHDProject)
	{
		re4t::cfg->bImproveWater = false; // Should be inside HDProject.ini instead, but we'll keep it here untill a new version of the HD Project is released
	}

	re4t::cfg->bEnableGCBlur = ini.getBool("DISPLAY", "EnableGCBlur", re4t::cfg->bEnableGCBlur);

	std::string GCBlurTypeStr = ini.getString("DISPLAY", "GCBlurType", "");
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

	re4t::cfg->bEnableGCScopeBlur = ini.getBool("DISPLAY", "EnableGCScopeBlur", re4t::cfg->bEnableGCScopeBlur);
	re4t::cfg->bWindowBorderless = ini.getBool("DISPLAY", "WindowBorderless", re4t::cfg->bWindowBorderless);
	re4t::cfg->iWindowPositionX = ini.getInt("DISPLAY", "WindowPositionX", re4t::cfg->iWindowPositionX);
	re4t::cfg->iWindowPositionY = ini.getInt("DISPLAY", "WindowPositionY", re4t::cfg->iWindowPositionY);
	re4t::cfg->bRememberWindowPos = ini.getBool("DISPLAY", "RememberWindowPos", re4t::cfg->bRememberWindowPos);

	// AUDIO
	re4t::cfg->iVolumeMaster = ini.getInt("AUDIO", "VolumeMaster", re4t::cfg->iVolumeMaster);
	re4t::cfg->iVolumeMaster = std::min(std::max(re4t::cfg->iVolumeMaster, 0), 100); // limit between 0 - 100

	re4t::cfg->iVolumeBGM = ini.getInt("AUDIO", "VolumeBGM", re4t::cfg->iVolumeBGM);
	re4t::cfg->iVolumeBGM = std::min(std::max(re4t::cfg->iVolumeBGM, 0), 100); // limit between 0 - 100

	re4t::cfg->iVolumeSE = ini.getInt("AUDIO", "VolumeSE", re4t::cfg->iVolumeSE);
	re4t::cfg->iVolumeSE = std::min(std::max(re4t::cfg->iVolumeSE, 0), 100); // limit between 0 - 100

	re4t::cfg->iVolumeCutscene = ini.getInt("AUDIO", "VolumeCutscene", re4t::cfg->iVolumeCutscene);
	re4t::cfg->iVolumeCutscene = std::min(std::max(re4t::cfg->iVolumeCutscene, 0), 100); // limit between 0 - 100

	re4t::cfg->bRestoreGCSoundEffects = ini.getBool("AUDIO", "RestoreGCSoundEffects", re4t::cfg->bRestoreGCSoundEffects);
	re4t::cfg->bSilenceArmoredAshley = ini.getBool("AUDIO", "SilenceArmoredAshley", re4t::cfg->bSilenceArmoredAshley);

	// MOUSE
	re4t::cfg->bCameraImprovements = ini.getBool("MOUSE", "CameraImprovements", re4t::cfg->bCameraImprovements);
	re4t::cfg->bResetCameraAfterUsingWeapons = ini.getBool("MOUSE", "ResetCameraAfterUsingWeapons", re4t::cfg->bResetCameraAfterUsingWeapons);
	re4t::cfg->bResetCameraAfterUsingKnife = ini.getBool("MOUSE", "ResetCameraAfterUsingKnife", re4t::cfg->bResetCameraAfterUsingKnife);
	re4t::cfg->bResetCameraWhenRunning = ini.getBool("MOUSE", "ResetCameraWhenRunning", re4t::cfg->bResetCameraWhenRunning);
	re4t::cfg->fCameraSensitivity = ini.getFloat("MOUSE", "CameraSensitivity", re4t::cfg->fCameraSensitivity);
	re4t::cfg->fCameraSensitivity = fmin(fmax(re4t::cfg->fCameraSensitivity, 0.5f), 2.0f); // limit between 0.5 - 2.0
	re4t::cfg->bUseMouseTurning = ini.getBool("MOUSE", "UseMouseTurning", re4t::cfg->bUseMouseTurning);
	
	std::string MouseTurnTypeStr = ini.getString("MOUSE", "MouseTurnType", "");
	if (!MouseTurnTypeStr.empty())
	{
		if (MouseTurnTypeStr == std::string("TypeA"))
			re4t::cfg->iMouseTurnType = MouseTurnTypes::TypeA;
		else if (MouseTurnTypeStr == std::string("TypeB"))
			re4t::cfg->iMouseTurnType = MouseTurnTypes::TypeB;
	}

	re4t::cfg->fTurnTypeBSensitivity = ini.getFloat("MOUSE", "TurnTypeBSensitivity", re4t::cfg->fTurnTypeBSensitivity);
	re4t::cfg->fTurnTypeBSensitivity = fmin(fmax(re4t::cfg->fTurnTypeBSensitivity, 0.5f), 2.0f); // limit between 0.5 - 2.0
	re4t::cfg->bUseRawMouseInput = ini.getBool("MOUSE", "UseRawMouseInput", re4t::cfg->bUseRawMouseInput);
	re4t::cfg->bDetachCameraFromAim = ini.getBool("MOUSE", "DetachCameraFromAim", re4t::cfg->bDetachCameraFromAim);
	re4t::cfg->bFixSniperZoom = ini.getBool("MOUSE", "FixSniperZoom", re4t::cfg->bFixSniperZoom);
	re4t::cfg->bFixSniperFocus = ini.getBool("MOUSE", "FixSniperFocus", re4t::cfg->bFixSniperFocus);
	re4t::cfg->bFixRetryLoadMouseSelector = ini.getBool("MOUSE", "FixRetryLoadMouseSelector", re4t::cfg->bFixRetryLoadMouseSelector);

	// KEYBOARD
	re4t::cfg->bFallbackToEnglishKeyIcons = ini.getBool("KEYBOARD", "FallbackToEnglishKeyIcons", re4t::cfg->bFallbackToEnglishKeyIcons);
	re4t::cfg->bAllowReloadWithoutAiming_kbm = ini.getBool("KEYBOARD", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_kbm);
	re4t::cfg->bReloadWithoutZoom_kbm = ini.getBool("KEYBOARD", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_kbm);

	// CONTROLLER
	re4t::cfg->bOverrideControllerSensitivity = ini.getBool("CONTROLLER", "OverrideControllerSensitivity", re4t::cfg->bOverrideControllerSensitivity);
	re4t::cfg->fControllerSensitivity = ini.getFloat("CONTROLLER", "ControllerSensitivity", re4t::cfg->fControllerSensitivity);
	re4t::cfg->fControllerSensitivity = fmin(fmax(re4t::cfg->fControllerSensitivity, 0.5f), 4.0f); // limit between 0.5 - 4.0
	re4t::cfg->bRemoveExtraXinputDeadzone = ini.getBool("CONTROLLER", "RemoveExtraXinputDeadzone", re4t::cfg->bRemoveExtraXinputDeadzone);
	re4t::cfg->bOverrideXinputDeadzone = ini.getBool("CONTROLLER", "OverrideXinputDeadzone", re4t::cfg->bOverrideXinputDeadzone);
	re4t::cfg->fXinputDeadzone = ini.getFloat("CONTROLLER", "XinputDeadzone", re4t::cfg->fXinputDeadzone);
	re4t::cfg->fXinputDeadzone = fmin(fmax(re4t::cfg->fXinputDeadzone, 0.0f), 3.5f); // limit between 0.0 - 3.5
	re4t::cfg->bAllowReloadWithoutAiming_controller = ini.getBool("CONTROLLER", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_controller);
	re4t::cfg->bReloadWithoutZoom_controller = ini.getBool("CONTROLLER", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_controller);

	// FRAME RATE
	re4t::cfg->bFixFallingItemsSpeed = ini.getBool("FRAME RATE", "FixFallingItemsSpeed", re4t::cfg->bFixFallingItemsSpeed);
	re4t::cfg->bFixCompartmentsOpeningSpeed = ini.getBool("FRAME RATE", "FixCompartmentsOpeningSpeed", re4t::cfg->bFixCompartmentsOpeningSpeed);
	re4t::cfg->bFixMovingGeometrySpeed = ini.getBool("FRAME RATE", "FixMovingGeometrySpeed", re4t::cfg->bFixMovingGeometrySpeed);
	re4t::cfg->bFixTurningSpeed = ini.getBool("FRAME RATE", "FixTurningSpeed", re4t::cfg->bFixTurningSpeed);
	re4t::cfg->bFixQTE = ini.getBool("FRAME RATE", "FixQTE", re4t::cfg->bFixQTE);
	re4t::cfg->bFixAshleyBustPhysics = ini.getBool("FRAME RATE", "FixAshleyBustPhysics", re4t::cfg->bFixAshleyBustPhysics);
	re4t::cfg->bEnableFastMath = ini.getBool("FRAME RATE", "EnableFastMath", re4t::cfg->bEnableFastMath);
	re4t::cfg->bReplaceFramelimiter = ini.getBool("FRAME RATE", "ReplaceFramelimiter", re4t::cfg->bReplaceFramelimiter);
	re4t::cfg->bMultithreadFix = ini.getBool("FRAME RATE", "MultithreadFix", re4t::cfg->bMultithreadFix);
	re4t::cfg->bPrecacheModels = ini.getBool("FRAME RATE", "PrecacheModels", re4t::cfg->bPrecacheModels);

	// GAMEPLAY
	re4t::cfg->bAshleyJPCameraAngles = ini.getBool("GAMEPLAY", "AshleyJPCameraAngles", re4t::cfg->bAshleyJPCameraAngles);
	re4t::cfg->bSeparateWaysProfessional = ini.getBool("GAMEPLAY", "SeparateWaysProfessional", re4t::cfg->bSeparateWaysProfessional);
	re4t::cfg->bEnableNTSCMode = ini.getBool("GAMEPLAY", "EnableNTSCMode", re4t::cfg->bEnableNTSCMode);
	re4t::cfg->bAllowAshleySuplex = ini.getBool("GAMEPLAY", "AllowAshleySuplex", re4t::cfg->bAllowAshleySuplex);
	re4t::cfg->bFixDitmanGlitch = ini.getBool("GAMEPLAY", "FixDitmanGlitch", re4t::cfg->bFixDitmanGlitch);
	re4t::cfg->bAllowSellingHandgunSilencer = ini.getBool("GAMEPLAY", "AllowSellingHandgunSilencer", re4t::cfg->bAllowSellingHandgunSilencer);
	re4t::cfg->bBalancedChicagoTypewriter = ini.getBool("GAMEPLAY", "BalancedChicagoTypewriter", re4t::cfg->bBalancedChicagoTypewriter);
	re4t::cfg->bUseSprintToggle = ini.getBool("GAMEPLAY", "UseSprintToggle", re4t::cfg->bUseSprintToggle);
	re4t::cfg->bRifleScreenShake = ini.getBool("GAMEPLAY", "RifleScreenShake", re4t::cfg->bRifleScreenShake);
	re4t::cfg->bDisableQTE = ini.getBool("GAMEPLAY", "DisableQTE", re4t::cfg->bDisableQTE);
	re4t::cfg->bAutomaticMashingQTE = ini.getBool("GAMEPLAY", "AutomaticMashingQTE", re4t::cfg->bAutomaticMashingQTE);
	re4t::cfg->bAllowMatildaQuickturn = ini.getBool("GAMEPLAY", "AllowMatildaQuickturn", re4t::cfg->bAllowMatildaQuickturn);
	re4t::cfg->bLimitMatildaBurst = ini.getBool("GAMEPLAY", "LimitMatildaBurst", re4t::cfg->bLimitMatildaBurst);

	// MISC
	re4t::cfg->bNeverCheckForUpdates = ini.getBool("MISC", "NeverCheckForUpdates", re4t::cfg->bNeverCheckForUpdates);
	re4t::cfg->bRestoreDemoVideos = ini.getBool("MISC", "RestoreDemoVideos", re4t::cfg->bRestoreDemoVideos);
	re4t::cfg->bRestoreAnalogTitleScroll = ini.getBool("MISC", "RestoreAnalogTitleScroll", re4t::cfg->bRestoreAnalogTitleScroll);
	re4t::cfg->iViolenceLevelOverride = ini.getInt("MISC", "ViolenceLevelOverride", re4t::cfg->iViolenceLevelOverride);
	re4t::cfg->iViolenceLevelOverride = std::min(std::max(re4t::cfg->iViolenceLevelOverride, -1), 2); // limit between -1 to 2
	re4t::cfg->bAllowMafiaLeonCutscenes = ini.getBool("MISC", "AllowMafiaLeonCutscenes", re4t::cfg->bAllowMafiaLeonCutscenes);
	re4t::cfg->bSkipIntroLogos = ini.getBool("MISC", "SkipIntroLogos", re4t::cfg->bSkipIntroLogos);
	re4t::cfg->bSkipMenuFades = ini.getBool("MISC", "SkipMenuFades", re4t::cfg->bSkipMenuFades);
	re4t::cfg->bSpeedUpQuitGame = ini.getBool("MISC", "SpeedUpQuitGame", re4t::cfg->bSpeedUpQuitGame);
	re4t::cfg->bEnableDebugMenu = ini.getBool("MISC", "EnableDebugMenu", re4t::cfg->bEnableDebugMenu);
	re4t::cfg->bShowGameOutput = ini.getBool("MISC", "ShowGameOutput", re4t::cfg->bShowGameOutput);
	re4t::cfg->bEnableModExpansion = ini.getBool("MISC", "EnableModExpansion", re4t::cfg->bEnableModExpansion);
	re4t::cfg->bForceETSApplyScale = ini.getBool("MISC", "ForceETSApplyScale", re4t::cfg->bForceETSApplyScale);
	re4t::cfg->bAlwaysShowOriginalTitleBackground = ini.getBool("MISC", "AlwaysShowOriginalTitleBackground", re4t::cfg->bAlwaysShowOriginalTitleBackground);
	re4t::cfg->bHideZoomControlHints = ini.getBool("MISC", "HideZoomControlHints", re4t::cfg->bHideZoomControlHints);
	re4t::cfg->bFixSilencedHandgunDescription = ini.getBool("MISC", "FixSilencedHandgunDescription", re4t::cfg->bFixSilencedHandgunDescription);

	// MEMORY
	re4t::cfg->bAllowHighResolutionSFD = ini.getBool("MEMORY", "AllowHighResolutionSFD", re4t::cfg->bAllowHighResolutionSFD);
	re4t::cfg->bRaiseVertexAlloc = ini.getBool("MEMORY", "RaiseVertexAlloc", re4t::cfg->bRaiseVertexAlloc);
	re4t::cfg->bRaiseInventoryAlloc = ini.getBool("MEMORY", "RaiseInventoryAlloc", re4t::cfg->bRaiseInventoryAlloc);

	// HOTKEYS
	re4t::cfg->sConfigMenuKeyCombo = StrToUpper(ini.getString("HOTKEYS", "ConfigMenu", re4t::cfg->sConfigMenuKeyCombo));
	re4t::cfg->sConsoleKeyCombo = StrToUpper(ini.getString("HOTKEYS", "Console", re4t::cfg->sConsoleKeyCombo));
	re4t::cfg->sFlipItemUp = StrToUpper(ini.getString("HOTKEYS", "FlipItemUp", re4t::cfg->sFlipItemUp));
	re4t::cfg->sFlipItemDown = StrToUpper(ini.getString("HOTKEYS", "FlipItemDown", re4t::cfg->sFlipItemDown));
	re4t::cfg->sFlipItemLeft = StrToUpper(ini.getString("HOTKEYS", "FlipItemLeft", re4t::cfg->sFlipItemLeft));
	re4t::cfg->sFlipItemRight = StrToUpper(ini.getString("HOTKEYS", "FlipItemRight", re4t::cfg->sFlipItemRight));

	re4t::cfg->sQTE_key_1 = StrToUpper(ini.getString("HOTKEYS", "QTE_key_1", re4t::cfg->sQTE_key_1));
	re4t::cfg->sQTE_key_2 = StrToUpper(ini.getString("HOTKEYS", "QTE_key_2", re4t::cfg->sQTE_key_2));
	
	// Check if the QTE bindings are valid for the current keyboard layout.
	// Try to reset them using VK Hex Codes if they aren't.
	if (pInput->vk_from_key_name(re4t::cfg->sQTE_key_1) == 0)
		re4t::cfg->sQTE_key_1 = pInput->key_name_from_vk(0x44); // Latin D

	if (pInput->vk_from_key_name(re4t::cfg->sQTE_key_2) == 0)
		re4t::cfg->sQTE_key_2 = pInput->key_name_from_vk(0x41); // Latin A

	re4t::cfg->sDebugMenuKeyCombo = StrToUpper(ini.getString("HOTKEYS", "DebugMenu", re4t::cfg->sDebugMenuKeyCombo));
	re4t::cfg->sMouseTurnModifierKeyCombo = StrToUpper(ini.getString("HOTKEYS", "MouseTurningModifier", re4t::cfg->sMouseTurnModifierKeyCombo));
	re4t::cfg->sJetSkiTrickCombo = StrToUpper(ini.getString("HOTKEYS", "JetSkiTricks", re4t::cfg->sJetSkiTrickCombo));

	// TRAINER
	re4t::cfg->bTrainerEnable = ini.getBool("TRAINER", "Enable", re4t::cfg->bTrainerEnable);

	// PATCHES
	re4t::cfg->bTrainerUseNumpadMovement = ini.getBool("PATCHES", "UseNumpadMovement", re4t::cfg->bTrainerUseNumpadMovement);
	re4t::cfg->bTrainerUseMouseWheelUpDown = ini.getBool("PATCHES", "UseMouseWheelUpDown", re4t::cfg->bTrainerUseMouseWheelUpDown);
	re4t::cfg->fTrainerNumMoveSpeed = ini.getFloat("PATCHES", "NumpadMovementSpeed", re4t::cfg->fTrainerNumMoveSpeed);
	re4t::cfg->fTrainerNumMoveSpeed = fmin(fmax(re4t::cfg->fTrainerNumMoveSpeed, 0.1f), 10.0f); // limit between 0.1 - 10
	re4t::cfg->bTrainerEnableFreeCam = ini.getBool("PATCHES", "EnableFreeCamera", re4t::cfg->bTrainerEnableFreeCam);
	re4t::cfg->fTrainerFreeCamSpeed = ini.getFloat("PATCHES", "FreeCamSpeed", re4t::cfg->fTrainerFreeCamSpeed);
	re4t::cfg->fTrainerFreeCamSpeed = fmin(fmax(re4t::cfg->fTrainerFreeCamSpeed, 0.1f), 10.0f); // limit between 0.1 - 10
	re4t::cfg->bTrainerDisableEnemySpawn = ini.getBool("PATCHES", "DisableEnemySpawn", re4t::cfg->bTrainerDisableEnemySpawn);
	re4t::cfg->bTrainerDeadBodiesNeverDisappear = ini.getBool("PATCHES", "DeadBodiesNeverDisappear", re4t::cfg->bTrainerDeadBodiesNeverDisappear);
	re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh = ini.getBool("PATCHES", "AllowEnterDoorsWithoutAshley", re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh);
	re4t::cfg->bTrainerEnableDebugTrg = ini.getBool("PATCHES", "EnableDebugTrg", re4t::cfg->bTrainerEnableDebugTrg);
	re4t::cfg->bTrainerShowDebugTrgHintText = ini.getBool("PATCHES", "ShowDebugTrgHintText", re4t::cfg->bTrainerShowDebugTrgHintText);
	re4t::cfg->bTrainerOpenInventoryOnItemAdd = ini.getBool("PATCHES", "OpenInventoryOnItemAdd", re4t::cfg->bTrainerOpenInventoryOnItemAdd);
	re4t::cfg->iTrainerLastAreaJumpStage = ini.getInt("PATCHES", "LastAreaJumpStage", re4t::cfg->iTrainerLastAreaJumpStage);
	re4t::cfg->iTrainerLastAreaJumpRoomIdx = ini.getInt("PATCHES", "LastAreaJumpRoomIdx", re4t::cfg->iTrainerLastAreaJumpRoomIdx);

	// OVERRIDES
	re4t::cfg->bOverrideCostumes = ini.getBool("OVERRIDES", "OverrideCostumes", re4t::cfg->bOverrideCostumes);
	std::string buf = ini.getString("OVERRIDES", "LeonCostume", "");
	if (!buf.empty())
	{
		if (buf == "Jacket") re4t::cfg->CostumeOverride.Leon = LeonCostume::Jacket;
		if (buf == "Normal") re4t::cfg->CostumeOverride.Leon = LeonCostume::Normal;
		if (buf == "Vest") re4t::cfg->CostumeOverride.Leon = LeonCostume::Vest;
		if (buf == "RPD") re4t::cfg->CostumeOverride.Leon = LeonCostume::RPD;
		if (buf == "Mafia") re4t::cfg->CostumeOverride.Leon = LeonCostume::Mafia;
	}

	buf = ini.getString("OVERRIDES", "AshleyCostume", "");
	if (!buf.empty())
	{
		if (buf == "Normal") re4t::cfg->CostumeOverride.Ashley = AshleyCostume::Normal;
		if (buf == "Popstar") re4t::cfg->CostumeOverride.Ashley = AshleyCostume::Popstar;
		if (buf == "Armor") re4t::cfg->CostumeOverride.Ashley = AshleyCostume::Armor;
	}

	buf = ini.getString("OVERRIDES", "AdaCostume", "");
	if (!buf.empty())
	{
		if (buf == "Resident Evil 2") re4t::cfg->CostumeOverride.Ada = AdaCostume::RE2;
		if (buf == "Spy") re4t::cfg->CostumeOverride.Ada = AdaCostume::Spy;
		if (buf == "Normal") re4t::cfg->CostumeOverride.Ada = AdaCostume::Normal;
	}

	re4t::cfg->bOverrideDifficulty = ini.getBool("OVERRIDES", "OverrideDifficulty", re4t::cfg->bOverrideDifficulty);
	buf = ini.getString("OVERRIDES", "NewDifficulty", "");
	if (!buf.empty())
	{
		if (buf == "Amateur") re4t::cfg->NewDifficulty = GameDifficulty::VeryEasy;
		if (buf == "Easy") re4t::cfg->NewDifficulty = GameDifficulty::Easy;
		if (buf == "Normal") re4t::cfg->NewDifficulty = GameDifficulty::Medium;
		if (buf == "Professional") re4t::cfg->NewDifficulty = GameDifficulty::Pro;
	}

	re4t::cfg->bTrainerPlayerSpeedOverride = ini.getBool("OVERRIDES", "EnablePlayerSpeedOverride", re4t::cfg->bTrainerPlayerSpeedOverride);
	re4t::cfg->fTrainerPlayerSpeedOverride = ini.getFloat("OVERRIDES", "PlayerSpeedOverride", re4t::cfg->fTrainerPlayerSpeedOverride);
	re4t::cfg->bTrainerEnemyHPMultiplier = ini.getBool("OVERRIDES", "EnableEnemyHPMultiplier", re4t::cfg->bTrainerEnemyHPMultiplier);
	re4t::cfg->fTrainerEnemyHPMultiplier = ini.getFloat("OVERRIDES", "EnemyHPMultiplier", re4t::cfg->fTrainerEnemyHPMultiplier);
	re4t::cfg->fTrainerEnemyHPMultiplier = fmin(fmax(re4t::cfg->fTrainerEnemyHPMultiplier, 0.1f), 15.0f); // limit between 0.1 - 15
	re4t::cfg->bTrainerRandomHPMultiplier = ini.getBool("OVERRIDES", "UseRandomHPMultiplier", re4t::cfg->bTrainerRandomHPMultiplier);
	re4t::cfg->fTrainerRandomHPMultiMin = ini.getFloat("OVERRIDES", "RandomHPMultiplierMin", re4t::cfg->fTrainerRandomHPMultiMin);
	re4t::cfg->fTrainerRandomHPMultiMin = fmin(fmax(re4t::cfg->fTrainerRandomHPMultiMin, 0.1f), 14.0f); // limit between 0.1 - 14
	re4t::cfg->fTrainerRandomHPMultiMax = ini.getFloat("OVERRIDES", "RandomHPMultiplierMax", re4t::cfg->fTrainerRandomHPMultiMax);
	re4t::cfg->fTrainerRandomHPMultiMax = fmin(fmax(re4t::cfg->fTrainerRandomHPMultiMax, fTrainerRandomHPMultiMin), 15.0f); // limit between fTrainerRandomHPMultiMin - 15

	// ESP
	re4t::cfg->bShowESP = ini.getBool("ESP", "ShowESP", re4t::cfg->bShowESP);
	re4t::cfg->bEspShowInfoOnTop = ini.getBool("ESP", "ShowInfoOnTop", re4t::cfg->bEspShowInfoOnTop);
	re4t::cfg->bEspOnlyShowEnemies = ini.getBool("ESP", "OnlyShowEnemies", re4t::cfg->bEspOnlyShowEnemies);
	re4t::cfg->bEspOnlyShowValidEms = ini.getBool("ESP", "OnlyShowValidEms", re4t::cfg->bEspOnlyShowValidEms);
	re4t::cfg->bEspOnlyShowESLSpawned = ini.getBool("ESP", "OnlyShowESLSpawned", re4t::cfg->bEspOnlyShowESLSpawned);
	re4t::cfg->bEspOnlyShowAlive = ini.getBool("ESP", "OnlyShowAlive", re4t::cfg->bEspOnlyShowAlive);
	re4t::cfg->fEspMaxEmDistance = ini.getFloat("ESP", "MaxEmDistance", re4t::cfg->fEspMaxEmDistance);
	re4t::cfg->bEspOnlyShowClosestEms = ini.getBool("ESP", "OnlyShowClosestEms", re4t::cfg->bEspOnlyShowClosestEms);
	re4t::cfg->iEspClosestEmsAmount = ini.getInt("ESP", "ClosestEmsAmount", re4t::cfg->iEspClosestEmsAmount);
	re4t::cfg->bEspDrawLines = ini.getBool("ESP", "DrawLines", re4t::cfg->bEspDrawLines);

	buf = ini.getString("ESP", "EmNameMode", "");
	if (!buf.empty())
	{
		if (buf == "DontShow") re4t::cfg->iEspEmNameMode = 0;
		if (buf == "Normal") re4t::cfg->iEspEmNameMode = 1;
		if (buf == "Simplified") re4t::cfg->iEspEmNameMode = 2;
	}

	buf = ini.getString("ESP", "EmHPMode", "");
	if (!buf.empty())
	{
		if (buf == "DontShow") re4t::cfg->iEspEmHPMode = 0;
		if (buf == "Bar") re4t::cfg->iEspEmHPMode = 1;
		if (buf == "Text") re4t::cfg->iEspEmHPMode = 2;
	}

	re4t::cfg->bEspDrawDebugInfo = ini.getBool("ESP", "DrawDebugInfo", re4t::cfg->bEspDrawDebugInfo);

	// SIDEINFO
	re4t::cfg->bShowSideInfo = ini.getBool("SIDEINFO", "ShowSideInfo", re4t::cfg->bShowSideInfo);
	re4t::cfg->bSideShowEmCount = ini.getBool("SIDEINFO", "ShowEmCount", re4t::cfg->bSideShowEmCount);
	re4t::cfg->bSideShowEmList = ini.getBool("SIDEINFO", "ShowEmList", re4t::cfg->bSideShowEmList);
	re4t::cfg->bSideOnlyShowESLSpawned = ini.getBool("SIDEINFO", "OnlyShowESLSpawned", re4t::cfg->bSideOnlyShowESLSpawned);
	re4t::cfg->bSideShowSimpleNames = ini.getBool("SIDEINFO", "ShowSimpleNames", re4t::cfg->bSideShowSimpleNames);
	re4t::cfg->iSideClosestEmsAmount = ini.getInt("SIDEINFO", "ClosestEmsAmount", re4t::cfg->iSideClosestEmsAmount);
	re4t::cfg->fSideMaxEmDistance = ini.getFloat("SIDEINFO", "MaxEmDistance", re4t::cfg->fSideMaxEmDistance);

	buf = ini.getString("SIDEINFO", "EmHPMode", "");
	if (!buf.empty())
	{
		if (buf == "DontShow") re4t::cfg->iSideEmHPMode = 0;
		if (buf == "Bar") re4t::cfg->iSideEmHPMode = 1;
		if (buf == "Text") re4t::cfg->iSideEmHPMode = 2;
	}

	// TRAINER HOTKEYS
	re4t::cfg->sTrainerFocusUIKeyCombo = ini.getString("TRAINER_HOTKEYS", "FocusUI", re4t::cfg->sTrainerFocusUIKeyCombo);
	re4t::cfg->sTrainerNoclipKeyCombo = ini.getString("TRAINER_HOTKEYS", "NoclipToggle", re4t::cfg->sTrainerNoclipKeyCombo);
	re4t::cfg->sTrainerFreeCamKeyCombo = ini.getString("TRAINER_HOTKEYS", "FreeCamToggle", re4t::cfg->sTrainerFreeCamKeyCombo);
	re4t::cfg->sTrainerSpeedOverrideKeyCombo = ini.getString("TRAINER_HOTKEYS", "SpeedOverrideToggle", re4t::cfg->sTrainerSpeedOverrideKeyCombo);
	re4t::cfg->sTrainerMoveAshToPlayerKeyCombo = ini.getString("TRAINER_HOTKEYS", "MoveAshleyToPlayer", re4t::cfg->sTrainerMoveAshToPlayerKeyCombo);
	re4t::cfg->sTrainerDebugTrgKeyCombo = ini.getString("TRAINER_HOTKEYS", "DebugTrg", re4t::cfg->sTrainerDebugTrgKeyCombo);

	// WEAPON HOTKEYS
	re4t::cfg->bWeaponHotkeysEnable = ini.getBool("WEAPON_HOTKEYS", "Enable", re4t::cfg->bWeaponHotkeysEnable);
	re4t::cfg->sWeaponHotkeys[0] = ini.getString("WEAPON_HOTKEYS", "WeaponHotkeySlot1", re4t::cfg->sWeaponHotkeys[0]);
	re4t::cfg->sWeaponHotkeys[1] = ini.getString("WEAPON_HOTKEYS", "WeaponHotkeySlot2", re4t::cfg->sWeaponHotkeys[1]);
	re4t::cfg->sWeaponHotkeys[2] = ini.getString("WEAPON_HOTKEYS", "WeaponHotkeySlot3", re4t::cfg->sWeaponHotkeys[2]);
	re4t::cfg->sWeaponHotkeys[3] = ini.getString("WEAPON_HOTKEYS", "WeaponHotkeySlot4", re4t::cfg->sWeaponHotkeys[3]);
	re4t::cfg->sWeaponHotkeys[4] = ini.getString("WEAPON_HOTKEYS", "WeaponHotkeySlot5", re4t::cfg->sWeaponHotkeys[4]);
	re4t::cfg->sLastWeaponHotkey = ini.getString("WEAPON_HOTKEYS", "LastWeaponHotkey", re4t::cfg->sLastWeaponHotkey);
	re4t::cfg->iWeaponHotkeyWepIds[0] = ini.getInt("WEAPON_HOTKEYS", "WeaponIdSlot1", re4t::cfg->iWeaponHotkeyWepIds[0]);
	re4t::cfg->iWeaponHotkeyWepIds[1] = ini.getInt("WEAPON_HOTKEYS", "WeaponIdSlot2", re4t::cfg->iWeaponHotkeyWepIds[1]);
	re4t::cfg->iWeaponHotkeyWepIds[2] = ini.getInt("WEAPON_HOTKEYS", "WeaponIdSlot3", re4t::cfg->iWeaponHotkeyWepIds[2]);
	re4t::cfg->iWeaponHotkeyWepIds[3] = ini.getInt("WEAPON_HOTKEYS", "WeaponIdSlot4", re4t::cfg->iWeaponHotkeyWepIds[3]);
	re4t::cfg->iWeaponHotkeyWepIds[4] = ini.getInt("WEAPON_HOTKEYS", "WeaponIdSlot5", re4t::cfg->iWeaponHotkeyWepIds[4]);
	auto readIntVect = [&ini](std::string section, std::string key, std::string& default_value)
	{
		std::vector<int> ret;

		default_value = ini.getString(section, key, default_value);

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
	re4t::cfg->bIgnoreFPSWarning = ini.getBool("WARNING", "IgnoreFPSWarning", re4t::cfg->bIgnoreFPSWarning);
	
	// IMGUI
	re4t::cfg->fFontSizeScale = ini.getFloat("IMGUI", "FontSizeScale", re4t::cfg->fFontSizeScale);
	re4t::cfg->fFontSizeScale = fmin(fmax(re4t::cfg->fFontSizeScale, 1.0f), 1.25f); // limit between 1.0 - 1.25

	re4t::cfg->bEnableDPIScale = ini.getBool("IMGUI", "EnableDPIScale", re4t::cfg->bEnableDPIScale);
	re4t::cfg->bDisableMenuTip = ini.getBool("IMGUI", "DisableMenuTip", re4t::cfg->bDisableMenuTip);

	// DEBUG
	re4t::cfg->bVerboseLog = ini.getBool("DEBUG", "VerboseLog", re4t::cfg->bVerboseLog);
	re4t::cfg->bNeverHideCursor = ini.getBool("DEBUG", "NeverHideCursor", re4t::cfg->bNeverHideCursor);
	re4t::cfg->bUseDynamicFrametime = ini.getBool("DEBUG", "UseDynamicFrametime", re4t::cfg->bUseDynamicFrametime);
	re4t::cfg->bDisableFramelimiting = ini.getBool("DEBUG", "DisableFramelimiting", re4t::cfg->bDisableFramelimiting);

	if (ini.getBool("DEBUG", "TweaksDevMode", TweaksDevMode))
		TweaksDevMode = true; // let the INI enable it if it's disabled, but not disable it
}

std::mutex settingsThreadRunningMutex;

void WriteSettings(std::wstring iniPath, bool trainerIni)
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // only allow single thread writing to INI at one time

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

	// trainer.ini-only settings
	if (trainerIni)
	{
		iniReader ini(WstrToStr(iniPath));

		// Trainer
		ini.writeBool("TRAINER", "Enable", re4t::cfg->bTrainerEnable);

		// PATCHES
		ini.writeBool("PATCHES", "UseNumpadMovement", re4t::cfg->bTrainerUseNumpadMovement);
		ini.writeBool("PATCHES", "UseMouseWheelUpDown", re4t::cfg->bTrainerUseMouseWheelUpDown);
		ini.writeFloat("PATCHES", "NumpadMovementSpeed", re4t::cfg->fTrainerNumMoveSpeed);
		ini.writeBool("PATCHES", "EnableFreeCamera", re4t::cfg->bTrainerEnableFreeCam);
		ini.writeFloat("PATCHES", "FreeCamSpeed", re4t::cfg->fTrainerFreeCamSpeed);
		ini.writeBool("PATCHES", "DisableEnemySpawn", re4t::cfg->bTrainerDisableEnemySpawn);
		ini.writeBool("PATCHES", "DeadBodiesNeverDisappear", re4t::cfg->bTrainerDeadBodiesNeverDisappear);
		ini.writeBool("PATCHES", "AllowEnterDoorsWithoutAshley", re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh);
		ini.writeBool("PATCHES", "EnableDebugTrg", re4t::cfg->bTrainerEnableDebugTrg);
		ini.writeBool("PATCHES", "ShowDebugTrgHintText", re4t::cfg->bTrainerShowDebugTrgHintText);
		ini.writeBool("PATCHES", "OpenInventoryOnItemAdd", re4t::cfg->bTrainerOpenInventoryOnItemAdd);
		ini.writeInt("PATCHES", "LastAreaJumpStage", re4t::cfg->iTrainerLastAreaJumpStage);
		ini.writeInt("PATCHES", "LastAreaJumpRoomIdx", re4t::cfg->iTrainerLastAreaJumpRoomIdx);

		// OVERRIDES
		ini.writeBool("OVERRIDES", "EnablePlayerSpeedOverride", re4t::cfg->bTrainerPlayerSpeedOverride);
		ini.writeFloat("OVERRIDES", "PlayerSpeedOverride", re4t::cfg->fTrainerPlayerSpeedOverride);
		ini.writeBool("OVERRIDES", "OverrideCostumes", re4t::cfg->bOverrideCostumes);
		ini.writeString("OVERRIDES", "LeonCostume", std::string(sLeonCostumeNames[int(re4t::cfg->CostumeOverride.Leon)]));
		ini.writeString("OVERRIDES", "AshleyCostume", std::string(sAshleyCostumeNames[int(re4t::cfg->CostumeOverride.Ashley)]));
		ini.writeString("OVERRIDES", "AdaCostume", std::string(sAdaCostumeNames[int(re4t::cfg->CostumeOverride.Ada)]));
		ini.writeBool("OVERRIDES", "OverrideDifficulty", re4t::cfg->bOverrideDifficulty);
		ini.writeString("OVERRIDES", "NewDifficulty", std::string(sGameDifficultyNames[int(re4t::cfg->NewDifficulty)]));
		ini.writeBool("OVERRIDES", "EnableEnemyHPMultiplier", re4t::cfg->bTrainerEnemyHPMultiplier);
		ini.writeFloat("OVERRIDES", "EnemyHPMultiplier", re4t::cfg->fTrainerEnemyHPMultiplier);
		ini.writeBool("OVERRIDES", "UseRandomHPMultiplier", re4t::cfg->bTrainerRandomHPMultiplier);
		ini.writeFloat("OVERRIDES", "RandomHPMultiplierMin", re4t::cfg->fTrainerRandomHPMultiMin);
		ini.writeFloat("OVERRIDES", "RandomHPMultiplierMax", re4t::cfg->fTrainerRandomHPMultiMax);

		// ESP
		ini.writeBool("ESP", "ShowESP", re4t::cfg->bShowESP);
		ini.writeBool("ESP", "ShowInfoOnTop", re4t::cfg->bEspShowInfoOnTop);
		ini.writeBool("ESP", "OnlyShowEnemies", re4t::cfg->bEspOnlyShowEnemies);
		ini.writeBool("ESP", "OnlyShowValidEms", re4t::cfg->bEspOnlyShowValidEms);
		ini.writeBool("ESP", "OnlyShowESLSpawned", re4t::cfg->bEspOnlyShowESLSpawned);
		ini.writeBool("ESP", "OnlyShowAlive", re4t::cfg->bEspOnlyShowAlive);
		ini.writeFloat("ESP", "MaxEmDistance", re4t::cfg->fEspMaxEmDistance);
		ini.writeBool("ESP", "OnlyShowClosestEms", re4t::cfg->bEspOnlyShowClosestEms);
		ini.writeInt("ESP", "ClosestEmsAmount", re4t::cfg->iEspClosestEmsAmount);
		ini.writeBool("ESP", "DrawLines", re4t::cfg->bEspDrawLines);

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
		} ini.writeString("ESP", "EmNameMode", buf);

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
		} ini.writeString("ESP", "EmHPMode", buf);

		ini.writeBool("ESP", "DrawDebugInfo", re4t::cfg->bEspDrawDebugInfo);

		// SIDEINFO
		ini.writeBool("SIDEINFO", "ShowSideInfo", re4t::cfg->bShowSideInfo);
		ini.writeBool("SIDEINFO", "ShowEmCount", re4t::cfg->bSideShowEmCount);
		ini.writeBool("SIDEINFO", "ShowEmList", re4t::cfg->bSideShowEmList);
		ini.writeBool("SIDEINFO", "OnlyShowESLSpawned", re4t::cfg->bSideOnlyShowESLSpawned);
		ini.writeBool("SIDEINFO", "ShowSimpleNames", re4t::cfg->bSideShowSimpleNames);
		ini.writeInt("SIDEINFO", "ClosestEmsAmount", re4t::cfg->iSideClosestEmsAmount);
		ini.writeFloat("SIDEINFO", "MaxEmDistance", re4t::cfg->fSideMaxEmDistance);

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
		} ini.writeString("SIDEINFO", "EmHPMode", buf);

		// TRAINER_HOTKEYS
		ini.writeString("TRAINER_HOTKEYS", "FocusUI", re4t::cfg->sTrainerFocusUIKeyCombo);
		ini.writeString("TRAINER_HOTKEYS", "NoclipToggle", re4t::cfg->sTrainerNoclipKeyCombo);
		ini.writeString("TRAINER_HOTKEYS", "FreeCamToggle", re4t::cfg->sTrainerFreeCamKeyCombo);
		ini.writeString("TRAINER_HOTKEYS", "SpeedOverrideToggle", re4t::cfg->sTrainerSpeedOverrideKeyCombo);
		ini.writeString("TRAINER_HOTKEYS", "MoveAshleyToPlayer", re4t::cfg->sTrainerMoveAshToPlayerKeyCombo);
		ini.writeString("TRAINER_HOTKEYS", "DebugTrg", re4t::cfg->sTrainerDebugTrgKeyCombo);

		// WEAPON HOTKEYS
		ini.writeBool("WEAPON_HOTKEYS", "Enable", re4t::cfg->bWeaponHotkeysEnable);
		ini.writeString("WEAPON_HOTKEYS", "WeaponHotkeySlot1", re4t::cfg->sWeaponHotkeys[0]);
		ini.writeString("WEAPON_HOTKEYS", "WeaponHotkeySlot2", re4t::cfg->sWeaponHotkeys[1]);
		ini.writeString("WEAPON_HOTKEYS", "WeaponHotkeySlot3", re4t::cfg->sWeaponHotkeys[2]);
		ini.writeString("WEAPON_HOTKEYS", "WeaponHotkeySlot4", re4t::cfg->sWeaponHotkeys[3]);
		ini.writeString("WEAPON_HOTKEYS", "WeaponHotkeySlot5", re4t::cfg->sWeaponHotkeys[4]);
		ini.writeInt("WEAPON_HOTKEYS", "WeaponIdSlot1", re4t::cfg->iWeaponHotkeyWepIds[0]);
		ini.writeInt("WEAPON_HOTKEYS", "WeaponIdSlot2", re4t::cfg->iWeaponHotkeyWepIds[1]);
		ini.writeInt("WEAPON_HOTKEYS", "WeaponIdSlot3", re4t::cfg->iWeaponHotkeyWepIds[2]);
		ini.writeInt("WEAPON_HOTKEYS", "WeaponIdSlot4", re4t::cfg->iWeaponHotkeyWepIds[3]);
		ini.writeInt("WEAPON_HOTKEYS", "WeaponIdSlot5", re4t::cfg->iWeaponHotkeyWepIds[4]);

		auto writeIntVect = [&ini](std::string section, std::string key, std::vector<int>& vect) {
			std::string val = "";
			for (int num : vect)
				val += std::to_string(num) + ", ";
			if (!val.empty())
				val = val.substr(0, val.size() - 2);
			ini.writeString(section, key, val);
		};

		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot1", re4t::cfg->iWeaponHotkeyCycle[0]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot2", re4t::cfg->iWeaponHotkeyCycle[1]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot3", re4t::cfg->iWeaponHotkeyCycle[2]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot4", re4t::cfg->iWeaponHotkeyCycle[3]);
		writeIntVect("WEAPON_HOTKEYS", "WeaponCycleSlot5", re4t::cfg->iWeaponHotkeyCycle[4]);

		ini.writeString("WEAPON_HOTKEYS", "LastWeaponHotkey", re4t::cfg->sLastWeaponHotkey);

		return;
	}

	// Regular .ini settings
	iniReader ini(WstrToStr(iniPath));

	// VULKAN
	ini.writeBool("VULKAN", "UseVulkanRenderer", re4t::dxvk::cfg->bUseVulkanRenderer);
	ini.writeBool("VULKAN", "ShowFPS", re4t::dxvk::cfg->bShowFPS);
	ini.writeBool("VULKAN", "ShowGPULoad", re4t::dxvk::cfg->bShowGPULoad);
	ini.writeBool("VULKAN", "ShowDeviceInfo", re4t::dxvk::cfg->bShowDeviceInfo);
	ini.writeBool("VULKAN", "DisableAsync", re4t::dxvk::cfg->bDisableAsync);

	// DISPLAY
	ini.writeFloat("DISPLAY", "FOVAdditional", re4t::cfg->fFOVAdditional);
	ini.writeBool("DISPLAY", "DisableVsync", re4t::cfg->bDisableVsync);

	ini.writeBool("DISPLAY", "UltraWideAspectSupport", re4t::cfg->bUltraWideAspectSupport);
	ini.writeBool("DISPLAY", "SideAlignHUD", re4t::cfg->bSideAlignHUD);
	ini.writeBool("DISPLAY", "StretchFullscreenImages", re4t::cfg->bStretchFullscreenImages);
	ini.writeBool("DISPLAY", "StretchVideos", re4t::cfg->bStretchVideos);
	ini.writeBool("DISPLAY", "Remove16by10BlackBars", re4t::cfg->bRemove16by10BlackBars);
	ini.writeBool("DISPLAY", "FixDPIScale", re4t::cfg->bFixDPIScale);
	ini.writeBool("DISPLAY", "FixDisplayMode", re4t::cfg->bFixDisplayMode);
	ini.writeInt("DISPLAY", "CustomRefreshRate", re4t::cfg->iCustomRefreshRate);
	ini.writeBool("DISPLAY", "OverrideLaserColor", re4t::cfg->bOverrideLaserColor);
	ini.writeBool("DISPLAY", "RainbowLaser", re4t::cfg->bRainbowLaser);

	ini.writeInt("DISPLAY", "LaserR", re4t::cfg->iLaserR);
	ini.writeInt("DISPLAY", "LaserG", re4t::cfg->iLaserG);
	ini.writeInt("DISPLAY", "LaserB", re4t::cfg->iLaserB);

	ini.writeBool("DISPLAY", "RestorePickupTransparency", re4t::cfg->bRestorePickupTransparency);
	ini.writeBool("DISPLAY", "DisableBrokenFilter03", re4t::cfg->bDisableBrokenFilter03);
	ini.writeBool("DISPLAY", "FixBlurryImage", re4t::cfg->bFixBlurryImage);
	ini.writeBool("DISPLAY", "DisableFilmGrain", re4t::cfg->bDisableFilmGrain);
	ini.writeBool("DISPLAY", "ImproveWater", re4t::cfg->bImproveWater);
	ini.writeBool("DISPLAY", "EnableGCBlur", re4t::cfg->bEnableGCBlur);

	if (re4t::cfg->bUseEnhancedGCBlur)
		ini.writeString("DISPLAY", "GCBlurType", "Enhanced");
	else
		ini.writeString("DISPLAY", "GCBlurType", "Classic");

	ini.writeBool("DISPLAY", "EnableGCScopeBlur", re4t::cfg->bEnableGCScopeBlur);
	ini.writeBool("DISPLAY", "WindowBorderless", re4t::cfg->bWindowBorderless);
	ini.writeInt("DISPLAY", "WindowPositionX", re4t::cfg->iWindowPositionX);
	ini.writeInt("DISPLAY", "WindowPositionY", re4t::cfg->iWindowPositionY);
	ini.writeBool("DISPLAY", "RememberWindowPos", re4t::cfg->bRememberWindowPos);

	// AUDIO
	ini.writeInt("AUDIO", "VolumeMaster", re4t::cfg->iVolumeMaster);
	ini.writeInt("AUDIO", "VolumeBGM", re4t::cfg->iVolumeBGM);
	ini.writeInt("AUDIO", "VolumeSE", re4t::cfg->iVolumeSE);
	ini.writeInt("AUDIO", "VolumeCutscene", re4t::cfg->iVolumeCutscene);
	ini.writeBool("AUDIO", "RestoreGCSoundEffects", re4t::cfg->bRestoreGCSoundEffects);
	ini.writeBool("AUDIO", "SilenceArmoredAshley", re4t::cfg->bSilenceArmoredAshley);

	// MOUSE
	ini.writeBool("MOUSE", "CameraImprovements", re4t::cfg->bCameraImprovements);
	ini.writeBool("MOUSE", "ResetCameraAfterUsingWeapons", re4t::cfg->bResetCameraAfterUsingWeapons);
	ini.writeBool("MOUSE", "ResetCameraAfterUsingKnife", re4t::cfg->bResetCameraAfterUsingKnife);
	ini.writeBool("MOUSE", "ResetCameraWhenRunning", re4t::cfg->bResetCameraWhenRunning);
	ini.writeFloat("MOUSE", "CameraSensitivity", re4t::cfg->fCameraSensitivity);
	ini.writeBool("MOUSE", "UseMouseTurning", re4t::cfg->bUseMouseTurning);

	if (re4t::cfg->iMouseTurnType == MouseTurnTypes::TypeA)
		ini.writeString("MOUSE", "MouseTurnType", "TypeA");
	else
		ini.writeString("MOUSE", "MouseTurnType", "TypeB");

	ini.writeFloat("MOUSE", "TurnTypeBSensitivity", re4t::cfg->fTurnTypeBSensitivity);
	ini.writeBool("MOUSE", "UseRawMouseInput", re4t::cfg->bUseRawMouseInput);
	ini.writeBool("MOUSE", "DetachCameraFromAim", re4t::cfg->bDetachCameraFromAim);
	ini.writeBool("MOUSE", "FixSniperZoom", re4t::cfg->bFixSniperZoom);
	ini.writeBool("MOUSE", "FixSniperFocus", re4t::cfg->bFixSniperFocus);
	ini.writeBool("MOUSE", "FixRetryLoadMouseSelector", re4t::cfg->bFixRetryLoadMouseSelector);

	// KEYBOARD
	ini.writeBool("KEYBOARD", "FallbackToEnglishKeyIcons", re4t::cfg->bFallbackToEnglishKeyIcons);
	ini.writeBool("KEYBOARD", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_kbm);
	ini.writeBool("KEYBOARD", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_kbm);

	// CONTROLLER
	ini.writeBool("CONTROLLER", "OverrideControllerSensitivity", re4t::cfg->bOverrideControllerSensitivity);
	ini.writeFloat("CONTROLLER", "ControllerSensitivity", re4t::cfg->fControllerSensitivity);
	ini.writeBool("CONTROLLER", "RemoveExtraXinputDeadzone", re4t::cfg->bRemoveExtraXinputDeadzone);
	ini.writeBool("CONTROLLER", "OverrideXinputDeadzone", re4t::cfg->bOverrideXinputDeadzone);
	ini.writeFloat("CONTROLLER", "XinputDeadzone", re4t::cfg->fXinputDeadzone);
	ini.writeBool("CONTROLLER", "AllowReloadWithoutAiming", re4t::cfg->bAllowReloadWithoutAiming_controller);
	ini.writeBool("CONTROLLER", "ReloadWithoutZoom", re4t::cfg->bReloadWithoutZoom_controller);

	// FRAME RATE
	ini.writeBool("FRAME RATE", "FixFallingItemsSpeed", re4t::cfg->bFixFallingItemsSpeed);
	ini.writeBool("FRAME RATE", "FixCompartmentsOpeningSpeed", re4t::cfg->bFixCompartmentsOpeningSpeed);
	ini.writeBool("FRAME RATE", "FixMovingGeometrySpeed", re4t::cfg->bFixMovingGeometrySpeed);
	ini.writeBool("FRAME RATE", "FixTurningSpeed", re4t::cfg->bFixTurningSpeed);
	ini.writeBool("FRAME RATE", "FixQTE", re4t::cfg->bFixQTE);
	ini.writeBool("FRAME RATE", "FixAshleyBustPhysics", re4t::cfg->bFixAshleyBustPhysics);
	ini.writeBool("FRAME RATE", "EnableFastMath", re4t::cfg->bEnableFastMath);
	ini.writeBool("FRAME RATE", "ReplaceFramelimiter", re4t::cfg->bReplaceFramelimiter);
	ini.writeBool("FRAME RATE", "MultithreadFix", re4t::cfg->bMultithreadFix);
	ini.writeBool("FRAME RATE", "PrecacheModels", re4t::cfg->bPrecacheModels);
	
	// GAMEPLAY
	ini.writeBool("GAMEPLAY", "AshleyJPCameraAngles", re4t::cfg->bAshleyJPCameraAngles);
	ini.writeBool("GAMEPLAY", "SeparateWaysProfessional", re4t::cfg->bSeparateWaysProfessional);
	ini.writeBool("GAMEPLAY", "EnableNTSCMode", re4t::cfg->bEnableNTSCMode);
	ini.writeBool("GAMEPLAY", "AllowAshleySuplex", re4t::cfg->bAllowAshleySuplex);
	ini.writeBool("GAMEPLAY", "FixDitmanGlitch", re4t::cfg->bFixDitmanGlitch);
	ini.writeBool("GAMEPLAY", "AllowSellingHandgunSilencer", re4t::cfg->bAllowSellingHandgunSilencer);
	ini.writeBool("GAMEPLAY", "BalancedChicagoTypewriter", re4t::cfg->bBalancedChicagoTypewriter);
	ini.writeBool("GAMEPLAY", "UseSprintToggle", re4t::cfg->bUseSprintToggle);
	ini.writeBool("GAMEPLAY", "RifleScreenShake", re4t::cfg->bRifleScreenShake);
	ini.writeBool("GAMEPLAY", "DisableQTE", re4t::cfg->bDisableQTE);
	ini.writeBool("GAMEPLAY", "AutomaticMashingQTE", re4t::cfg->bAutomaticMashingQTE);
	ini.writeBool("GAMEPLAY", "AllowMatildaQuickturn", re4t::cfg->bAllowMatildaQuickturn);
	ini.writeBool("GAMEPLAY", "LimitMatildaBurst", re4t::cfg->bLimitMatildaBurst);

	// MISC
	ini.writeBool("MISC", "NeverCheckForUpdates", re4t::cfg->bNeverCheckForUpdates);
	ini.writeBool("MISC", "RestoreDemoVideos", re4t::cfg->bRestoreDemoVideos);
	ini.writeBool("MISC", "RestoreAnalogTitleScroll", re4t::cfg->bRestoreAnalogTitleScroll);
	ini.writeInt("MISC", "ViolenceLevelOverride", re4t::cfg->iViolenceLevelOverride);
	ini.writeBool("MISC", "AllowMafiaLeonCutscenes", re4t::cfg->bAllowMafiaLeonCutscenes);
	ini.writeBool("MISC", "SkipIntroLogos", re4t::cfg->bSkipIntroLogos);
	ini.writeBool("MISC", "SkipMenuFades", re4t::cfg->bSkipMenuFades);
	ini.writeBool("MISC", "SpeedUpQuitGame", re4t::cfg->bSpeedUpQuitGame);
	ini.writeBool("MISC", "AlwaysShowOriginalTitleBackground", re4t::cfg->bAlwaysShowOriginalTitleBackground);
	ini.writeBool("MISC", "EnableDebugMenu", re4t::cfg->bEnableDebugMenu);
	ini.writeBool("MISC", "ShowGameOutput", re4t::cfg->bShowGameOutput);
	ini.writeBool("MISC", "HideZoomControlHints", re4t::cfg->bHideZoomControlHints);
	ini.writeBool("MISC", "FixSilencedHandgunDescription", re4t::cfg->bFixSilencedHandgunDescription);
	// Not writing EnableModExpansion / ForceETSApplyScale back to users INI in case those were enabled by a mod override INI (which the user might want to remove later)
	// We don't have any UI options for those anyway, so pointless for us to write it back

	// MEMORY
	ini.writeBool("MEMORY", "AllowHighResolutionSFD", re4t::cfg->bAllowHighResolutionSFD);
	ini.writeBool("MEMORY", "RaiseVertexAlloc", re4t::cfg->bRaiseVertexAlloc);
	ini.writeBool("MEMORY", "RaiseInventoryAlloc", re4t::cfg->bRaiseInventoryAlloc);

	// HOTKEYS
	ini.writeString("HOTKEYS", "ConfigMenu", re4t::cfg->sConfigMenuKeyCombo);
	ini.writeString("HOTKEYS", "Console", re4t::cfg->sConsoleKeyCombo);
	ini.writeString("HOTKEYS", "FlipItemUp", re4t::cfg->sFlipItemUp);
	ini.writeString("HOTKEYS", "FlipItemDown", re4t::cfg->sFlipItemDown);
	ini.writeString("HOTKEYS", "FlipItemLeft", re4t::cfg->sFlipItemLeft);
	ini.writeString("HOTKEYS", "FlipItemRight", re4t::cfg->sFlipItemRight);
	ini.writeString("HOTKEYS", "QTE_key_1", re4t::cfg->sQTE_key_1);
	ini.writeString("HOTKEYS", "QTE_key_2", re4t::cfg->sQTE_key_2);
	ini.writeString("HOTKEYS", "DebugMenu", re4t::cfg->sDebugMenuKeyCombo);
	ini.writeString("HOTKEYS", "MouseTurningModifier", re4t::cfg->sMouseTurnModifierKeyCombo);
	ini.writeString("HOTKEYS", "JetSkiTricks", re4t::cfg->sJetSkiTrickCombo);

	// IMGUI
	ini.writeFloat("IMGUI", "FontSizeScale", re4t::cfg->fFontSizeScale);
}

DWORD WINAPI WriteSettingsThread(LPVOID lpParameter)
{
	bool writeTrainerOnly = bool(lpParameter);

	std::wstring iniPathMain = rootPath + wrapperName + L".ini";
	std::wstring iniPathTrainer = rootPath + L"re4_tweaks\\trainer.ini";

	WriteSettings(iniPathTrainer, true);

	if (!writeTrainerOnly)
	{
		WriteSettings(iniPathMain, false);
		re4t::cfg->HasUnsavedChanges = false;
	}

	return 0;
}

void re4t_cfg::WriteSettings(bool trainerOnly)
{
	std::lock_guard<std::mutex> guard(settingsThreadRunningMutex); // if thread is already running, wait for it to finish

	// Spawn a new thread to handle writing settings, as INI writing funcs that get used are pretty slow
	CreateThreadAutoClose(NULL, 0, WriteSettingsThread, (LPVOID)trainerOnly, 0, NULL);
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
	spd::log()->info("| {:<30} | {:>15} |", "SilenceArmoredAshley", re4t::cfg->bSilenceArmoredAshley ? "true" : "false");
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

	// GAMEPLAY
	spd::log()->info("+ GAMEPLAY-----------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "AshleyJPCameraAngles", re4t::cfg->bAshleyJPCameraAngles ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SeparateWaysProfessional", re4t::cfg->bSeparateWaysProfessional ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableNTSCMode", re4t::cfg->bEnableNTSCMode ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowAshleySuplex", re4t::cfg->bAllowAshleySuplex ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixDitmanGlitch", re4t::cfg->bFixDitmanGlitch ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowSellingHandgunSilencer", re4t::cfg->bAllowSellingHandgunSilencer ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "BalancedChicagoTypewriter", re4t::cfg->bBalancedChicagoTypewriter ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "UseSprintToggle", re4t::cfg->bUseSprintToggle ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RifleScreenShake", re4t::cfg->bRifleScreenShake ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DisableQTE", re4t::cfg->bDisableQTE ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AutomaticMashingQTE", re4t::cfg->bAutomaticMashingQTE ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowMatildaQuickturn", re4t::cfg->bAllowMatildaQuickturn ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "LimitMatildaBurst", re4t::cfg->bLimitMatildaBurst ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// MISC
	spd::log()->info("+ MISC---------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "WrappedDllPath", re4t::cfg->sWrappedDllPath.data());
	spd::log()->info("| {:<30} | {:>15} |", "NeverCheckForUpdates", re4t::cfg->bNeverCheckForUpdates ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RestoreDemoVideos", re4t::cfg->bRestoreDemoVideos ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RestoreAnalogTitleScroll", re4t::cfg->bRestoreAnalogTitleScroll ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ViolenceLevelOverride", re4t::cfg->iViolenceLevelOverride);
	spd::log()->info("| {:<30} | {:>15} |", "AllowMafiaLeonCutscenes", re4t::cfg->bAllowMafiaLeonCutscenes ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SkipIntroLogos", re4t::cfg->bSkipIntroLogos ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SkipMenuFades", re4t::cfg->bSkipMenuFades ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "SpeedUpQuitGame", re4t::cfg->bSpeedUpQuitGame ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableDebugMenu", re4t::cfg->bEnableDebugMenu ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowGameOutput", re4t::cfg->bShowGameOutput ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableModExpansion", re4t::cfg->bEnableModExpansion ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ForceETSApplyScale", re4t::cfg->bForceETSApplyScale ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AlwaysShowOriginalTitleBg", re4t::cfg->bAlwaysShowOriginalTitleBackground ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "HideZoomControlHints", re4t::cfg->bHideZoomControlHints ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FixSilencedHandgunDescription", re4t::cfg->bFixSilencedHandgunDescription ? "true" : "false");
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
	spd::log()->info("+ HDPROJECT----------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "IsUsingHDProject", bIsUsingHDProject ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// TRAINER
	spd::log()->info("+ TRAINER------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "TrainerEnable", re4t::cfg->bTrainerEnable ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// PATCHES
	spd::log()->info("| {:<30} | {:>15} |", "UseNumpadMovement", re4t::cfg->bTrainerUseNumpadMovement ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "NumpadMovementSpeed", re4t::cfg->fTrainerNumMoveSpeed);
	spd::log()->info("| {:<30} | {:>15} |", "UseMouseWheelUpDown", re4t::cfg->bTrainerUseMouseWheelUpDown ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableFreeCamera", re4t::cfg->bTrainerEnableFreeCam ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "FreeCamSpeed", re4t::cfg->fTrainerFreeCamSpeed);
	spd::log()->info("| {:<30} | {:>15} |", "DisableEnemySpawn", re4t::cfg->bTrainerDisableEnemySpawn ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "DeadBodiesNeverDisappear", re4t::cfg->bTrainerDeadBodiesNeverDisappear ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "AllowEnterDoorsWithoutAshley", re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnableDebugTrg", re4t::cfg->bTrainerEnableDebugTrg ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowDebugTrgHintText", re4t::cfg->bTrainerShowDebugTrgHintText ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OpenInventoryOnItemAdd", re4t::cfg->bTrainerOpenInventoryOnItemAdd ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "LastAreaJumpStage", re4t::cfg->iTrainerLastAreaJumpStage);
	spd::log()->info("| {:<30} | {:>15} |", "LastAreaJumpRoomIdx", re4t::cfg->iTrainerLastAreaJumpRoomIdx);

	// OVERRIDES
	spd::log()->info("+ OVERRIDES----------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "OverrideCostumes", re4t::cfg->bOverrideCostumes ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "LeonCostume", sLeonCostumeNames[int(re4t::cfg->CostumeOverride.Leon)]);
	spd::log()->info("| {:<30} | {:>15} |", "AshleyCostume", sAshleyCostumeNames[int(re4t::cfg->CostumeOverride.Ashley)]);
	spd::log()->info("| {:<30} | {:>15} |", "AdaCostume", sAdaCostumeNames[int(re4t::cfg->CostumeOverride.Ada)]);
	spd::log()->info("| {:<30} | {:>15} |", "OverrideDifficulty", re4t::cfg->bOverrideDifficulty ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "NewDifficulty", sGameDifficultyNames[int(re4t::cfg->NewDifficulty)]);
	spd::log()->info("| {:<30} | {:>15} |", "EnablePlayerSpeedOverride", re4t::cfg->bTrainerPlayerSpeedOverride ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "PlayerSpeedOverride", re4t::cfg->fTrainerPlayerSpeedOverride);
	spd::log()->info("| {:<30} | {:>15} |", "EnableEnemyHPMultiplier", re4t::cfg->bTrainerEnemyHPMultiplier ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EnemyHPMultiplier", re4t::cfg->fTrainerEnemyHPMultiplier);
	spd::log()->info("| {:<30} | {:>15} |", "UseRandomHPMultiplier", re4t::cfg->bTrainerRandomHPMultiplier ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "RandomHPMultiplierMin", re4t::cfg->fTrainerRandomHPMultiMin);
	spd::log()->info("| {:<30} | {:>15} |", "RandomHPMultiplierMax", re4t::cfg->fTrainerRandomHPMultiMax);
	spd::log()->info("+--------------------------------+-----------------+");

	// WEAPON_HOTKEYS
	spd::log()->info("+ WEAPON_HOTKEYS-----------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "Enable", re4t::cfg->bWeaponHotkeysEnable ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// TRAINER_HOTKEYS
	spd::log()->info("+ TRAINER_HOTKEYS----------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "FocusUI", re4t::cfg->sTrainerFocusUIKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "NoclipToggle", re4t::cfg->sTrainerNoclipKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "FreeCamToggle", re4t::cfg->sTrainerFreeCamKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "SpeedOverrideToggle", re4t::cfg->sTrainerSpeedOverrideKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "MoveAshleyToPlayer", re4t::cfg->sTrainerMoveAshToPlayerKeyCombo.data());
	spd::log()->info("| {:<30} | {:>15} |", "DebugTrg", re4t::cfg->sTrainerDebugTrgKeyCombo.data());
	spd::log()->info("+--------------------------------+-----------------+");

	// ESP
	spd::log()->info("+ ESP----------------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "ShowESP", re4t::cfg->bShowESP ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowInfoOnTop", re4t::cfg->bEspShowInfoOnTop ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OnlyShowEnemies", re4t::cfg->bEspOnlyShowEnemies ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OnlyShowValidEms", re4t::cfg->bEspOnlyShowValidEms ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OnlyShowESLSpawned", re4t::cfg->bEspOnlyShowESLSpawned ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OnlyShowAlive", re4t::cfg->bEspOnlyShowAlive ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "MaxEmDistance", re4t::cfg->fEspMaxEmDistance);
	spd::log()->info("| {:<30} | {:>15} |", "OnlyShowClosestEms", re4t::cfg->bEspOnlyShowClosestEms ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ClosestEmsAmount", re4t::cfg->iEspClosestEmsAmount);
	spd::log()->info("| {:<30} | {:>15} |", "DrawLines", re4t::cfg->bEspDrawLines ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "EmNameMode", re4t::cfg->iEspEmNameMode);
	spd::log()->info("| {:<30} | {:>15} |", "EmHPMode", re4t::cfg->iEspEmHPMode);
	spd::log()->info("| {:<30} | {:>15} |", "DrawDebugInfo", re4t::cfg->bEspDrawDebugInfo ? "true" : "false");
	spd::log()->info("+--------------------------------+-----------------+");

	// SIDEINFO
	spd::log()->info("+ SIDEINFO-----------------------+-----------------+");
	spd::log()->info("| {:<30} | {:>15} |", "ShowSideInfo", re4t::cfg->bShowSideInfo ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowEmCount", re4t::cfg->bSideShowEmCount ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowEmList", re4t::cfg->bSideShowEmList ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "OnlyShowESLSpawned", re4t::cfg->bSideOnlyShowESLSpawned ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ShowSimpleNames", re4t::cfg->bSideShowSimpleNames ? "true" : "false");
	spd::log()->info("| {:<30} | {:>15} |", "ClosestEmsAmount", re4t::cfg->iSideClosestEmsAmount);
	spd::log()->info("| {:<30} | {:>15} |", "MaxEmDistance", re4t::cfg->fSideMaxEmDistance);
	spd::log()->info("| {:<30} | {:>15} |", "EmHPMode", re4t::cfg->iSideEmHPMode);
	spd::log()->info("+--------------------------------+-----------------+");
}