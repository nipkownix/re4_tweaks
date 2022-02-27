#pragma once
#include <iostream>
#include <vector>

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
	bool bFixQTE;
	bool bFixAshleyBustPhysics;

	// MISC
	std::string sWrappedDllPath;
	bool bAshleyJPCameraAngles;
	bool bAllowSellingHandgunSilencer;
	bool bAllowMafiaLeonCutscenes;
	bool bSilenceArmoredAshley;
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

// Parses an key combination string into a vector of VKs
std::vector<uint32_t> ParseKeyCombo(std::string_view in_combo);