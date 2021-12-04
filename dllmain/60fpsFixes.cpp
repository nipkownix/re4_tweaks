#include <iostream>
#include "..\includes\stdafx.h"
#include "..\external\MinHook\MinHook.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"

static uint32_t* ptrGameFrameRate;

extern uint32_t* ptrpG; // HandleLimits.cpp

int* g_ConfigVariableFramerate;
LONGLONG* g_PrevFrameTime;
LARGE_INTEGER* g_PerformanceFrequency;

void __cdecl WinMain_DeltaTime_Hook()
{
	uint8_t* pG = *(uint8_t**)(ptrpG);

	//void* EvtMgr = (void*)0xC03C48;
	//uint8_t byte_C03C64 = *(uint8_t*)0xC03C64;

	DWORD threadPrevAffinity = SetThreadAffinityMask(GetCurrentThread(), 0);

	LARGE_INTEGER perfCount;
	QueryPerformanceCounter(&perfCount);

	int variableFramerate = *g_ConfigVariableFramerate;

	double configFramerateTime = 0;
	if (variableFramerate)
		configFramerateTime = double(1.0) / double(variableFramerate);

	// Limit game FPS to variableFramerate
	double deltaTime;
	do
	{
		QueryPerformanceCounter(&perfCount);
		deltaTime = (double(perfCount.QuadPart) - double(*g_PrevFrameTime)) / double(g_PerformanceFrequency->QuadPart);
		if (configFramerateTime > deltaTime)
		{
			Sleep(0);
		}
	} while (configFramerateTime > deltaTime);

	// RE4 has a bug with the new-game option, when deltaTime is set too high (which happens when files are loading)
	// seems it aborts the load for some reason, so here we'll check if deltaTime is above the lowest working FPS (seems to be variableFramerate / 2?)
	// and if so then tell game it was variableFramerate / 2 instead
	// (this means framerate dips below that will cause slowdown though, but those should be rare... TODO: find out what's causing the load to abort & adjust that instead)
	// The cause of this seems to be related to R120Event, which uses cSofdec::Initialize - likely something movie related is breaking
	// TODO: think this should be using displayFramerate / 2 instead of variableFramerate / 2, as in the actual display framerate, not sure how we could get that though

	double maxFrameTime = double(1) / double((variableFramerate / 2) + 1); // seems to cancel load if it was at variableFramerate/2, but adding 1 lets it work

	if (variableFramerate != 30 && deltaTime > maxFrameTime
		// TODO: seems this is trying to check for a problematic event, maybe trying to check for the new-game load screen?
		// doesn't seem to work for new-game bug though sadly, so commented out for now so this can be applied anywhere
		// (would be great to find a way to limit this so it's event-only, so that going under (variableFramerate / 2) would also have slowdown fixed)
		// && EventMgr__IsAliveEvt(EvtMgr, 0, byte_C03C64, 0, 0)
		)
	{
		deltaTime = maxFrameTime;
	}
	// We want to limit update-FPS to variableFramerate, so change deltaTime to configFramerateTime if deltaTime is lower
	else if (deltaTime < configFramerateTime)
	{
		deltaTime = configFramerateTime;
	}

	*(float*)(pG + 0x70) = float(deltaTime * 30);

	SetThreadAffinityMask(GetCurrentThread(), threadPrevAffinity);

	*g_PrevFrameTime = perfCount.QuadPart;
}

struct INIConfig
{
	int resolutionWidth;
	int resolutionHeight;
	int resolutionRefreshRate;
	int vsync;
	int fullscreen;
	int adapter;
	int shadowQuality;
	int motionblurEnabled;
	int ppEnabled;
	int useHDTexture;
	int antialiasing;
	int anisotropy;
	int variableframerate;
	int subtitle;
	int laserR;
	int laserG;
	int laserB;
	int laserA;
};

typedef void(__cdecl* ConfigReadINI_Fn)(INIConfig*);
ConfigReadINI_Fn ConfigReadINI_Orig;

int g_UserRefreshRate = 0;

void ConfigReadINI_Hook(INIConfig* a1)
{
	ConfigReadINI_Orig(a1);

	// Game is hardcoded to only allow 60Hz display modes for some reason..
	// Save the refresh rate from the INI so we can check against it later
	g_UserRefreshRate = a1->resolutionRefreshRate;
}

void FPSSlowdownFix_GetPointers()
{
	auto pattern = hook::pattern("A3 ? ? ? ? 8B 46 ? D9 1D ? ? ? ? DB 46 3C 89 0D ? ? ? ?");
	g_ConfigVariableFramerate = (int*)*pattern.count(1).get(0).get<uint32_t>(0x13);

	pattern = hook::pattern("8B 8D ? ? ? ? 8B 95 ? ? ? ? 89 0D ? ? ? ? 89 15 ? ? ? ? E8 ? ? ? ? 8D 85");
	g_PrevFrameTime = (LONGLONG*)*pattern.count(1).get(0).get<uint32_t>(0xE);

	pattern = hook::pattern("FF 15 ? ? ? ? 50 FF 15 ? ? ? ? 68 ? ? ? ? 8B F0 FF D7 68 ? ? ? ? FF D3");
	g_PerformanceFrequency = (LARGE_INTEGER*)*pattern.count(1).get(0).get<uint32_t>(0xE);
}

void Init_60fpsFixes()
{
	auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
	ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);

	// Treasure items fall speed
	pattern = hook::pattern("DC 25 ? ? ? ? 6A ? 83 EC ? 8D BE ? ? ? ? D9 9E ? ? ? ? B9");
	struct TreasureSpeed
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaMulti = 10.0;
			int curFPS = *(int32_t*)(ptrGameFrameRate);
			float newMulti = static_cast<float>(30) / curFPS * 10;

			if (cfg.b60fpsFixes)
				_asm{fsub newMulti}
			else
				_asm {fsub vanillaMulti}
		}
	}; injector::MakeInline<TreasureSpeed>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Ammo boxes fall speed
	pattern = hook::pattern("DC 0D ? ? ? ? D8 83 ? ? ? ? D9 9B ? ? ? ? D9 83");
	struct AmmoBoxSpeed
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaMulti = 10.0;
			int curFPS = *(int32_t*)(ptrGameFrameRate);
			float newMulti = static_cast<float>(30) / curFPS * 10;

			if (cfg.b60fpsFixes)
				_asm {fmul newMulti}
			else
				_asm {fmul vanillaMulti}
		}
	}; injector::MakeInline<AmmoBoxSpeed>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));

	if (cfg.bFixDisplayMode)
	{
		// Hook ConfigReadINI so we can store the users refresh-rate choice
		pattern = hook::pattern("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? 53 56 8B ? ? 68 FF 03 00 00 8D 85 ? ? ? ?");
		MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), ConfigReadINI_Hook, (LPVOID*)&ConfigReadINI_Orig);

		// Hook func that runs EnumAdapterModes & checks for 60Hz modes, make it use the refresh rate from INI instead
		pattern = hook::pattern("8B 45 ? 83 F8 3C 75");
		struct DisplayModeFix
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.eax = g_UserRefreshRate;
				regs.ef |= (1 << injector::reg_pack::ef_flag::zero_flag);
			}
		}; injector::MakeInline<DisplayModeFix>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Fix for slowdown when FPS drops, and allow using framerates above 60
	// (reimplemented part of WinMain)
	FPSSlowdownFix_GetPointers();

	if (cfg.bFixFPSSlowdown)
	{
		pattern = hook::pattern("89 15 ? ? ? ? A3 ? ? ? ? 6A 00 FF 15 ? ? ? ? 50");
		uint8_t* WinMain_DeltaTime_start = pattern.count(1).get(0).get<uint8_t>(0xB);
		injector::MakeCALL(WinMain_DeltaTime_start, WinMain_DeltaTime_Hook, true);

		// fix ESP, not sure why this is needed...
		uint8_t dec_esp_4[] = { 0x83, 0xC4, 0xFC };

		injector::WriteMemoryRaw(WinMain_DeltaTime_start + 5, dec_esp_4, 3, true);

		// JMP over stuff that we reimplemented...
		pattern = hook::pattern("E8 ? ? ? ? 8D 85 ? ? ? ? 50 FF D3 DF AD ? ? ? ? 8D 8D ? ? ? ? 51 DC 35 ? ? ? ? DC 25 ? ? ? ? DD 1D ? ? ? ?");
		uint8_t* WinMain_DeltaTime_end = pattern.count(1).get(0).get<uint8_t>(0);
		injector::MakeJMP(WinMain_DeltaTime_start + 5 + 3, WinMain_DeltaTime_end, true);

		// MotionSequenceCtrl fix: stop it from subtracting deltaTime from the anim frame number
		// (with bFixSlowdown enabled deltatime varies a lot every frame, breaking their workaround of subtracting deltaTime if anim overshot frame count)
		pattern = hook::pattern("8B 15 ? ? ? ? D8 62 70 B8 04 00 00");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(6), 3, true);
	}
}
