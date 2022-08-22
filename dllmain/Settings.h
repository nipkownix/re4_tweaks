#pragma once
#include <iostream>
#include <vector>
#include "Game.h"
#include "Patches.h"

class Config
{
public:
	// DISPLAY
	float fFOVAdditional = 0.0f;
	bool bEnableFOV = false;
	bool bUltraWideAspectSupport = true;
	bool bSideAlignHUD = true;
	bool bStretchFullscreenImages = false;
	bool bStretchVideos = false;
	bool bRemove16by10BlackBars = true;
	bool bDisableVsync = false;
	bool bFixDPIScale = true;
	bool bFixDisplayMode = true;
	int iCustomRefreshRate = -1;
	bool bOverrideLaserColor = false;
	bool bRainbowLaser = false;
	int iLaserR = 255;
	int iLaserG = 0;
	int iLaserB = 0;
	bool bRestorePickupTransparency = true;
	bool bDisableBrokenFilter03 = true;
	bool bFixBlurryImage = true;
	bool bDisableFilmGrain = true;
	bool bEnableGCBlur = true;
	bool bUseEnhancedGCBlur = true;
	bool bEnableGCScopeBlur = true;
	bool bWindowBorderless = false;
	int iWindowPositionX = -1;
	int iWindowPositionY = -1;
	bool bRememberWindowPos = false;

	// AUDIO
	int iVolumeMaster = 100;
	int iVolumeBGM = 100;
	int iVolumeSE = 100;
	int iVolumeCutscene = 100;

	// MOUSE
	bool bCameraImprovements = true;
	bool bResetCameraWhenRunning = true;
	float fCameraSensitivity = 1.0f;
	bool bUseMouseTurning = true;
	int iMouseTurnType = MouseTurnTypes::TypeA;
	float fTurnTypeBSensitivity = 1.0f;
	bool bUseRawMouseInput = true;
	bool bDetachCameraFromAim = false;
	bool bFixSniperZoom = true;
	bool bFixSniperFocus = true;
	bool bFixRetryLoadMouseSelector = true;

	// KEYBOARD
	bool bFallbackToEnglishKeyIcons = true;
	bool bAllowReloadWithoutAiming_kbm = true;
	bool bReloadWithoutZoom_kbm = false;

	// CONTROLLER
	bool bOverrideControllerSensitivity = false;
	float fControllerSensitivity = 1.0f;
	bool bRemoveExtraXinputDeadzone = true;
	bool bOverrideXinputDeadzone = true;
	float fXinputDeadzone = 0.4f;
	bool bAllowReloadWithoutAiming_controller = false;
	bool bReloadWithoutZoom_controller = false;

	// FRAME RATE
	bool bFixFallingItemsSpeed = true;
	bool bFixTurningSpeed = true;
	bool bFixQTE = true;
	bool bFixAshleyBustPhysics = true;
	bool bEnableFastMath = true;
	bool bPrecacheModels = false;

	// MISC
	std::string sWrappedDllPath = "";
	bool bOverrideCostumes = false;

	struct CostumeOverrideList
	{
		LeonCostume Leon;
		AshleyCostume Ashley;
		AdaCostume Ada;
	} CostumeOverride;

	bool bAshleyJPCameraAngles = false;
	int iViolenceLevelOverride = -1;
	bool bAllowSellingHandgunSilencer = true;
	bool bAllowMafiaLeonCutscenes = true;
	bool bSilenceArmoredAshley = false;
	bool bAllowAshleySuplex = false;
	bool bAllowMatildaQuickturn = false;
	bool bFixDitmanGlitch = false;
	bool bUseSprintToggle = false;
	bool bDisableQTE = false;
	bool bAutomaticMashingQTE = false;
	bool bSkipIntroLogos = false;
	bool bEnableDebugMenu = false;
	bool bEnableModExpansion = false;

	// MEMORY
	bool bAllowHighResolutionSFD = true;
	bool bRaiseVertexAlloc = true;
	bool bRaiseInventoryAlloc = true;

	// HOTKEYS
	std::string sConfigMenuKeyCombo = "F1";
	std::string sConsoleKeyCombo = "F2";
	std::string sFlipItemUp = "HOME";
	std::string sFlipItemDown = "END";
	std::string sFlipItemLeft = "INSERT";
	std::string sFlipItemRight = "PAGEUP";
	std::string sQTE_key_1 = "D";
	std::string sQTE_key_2 = "A";
	std::string sDebugMenuKeyCombo = "CTRL+F3";
	std::string sMouseTurnModifierKeyCombo = "ALT";
	std::string sJetSkiTrickCombo = "LMOUSE+RMOUSE";

	// TRAINER_HOTKEYS
	std::string sTrainerFocusUIKeyCombo = "F5";
	std::string sTrainerNoclipKeyCombo = "";
	std::string sTrainerSpeedOverrideKeyCombo = "";
	std::string sTrainerMoveAshToPlayerKeyCombo = "";

	// WEAPON_HOTKEYS
	bool bWeaponHotkeysEnable = false;
	std::string sWeaponHotkeys[5] = { "1", "2", "3", "4", "5" };
	int iWeaponHotkeyWepIds[5] = { 0,0,0,0,0 };
	std::vector<int> iWeaponHotkeyCycle[5] = { 
		{ 35,37,33,39,3,41,42,55 }, // pistols
		{ 44,45,71,148 }, // shotguns
		{ 16,46,47,48,52,83 }, // rifle/machine guns
		{ 53,109,54 }, // rocket/mine launchers
		{ 1,2,14,8,9,10 }, // throwables
	};
	std::string iWeaponHotkeyCycleString[5] = {
		"35,37,33,39,3,41,42,55",
		"44,45,71,148",
		"16,46,47,48,52,83",
		"53,109,54",
		"1,2,14,8,9,10"
	};

	// TRAINER
	bool bTrainerEnable = false;
	bool bTrainerPlayerSpeedOverride = false;
	float fTrainerPlayerSpeedOverride = 1.0f;

	bool bTrainerUseNumpadMovement = true;
	bool bTrainerUseMouseWheelUpDown = true;

	// WARNING
	bool bIgnoreFPSWarning = false;

	// IMGUI
	float fFontSizeScale = 1.0f;
	bool bDisableMenuTip = false;
	
	// DEBUG
	bool bVerboseLog = false;
	bool bNeverHideCursor = false;

	bool HasUnsavedChanges = false;

	float fdbg1 = 1.0f;
	float fdbg2 = 1.0f;
	float fdbg3 = 1.0f;
	float fdbg4 = 1.0f;
	float fdbg5 = 1.0f;
	float fdbg6 = 1.0f;
	bool bdbg1;
	bool bdbg2;
	bool bdbg3;
	bool bdbg4;

	void ReadSettings();
	void ParseHotkeys();
	void ReadSettings(std::string_view ini_path);
	void WriteSettings();

	void LogSettings();
};

extern std::shared_ptr<class Config> pConfig;

extern const char* sLeonCostumeNames[5];
extern const char* sAshleyCostumeNames[3];
extern const char* sAdaCostumeNames[3];

extern bool bIsUsingHDProject;

// Parses an key combination string into a vector of VKs
std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo);