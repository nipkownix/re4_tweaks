#include <iostream>
#include "dllmain.h"
#include "Patches.h"

uintptr_t* ptrSFDMovAddr;

uintptr_t ptrcSofdec__startApp;
uintptr_t ptrmwPlyCalcWorkCprmSfd;
uintptr_t ptrcSofdec__finishMovie;
static uint32_t* ptrMemPoolMovie;

float newMovPosX;
float newMovNegX;
float newMovPosY;
float newMovNegY;

// SFD functions
int (__cdecl* mwPlyCalcWorkCprmSfd_Orig)(/* MwsfdCrePrm * */ void* cprm);
int __cdecl mwPlyCalcWorkCprmSfd_Hook(/* MwsfdCrePrm * */ void* cprm)
{
	int workarea_size = mwPlyCalcWorkCprmSfd_Orig(cprm);

	// alloc a workarea based on what mwPlyCalcWorkCprmSfd_Orig returned - allows high-res videos to play without crashing :)
	void* workarea = malloc(workarea_size);

	void** g_MemPoolMovie = (void**)(ptrMemPoolMovie);
	*g_MemPoolMovie = workarea;

	return workarea_size;
}

void (__fastcall* cSofdec__startApp_Orig)(void* thisptr);
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

void (__fastcall* cSofdec__finishMovie_Orig)(uint8_t* thisptr);
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
	auto pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 81 88 ? ? ? ? ? ? ? ? 8B CE E8 ? ? ? ? 53");
	ptrcSofdec__startApp = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int();

	pattern = hook::pattern("E8 ? ? ? ? 50 89 46 1C 8B 15 ? ? ? ? 6A");
	ptrmwPlyCalcWorkCprmSfd = (uintptr_t)pattern.count(1).get(0).get<uintptr_t>(0);

	pattern = hook::pattern("E8 ? ? ? ? EB 08 7E 06 40 A3 ? ? ? ? 83 3D ? ? ? ? ? 7E 31");
	ptrcSofdec__finishMovie = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int();

	pattern = hook::pattern("8B 15 ? ? ? ? 81 C2 ? ? ? ? 6A ? 51 89 56 ? E8 ? ? ? ? 8B 4E");
	ptrMemPoolMovie = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(2), true);
}

void Init_sofdec()
{
	GetSofdecPointers();

	// Create mem hooks
	ReadCall(ptrcSofdec__startApp, cSofdec__startApp_Orig);
	InjectHook(ptrcSofdec__startApp, cSofdec__startApp_Hook);

	ReadCall(ptrmwPlyCalcWorkCprmSfd, mwPlyCalcWorkCprmSfd_Orig);
	InjectHook(ptrmwPlyCalcWorkCprmSfd, mwPlyCalcWorkCprmSfd_Hook);

	ReadCall(ptrcSofdec__finishMovie, cSofdec__finishMovie_Orig);
	InjectHook(ptrcSofdec__finishMovie, cSofdec__finishMovie_Hook);

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
			newMovPosX = MovResX / 2.0f;
			newMovNegX = -newMovPosX;

			newMovPosY = MovResY / 1.54f;
			newMovNegY = -newMovPosY;

			#ifdef VERBOSE
			con.AddConcatLog("newMovPosX = ", static_cast<int>(newMovPosX));
			con.AddConcatLog("newMovNegX = ", static_cast<int>(newMovNegX));

			con.AddConcatLog("newMovPosY = ", static_cast<int>(newMovPosY));
			con.AddConcatLog("newMovNegY = ", static_cast<int>(newMovNegY));
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

	spd::log()->info("{} -> AllowHighResolutionSFD enabled", __FUNCTION__);
}