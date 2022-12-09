#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "input.hpp"

uintptr_t* ptrSFDMovAddr;

uintptr_t ptrcSofdec__startApp;
uintptr_t ptrmwPlyCalcWorkCprmSfd;
uintptr_t ptrcSofdec__finishMovie;
static uint32_t* ptrMemPoolMovie;

float newMovPosX;
float newMovNegX;
float newMovPosY;
float newMovNegY;

uint8_t* fadeflg;

std::vector<std::string> demoSfdVec;

int vidIndex = 0;

void(__cdecl* FadeSet)(int no, uint32_t time, uint32_t ctrl_flag);

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

bool(__fastcall* cSofdec__Initialize_orig)(void* thisptr, void* unused, char* fname, uint32_t flag);
bool __fastcall cSofdec__Initialize_hook(void* thisptr, void* unused, char* fname, uint32_t flag)
{
	static bool init = false;

	// We do this here instead of in re4t::init::Sofdec() so we can access SystemSavePtr()->language_8...
	if (!init)
	{
		const std::string bio4Path = std::filesystem::canonical(rootPath).parent_path().string() + "\\BIO4\\";

		const std::string demo0_gc_path = "movie/demo0_gc.sfd"; // GC
		const std::string demo1_gc_path = "movie/demo1_gc.sfd"; // GC
		const std::string demo2_path = "movie/demo0eng.sfd"; // Vanilla PC file, present on the Wii version as "demo2.sfd"
		const std::string demo2jp_path = "movie/demo0.sfd"; // demo2.sfd, but with Japanese subtitles

		const bool isJapanese = !SystemSavePtr()->language_8;

		if (std::filesystem::exists(bio4Path + demo0_gc_path))
			demoSfdVec.push_back(demo0_gc_path);

		if (std::filesystem::exists(bio4Path + demo1_gc_path))
			demoSfdVec.push_back(demo1_gc_path);

		if (std::filesystem::exists(bio4Path + demo2_path) && !isJapanese)
			demoSfdVec.push_back(demo2_path);

		if (std::filesystem::exists(bio4Path + demo2jp_path) && isJapanese)
			demoSfdVec.push_back(demo2jp_path);

		// Log
		spd::log()->info("RestoreDemoVideos -> Demo video list:");
		spd::log()->info("+-------------+--------------+");

		for (std::string video : demoSfdVec)
			spd::log()->info("| ../BIO4/{:<18} |", video);

		spd::log()->info("+-------------+--------------+");

		// Start at a random vid
		vidIndex = GetRandomInt(0, demoSfdVec.size() - 1);

		init = true;
	}

	bool ret = cSofdec__Initialize_orig(thisptr, unused, demoSfdVec[vidIndex].data(), flag);

	// Update index for the next time
	if (vidIndex < (int)(demoSfdVec.size() - 1))
		vidIndex++;
	else
		vidIndex = 0;

	return ret;
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

void re4t::init::Sofdec()
{
	GetSofdecPointers();

	if (re4t::cfg->bRestoreDemoVideos)
	{
		// Hook both cSofdec::Initialize calls in routine 7 inside titleMain so we can play GC demo videos, if they exist in BIO4/movies
		auto pattern = hook::pattern("E8 ? ? ? ? FE 46 ? E9 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? FE");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cSofdec__Initialize_orig);
		InjectHook(pattern.count(1).get(0).get<uint32_t>(0), cSofdec__Initialize_hook, PATCH_CALL);
		InjectHook(pattern.count(1).get(0).get<uint32_t>(18), cSofdec__Initialize_hook, PATCH_CALL);

		// Nop call to FadeKill that is messing with our custom fade
		pattern = hook::pattern("E8 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 83 C4 ? 53");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 5, true);

		// Get the address of the instruction that sets the next routine to be executed after demo videos are done playing
		static auto pattern_nextRno1_1 = hook::pattern("C6 46 ? ? E9 ? ? ? ? 53 E8 ? ? ? ? 83 C4 ? 3C ? 0F 85").count(1).get(0).get<uint32_t>(3);

		// Hook titleStart to have a new demo video timer running in the "press any key" screen.
		// This is not how it worked on the GC, since there wasn't a "press any key" screen there. Figured we should add it here regardless.
		pattern = hook::pattern("D9 05 ? ? ? ? 51 D9 1C ? 68 ? ? ? ? 8B C8");
		struct titleStart_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				float tmp = 0.2f;
				__asm {fld tmp}

				static bool triggered = false;

				if (!triggered)
				{
					// Set up new timer
					float defaultTime = 600.0f; // 20 seconds

					static float timer = defaultTime;

					timer -= GlobalPtr()->deltaTime_70;

					// Reset timer if any key is pressed, or if the mouse is being moved
					if (Key_btn_trg() || pInput->raw_mouse_delta_x() || pInput->raw_mouse_delta_y())
						timer = defaultTime;

					if (timer <= 0.0f)
					{
						timer = defaultTime; // Reset timer

						// Custom fade. Seems it was present on GC, but it isn't on the PC version.
						FadeWorkPtr(FADE_NO_SYSTEM)->FadeSet(FADE_NO_SYSTEM, 30, 0);

						triggered = true;
					}
				}
				else
				{
					// Wait for the fade to finish before starting the video
					if ((FadeWorkPtr(FADE_NO_SYSTEM)->be_flg_18 & FADE_BE_ALIVE) == 0) // Finished fade
					{
						triggered = false;

						// The end of routine 7 puts the menu into routine 1, which is the main menu (after the "press any key" screen).
						// We change it to 17 here, to go back to the "press any key" screen instead.
						injector::WriteMemory(pattern_nextRno1_1, uint8_t(17), true); // Rno1_1 = 17

						// Need to call KeyStop here to set a flag that will allow the demo videos to be skipped using the keyboard.
						bio4::KeyStop((uint64_t(0xF12000EFCF1000)));

						TitleWorkPtr()->SetRoutine(TITLE_WORK::Routine0::Main, 7, 1, 0);
					}
				}
			}
		}; injector::MakeInline<titleStart_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Hook titleMain to have a new demo video timer running in the main menu screen.
		pattern = hook::pattern("88 98 ? ? ? ? 8D 47 ? 83 C4 ? 83 F8");
		struct titleMain_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				*(uint8_t*)(regs.eax + 0x847D) = uint8_t(regs.ebx);

				static bool triggered = false;

				if (!triggered)
				{
					// Set up new timer
					float defaultTime = 600.0f; // 20 seconds

					static float timer = defaultTime;

					timer -= GlobalPtr()->deltaTime_70;

					// Reset timer if any key is pressed, or if the mouse is being moved
					if (Key_btn_trg() || pInput->raw_mouse_delta_x() || pInput->raw_mouse_delta_y())
						timer = defaultTime;

					if (timer <= 0.0f)
					{
						timer = defaultTime; // Reset timer

						// Custom fade. Seems it was present on GC, but it isn't on the PC version.
						FadeWorkPtr(FADE_NO_SYSTEM)->FadeSet(FADE_NO_SYSTEM, 30, 0);

						triggered = true;
					}
				}
				else 
				{
					// Wait for the fade to finish before starting the video
					if ((FadeWorkPtr(FADE_NO_SYSTEM)->be_flg_18 & FADE_BE_ALIVE) == 0) // Finished fade
					{
						triggered = false;

						// Make sure Rno1_1 is 1 in case it was modified by the previous hook
						injector::WriteMemory(pattern_nextRno1_1, uint8_t(0x1), true); // Rno1_1 = 1

						// Need to call KeyStop here to set a flag that will allow the demo videos to be skipped using the keyboard.
						bio4::KeyStop((uint64_t(0xF12000EFCF1000)));

						TitleWorkPtr()->SetRoutine(TITLE_WORK::Routine0::Main, 7, 1, 0);
					}
				}
			}
		}; injector::MakeInline<titleMain_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		spd::log()->info("{} -> RestoreDemoVideos enabled", __FUNCTION__);
	}

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

	// Write new pointers to cSofdec::setCamera
	pattern = hook::pattern("D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 1C ? 50 E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 56 E8 ? ? ? ? 8B 4D ? 33 CD");
	injector::WriteMemory(pattern.get_first(2), &newMovPosX, true);
	injector::WriteMemory(pattern.get_first(12), &newMovNegX, true);
	injector::WriteMemory(pattern.get_first(32), &newMovPosY, true);
	injector::WriteMemory(pattern.get_first(22), &newMovNegY, true);

	spd::log()->info("{} -> AllowHighResolutionSFD enabled", __FUNCTION__);
}