#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"

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
		if (value == prevFlagValue)
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

void SetEmListParamExtensions(cEm* em, EM_LIST* emData)
{
	// Game sometimes uses hardcoded stack-based EM_LIST to spawn certain enemies
	// In those cases, 0xA & 0x1C usually are left untouched, resulting in random stack data being used
	// To protect against that, we'll only allow param extensions from emData that came from pG->emList_5410
	GLOBALS* pG = GlobalPtr();
	if (emData < pG->emList_5410 || emData >= &pG->emList_5410[256])
		return;

	//emData->percentageMotionSpeed_1C = 200;
	//emData->percentageScale_1E = 150;
	//emData->useExtendedParams_A = 1;

	if (emData->useExtendedParams_A != 1)
		return;

	if (!emData->percentageMotionSpeed_1C && !emData->percentageScale_1E)
		return;

	// let it run the cEm's EmXX_R0_init function before we apply changes
	// (since that has a good chance of overwriting scale value)
	if (em->r_no_0_FC == uint8_t(cEm::Routine0::Init))
		em->move();

	if (emData->percentageMotionSpeed_1C)
		em->MotInfo_1D8.speed_C0 = float(double(emData->percentageMotionSpeed_1C) / 100.0f);

	if (emData->percentageScale_1E)
	{
		auto scalePercent = float(double(emData->percentageScale_1E) / 100.0f);
		em->scale_AC.x *= scalePercent;
		em->scale_AC.y *= scalePercent;
		em->scale_AC.z *= scalePercent;

		if (IsGanado(em->ID_100))
		{
			// cEm10 holds a seperate scale value that it seems to grow/shrink the actual scale towards each frame
			// make sure we update that as well
			Vec* scale_bk_498 = (Vec*)((uint8_t*)em + 0x498);
			*scale_bk_498 = em->scale_AC;
		}
	}
}

cEm* (__cdecl* EmSetEvent)(EM_LIST* emData);
cEm* __cdecl EmSetEvent_Hook(EM_LIST* emData)
{
	cEm* result = EmSetEvent(emData);
	// TODO: check if result == errEm

	SetEmListParamExtensions(result, emData);
	return result;
}

cEm* (__cdecl* EmSetFromList2)(unsigned int em_list_no, int a2);
cEm* __cdecl EmSetFromList2_Hook(unsigned int em_list_no, int a2)
{
	cEm* result = EmSetFromList2(em_list_no, a2);
	SetEmListParamExtensions(result, &GlobalPtr()->emList_5410[em_list_no]);

	return result;
}

void __fastcall EmSetFromList_Hook(cEm* em, uint32_t unused)
{
	SetEmListParamExtensions(em, &GlobalPtr()->emList_5410[em->emListIndex_3A0]);
	em->move(); // code we patched to jump here
}

void __declspec(naked) EmSetFromList_Hook_Trampoline()
{
	__asm
	{
		mov ecx, eax
		jmp EmSetFromList_Hook
	}
}

void Trainer_Init()
{
	GLOBALS* globals = GlobalPtr();
	if (!globals)
		return;

	auto pattern = hook::pattern("52 66 89 45 E4 66 89 4D F6 E8");
	auto EmSetEvent_thunk = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(9)).as_int();
	ReadCall(EmSetEvent_thunk, EmSetEvent);
	InjectHook(EmSetEvent_thunk, EmSetEvent_Hook, PATCH_JUMP);

	pattern = hook::pattern("6A 01 6A 32 E8 ? ? ? ? 83 C4 20");
	auto EmSetFromList2_thunk = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(4)).as_int();
	ReadCall(EmSetFromList2_thunk, EmSetFromList2);
	InjectHook(EmSetFromList2_thunk, EmSetFromList2_Hook, PATCH_JUMP);

	pattern = hook::pattern("D9 98 7C 03 00 00 8B 42 10");
	auto EmSetFromList_mov = pattern.count(1).get(0).get<uint8_t>(6);
	InjectHook(EmSetFromList_mov, EmSetFromList_Hook_Trampoline, PATCH_CALL);

	// DBG_PL_NOHIT
	{
		// EmAtkHitCk patch
		auto pattern = hook::pattern("05 88 00 00 00 50 53 57 E8");
		auto EmAtkHitCk_thunk = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(8));
		FlagPatch patch_EmAtkHitCk(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH));
		patch_EmAtkHitCk.SetPatch((uint8_t*)EmAtkHitCk_thunk.as_int(), {0x33, 0xC0, 0xC3});
		flagPatches.push_back(patch_EmAtkHitCk);

		// LifeDownSet2 patch
		pattern = hook::pattern("0F 85 ? ? ? ? A1 ? ? ? ? 66 83 B8 B4 4F 00 00 00 7F");
		auto LifeDownSet2_jg = pattern.count(1).get(0).get<uint8_t>(0x13);
		FlagPatch patch_LifeDownSet2(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH));
		patch_LifeDownSet2.SetPatch(LifeDownSet2_jg, { 0x90, 0x90 });
		flagPatches.push_back(patch_LifeDownSet2);

		// PlGachaGet patch
		pattern = hook::pattern("A1 ? ? ? ? 8B 15 ? ? ? ? 80 BA 98 4F 00 00 03");
		auto PlGachaGet = pattern.count(1).get(0).get<uint8_t>(0);
		FlagPatch patch_PlGachaGet(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH));
		patch_PlGachaGet.SetPatch(PlGachaGet, { 0xB8, 0x7F, 0x00, 0x00, 0x00, 0xC3 });
		flagPatches.push_back(patch_PlGachaGet);

		// PlGachaMove patch (prevents button prompt when grabbed)
		pattern = hook::pattern("57 8B 3D ? ? ? ? 6A 00 6A 00 6A 11 6A 0B");
		auto PlGachaMove = pattern.count(1).get(0).get<uint8_t>(0);
		FlagPatch patch_PlGachaMove(globals->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH));
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

	if (FlagIsSet(GlobalPtr()->flags_DEBUG_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT)))
	{
		cPlayer* player = PlayerPtr();
		if (!player)
			return;

		static uint16_t atariInfoFlagBackup = 0;
		static bool atariInfoFlagSet = false;
		if (pInput->is_key_released(0x68) ||
			pInput->is_key_released(0x62) ||
			pInput->is_key_released(0x64) ||
			pInput->is_key_released(0x66) ||
			pInput->is_key_released(0x61) ||
			pInput->is_key_released(0x67))
		{
			player->atariInfo_2B4.flags_1A = atariInfoFlagBackup;
			atariInfoFlagSet = false;
		}

		Vec positionMod { 0 };
		bool positionModded = false;

		if (pInput->is_key_down(0x68)) //VK_NUMPAD_8
		{
			positionMod.x = 100;
			positionModded = true;
		}
		if (pInput->is_key_down(0x62)) //VK_NUMPAD_2
		{
			positionMod.x = -100;
			positionModded = true;
		}
		if (pInput->is_key_down(0x64)) //VK_NUMPAD_4
		{
			positionMod.z = -100;
			positionModded = true;
		}
		if (pInput->is_key_down(0x66)) //VK_NUMPAD_6
		{
			positionMod.z = 100;
			positionModded = true;
		}
		if (pInput->is_key_down(0x61)) //VK_NUMPAD_1
		{
			positionMod.y = -100;
			positionModded = true;
		}
		if (pInput->is_key_down(0x67)) //VK_NUMPAD_7
		{
			positionMod.y = 100;
			positionModded = true;
		}
		if (pInput->is_key_down(0x65)) // VK_NUMPAD_5
		{
			positionModded = true; // keep collision disabled if 5 held
		}

		if (positionModded)
		{
			player->position_94.x += positionMod.x;
			player->oldPos_110.x += positionMod.x;
			player->position_94.y += positionMod.y;
			player->oldPos_110.y += positionMod.y;
			player->position_94.z += positionMod.z;
			player->oldPos_110.z += positionMod.z;
			if (!atariInfoFlagSet)
			{
				atariInfoFlagBackup = player->atariInfo_2B4.flags_1A;
				atariInfoFlagSet = true;
			}
			player->atariInfo_2B4.flags_1A &= ~0x100;
			player->matUpdate();
			player->move();
		}
	}
}
