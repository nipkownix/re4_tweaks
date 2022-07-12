#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"

// Trainer.cpp: checks certain game flags & patches code in response to them
// Ideally we would hook each piece of code instead and add a flag check there
// Unfortunately, most existing trainer-like-mods for the game are done as code patches
// So to make it easier on us, this will act as a manager to tie code patches with game flags
// (maybe over time these patches can be converted to work as hooks instead)

// Holds data about a patch, which flag it should be affected by, address to patch, undo data...
struct FlagPatch
{
	uint32_t* flagValuesPtr;
	uint32_t flagIndex;
	uint8_t* patchAddress;
	std::vector<uint8_t> patchData;

	bool prevFlagValue = false;
	std::vector<uint8_t> origData;

	FlagPatch(uint32_t* FlagValuesPtr, uint32_t FlagIndex) : flagValuesPtr(FlagValuesPtr), flagIndex(FlagIndex) {}

	void SetPatch(uint8_t* address, std::initializer_list<uint8_t> data)
	{
		patchAddress = address;
		patchData = data;

		if (!patchData.size())
			return;
		origData.resize(patchData.size());
		memcpy(origData.data(), patchAddress, patchData.size());
	}

	void Update()
	{
		bool value = CurValue();
		if(value == prevFlagValue)
			return; // flag not changed, no need to do anything

		auto& data = value ? patchData : origData;
		DWORD		dwProtect;
		VirtualProtect((void*)patchAddress, data.size(), PAGE_EXECUTE_READWRITE, &dwProtect);
		memcpy(patchAddress, data.data(), data.size());
		VirtualProtect((void*)patchAddress, data.size(), dwProtect, &dwProtect);

		prevFlagValue = value;
	}

	bool CurValue()
	{
		return FlagIsSet(flagValuesPtr, flagIndex);
	}

};

std::vector<FlagPatch> flagPatches;

void Trainer_Init()
{
	GLOBALS* globals = GlobalPtr();
	if (!globals)
		return;

	// DBG_PL_NOHIT
	{
		// EmAtkHitCk patch
		auto pattern = hook::pattern("05 88 00 00 00 50 53 57 E8");
		auto EmAtkHitCk_thunk = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(8));
		FlagPatch patch_EmAtkHitCk(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
		patch_EmAtkHitCk.SetPatch((uint8_t*)EmAtkHitCk_thunk.as_int(), {0x33, 0xC0, 0xC3});
		flagPatches.push_back(patch_EmAtkHitCk);

		// LifeDownSet2 patch
		pattern = hook::pattern("0F 85 ? ? ? ? A1 ? ? ? ? 66 83 B8 B4 4F 00 00 00 7F");
		auto LifeDownSet2_jg = pattern.count(1).get(0).get<uint8_t>(0x13);
		FlagPatch patch_LifeDownSet2(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
		patch_LifeDownSet2.SetPatch(LifeDownSet2_jg, { 0x90, 0x90 });
		flagPatches.push_back(patch_LifeDownSet2);

		// PlGachaGet patch
		pattern = hook::pattern("A1 ? ? ? ? 8B 15 ? ? ? ? 80 BA 98 4F 00 00 03");
		auto PlGachaGet = pattern.count(1).get(0).get<uint8_t>(0);
		FlagPatch patch_PlGachaGet(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
		patch_PlGachaGet.SetPatch(PlGachaGet, { 0xB8, 0x7F, 0x00, 0x00, 0x00, 0xC3 });
		flagPatches.push_back(patch_PlGachaGet);

		// PlGachaMove patch (prevents button prompt when grabbed)
		pattern = hook::pattern("57 8B 3D ? ? ? ? 6A 00 6A 00 6A 11 6A 0B");
		auto PlGachaMove = pattern.count(1).get(0).get<uint8_t>(0);
		FlagPatch patch_PlGachaMove(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
		patch_PlGachaMove.SetPatch(PlGachaMove, { 0xC3 });
		flagPatches.push_back(patch_PlGachaMove);
	}

	// DBG_EM_WEAK
	{
		// LifeDownSet2 patch
		auto pattern = hook::pattern("0F BF 86 24 03 00 00 3B C7 7D");
		auto LifeDownSet2_jg = pattern.count(1).get(0).get<uint8_t>(9);
		FlagPatch patch_LifeDownSet2(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK));
		patch_LifeDownSet2.SetPatch(LifeDownSet2_jg, { 0x90, 0x90 });
		flagPatches.push_back(patch_LifeDownSet2);
	}

	// DBG_INF_BULLET
	{
		auto pattern = hook::pattern("0B D1 66 89 57 08");
		auto cItemMgr__trigger_0_mov = pattern.count(1).get(0).get<uint8_t>(2);
		FlagPatch patch_cItemMgr__trigger_0(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
		patch_cItemMgr__trigger_0.SetPatch(cItemMgr__trigger_0_mov, { 0x90, 0x90, 0x90, 0x90 });
		flagPatches.push_back(patch_cItemMgr__trigger_0);

		pattern = hook::pattern("4A 66 89 50 02");
		auto cItemMgr__dump_0_mov = pattern.count(1).get(0).get<uint8_t>(1);
		FlagPatch patch_cItemMgr__dump_0(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
		patch_cItemMgr__dump_0.SetPatch(cItemMgr__dump_0_mov, { 0x90, 0x90, 0x90, 0x90 });
		flagPatches.push_back(patch_cItemMgr__dump_0);
	}
}

void Trainer_Update()
{
	for (auto& flagPatch : flagPatches)
	{
		flagPatch.Update();
	}
}
