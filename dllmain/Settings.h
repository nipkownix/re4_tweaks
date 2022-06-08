#pragma once
#include <iostream>
#include <vector>
#include "Game.h"

struct Settings
{
	int KeyMap(const char* key, bool get_vk);

	// DISPLAY
	float fFOVAdditional;
	bool bEnableFOV;
	bool bFixUltraWideAspectRatio;
	bool bDisableVsync;
	bool bFixDisplayMode;
	int iCustomRefreshRate;
	bool bRestorePickupTransparency;
	bool bDisableBrokenFilter03;
	bool bFixBlurryImage;
	bool bDisableFilmGrain;
	bool bEnableGCBlur;
	bool bEnableGCScopeBlur;
	bool bWindowBorderless;
	int iWindowPositionX;
	int iWindowPositionY;
	bool bRememberWindowPos;

	// AUDIO
	float fVolumeBGM;
	float fVolumeSE;
	float fVolumeCutscene;

	// MOUSE
	bool bUseMouseTurning;
	float fTurnSensitivity;
	bool bUseRawMouseInput;
	bool bUnlockCameraFromAim;
	bool bFixSniperZoom;
	bool bFixSniperFocus;
	bool bFixRetryLoadMouseSelector;

	// KEYBOARD
	bool bUseSprintToggle;
	bool bFallbackToEnglishKeyIcons;
	std::string sFlipItemUp;
	std::string sFlipItemDown;
	std::string sFlipItemLeft;
	std::string sFlipItemRight;
	std::string sQTE_key_1;
	std::string sQTE_key_2;

	// CONTROLLER
	float fControllerSensitivity;
	bool bEnableControllerSens;
	bool bRemoveExtraXinputDeadzone;
	float fXinputDeadzone;

	// FRAME RATE
	bool bFixFallingItemsSpeed;
	bool bFixTurningSpeed;
	bool bFixQTE;
	bool bFixAshleyBustPhysics;

	// MISC
	std::string sWrappedDllPath;
	bool bOverrideCostumes;

	struct CostumeOverrideList
	{
		LeonCostumes Leon;
		AshleyCostumes Ashley;
		AdaCostumes Ada;
	} CostumeOverride;

	bool bAshleyJPCameraAngles;
	int iViolenceLevelOverride;
	bool bAllowSellingHandgunSilencer;
	bool bAllowMafiaLeonCutscenes;
	bool bSilenceArmoredAshley;
	bool bAllowAshleySuplex;
	bool bDisableQTE;
	bool bAutomaticMashingQTE;
	bool bSkipIntroLogos;
	bool bEnableDebugMenu;

	// MEMORY
	bool bAllowHighResolutionSFD;
	bool bRaiseVertexAlloc;
	bool bRaiseInventoryAlloc;

	// HOTKEYS
	std::string sConfigMenuKeyCombo;
	std::string sConsoleKeyCombo;
	std::string sDebugMenuKeyCombo;
	std::string sMouseTurnModifierKeyCombo;
	std::string sJetSkiTrickCombo;

	// WARNING
	bool bIgnoreFPSWarning;

	// IMGUI
	float fFontSize;
	bool bDisableMenuTip;
	bool bVerboseLog;

	bool HasUnsavedChanges;

	void ReadSettings();
	void WriteSettings();
};

extern Settings cfg;

extern const char* sLeonCostumeNames[5];
extern const char* sAshleyCostumeNames[3];
extern const char* sAdaCostumeNames[3];

// Parses an key combination string into a vector of VKs
std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo);