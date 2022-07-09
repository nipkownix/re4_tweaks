#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include <timeapi.h>

int g_UserRefreshRate;

uintptr_t ptrGXCopyTex;
uintptr_t ptrAfterItemExamineHook;
uintptr_t ptrFilter03;
uintptr_t ptrAfterEsp04TransHook;

uintptr_t* ptrNonBlurryVertex;
uintptr_t* ptrBlurryVertex;

uint32_t* ptrLaserR;
uint32_t* ptrLaserG;
uint32_t* ptrLaserB;
uint32_t* ptrLaserA;

const uint8_t rgbcycle[360] = {
  0, 0, 0, 0, 0, 1, 1, 2, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 15, 17, 18, 20, 22, 24, 26, 28, 30, 32, 35, 37, 39,
 42, 44, 47, 49, 52, 55, 58, 60, 63, 66, 69, 72, 75, 78, 81, 85, 88, 91, 94, 97, 101, 104, 107, 111, 114, 117, 121, 124, 127, 131, 134, 137,
141, 144, 147, 150, 154, 157, 160, 163, 167, 170, 173, 176, 179, 182, 185, 188, 191, 194, 197, 200, 202, 205, 208, 210, 213, 215, 217, 220, 222, 224, 226, 229,
231, 232, 234, 236, 238, 239, 241, 242, 244, 245, 246, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 255, 255, 255, 255, 255, 255, 255, 254, 254, 253, 253,
252, 251, 251, 250, 249, 248, 246, 245, 244, 242, 241, 239, 238, 236, 234, 232, 231, 229, 226, 224, 222, 220, 217, 215, 213, 210, 208, 205, 202, 200, 197, 194,
191, 188, 185, 182, 179, 176, 173, 170, 167, 163, 160, 157, 154, 150, 147, 144, 141, 137, 134, 131, 127, 124, 121, 117, 114, 111, 107, 104, 101, 97, 94, 91,
 88, 85, 81, 78, 75, 72, 69, 66, 63, 60, 58, 55, 52, 49, 47, 44, 42, 39, 37, 35, 32, 30, 28, 26, 24, 22, 20, 18, 17, 15, 13, 12,
 11, 9, 8, 7, 6, 5, 4, 3, 2, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void __declspec(naked) ItemExamineHook()
{
	if (!pConfig->bRestorePickupTransparency)
		_asm
		{
			call ptrGXCopyTex
			jmp ptrAfterItemExamineHook
		}
	else
		_asm {jmp ptrAfterItemExamineHook}
}

void __declspec(naked) Esp04TransHook()
{
	if (!pConfig->bDisableFilmGrain)
		_asm
		{
			push ebp
			mov  ebp, esp
			sub  esp, 0x68
			jmp ptrAfterEsp04TransHook
		}
	else
		_asm {ret}
}

BOOL __stdcall SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	int windowX = pConfig->iWindowPositionX < 0 ? 0 : pConfig->iWindowPositionX;
	int windowY = pConfig->iWindowPositionY < 0 ? 0 : pConfig->iWindowPositionY;
	return SetWindowPos(hWnd, hWndInsertAfter, windowX, windowY, cx, cy, uFlags);
}

BOOL __stdcall MoveWindow_Hook(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint)
{
	// Early return if bWindowBorderless is set to true, as this call changes the window size to include borders.
	if (pConfig->bWindowBorderless)
		return true;

	int windowX = pConfig->iWindowPositionX < 0 ? 0 : pConfig->iWindowPositionX;
	int windowY = pConfig->iWindowPositionY < 0 ? 0 : pConfig->iWindowPositionY;
	return MoveWindow(hWnd, windowX, windowY, nWidth, nHeight, bRepaint);
}

double FramelimiterFrequency = 0;
double FramelimiterCurTicks = 0;
double FramelimiterLoop(double targetFrametime)
{
	// Framelimiter based on FPS_ACCURATE code from
	// https://github.com/ThirteenAG/d3d9-wrapper/blob/c1480b0c1b40e0ba7b55b8660bd67f911a967421/source/dllmain.cpp#L46
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	double millis_current = (double)counter.QuadPart / FramelimiterFrequency;
	double millis_delta = millis_current - FramelimiterCurTicks;
	if (targetFrametime <= millis_delta)
	{
		FramelimiterCurTicks = millis_current;
		return millis_delta;
	}
	else if (targetFrametime - millis_delta > 2.0) // > 2ms
		Sleep(1); // Sleep for ~1ms
	else
		Sleep(0); // yield thread's time-slice (does not actually sleep)

	return 0;
}

void FramelimiterHook(uint8_t isAliveEvt_result)
{
	// Change thread to core 0 before running QueryPerformance* funcs, game does this, so guess we should too
	HANDLE curThread = GetCurrentThread();
	DWORD_PTR prevAffinityMask = SetThreadAffinityMask(curThread, 0);

	static bool framelimiterInited = false;
	if (!framelimiterInited)
	{
		LARGE_INTEGER result;
		QueryPerformanceFrequency(&result);
		FramelimiterFrequency = (double)result.QuadPart / 1000.0;

		QueryPerformanceCounter(&result);
		FramelimiterCurTicks = (double)result.QuadPart / FramelimiterFrequency;

		typedef MMRESULT(__stdcall* timeBeginPeriod_Fn) (UINT Period);
		timeBeginPeriod_Fn timeBeginPeriod_actual = (timeBeginPeriod_Fn)GetProcAddress(LoadLibraryA("winmm.dll"), "timeBeginPeriod");
		timeBeginPeriod_actual(1);

		framelimiterInited = true;
	}

	int gameFramerate = GameVariableFrameRate();

	// The games IsAliveEvt check seems to (indirectly) result in framelimiter loop limiting to 60FPS
	// maybe a remnant of some time when more framerate options were available?
	if (isAliveEvt_result && gameFramerate != 30)
		gameFramerate = 60;

	double TargetFrametime = 1000.0 / (double)gameFramerate;

	double timeElapsed = 0;
	if (pConfig->bDisableFramelimiting)
	{
		// If framelimiter is disabled, we'll have to figure out elapsed time outside of the FramelimiterLoop fn
		// (allows DisableFixedFrametime to work properly with framelimiting disabled)
		LARGE_INTEGER counter;
		QueryPerformanceCounter(&counter);
		double millis_current = (double)counter.QuadPart / FramelimiterFrequency;
		timeElapsed = millis_current - FramelimiterCurTicks;
		FramelimiterCurTicks = millis_current;
	}
	else
		while ((timeElapsed = FramelimiterLoop(TargetFrametime)) == 0);

	// Not really sure what the second part of IsAliveEvt check is doing
	// Seems to skip setting timeElapsed to the fixed FramelimiterTargetFrametime at least
	// Guess that means the true timeElapsed gets passed to the game? (after being limited to 60 like above)
	if (isAliveEvt_result && gameFramerate != 30)
	{
		if (timeElapsed > 33.333333333333333)
			timeElapsed = 33.333333333333333;
	}
	else
	{
		// TODO: using the actual time elapsed since last frame instead of FramelimiterTargetFrametime would solve slowdown issues
		// (similar to https://github.com/nipkownix/re4_tweaks/pull/25)
		// but it's not known how well the game works with values that aren't 0.5 (60fps) or 1 (30fps)
		// so for now we'll just work pretty much the same as the game itself, unless DisableFixedFrametime is set
		if (!pConfig->bUseDynamicFrametime)
			timeElapsed = TargetFrametime;
	}

	GlobalPtr()->deltaTime_70 = float((timeElapsed / 1000) * 30.0);

	SetThreadAffinityMask(curThread, prevAffinityMask);
}

void Init_DisplayTweaks()
{
	if (pConfig->bReplaceFramelimiter)
	{
		// nop beginning of framelimiter code (sets up thread affinity to core 0)
		auto pattern = hook::pattern("A3 ? ? ? ? 6A 00 FF 15 ? ? ? ? 50 FF");
		uint8_t* framelimiterStart = pattern.count(1).get(0).get<uint8_t>(5);
		pattern = hook::pattern("E8 ? ? ? ? 85 C0 75 ? D9 EE EB ?");
		uint8_t* framelimiterEnd = pattern.count(1).get(0).get<uint8_t>(0);
		Nop(framelimiterStart, framelimiterEnd - framelimiterStart); // 6549C3 to 6549D9 (1.1.0)

		pattern = hook::pattern("B9 ? ? ? ? E8 ? ? ? ? 84 C0 74 ? E8 ? ? ? ? 83 F8 1E");
		framelimiterStart = pattern.count(1).get(0).get<uint8_t>(0xA); // 654A1E
		pattern = hook::pattern("8D 85 ? ? ? ? 50 FF D3 DF AD ? ? ? ? 8D 8D");
		framelimiterEnd = pattern.count(1).get(0).get<uint8_t>(0); // 654B0A

		Nop(framelimiterStart, framelimiterEnd - framelimiterStart);

		Patch(framelimiterStart, uint8_t(0x50)); // push eax (pass return value of EventMgr::IsAliveEvt)
		InjectHook(framelimiterStart + 1, FramelimiterHook, PATCH_CALL);
		Patch(framelimiterStart + 1 + 5, { 0x83, 0xc4, 0x04 }); // add esp, 4 (needed to allow WinMain to exit properly)
	}

	// Fix broken effects
	Init_FilterXXFixes();

	// Install a D3D9 hook
	Init_D3D9Hook();

	// Aspect ratio-related tweaks
	Init_AspectRatioTweaks();

	// Hook function that loads the FOV
	{
		auto pattern = hook::pattern("D9 45 ? 89 4E ? D9 5E ? 8B 8D ? ? ? ? 89 46 ? 8B 85 ? ? ? ? 8D 7E");
		struct ScaleFOV
		{
			void operator()(injector::reg_pack& regs)
			{
				float FOV = *(float*)(regs.ebp - 0x34);

				if (pConfig->fFOVAdditional > 0.0f)
					FOV += pConfig->fFOVAdditional;

				_asm {fld FOV}

				*(int32_t*)(regs.esi + 0x4) = regs.ecx;
			}
		}; injector::MakeInline<ScaleFOV>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		if (pConfig->fFOVAdditional > 0.0f)
			spd::log()->info("FOV increased");
	}

	// Force v-sync off
	if (pConfig->bDisableVsync)
	{
		// See D3D9Hook.cpp -> hook_Direct3D9::CreateDevice and hook_Direct3D9::Reset

		auto pattern = hook::pattern("8B 56 ? 8B 85 ? ? ? ? 83 C4 ? 52 68 ? ? ? ? 50");
		struct WriteIniHook
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.edx = 0; // This is what the game will write as the v-sync value inside config.ini

				regs.eax = *(int32_t*)(regs.ebp - 0x808);
			}
		}; injector::MakeInline<WriteIniHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(9));

		spd::log()->info("Vsync disabled");
	}

	// Game is hardcoded to only allow 60Hz display modes for some reason...
	// Hook func that runs EnumAdapterModes & checks for 60Hz modes, make it use the refresh rate from Windows instead
	if (pConfig->bFixDisplayMode)
	{
		if (pConfig->iCustomRefreshRate > -1)
		{
			g_UserRefreshRate = pConfig->iCustomRefreshRate;
		}
		else
		{
			DISPLAY_DEVICE device{ 0 };
			device.cb = sizeof(DISPLAY_DEVICE);
			DEVMODE lpDevMode{ 0 };
			if (EnumDisplayDevices(NULL, 0, &device, 0) == false ||
				EnumDisplaySettings(device.DeviceName, ENUM_CURRENT_SETTINGS, &lpDevMode) == false)
			{
				#ifdef VERBOSE
				con.AddLogChar("Couldn't read the display refresh rate. Using fallback value.");
				#endif

				spd::log()->info("FixDisplayMode -> Couldn't read the display refresh rate. Using fallback value.");

				g_UserRefreshRate = 60; // Fallback value.
			}
			else
			{
				#ifdef VERBOSE
				con.AddLogChar("Device 0 name: %s", device.DeviceName);
				#endif
				g_UserRefreshRate = lpDevMode.dmDisplayFrequency;

				// Log display modes for troubleshooting
				if (pConfig->bVerboseLog)
				{
					DEVMODE dm = { 0 };
					dm.dmSize = sizeof(dm);
					spd::log()->info("+----------+----------+");
					spd::log()->info("| Display modes       |");
					spd::log()->info("+----------+----------+");
					for (int iModeNum = 0; EnumDisplaySettings(device.DeviceName, iModeNum, &dm) != 0; iModeNum++)
					{
						spd::log()->info("| {:<5}x {:<5} {:>3} Hz |", dm.dmPelsWidth, dm.dmPelsHeight, dm.dmDisplayFrequency);
					}
					spd::log()->info("+----------+----------+");
				}
			}
		}

		#ifdef VERBOSE
		con.AddConcatLog("New refresh rate = ", g_UserRefreshRate);
		#endif

		spd::log()->info("FixDisplayMode -> New refresh rate = {}", g_UserRefreshRate);

		auto pattern = hook::pattern("8B 45 ? 83 F8 ? 75 ? 8B 4D ? 8B 7D ? 3B 4D");
		struct DisplayModeFix
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.eax = g_UserRefreshRate;
				regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<DisplayModeFix>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Patch function that fills resolution list, normally it filters resolutions to only 60Hz modes, which is bad for non-60Hz players...
		// Changing it to check for desktop refresh-rate helped, but unfortunately D3D doesn't always match desktop refresh rate
		// Luckily it turns out this function has an unused code-path which filters by checking for duplicate width/height combos instead, so any refresh rate can be allowed
		// who knows why they chose not to use it...
		pattern = hook::pattern("81 FA 98 26 00 00 77 ? 38 45 ? 75 ?");
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(11), uint8_t(0xEB), true);

		// Patch above function to only allow 59Hz+ modes
		// (workaround for game potentially displaying invalid modes that some displays expose)
		struct DisplayModeFilter
		{
			void operator()(injector::reg_pack& regs)
			{
				const int GameMaxHeight = 10480 - 600; // game does strange optimized check involving 600 taken away from edx...

				// Game does JA after the CMP we patched, so we set ZF and CF both to 1 here
				// then if we want game to take the JA, we clear them both
				regs.ef |= (1 << regs.zero_flag);
				regs.ef |= (1 << regs.carry_flag);

				// Make game skip any mode under 59, since they're likely invalid
				// (old game code would only accept 60Hz modes, so I think this is fine for us to do)
				int refreshRate = *(int*)(regs.ebp - 0xC);
				if (regs.edx > GameMaxHeight || refreshRate < 59)
				{
					// Clear both flags to make game take JA and skip this mode
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}
			}
		}; injector::MakeInline<DisplayModeFilter>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Apply window changes
	if (pConfig->bWindowBorderless || pConfig->iWindowPositionX > -1 || pConfig->iWindowPositionY > -1)
	{
		// hook SetWindowPos, can't nop as it's used for resizing window on resolution changes
		auto pattern = hook::pattern("FF 15 ? ? ? ? 56 53 57 8D 45 EC 50 FF 15 ? ? ? ? 8B");
		injector::MakeNOP(pattern.get_first(0), 6);
		InjectHook(pattern.get_first(0), SetWindowPos_Hook, PATCH_CALL);

		// hook MoveWindow. Also can't nop this one as it seems to set up the correct window size on startup
		pattern = hook::pattern("FF 15 ? ? ? ? 8B 0D ? ? ? ? 6A 01");
		injector::MakeNOP(pattern.get_first(0), 6);
		InjectHook(pattern.get_first(0), MoveWindow_Hook, PATCH_CALL);

		if (pConfig->bWindowBorderless)
		{
			// if borderless, update the style set by SetWindowLongA
			pattern = hook::pattern("25 00 00 38 7F 05 00 00 C8 00");
			injector::WriteMemory(pattern.get_first(5), uint8_t(0xb8), true); // mov eax, XXXXXXXX
			injector::WriteMemory(pattern.get_first(6), uint32_t(WS_POPUP), true);
		}
	}

	// Restore missing transparency in the item pickup screen by
	// removing a call to GXCopyTex inside ItemExamine::gxDraw
	{
		auto pattern = hook::pattern("E8 ? ? ? ? D9 05 ? ? ? ? D9 7D ? 6A ? 0F B7 45");
		ptrGXCopyTex = injector::GetBranchDestination(pattern.get_first(0)).as_int();
		ptrAfterItemExamineHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(5);
		injector::MakeNOP(pattern.get_first(0), 5);
		injector::MakeJMP(pattern.get_first(0), ItemExamineHook, true);

		if (pConfig->bRestorePickupTransparency)
			spd::log()->info("RestorePickupTransparency enabled");
	}

	// Override laser sight colors
	{
		auto pattern = hook::pattern("D9 05 ? ? ? ? D9 99 ? ? ? ? 8B 55 ? D9 05 ? ? ? ? D9");
		ptrLaserR = *pattern.count(1).get(0).get<uint32_t*>(2);
		ptrLaserG = *pattern.count(1).get(0).get<uint32_t*>(17);
		ptrLaserB = *pattern.count(1).get(0).get<uint32_t*>(32);
		ptrLaserA = *pattern.count(1).get(0).get<uint32_t*>(47);

		static int rgbindex = 0;

		struct DrawLaserStruct
		{
			void operator()(injector::reg_pack& regs)
			{
				if (pConfig->bOverrideLaserColor)
				{
					if (pConfig->bRainbowLaser)
					{
						*(float*)(ptrLaserR) = rgbcycle[(rgbindex + 120) % 360];
						*(float*)(ptrLaserG) = rgbcycle[rgbindex];
						*(float*)(ptrLaserB) = rgbcycle[(rgbindex + 240) % 360];

						rgbindex++;
						if (rgbindex > 360)
							rgbindex = 0;
					}
					else
					{
						*(float*)(ptrLaserR) = (float)pConfig->iLaserR;
						*(float*)(ptrLaserG) = (float)pConfig->iLaserG;
						*(float*)(ptrLaserB) = (float)pConfig->iLaserB;

					}

					// Seems the game disables reading custom RGB colors if the alpha is set to 255?
					// The alpha value also seems to be doing nothing useful. It does affect the opacity of the laser dot, but not the line itself.
					// Forcing 220 makes it look okay-ish.
					*(int8_t*)(ptrLaserA) = (int8_t)220;
				}
				else
				{
					*(float*)(ptrLaserR) = 255.0f;
					*(float*)(ptrLaserG) = 0.0f;
					*(float*)(ptrLaserB) = 0.0f;
					*(int8_t*)(ptrLaserA) = (int8_t)255;
				}

				regs.eax = *(int8_t*)(ptrLaserA);
			}
		}; 
		
		pattern = hook::pattern("A1 ? ? ? ? D9 5D ? 83 F8 ? 75 ? 8B 15 ? ? ? ? D9 45 ? 8B 0D ? ? ? ? 83");
		injector::MakeInline<DrawLaserStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		pattern = hook::pattern("a1 ? ? ? ? 83 f8 ? 75 ? 8b 0d ? ? ? ? d9 45 ? 8b 15 ? ? ? ? 83 ec");
		injector::MakeInline<DrawLaserStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	}

	// Disable Filter03 for now, as we have yet to find a way to actually fix it
	{
		auto pattern = hook::pattern("E8 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 39 1D");
		ptrFilter03 = injector::GetBranchDestination(pattern.get_first(0)).as_int();
		struct DisableBrokenFilter03
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bDisableBrokenFilter03)
					_asm {call ptrFilter03}
			}
		}; injector::MakeInline<DisableBrokenFilter03>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		spd::log()->info("DisableBrokenFilter03 applied");
	}

	// Fix a problem related to a vertex buffer that caused the image to be slightly blurred
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 6A ? 6A ? 52");
		ptrNonBlurryVertex = *pattern.count(1).get(0).get<uint32_t*>(7); // Replacement buffer

		// Hook struct
		struct BlurryBuffer
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bFixBlurryImage)
					regs.edx = *(int32_t*)ptrBlurryVertex;
				else
					regs.edx = *(int32_t*)ptrNonBlurryVertex;
			}
		};

		// First buffer
		pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 56 57 6A ? 6A");
		ptrBlurryVertex = *pattern.count(1).get(0).get<uint32_t*>(2);
		injector::MakeInline<BlurryBuffer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Second buffer
		pattern = hook::pattern("D9 5D ? FF D0 D9 E8 A1 ? ? ? ? 8B 08 8B 91 ? ? ? ? 6A ? 51 D9 1C ? 68");
		injector::MakeInline<BlurryBuffer>(pattern.count(1).get(0).get<uint32_t>(40), pattern.count(1).get(0).get<uint32_t>(46));

		if (pConfig->bFixBlurryImage)
			spd::log()->info("FixBlurryImage enabled");
	}

	// Disable film grain (Esp04)
	{
		auto pattern = hook::pattern("55 8B EC 83 EC ? A1 ? ? ? ? 33 C5 89 45 ? 56 8B 75 ? 0F B6 4E");
		ptrAfterEsp04TransHook = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(6);
		injector::MakeNOP(pattern.get_first(0), 6);
		injector::MakeJMP(pattern.get_first(0), Esp04TransHook, true);

		if (pConfig->bDisableFilmGrain)
			spd::log()->info("DisableFilmGrain applied");
	}

	// Disable Windows DPI scaling
	if (pConfig->bFixDPIScale)
	{
		SetProcessDPIAware();
	}
}
