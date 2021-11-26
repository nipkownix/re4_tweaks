#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\ModUtils\MemoryMgr.h"
#include "..\external\MinHook\MinHook.h"
#include "WndProcHook.h"
#include "dllmain.h"
#include "cfgMenu.h"
#include "HandleLimits.h"
#include "Sofdec.h"
#include "FilterXXFixes.h"
#include "UltraWideFix.h"
#include "tool_menu.h"

std::string RealDllPath;
std::string WrapperMode;
std::string WrapperName;
std::string game_version;
std::string QTE_key_1;
std::string QTE_key_2;
std::string flip_item_up;
std::string flip_item_down;
std::string flip_item_left;
std::string flip_item_right;

HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;

CIniReader iniReader("");

float fFOVAdditional = 0.0f;
float fQTESpeedMult = 1.5f;

bool bCfgMenuOpen;
bool bShouldFlipX;
bool bShouldFlipY;
bool bIsItemUp;
bool bFixSniperZoom;
bool bFixVsyncToggle;
bool bFixUltraWideAspectRatio;
bool bFixQTE;
bool bSkipIntroLogos;
bool bFixRetryLoadMouseSelector;
bool bRestorePickupTransparency;
bool bDisableBrokenFilter03;
bool bFixBlurryImage;
bool bAllowHighResolutionSFD;
bool bDisableFilmGrain;
bool bRaiseVertexAlloc;
bool bRaiseInventoryAlloc;
bool bUseMemcpy;
bool bIgnoreFPSWarning;
bool bWindowBorderless;
bool bRememberWindowPos;
bool bEnableGCBlur;
bool bEnableGCScopeBlur;
bool bEnableDebugMenu;
bool bisDebugBuild;

uintptr_t* ptrInvMovAddr;
uintptr_t ptrRetryLoadDLGstate;

static uint32_t* ptrGameFrameRate;

int iWindowPositionX;
int iWindowPositionY;
int intQTE_key_1;
int intQTE_key_2;
int intGameState;

BOOL __stdcall SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	int windowX = iWindowPositionX < 0 ? 0 : iWindowPositionX;
	int windowY = iWindowPositionY < 0 ? 0 : iWindowPositionY;
	return SetWindowPos(hWnd, hWndInsertAfter, windowX, windowY, cx, cy, uFlags);
}

void ReadSettings()
{
	fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", 0.0f);
	bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", true);
	bFixVsyncToggle = iniReader.ReadBoolean("DISPLAY", "FixVsyncToggle", true);
	bRestorePickupTransparency = iniReader.ReadBoolean("DISPLAY", "RestorePickupTransparency", true);
	bDisableBrokenFilter03 = iniReader.ReadBoolean("DISPLAY", "DisableBrokenFilter03", true);
	bFixBlurryImage = iniReader.ReadBoolean("DISPLAY", "FixBlurryImage", true);
	bDisableFilmGrain = iniReader.ReadBoolean("DISPLAY", "DisableFilmGrain", true);
	bEnableGCBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCBlur", true);
	bEnableGCScopeBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCScopeBlur", true);
	bWindowBorderless = iniReader.ReadBoolean("DISPLAY", "WindowBorderless", false);
	iWindowPositionX = iniReader.ReadInteger("DISPLAY", "WindowPositionX", -1);
	iWindowPositionY = iniReader.ReadInteger("DISPLAY", "WindowPositionY", -1);
	bRememberWindowPos = iniReader.ReadBoolean("DISPLAY", "RememberWindowPos", false);
	bFixQTE = iniReader.ReadBoolean("MISC", "FixQTE", true);
	bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", false);
	bEnableDebugMenu = iniReader.ReadBoolean("MISC", "EnableDebugMenu", false);
	bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", true);
	bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", true);
	flip_item_up = iniReader.ReadString("KEYBOARD", "flip_item_up", "HOME");
	flip_item_down = iniReader.ReadString("KEYBOARD", "flip_item_down", "END");
	flip_item_left = iniReader.ReadString("KEYBOARD", "flip_item_left", "INSERT");
	flip_item_right = iniReader.ReadString("KEYBOARD", "flip_item_right", "PAGEUP");
	QTE_key_1 = iniReader.ReadString("KEYBOARD", "QTE_key_1", "D");
	QTE_key_2 = iniReader.ReadString("KEYBOARD", "QTE_key_2", "A");
	bAllowHighResolutionSFD = iniReader.ReadBoolean("MOVIE", "AllowHighResolutionSFD", true);
	bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", true);
	bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", true);
	bUseMemcpy = iniReader.ReadBoolean("MEMORY", "UseMemcpy", true);
	bIgnoreFPSWarning = iniReader.ReadBoolean("FRAME RATE", "IgnoreFPSWarning", false);
}

void HandleAppID()
{
	//Create missing steam_appid file
	const char *filename = "steam_appid.txt";
	if (std::filesystem::exists(filename) == false) {
		std::ofstream appid(filename);
		appid << "254700";
		appid.close();
	}
}

void Init_Main()
{
	std::cout << "Big ironic thanks to QLOC S.A." << std::endl;

	// Detect game version
	auto pattern = hook::pattern("31 2E ? ? ? 00 00 00 6D 6F 76 69 65 2F 64 65 6D 6F 30 65 6E 67 2E 73 66 64");
	int ver = injector::ReadMemory<int>(pattern.get_first(2));

	if (ver == 0x362E30) {
		game_version = "1.0.6";
	} else if (ver == 0x302E31) {
		game_version = "1.1.0";
	} else {
		MessageBoxA(NULL, "This version of RE4 is not supported.\nre4_tweaks will be disabled.", "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		return;
	}

	// Check for part of gameDebug function, if exists this must be a debug-enabled build
	pattern = hook::pattern("6A 00 6A 00 6A 08 68 AE 01 00 00 6A 10 6A 0A");
	if (pattern.size() > 0)
	{
		game_version += "d";
		bisDebugBuild = true;
	}

	#ifdef VERBOSE
	std::cout << "Game version = " << game_version << std::endl;
	#endif

	MH_Initialize();

	ReadSettings();

	HandleAppID();

	Init_WndProcHook();

	Init_cfgMenu();

	Init_HandleLimits();

	Init_FilterXXFixes();

	// SFD size
	if (bAllowHighResolutionSFD)
		Init_sofdec();

	// FOV
	if (fFOVAdditional != 0.0f)
	{
		// Hook function that loads the FOV
		auto pattern = hook::pattern("D9 45 ? 89 4E ? D9 5E ? 8B 8D ? ? ? ? 89 46 ? 8B 85 ? ? ? ? 8D 7E");
		struct ScaleFOV
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaFov = *(float*)(regs.ebp - 0x34);
				_asm
				{
					fld   vanillaFov
					fadd  fFOVAdditional
				}
				*(int32_t*)(regs.esi + 0x4) = regs.ecx;
			}
		}; injector::MakeInline<ScaleFOV>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
	
	// Fix vsync option
	if (bFixVsyncToggle)
	{
		auto pattern = hook::pattern("50 E8 ? ? ? ? C7 07 01 00 00 00 E9");
		injector::MakeNOP(pattern.get_first(6), 6);
	}

	// Apply window changes
	if (bWindowBorderless || iWindowPositionX > -1 || iWindowPositionY > -1)
	{
		// hook SetWindowPos, can't nop as it's used for resizing window on resolution changes
		auto pattern = hook::pattern("BE 00 01 04 00 BF 00 00 C8 00");
		injector::MakeNOP(pattern.get_first(0xA), 6);
		injector::MakeCALL(pattern.get_first(0xA), SetWindowPos_Hook, true);

		// nop MoveWindow
		pattern = hook::pattern("53 51 52 53 53 50");
		injector::MakeNOP(pattern.get_first(6), 6);

		if (bWindowBorderless)
		{
			// if borderless, update the style set by SetWindowLongA
			pattern = hook::pattern("25 00 00 38 7F 05 00 00 C8 00");
			injector::WriteMemory(pattern.get_first(5), uint8_t(0xb8), true); // mov eax, XXXXXXXX
			injector::WriteMemory(pattern.get_first(6), uint32_t(WS_POPUP), true);
		}
	}

	// Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action.
	if (bFixRetryLoadMouseSelector)
	{
		// Get pointer for the state. Only reliable way I found to achieve this.
		pattern = hook::pattern("C7 06 ? ? ? ? A1 ? ? ? ? F7 80 ? ? ? ? ? ? ? ? 74"); //0x48C1C0
		struct RLDLGState
		{
			void operator()(injector::reg_pack& regs)
			{
				*(int*)(regs.esi) = 0;
				ptrRetryLoadDLGstate = regs.esi + 0x3;
			}
		}; injector::MakeInline<RLDLGState>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Check if the "yes/no" prompt is open before sending any index updates
		pattern = hook::pattern("89 8F ? ? ? ? FF 85 ? ? ? ? 83 C6 ? 3B 77");
		struct MouseMenuSelector
		{
			void operator()(injector::reg_pack& regs)
			{
				if (*(int32_t*)ptrRetryLoadDLGstate != 1)
				{
					*(int32_t*)(regs.edi + 0x160) = regs.ecx;
				}
			}
		}; injector::MakeInline<MouseMenuSelector>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Check if the game is running at a valid frame rate
	if (!bIgnoreFPSWarning)
	{
		// Hook function to read the FPS value from config.ini
		auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
		ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);
		struct ReadFPS
		{
			void operator()(injector::reg_pack& regs)
			{
				int intIniFPS = regs.ecx;
				int intNewFPS;

				#ifdef VERBOSE
				std::cout << "Config.ini frame rate = " << intIniFPS << std::endl;
				#endif

				if (intIniFPS != 30 && intIniFPS != 60) {

					// Calculate new fps
					if (intIniFPS > 45)
						intNewFPS = 60;
					else
						intNewFPS = 30;

					// Show warning
					std::string Msg = "ERROR: Invalid frame rate detected\n\nYour game appears to be configured to run at " + std::to_string(intIniFPS) +
						" FPS, which isn't supported and will break multiple aspects of the game.\nThis could mean you edited the game's \"config.ini\" file" +
						" and changed the \"variableframerate\" value to something other than 30 or 60.\n\nTo prevent the problems mentioned, re4_tweaks automatically" +
						" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' \"winmm.ini\" file.";

					MessageBoxA(NULL, Msg.c_str(), "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);

					// Use new FPS value
					*(int32_t*)(ptrGameFrameRate) = intNewFPS;
				} else {
					// Use .ini FPS value.
					*(int32_t*)(ptrGameFrameRate) = intIniFPS;
				}
			}
		}; injector::MakeInline<ReadFPS>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Replace MemorySwap with memcpy
	if (bUseMemcpy)
	{
		auto pattern = hook::pattern("8b 49 14 33 c0 a3 ? ? ? ? 85 c9 74 ? 8b ff f6 81 ? ? ? ? 20 8b 49 ? 74 ? b8 01 00 00 00 a3 ? ? ? ? 85 c9 75 ? c3");
		injector::MakeJMP(pattern.get_first(0), memcpy, true);
	}

	// Fix QTE mashing speed issues
	if (bFixQTE)
	{
		// Each one of these uses a different way to calculate how fast you're pressing the QTE key.

		// Running from boulders
		auto pattern = hook::pattern("D9 87 ? ? ? ? D8 87 ? ? ? ? D9 9F ? ? ? ? D9 EE D9 9F ? ? ? ? D9 05 ? ? ? ? D8 97 ? ? ? ? DF E0 F6 C4 ? 7A 20");
		struct BouldersQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaSpeed = *(float*)(regs.edi + 0x418);
				_asm
				{
					fld  vanillaSpeed
					fmul fQTESpeedMult
				}
			}
		}; injector::MakeInline<BouldersQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Climbing up after the minecart ride
		pattern = hook::pattern("D9 86 ? ? ? ? 8B 0D ? ? ? ? D8 61 ? D9 9E ? ? ? ? 6A ? 56 E8 ? ? ? ? D9 EE");
		struct MinecartQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaSpeed = *(float*)(regs.esi + 0x40C);
				_asm
				{
					fld  vanillaSpeed
					fdiv fQTESpeedMult
				}
			}
		}; injector::MakeInline<MinecartQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Running from Salazar's statue
		pattern = hook::pattern("8B 7D 14 D9 07 E8 ? ? ? ? 0F AF 5D 24 D9 EE 01 06 8D 43 FF D9 1F 39 06");
		struct StatueRunQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.edi = *(int32_t*)(regs.ebp + 0x14);
				float vanillaSpeed = *(float*)(regs.edi);
				_asm
				{
					fld  vanillaSpeed
					fmul fQTESpeedMult
				}
			}
		}; injector::MakeInline<StatueRunQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// Climbing up after running from Salazar's statue
		pattern = hook::pattern("FF 80 ? ? ? ? 6A 00 6A 00 6A 02 6A 02 6A 02 6A 00 6A 00 6A 05 6A 19");
		struct StatueClimbQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				*(int32_t*)(regs.eax + 0x168) += 3;
			}
		};
		injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(0).get<uint32_t>(0), pattern.count(2).get(0).get<uint32_t>(6));
		injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));
	}

	if (bSkipIntroLogos)
	{
		pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);
	}

	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (bFixUltraWideAspectRatio)
		Init_UltraWideFix();

	// Fix camera after zooming with the sniper
	if (bFixSniperZoom)
	{
		auto pattern = hook::pattern("6A 7F 6A 81 6A 7F E8 ? ? ? ? 83 C4 0C A2");
		injector::MakeNOP(pattern.get_first(14), 5, true);
	}
	
	// Restore missing transparency in the item pickup screen
	if (bRestorePickupTransparency)
	{
		// Remove call to GXCopyTex inside ItemExamine::gxDraw
		auto pattern = hook::pattern("56 6A 00 6A 00 50 8B F1 E8");
		injector::MakeNOP(pattern.get_first(8), 5);
	}
	
	// Disable Filter03 for now, as we have yet to find a way to actually fix it
	if (bDisableBrokenFilter03)
	{
		auto pattern = hook::pattern("E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 39 1D");
		injector::MakeNOP(pattern.get_first(0), 5);
	}

	// Fix a problem related to a vertex buffer that caused the image to be slightly blurred
	if (bFixBlurryImage) 
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 6A ? 6A ? 52");
		uint32_t* ptrNewVertex = *pattern.count(1).get(0).get<uint32_t*>(7);

		pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 56 57 6A ? 6A");
		injector::WriteMemory(pattern.get_first(2), ptrNewVertex, true);

		pattern = hook::pattern("D9 5D ? FF D0 D9 E8 A1 ? ? ? ? 8B 08 8B 91 ? ? ? ? 6A ? 51 D9 1C ? 68");
		injector::WriteMemory(pattern.get_first(42), ptrNewVertex, true);
	}

	// Disable film grain
	if (bDisableFilmGrain)
	{
		auto pattern = hook::pattern("75 ? 0F B6 56 ? 52 68 ? ? ? ? 50 50 E8 ? ? ? ? 83 C4 ? 5E 8B 4D ? 33 CD E8 ? ? ? ? 8B E5 5D C3 53");
		injector::MakeNOP(pattern.get_first(0), 2, true);
	}
  
	if (bEnableDebugMenu)
	{
		Init_ToolMenu();
		if (bisDebugBuild)
			Init_ToolMenuDebug();
	}

	// QTE bindings and icons
	// KEY_1 binding hook
	intQTE_key_1 = getMapKey(QTE_key_1, "dik");
	pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
	struct QTEkey1
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebx = intQTE_key_1;
			regs.eax = *(int32_t*)(regs.eax + 0x1C);
		}
	}; injector::MakeInline<QTEkey1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// KEY_2 binding hook
	intQTE_key_2 = getMapKey(QTE_key_2, "dik");
	pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
	struct QTEkey2
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.edx = intQTE_key_2;
			regs.eax = *(int32_t*)(regs.eax + 0x1C);
		}
	}; injector::MakeInline<QTEkey2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// KEY_1 icon prompts
	pattern = hook::pattern("8D ? ? ? ? ? 6A 2D ? E8");
	injector::WriteMemory<uint8_t>(pattern.count(3).get(0).get<uint32_t>(7), intQTE_key_1, true);
	injector::WriteMemory<uint8_t>(pattern.count(3).get(1).get<uint32_t>(7), intQTE_key_1, true);
	injector::WriteMemory<uint8_t>(pattern.count(3).get(2).get<uint32_t>(7), intQTE_key_1, true);

	// KEY_2 icon prompts
	pattern = hook::pattern("8D ? ? ? ? ? 6A 2E ? E8");
	injector::WriteMemory<uint8_t>(pattern.count(4).get(0).get<uint32_t>(7), intQTE_key_2, true);
	injector::WriteMemory<uint8_t>(pattern.count(4).get(1).get<uint32_t>(7), intQTE_key_2, true);
	injector::WriteMemory<uint8_t>(pattern.count(4).get(2).get<uint32_t>(7), intQTE_key_2, true);

	// Inventory item flip binding
	pattern = hook::pattern("A1 ? ? ? ? 75 ? A8 ? 74 ? 6A ? 8B CE E8 ? ? ? ? BB");
	ptrInvMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct InvFlip
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int32_t*)ptrInvMovAddr;

			if (bShouldFlipX)
			{
				regs.eax = 0x00300000;
				bShouldFlipX = false;
			}

			if (bShouldFlipY)
			{
				regs.eax = 0x00C00000;
				bShouldFlipY = false;
			}
		}
	}; injector::MakeInline<InvFlip>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Enable MH hooks
	MH_EnableHook(MH_ALL_HOOKS);
}

void LoadRealDLL(HMODULE hModule)
{
	char configname[MAX_PATH];
	GetModuleFileNameA(hModule, configname, MAX_PATH);
	WrapperName.assign(strrchr(configname, '\\') + 1);

	// Get wrapper mode
	const char* RealWrapperMode = Wrapper::GetWrapperName((WrapperMode.size()) ? WrapperMode.c_str() : WrapperName.c_str());

	proxy_dll = Wrapper::CreateWrapper((RealDllPath.size()) ? RealDllPath.c_str() : nullptr, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr, WrapperName.c_str());
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

		#ifdef VERBOSE
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
		#endif
		LoadRealDLL(hModule);

		Init_Main();

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}