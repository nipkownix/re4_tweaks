#include <iostream>
#include "..\includes\stdafx.h"
#include "..\external\MinHook\MinHook.h"
#include "dllmain.h"
#include "ConsoleWnd.h"

uintptr_t* ptrSFDMovAddr;

static uint32_t* ptrcSofdec__startApp;
static uint32_t* ptrmwPlyCalcWorkCprmSfd;
static uint32_t* ptrcSofdec__finishMovie;
static uint32_t* ptrMemPoolMovie;

float newMovPosX;
float newMovNegX;
float newMovPosY;
float newMovNegY;

// SFD functions
typedef int(__cdecl* mwPlyCalcWorkCprmSfd_Fn)(/* MwsfdCrePrm * */ void* cprm);
mwPlyCalcWorkCprmSfd_Fn mwPlyCalcWorkCprmSfd_Orig;
int __cdecl mwPlyCalcWorkCprmSfd_Hook(/* MwsfdCrePrm * */ void* cprm)
{
	int workarea_size = mwPlyCalcWorkCprmSfd_Orig(cprm);

	// alloc a workarea based on what mwPlyCalcWorkCprmSfd_Orig returned - allows high-res videos to play without crashing :)
	void* workarea = malloc(workarea_size);

	void** g_MemPoolMovie = (void**)(ptrMemPoolMovie);
	*g_MemPoolMovie = workarea;

	return workarea_size;
}

typedef void(__fastcall* cSofdec__startApp_Fn)(void* thisptr);
cSofdec__startApp_Fn cSofdec__startApp_Orig;

void __fastcall cSofdec__startApp_Hook(void* thisptr)
{
	// Backup original g_MemPoolMovie value
	// we need to change this so that cSofdec::startApp uses our own memory block instead of the hardcoded (& incorrectly-sized) memory pool
	// a few other things rely on g_MemPoolMovie value though, so it needs to be changed back after this func has ran
	// unfortunately we dont know the size of g_MemPoolMovie until mwPlyCalcWorkCprmSfd is called though (during cSofdec::startApp)
	// so a mwPlyCalcWorkCprmSfd hook is used to handle allocating & updating g_MemPoolMovie, while cSofdec::startApp hook is just used to backup/restore original addr

	void** g_MemPoolMovie = (void**)(ptrMemPoolMovie);
	void* g_MemPoolMovie_Original = *g_MemPoolMovie;

	cSofdec__startApp_Orig(thisptr);

	*g_MemPoolMovie = g_MemPoolMovie_Original;
}

typedef void(__fastcall* cSofdec__finishMovie_Fn)(uint8_t* thisptr);
cSofdec__finishMovie_Fn cSofdec__finishMovie_Orig;
void __fastcall cSofdec__finishMovie_Hook(uint8_t* thisptr)
{
	cSofdec__finishMovie_Orig(thisptr);

	// cleanup the workarea we allocated
	// ps2 version seems to memset(0) the workarea here, PC version doesn't do this though
	// should mean it's safe to assume the workarea isn't being used anymore, so we can free it here
	void* workarea = *(void**)(thisptr + 0x130);
	free(workarea);
}

void GetSofdecPointers()
{
	// SFD
	auto pattern = hook::pattern("56 57 8B F9 6A ? 8D 77 ? 6A ? 56 E8 ? ? ? ? C7 46 ? ? ? ? ? C7 06");
	ptrcSofdec__startApp = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B 74 24 ? 85 F6 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 33 C0 5E C3 8B 06");
	ptrmwPlyCalcWorkCprmSfd = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B F1 57 8D 86 ? ? ? ? 50 33 FF 68 ? ? ? ? 89 3D ? ? ? ? E8 ? ? ? ? 8B 4E");
	ptrcSofdec__finishMovie = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 15 ? ? ? ? 81 C2 ? ? ? ? 6A ? 51 89 56 ? E8 ? ? ? ? 8B 4E");
	ptrMemPoolMovie = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(2), true);
}

void Init_sofdec()
{
	GetSofdecPointers();

	// Create mem hooks
	MH_CreateHook(ptrcSofdec__startApp, cSofdec__startApp_Hook, (LPVOID*)&cSofdec__startApp_Orig);
	MH_CreateHook(ptrmwPlyCalcWorkCprmSfd, mwPlyCalcWorkCprmSfd_Hook, (LPVOID*)&mwPlyCalcWorkCprmSfd_Orig);
	MH_CreateHook(ptrcSofdec__finishMovie, cSofdec__finishMovie_Hook, (LPVOID*)&cSofdec__finishMovie_Orig);

	// Get resolution and calculate new position
	auto pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? DB 45 ? 0F B7 0D");
	ptrSFDMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct SFDRes
	{
		void operator()(injector::reg_pack& regs)
		{
			int MovResX = regs.ecx;
			int MovResY = regs.eax;

			#ifdef VERBOSE
			con.AddConcatLog("MovResX = ", MovResX);
			con.AddConcatLog("MovResY = ", MovResY);
			#endif

			// Calculate new position
			newMovPosX = MovResX / 2;
			newMovNegX = -newMovPosX;

			newMovPosY = MovResY / 1.54;
			newMovNegY = -newMovPosY;

			#ifdef VERBOSE
			con.AddConcatLog("newMovPosX = ", newMovPosX);
			con.AddConcatLog("newMovNegX = ", newMovNegX);

			con.AddConcatLog("newMovPosY = ", newMovPosY);
			con.AddConcatLog("newMovNegY = ", newMovNegY);
			#endif

			* (int32_t*)(ptrSFDMovAddr) = regs.eax;
		}
	}; injector::MakeInline<SFDRes>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Write new pointers
	pattern = hook::pattern("D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 1C ? 50 E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 56 E8 ? ? ? ? 8B 4D ? 33 CD");
	injector::WriteMemory(pattern.get_first(2), &newMovPosX, true);
	injector::WriteMemory(pattern.get_first(12), &newMovNegX, true);
	injector::WriteMemory(pattern.get_first(32), &newMovPosY, true);
	injector::WriteMemory(pattern.get_first(22), &newMovNegY, true);
}