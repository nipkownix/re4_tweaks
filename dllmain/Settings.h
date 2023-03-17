#pragma once
#include <iostream>
#include <vector>
#include "Game.h"
#include "Patches.h"

enum MouseTurnTypes
{
	TypeA,
	TypeB
};

class re4t_cfg
{
public:
	// Funcs
	void ReadSettings();
	void WriteSettings(bool trainerOnly);
	void LogSettings();

	void ParseHotkeys();
	std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo); // Parses an key combination string into a vector of VKs

	bool HasUnsavedChanges = false;

	// Vars
	//=//=//
	// DISPLAY
	float fFOVAdditional = 0.0f;
	bool bEnableFOV = false;
	bool bDisableVsync = false;
	bool bUltraWideAspectSupport = true;
	bool bSideAlignHUD = true;
	bool bStretchFullscreenImages = false;
	bool bStretchVideos = false;
	bool bRemove16by10BlackBars = true;
	bool bFixDPIScale = true;
	bool bFixDisplayMode = true;
	bool bOnlyShowHighestRefreshRates = false;
	bool bOverrideLaserColor = false;
	bool bRainbowLaser = false;
	int iLaserR = 255;
	int iLaserG = 0;
	int iLaserB = 0;
	bool bRestorePickupTransparency = true;
	bool bDisableBrokenFilter03 = true;
	bool bFixBlurryImage = true;
	bool bDisableFilmGrain = true;
	bool bImproveWater = true;
	bool bEnableGCBlur = true;
	bool bUseEnhancedGCBlur = true;
	bool bEnableGCScopeBlur = true;
	bool bWindowBorderless = false;
	bool bEnableWindowResize = false;
	int iWindowPositionX = -1;
	int iWindowPositionY = -1;
	bool bRememberWindowPos = false;
	bool bRepositionHUD = false;
	float fHUDOffsetX = 0.0f;
	float fHUDOffsetY = 0.0f;
	bool bSmallerHUD = false;
	bool bSmallerActionPrompts = false;
	bool bHideZoomControlHints = false;

	// AUDIO
	int iVolumeMaster = 100;
	int iVolumeBGM = 100;
	int iVolumeSE = 100;
	int iVolumeCutscene = 100;
	bool bSilenceArmoredAshley = false;
	bool bRestoreGCSoundEffects = false;

	// MOUSE
	bool bCameraImprovements = true;
	bool bResetCameraAfterUsingWeapons = true;
	bool bResetCameraAfterUsingKnife = true;
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
	bool bSmoothAnalogTurning = false;

	// FRAME RATE
	bool bFixFallingItemsSpeed = true;
	bool bFixCompartmentsOpeningSpeed = true;
	bool bFixMovingGeometrySpeed = true;
	bool bFixTurningSpeed = true;
	bool bFixQTE = true;
	bool bFixAshleyBustPhysics = true;
	bool bEnableFastMath = true;
	bool bReplaceFramelimiter = true;
	bool bMultithreadFix = true;
	bool bPrecacheModels = false;

	// GAMEPLAY
	bool bAshleyJPCameraAngles = false;
	bool bSeparateWaysProfessional = true;
	bool bEnableNTSCMode = false;
	bool bAllowAshleySuplex = false;
	bool bFixDitmanGlitch = false;
	bool bAllowSellingHandgunSilencer = true;
	bool bBalancedChicagoTypewriter = false;
	bool bUseSprintToggle = false;
	bool bDisableAutomaticReload = false;
	bool bRifleScreenShake = false;
	bool bDisableQTE = false;
	bool bAutomaticMashingQTE = false;
	bool bAllowMatildaQuickturn = false;
	bool bLimitMatildaBurst = false;

	// MISC
	std::string sWrappedDllPath = "";
	bool bNeverCheckForUpdates = false;
	bool bRestoreDemoVideos = true;
	bool bRestoreAnalogTitleScroll = true;
	int iViolenceLevelOverride = -1;
	bool bAllowMafiaLeonCutscenes = true;
	bool bSkipIntroLogos = false;
	bool bSkipMenuFades = false;
	bool bSpeedUpQuitGame = true;
	bool bAlwaysShowOriginalTitleBackground = false;
	bool bFixSilencedHandgunDescription = true;
	bool bEnableDebugMenu = false;
	bool bShowGameOutput = false;
	bool bEnableModExpansion = false;
	bool bForceETSApplyScale = false;
  
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
	float fFontSizeScale = 1.0f;
	bool bEnableDPIScale = true;
	bool bDisableMenuTip = false;
	
	// DEBUG
	bool bVerboseLog = false;
	bool bNeverHideCursor = false;
	bool bUseDynamicFrametime = false;
	bool bDisableFramelimiting = false;

	float fdbg1 = 1.0f;
	float fdbg2 = 1.0f;
	float fdbg3 = 1.0f;
	float fdbg4 = 1.0f;
	float fdbg5 = 1.0f;
	float fdbg6 = 1.0f;
	bool bdbg1 = false;
	bool bdbg2 = false;
	bool bdbg3 = false;
	bool bdbg4 = false;

	// HD Project
	bool bIsUsingHDProject = false;

	// TRAINER
	bool bTrainerEnable = false;

	// PATCHES
	bool bTrainerUseNumpadMovement = true;
	float fTrainerNumMoveSpeed = 1.0f;
	bool bTrainerUseMouseWheelUpDown = true;
	bool bTrainerEnableFreeCam = false;
	float fTrainerFreeCamSpeed = 1.0f;
	bool bTrainerDisableEnemySpawn = false;
	bool bTrainerDeadBodiesNeverDisappear = false;
	bool bTrainerAllowEnterDoorsWithoutAsh = false;
	bool bTrainerEnableDebugTrg = false;
	bool bTrainerShowDebugTrgHintText = true;
	bool bTrainerOpenInventoryOnItemAdd = true;
	int iTrainerLastAreaJumpStage = 1;
	int iTrainerLastAreaJumpRoomIdx = 0;

	// OVERRIDES
	bool bOverrideCostumes = false;

	struct CostumeOverrideList
	{
		LeonCostume Leon = LeonCostume::Jacket;
		AshleyCostume Ashley = AshleyCostume::Normal;
		AdaCostume Ada = AdaCostume::Normal;
	} CostumeOverride;

	bool bOverrideDifficulty = false;
	GameDifficulty NewDifficulty = GameDifficulty::Medium;
	bool bTrainerOverrideDynamicDifficulty = false;
	int iTrainerDynamicDifficultyLevel = 5;
	bool bTrainerPlayerSpeedOverride = false;
	float fTrainerPlayerSpeedOverride = 1.0f;
	bool bTrainerEnemyHPMultiplier = false;
	float fTrainerEnemyHPMultiplier = 1.0f;
	bool bTrainerRandomHPMultiplier = false;
	float fTrainerRandomHPMultiMin = 0.3f;
	float fTrainerRandomHPMultiMax = 7.0f;

	// TRAINER_HOTKEYS
	std::string sTrainerFocusUIKeyCombo = "F5";
	std::string sTrainerNoclipKeyCombo = "";
	std::string sTrainerFreeCamKeyCombo = "";
	std::string sTrainerSpeedOverrideKeyCombo = "";
	std::string sTrainerMoveAshToPlayerKeyCombo = "";
	std::string sTrainerDebugTrgKeyCombo = "SHIFT+ALT";

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
	std::string sLastWeaponHotkey = "C";

	// ESP
	bool bShowESP = false;
	bool bEspShowInfoOnTop = false;
	bool bEspOnlyShowEnemies = true;
	bool bEspOnlyShowValidEms = true;
	bool bEspOnlyShowESLSpawned = false;
	bool bEspOnlyShowAlive = true;
	float fEspMaxEmDistance = 30000.0f;
	bool bEspOnlyShowClosestEms = false;
	int iEspClosestEmsAmount = 3;
	bool bEspDrawLines = false;
	int iEspEmNameMode = 1;
	int iEspEmHPMode = 1;
	bool bEspDrawDebugInfo = false;

	// SIDEINFO
	bool bShowSideInfo = false;
	bool bSideShowEmCount = true;
	bool bSideShowEmList = true;
	bool bSideOnlyShowESLSpawned = false;
	bool bSideShowSimpleNames = false;
	int iSideClosestEmsAmount = 5;
	float fSideMaxEmDistance = 30000.0f;
	int iSideEmHPMode = 1;
};

namespace re4t
{
	inline re4t_cfg* cfg = new re4t_cfg;
}

extern const char* sLeonCostumeNames[5];
extern const char* sAshleyCostumeNames[3];
extern const char* sAdaCostumeNames[4];

extern const char* sGameDifficultyNames[7];
