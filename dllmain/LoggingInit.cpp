#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
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

	sprintf(settingBuf, "| %-30s | %15f |", "FOVAdditional", cfg.fFOVAdditional);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixUltraWideAspectRatio", cfg.bFixUltraWideAspectRatio ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableVsync", cfg.bDisableVsync ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixDisplayMode", cfg.bFixDisplayMode ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "CustomRefreshRate", cfg.iCustomRefreshRate);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RestorePickupTransparency", cfg.bRestorePickupTransparency ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableBrokenFilter03", cfg.bDisableBrokenFilter03 ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixBlurryImage", cfg.bFixBlurryImage ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableFilmGrain", cfg.bDisableFilmGrain ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableGCBlur", cfg.bEnableGCBlur ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableGCScopeBlur", cfg.bEnableGCScopeBlur ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "WindowBorderless", cfg.bWindowBorderless ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "WindowPositionX", cfg.iWindowPositionX);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15i |", "WindowPositionY", cfg.iWindowPositionY);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RememberWindowPos", cfg.bRememberWindowPos ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// MOUSE
	Logging::Log() << "+ MOUSE--------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "UseMouseTurning", cfg.bUseMouseTurning ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "TurnSensitivity", cfg.fTurnSensitivity);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "UseRawMouseInput", cfg.bUseRawMouseInput ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "UnlockCameraFromAim", cfg.bUnlockCameraFromAim ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixSniperZoom", cfg.bFixSniperZoom ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixSniperFocus", cfg.bFixSniperFocus ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixRetryLoadMouseSelector", cfg.bFixRetryLoadMouseSelector ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// KEYBOARD
	Logging::Log() << "+ KEYBOARD-----------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "UseSprintToggle", cfg.bUseSprintToggle ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FallbackToEnglishKeyIcons", cfg.bFallbackToEnglishKeyIcons ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemUp", cfg.sFlipItemUp.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemDown", cfg.sFlipItemDown.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemLeft", cfg.sFlipItemLeft.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FlipItemRight", cfg.sFlipItemRight.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "QTE_key_1", cfg.sQTE_key_1.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "QTE_key_2", cfg.sQTE_key_2.data());
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// CONTROLLER
	Logging::Log() << "+ CONTROLLER---------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15f |", "ControllerSensitivity", cfg.fControllerSensitivity);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RemoveExtraXinputDeadzone", cfg.bRemoveExtraXinputDeadzone ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15f |", "XinputDeadzone", cfg.fXinputDeadzone);
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// FRAME RATE
	Logging::Log() << "+ FRAME RATE---------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "FixFallingItemsSpeed", cfg.bFixFallingItemsSpeed ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixTurningSpeed", cfg.bFixTurningSpeed ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixQTE", cfg.bFixQTE ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "FixAshleyBustPhysics", cfg.bFixAshleyBustPhysics ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableReimplementedCloth", cfg.bEnableReimplementedCloth ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// MISC
	Logging::Log() << "+ MISC---------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "WrappedDllPath", cfg.sWrappedDllPath.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AshleyJPCameraAngles", cfg.bAshleyJPCameraAngles ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15d |", "ViolenceLevelOverride", cfg.iViolenceLevelOverride);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowSellingHandgunSilencer", cfg.bAllowSellingHandgunSilencer ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowMafiaLeonCutscenes", cfg.bAllowMafiaLeonCutscenes ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "SilenceArmoredAshley", cfg.bSilenceArmoredAshley ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "AllowAshleySuplex", cfg.bAllowAshleySuplex ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableQTE", cfg.bDisableQTE ? "true" : "false");
	Logging::Log() << settingBuf;
	
	sprintf(settingBuf, "| %-30s | %15s |", "AutomaticMashingQTE", cfg.bAutomaticMashingQTE ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "SkipIntroLogos", cfg.bSkipIntroLogos ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "EnableDebugMenu", cfg.bEnableDebugMenu ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// MEMORY
	Logging::Log() << "+ MEMORY-------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "AllowHighResolutionSFD", cfg.bAllowHighResolutionSFD ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RaiseVertexAlloc", cfg.bRaiseVertexAlloc ? "true" : "false");
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "RaiseInventoryAlloc", cfg.bRaiseInventoryAlloc ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// HOTKEYS
	Logging::Log() << "+ HOTKEYS------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "ConfigMenu", cfg.sConfigMenuKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "Console", cfg.sConsoleKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DebugMenu", cfg.sDebugMenuKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "MouseTurningModifier", cfg.sMouseTurnModifierKeyCombo.data());
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "JetSkiTricks", cfg.sJetSkiTrickCombo.data());
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// FPS WARNING
	Logging::Log() << "+ WARNING------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15s |", "IgnoreFPSWarning", cfg.bIgnoreFPSWarning ? "true" : "false");
	Logging::Log() << settingBuf;

	Logging::Log() << "+--------------------------------+-----------------+";

	// IMGUI
	Logging::Log() << "+ IMGUI--------------------------+-----------------+";

	sprintf(settingBuf, "| %-30s | %15f |", "FontSize", cfg.fFontSize);
	Logging::Log() << settingBuf;

	sprintf(settingBuf, "| %-30s | %15s |", "DisableMenuTip", cfg.bDisableMenuTip ? "true" : "false");
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