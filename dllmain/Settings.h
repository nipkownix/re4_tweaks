#pragma once
#include <iostream>
#include <vector>


// Globals
struct Settings
{
	int Tab = 0;

	// Those save
	float fFOVAdditional;
	bool bEnableFOV;
	bool bFixUltraWideAspectRatio;
	bool bFixVsyncToggle;
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
	bool bFixQTE;
	bool bSkipIntroLogos;
	bool bEnableDebugMenu;
	bool bFixSniperZoom;
	bool bFixRetryLoadMouseSelector;
	std::string flip_item_up;
	std::string flip_item_down;
	std::string flip_item_left;
	std::string flip_item_right;
	std::string QTE_key_1;
	std::string QTE_key_2;
	bool bAllowHighResolutionSFD;
	bool bRaiseVertexAlloc;
	bool bRaiseInventoryAlloc;
	bool bUseMemcpy;
	bool bIgnoreFPSWarning;

	void ReadSettings();
	void WriteSettings();
};

extern Settings cfg;