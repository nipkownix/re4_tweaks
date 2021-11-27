#include <iostream>
#include "..\includes\stdafx.h"
#include "..\external\MinHook\MinHook.h"
#include "dllmain.h"
#include "Settings.h"

uint32_t* ptrpG;

static uint32_t* ptrpzzl_size;
static uint32_t* ptrstageInit;
static uint32_t* ptrSubScreenAramRead;
static uint32_t* ptrp_MemPool_SubScreen1;
static uint32_t* ptrp_MemPool_SubScreen2;
static uint32_t* ptrp_MemPool_SubScreen3;
static uint32_t* ptrp_MemPool_SubScreen4;
static uint32_t* ptrp_MemPool_SubScreen5;
static uint32_t* ptrp_MemPool_SubScreen6;
static uint32_t* ptrp_MemPool_SubScreen7;
static uint32_t* ptrp_MemPool_SubScreen8;
static uint32_t* ptrp_MemPool_SubScreen9;
static uint32_t* ptrp_MemPool_SubScreen10;
static uint32_t* ptrp_MemPool_SubScreen11;
static uint32_t* ptrp_MemPool_SubScreen12;
static uint32_t* ptrp_MemPool_SubScreen13;
static uint32_t* ptrp_MemPool_SubScreen14;
static uint32_t* ptrp_MemPool_SubScreen15;
static uint32_t* ptrp_MemPool_SubScreen16;
static uint32_t* ptrp_MemPool_SubScreen17;
static uint32_t* ptrg_MemPool_SubScreen1;
static uint32_t* ptrg_MemPool_SubScreen2;
static uint32_t* ptrg_MemPool_SubScreen3;
static uint32_t* ptrg_MemPool_SubScreen4;
static uint32_t* ptrg_MemPool_SubScreen5;
static uint32_t* ptrg_MemPool_SubScreen6;

void GetLimitPointers()
{
	// pG (globals?) pointer
	auto pattern = hook::pattern("A1 ? ? ? ? B9 FF FF FF 7F 21 48 ? A1");
	ptrpG = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Inventory screen mem
	pattern = hook::pattern("05 ? ? ? ? F6 C1 ? 8B 0D ? ? ? ? A3 ? ? ? ? 8D 91 ? ? ? ? 75 ? 8D 91 ? ? ? ? 52 03 C1");
	ptrpzzl_size = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? A1 ? ? ? ? 8A 80 ? ? ? ? 56 8B F1 84 C0 0F 84 ? ? ? ? 0F B6 C8 51");
	ptrstageInit = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("55 8B EC 83 EC ? A1 ? ? ? ? 56 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 05");
	ptrSubScreenAramRead = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 0D ? ? ? ? 03 F8 81 C1 ? ? ? ? 3B F9 73 ? 89 46 ? EB ? 81 FF ? ? ? ? 73 ? 8B 15");
	ptrp_MemPool_SubScreen1 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("8B 15 ? ? ? ? 81 C2 ? ? ? ? 89 56 ? 83 0E ? 8B 06 A8 ? 74 ? 8B 4E ? 8B 56 ? 8B 45");
	ptrp_MemPool_SubScreen2 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("A1 ? ? ? ? 05 ? ? ? ? 39 46 ? 72 ? 6A ? E8 ? ? ? ? 83 C4 ? 8B 4E ? 51");
	ptrp_MemPool_SubScreen3 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("08 A1 ? ? ? ? 56 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 05 ? ? ? ? 50 6A ? 68");
	ptrp_MemPool_SubScreen4 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("8B 0D ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 8D 94 ? ? ? ? ? 52 6A");
	if (game_version == "1.0.6") // only in 1.0.6 non-debug build
		ptrp_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(2);
	else
		ptrp_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(-4);

	pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 8D 8C");
	if (game_version == "1.0.6") // only in 1.0.6 non-debug build
		ptrp_MemPool_SubScreen6 = pattern.count(1).get(0).get<uint32_t>(2);
	else
		ptrp_MemPool_SubScreen6 = pattern.count(1).get(0).get<uint32_t>(7);

	pattern = hook::pattern("8B 0D ? ? ? ? A1 ? ? ? ? 8B 40 ? 68 ? ? ? ? 81 C1 ? ? ? ? 51 50 A3");
	ptrp_MemPool_SubScreen7 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("15 ? ? ? ? 8B 86 ? ? ? ? 68 ? ? ? ? 81 C2 ? ? ? ? 52 50 E8 ? ? ? ? 6A");
	ptrp_MemPool_SubScreen8 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("8B 0D ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 53 53 8D 94 08 ? ? ? ? 52 53");
	ptrp_MemPool_SubScreen9 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("05 ? ? ? ? 39 46 ? 72 ? 6A ? E8 ? ? ? ? 83 C4 ? 8B 4E ? 51");
	ptrp_MemPool_SubScreen11 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("81 C1 ? ? ? ? 51 50 A3 ? ? ? ? 88 1D ? ? ? ? E8 ? ? ? ? 0F BE 05");
	ptrp_MemPool_SubScreen12 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("05 ? ? ? ? 50 6A ? 68 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? E8");
	ptrp_MemPool_SubScreen13 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("6A 09 6A 00 6A 00 ? ? ? ? ? ? ? ? 6A 00 68");
	ptrp_MemPool_SubScreen14 = pattern.count(2).get(0).get<uint32_t>(9);
	ptrp_MemPool_SubScreen15 = pattern.count(2).get(1).get<uint32_t>(9);

	pattern = hook::pattern("81 C2 ? ? ? ? 52 50 E8 ? ? ? ? 6A ? E8 ? ? ? ? 6A ? E8");
	ptrp_MemPool_SubScreen16 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("8D 94 08 ? ? ? ? 52 53 68 ? ? ? ? E8 ? ? ? ? 83");
	ptrp_MemPool_SubScreen17 = pattern.count(1).get(0).get<uint32_t>(3);

	pattern = hook::pattern("68 ? ? ? ? 81 C1 ? ? ? ? 51 50 A3 ? ? ? ? 88 1D ? ? ? ? E8");
	ptrg_MemPool_SubScreen1 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("68 ? ? ? ? 81 C2 ? ? ? ? 52 50 E8 ? ? ? ? 6A ? E8 ? ? ? ? 6A ? E8 ");
	ptrg_MemPool_SubScreen2 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("8D 91 ? ? ? ? 75 ? 8D 91 ? ? ? ? 52 03 C1 50 6A ? E8 ? ? ? ? 83");
	ptrg_MemPool_SubScreen3 = pattern.count(1).get(0).get<uint32_t>(2);
	ptrg_MemPool_SubScreen4 = pattern.count(1).get(0).get<uint32_t>(10);

	pattern = hook::pattern("81 C2 ? ? ? ? 89 15 ? ? ? ? 80 B8 ? ? ? ? ? 74 ? 0F B7 0D ? ? ? ? 51");
	ptrg_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("81 FF ? ? ? ? 73 ? 8B 15 ? ? ? ? 81 C2 ? ? ? ? 89 56 ? 83 0E");
	ptrg_MemPool_SubScreen6 = pattern.count(1).get(0).get<uint32_t>(2);
	ptrp_MemPool_SubScreen10 = pattern.count(1).get(0).get<uint32_t>(16);
}

// Inventory screen mem functions
uint8_t g_MemPool_SubScreen[0x4000000];
uint8_t g_MemPool_SubScreen2[0x4000000];

uint8_t* p_MemPool_SubScreen = (uint8_t*)&g_MemPool_SubScreen;

typedef bool(__fastcall* MessageControl__stageInit_Fn)(void* thisptr);
MessageControl__stageInit_Fn MessageControl__stageInit_Orig;

bool __fastcall MessageControl__stageInit_Hook(void* thisptr)
{
	bool ret = MessageControl__stageInit_Orig(thisptr);

	// Set pG+0x3C to point to an extended buffer we control
	// The game uses this for some MemorySwap operation later on - not really sure why that swap is needed though, but oh well
	uint8_t* pG = *(uint8_t**)(ptrpG);

	*(void**)(pG + 0x3C) = &g_MemPool_SubScreen2;

	return ret;
}

typedef int(*SubScreenAramRead_Fn)();
SubScreenAramRead_Fn SubScreenAramRead_Orig;
int SubScreenAramRead_Hook()
{
	int pzzl_size = SubScreenAramRead_Orig();

	// Patch SubScreenExec heap size, seems to be max size to read from ss_pzzl/omk_pzzl file
	injector::WriteMemory<int>(ptrpzzl_size, pzzl_size, true);

	return pzzl_size;
}

void Init_HandleLimits()
{
	GetLimitPointers();

	// vertex buffers
	if (cfg.bRaiseVertexAlloc)
	{
		auto pattern = hook::pattern("68 80 1A 06 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? 46");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68000C3500, true);  // 400000 -> 800000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68000C3500, true);

		pattern = hook::pattern("68 00 09 3D 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? A1 ? ? ? ? 8B 08");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68007A1200, true);  // 4000000 -> 8000000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68007A1200, true);

		pattern = hook::pattern("68 80 84 1E 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? A1");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68003D0900, true);  // 2000000 -> 4000000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68003D0900, true);

		pattern = hook::pattern("68 80 8D 5B 00 50 8B 41 ? FF D0 85 C0 0F 85");
		injector::WriteMemory<int>(pattern.count(4).get(0).get<uint32_t>(1), 0x6800B71B00, true);  // 6000000 -> 12000000
		injector::WriteMemory<int>(pattern.count(4).get(1).get<uint32_t>(1), 0x6800B71B00, true);
		injector::WriteMemory<int>(pattern.count(4).get(2).get<uint32_t>(1), 0x6800B71B00, true);
		injector::WriteMemory<int>(pattern.count(4).get(3).get<uint32_t>(1), 0x6800B71B00, true);
	}

	// Inventory screen mem
	if (cfg.bRaiseInventoryAlloc)
	{
		MH_CreateHook(ptrstageInit, MessageControl__stageInit_Hook, (LPVOID*)&MessageControl__stageInit_Orig);
		MH_CreateHook(ptrSubScreenAramRead, SubScreenAramRead_Hook, (LPVOID*)&SubScreenAramRead_Orig);

		p_MemPool_SubScreen = (uint8_t*)&g_MemPool_SubScreen;

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen1, (uintptr_t)&p_MemPool_SubScreen, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen2, (uintptr_t)&p_MemPool_SubScreen, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen3, (uintptr_t)&p_MemPool_SubScreen, true);

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen4, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenAramRead, reads rel/Sscrn.rel
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen5, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenAramRead, reads  ss_cmmn.dat read
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen6, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenAramRead, reads  omk_pzzl.dat / ss_pzzl.dat

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen7, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenExec MemorySwap call
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen8, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenExitCore MemorySwap call
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen9, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenExit

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen10, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen11, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen12, 0x000000, true);

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen13, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen14, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen15, 0x000000, true);

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen16, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen17, 0x000000, true);

		injector::WriteMemory<int>(ptrg_MemPool_SubScreen1, (uint32_t)sizeof(g_MemPool_SubScreen), true); // SubScreenExec MemorySwap size
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen2, (uint32_t)sizeof(g_MemPool_SubScreen), true); // SubScreenExitCore MemorySwap size

		injector::WriteMemory<int>(ptrg_MemPool_SubScreen3, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x1000000), true); // SubScreenExec, some heap size
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen4, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true); // SubScreenExec, some heap size
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen5, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true);
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen6, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true);
	}
}