#pragma once
#include <iostream>
#include <vector>
#include "Game.h"

class Config
{
public:
	// DISPLAY
	float fFOVAdditional = 0.0f;
	bool bEnableFOV = false;
	bool bFixUltraWideAspectRatio = true;
	bool bDisableVsync = false;
	bool bFixDPIScale = true;
	bool bFixDisplayMode = true;
	int iCustomRefreshRate = -1;
	bool bOverrideLaserColor = false;
	bool bRainbowLaser = false;
	float fLaserRGB[3] = { 1.0f, 0.0f, 0.0f };
	bool bRestorePickupTransparency = true;
	bool bDisableBrokenFilter03 = true;
	bool bFixBlurryImage = true;
	bool bDisableFilmGrain = true;
	bool bEnableGCBlur = true;
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
	bool bUseMouseTurning = true;
	float fTurnSensitivity = 1.0f;
	bool bUseRawMouseInput = true;
	bool bDetachCameraFromAim = false;
	bool bFixSniperZoom = true;
	bool bFixSniperFocus = true;
	bool bFixRetryLoadMouseSelector = true;

	// KEYBOARD
	bool bFallbackToEnglishKeyIcons = true;

	// CONTROLLER
	bool bOverrideControllerSensitivity = false;
	float fControllerSensitivity = 1.0f;
	bool bRemoveExtraXinputDeadzone = true;
	bool bOverrideXinputDeadzone = true;
	float fXinputDeadzone = 0.4f;

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
		LeonCostumes Leon;
		AshleyCostumes Ashley;
		AdaCostumes Ada;
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

	// WARNING
	bool bIgnoreFPSWarning = false;

	// IMGUI
	float fFontSize = 1.0f;
	bool bDisableMenuTip = false;
	
	// DEBUG
	bool bVerboseLog = false;
	bool bNeverHideCursor = false;

	bool HasUnsavedChanges = false;

	void ReadSettings();
	void ReadSettings(std::string_view ini_path);
	void WriteSettings();
};

extern std::shared_ptr<class Config> pConfig;

extern const char* sLeonCostumeNames[5];
extern const char* sAshleyCostumeNames[3];
extern const char* sAdaCostumeNames[3];

// Parses an key combination string into a vector of VKs
std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo);