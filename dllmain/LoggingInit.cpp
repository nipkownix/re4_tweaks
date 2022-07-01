#include <iostream>
#include "stdafx.h"
#include "Patches.h"
#include "Settings.h"
#include "Logging/Logging.h"
#include "gitparams.h"
#include "resource.h"
#include "Game.h"

std::ofstream LOG;

const char* game_lang;

void LogSettings()
{
	char settingBuf[100];

	Logging::Log() << "+--------------------------------+-----------------+";
	Logging::Log() << "| Setting                        | Value           |";
	Logging::Log() << "+--------------------------------+-----------------+";

	// DISPLAY
	Logging::Log() << "+ DISPLAY------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15f |", "FOVAdditional", pConfig->fFOVAdditional);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixUltraWideAspectRatio", pConfig->bFixUltraWideAspectRatio ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableVsync", pConfig->bDisableVsync ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixDPIScale", pConfig->bFixDPIScale ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixDisplayMode", pConfig->bFixDisplayMode ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "CustomRefreshRate", pConfig->iCustomRefreshRate);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "OverrideLaserColor", pConfig->bOverrideLaserColor ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RainbowLaser", pConfig->bRainbowLaser ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "LaserR", pConfig->fLaserRGB[0] * 255.0f);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "LaserG", pConfig->fLaserRGB[1] * 255.0f);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "LaserB", pConfig->fLaserRGB[2] * 255.0f);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RestorePickupTransparency", pConfig->bRestorePickupTransparency ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableBrokenFilter03", pConfig->bDisableBrokenFilter03 ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixBlurryImage", pConfig->bFixBlurryImage ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableFilmGrain", pConfig->bDisableFilmGrain ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableGCBlur", pConfig->bEnableGCBlur ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableGCScopeBlur", pConfig->bEnableGCScopeBlur ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "WindowBorderless", pConfig->bWindowBorderless ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "WindowPositionX", pConfig->iWindowPositionX);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "WindowPositionY", pConfig->iWindowPositionY);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RememberWindowPos", pConfig->bRememberWindowPos ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// AUDIO
	Logging::Log() << "+ AUDIO--------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15i |", "VolumeMaster", pConfig->iVolumeMaster);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "VolumeBGM", pConfig->iVolumeBGM);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "VolumeSE", pConfig->iVolumeSE);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "VolumeCutscene", pConfig->iVolumeCutscene);
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// MOUSE
	Logging::Log() << "+ MOUSE--------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "UseMouseTurning", pConfig->bUseMouseTurning ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "TurnSensitivity", pConfig->fTurnSensitivity);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "UseRawMouseInput", pConfig->bUseRawMouseInput ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DetachCameraFromAim", pConfig->bDetachCameraFromAim ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixSniperZoom", pConfig->bFixSniperZoom ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixSniperFocus", pConfig->bFixSniperFocus ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixRetryLoadMouseSelector", pConfig->bFixRetryLoadMouseSelector ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// KEYBOARD
	Logging::Log() << "+ KEYBOARD-----------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "FallbackToEnglishKeyIcons", pConfig->bFallbackToEnglishKeyIcons ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// CONTROLLER
	Logging::Log() << "+ CONTROLLER---------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "OverrideControllerSensitivity", pConfig->bOverrideControllerSensitivity ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "ControllerSensitivity", pConfig->fControllerSensitivity);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RemoveExtraXinputDeadzone", pConfig->bRemoveExtraXinputDeadzone ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "OverrideXinputDeadzone", pConfig->bOverrideXinputDeadzone ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "XinputDeadzone", pConfig->fXinputDeadzone);
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// FRAME RATE
	Logging::Log() << "+ FRAME RATE---------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "FixFallingItemsSpeed", pConfig->bFixFallingItemsSpeed ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixTurningSpeed", pConfig->bFixTurningSpeed ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixQTE", pConfig->bFixQTE ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixAshleyBustPhysics", pConfig->bFixAshleyBustPhysics ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableFastMath", pConfig->bEnableFastMath ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "PrecacheModels", pConfig->bPrecacheModels ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// MISC
	Logging::Log() << "+ MISC---------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "WrappedDllPath", pConfig->sWrappedDllPath.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "OverrideCostumes", pConfig->bOverrideCostumes ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "LeonCostume", sLeonCostumeNames[(int)pConfig->CostumeOverride.Leon]);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AshleyCostume", sAshleyCostumeNames[(int)pConfig->CostumeOverride.Ashley]);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AdaCostume", sAdaCostumeNames[(int)pConfig->CostumeOverride.Ada]);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AshleyJPCameraAngles", pConfig->bAshleyJPCameraAngles ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15d |", "ViolenceLevelOverride", pConfig->iViolenceLevelOverride);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowSellingHandgunSilencer", pConfig->bAllowSellingHandgunSilencer ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowMafiaLeonCutscenes", pConfig->bAllowMafiaLeonCutscenes ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "SilenceArmoredAshley", pConfig->bSilenceArmoredAshley ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowAshleySuplex", pConfig->bAllowAshleySuplex ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowMatildaQuickturn", pConfig->bAllowMatildaQuickturn ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixDitmanGlitch", pConfig->bFixDitmanGlitch ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "UseSprintToggle", pConfig->bUseSprintToggle ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableQTE", pConfig->bDisableQTE ? "true" : "false");
	Logging::Log() << settingBuf;
	
	sprintf(settingBuf, "| %-30s | %15s |", "AutomaticMashingQTE", pConfig->bAutomaticMashingQTE ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "SkipIntroLogos", pConfig->bSkipIntroLogos ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableDebugMenu", pConfig->bEnableDebugMenu ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// MEMORY
	Logging::Log() << "+ MEMORY-------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "AllowHighResolutionSFD", pConfig->bAllowHighResolutionSFD ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RaiseVertexAlloc", pConfig->bRaiseVertexAlloc ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RaiseInventoryAlloc", pConfig->bRaiseInventoryAlloc ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// HOTKEYS
	Logging::Log() << "+ HOTKEYS------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "ConfigMenu", pConfig->sConfigMenuKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "Console", pConfig->sConsoleKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemUp", pConfig->sFlipItemUp.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemDown", pConfig->sFlipItemDown.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemLeft", pConfig->sFlipItemLeft.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemRight", pConfig->sFlipItemRight.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "QTE_key_1", pConfig->sQTE_key_1.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "QTE_key_2", pConfig->sQTE_key_2.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DebugMenu", pConfig->sDebugMenuKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "MouseTurningModifier", pConfig->sMouseTurnModifierKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "JetSkiTricks", pConfig->sJetSkiTrickCombo.data());
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// FPS WARNING
	Logging::Log() << "+ WARNING------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "IgnoreFPSWarning", pConfig->bIgnoreFPSWarning ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// IMGUI
	Logging::Log() << "+ IMGUI--------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15f |", "FontSize", pConfig->fFontSize);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableMenuTip", pConfig->bDisableMenuTip ? "true" : "false");
	Logging::Log() << settingBuf;

	// DEBUG
	Logging::Log() << "+ DEBUG--------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "VerboseLog", pConfig->bVerboseLog ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "NeverHideCursor", pConfig->bNeverHideCursor ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";
}


void Init_Logging()
{
	// Start logging
	std::string logPath = rootPath + WrapperName.substr(0, WrapperName.find_last_of('.')) + ".log";
	Logging::Open(logPath);

	std::string commit = GIT_CUR_COMMIT;
	commit.resize(8);

	Logging::Log() << "Starting re4_tweaks v" << APP_VERSION << "-" << commit << "-" << GIT_BRANCH;

	Logging::LogComputerManufacturer();
	Logging::LogOSVersion();
	Logging::LogProcessNameAndPID();

	Logging::Log() << "Running from: " << rootPath;

		// Log current game language
	{
		auto pattern = hook::pattern("8A ? 08 8B ? ? ? ? ? 88 ? ? ? ? ? C3 8B FF");
		struct GameLangLog
		{
			void operator()(injector::reg_pack& regs)
			{
				if (GameVersion() == "1.1.0")
					*(uint8_t*)(regs.ecx + 0x4FA3) = (uint8_t)regs.eax;
				else
					*(uint8_t*)(regs.edx + 0x4FA3) = (uint8_t)regs.ecx;
					

				switch (GameVersion() == "1.1.0" ? (uint8_t)regs.eax : (uint8_t)regs.ecx)
				{
				case 2:
					game_lang = "English";
					break;				
				case 3:
					game_lang = "German";
					break;
				case 4:
					game_lang = "French";
					break;
				case 5:
					game_lang = "Spanish";
					break;
				case 6:
					game_lang = GameVersion() == "1.1.0" ? "Traditional Chinese" : "Italian";
					break;
				case 7:
					game_lang = "Simplified Chinese";
					break;
				case 8:
					game_lang = "Italian";
					break;
				default:
					game_lang = "Unknown"; // We should never reach this.
				}

				#ifdef VERBOSE
				con.AddConcatLog("Game language: ", game_lang);
				#endif

				Logging::Log() << "Game language: " << game_lang;
			}
		}; 
		
		// Japanese exe doesn't have the setLanguage function, so we end up hooking nothing
		if (pattern.size() != 1)
		{
			#ifdef VERBOSE
			con.AddLogChar("Game language: Japanese");
			#endif

			Logging::Log() << "Game language: Japanese";
		}
		else
			injector::MakeInline<GameLangLog>(pattern.count(1).get(0).get<uint32_t>(9), pattern.count(1).get(0).get<uint32_t>(15));
	}
}