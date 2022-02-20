#pragma once
#include <iostream>
#include <vector>

// Globals
struct Settings
{
	int Tab = 0;

	int KeyMap(const char* key, bool get_vk);

	// Those save
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
	bool bAshleyJPCameraAngles;
	bool bWindowBorderless;
	int iWindowPositionX;
	int iWindowPositionY;
	bool bRememberWindowPos;
	std::string sWrappedDllPath;
	bool bSkipIntroLogos;
	bool bAllowMafiaLeonCutscenes;
	bool bSilenceArmoredAshley;
	bool bEnableDebugMenu;
	bool bAllowSellingHandgunSilencer;
	float fControllerSensitivity;
	bool bEnableControllerSens;
	bool bUseMouseTurning;
	float fTurnSensitivity;
	bool bUseRawMouseInput;
	bool bFallbackToEnglishKeyIcons;
	bool bFixSniperZoom;
	bool bFixSniperFocus;
	bool bFixRetryLoadMouseSelector;
	std::string sFlipItemUp;
	std::string sFlipItemDown;
	std::string sFlipItemLeft;
	std::string sFlipItemRight;
	std::string sQTE_key_1;
	std::string sQTE_key_2;
	bool bFixFallingItemsSpeed;
	bool bFixQTE;
	bool bFixAshleyBustPhysics;
	bool bAllowHighResolutionSFD;
	bool bRaiseVertexAlloc;
	bool bRaiseInventoryAlloc;
	std::string sConfigMenuKeyCombo;
	std::string sConsoleKeyCombo;
	std::string sDebugMenuKeyCombo;
	std::string sMouseTurnModifierKeyCombo;
	bool bIgnoreFPSWarning;
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