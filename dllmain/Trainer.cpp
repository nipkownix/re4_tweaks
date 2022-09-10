#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"
#include "imgui.h"
#include <FAhashes.h>
#include "UI_DebugWindows.h"
#include "UI_Utility.h"
#include "Trainer.h"
#include <DirectXMath.h>

int AshleyStateOverride;
int last_weaponId;

float fFreeCam_direction;
float fFreeCam_depression;

bool bRequestedPlayerTPtoCam;
bool bRequestedAshleyTPtoCam;

uint16_t pl_atariInfoFlagBackup = 0;
bool pl_atariInfoFlagSet = false;

uint16_t ash_atariInfoFlagBackup = 0;
bool ash_atariInfoFlagSet = false;

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

void MoveAshleyToPlayer()
{
	cPlayer* ashley = AshleyPtr();
	cPlayer* player = PlayerPtr();

	if (player && ashley)
	{
		// Backup original collision flags and then unset collision-enabled bit
		if (!ash_atariInfoFlagSet)
		{
			ash_atariInfoFlagBackup = ashley->atari_2B4.m_flag_1A;
			ash_atariInfoFlagSet = true;
		}
		ashley->atari_2B4.m_flag_1A &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE); // map collision | em collision

		ashley->pos_94 = player->pos_94;
		ashley->pos_old_110 = player->pos_old_110;
	}
}

std::vector<uint32_t> KeyComboNoclipToggle;
std::vector<uint32_t> KeyComboFreeCamToggle;
std::vector<uint32_t> KeyComboSpeedOverride;
std::vector<uint32_t> KeyComboAshToPlayer;

std::vector<uint32_t> KeyComboWeaponHotkey[5];
std::vector<uint32_t> KeyComboLastWeaponHotkey;

void HotkeySlotPressed(int slotIdx, bool forceUseWepID = false)
{
	if (!pConfig->bWeaponHotkeysEnable)
		return;

	if (ImGuiShouldAcceptInput())
		return; // don't apply hotkeys while in ImGui

	if (forceUseWepID && slotIdx < 1)
		return;

	cPlayer* player = PlayerPtr();
	if (!player)
		return;

	// Routine 4 = reloading, 2 = shooting
	if (player->r_no_2_FE == 4 || player->r_no_2_FE == 2)
		return;

	// If forceUseWepID is true, we are probably just trying to equip the last wep id, so we can skip this part
	if (SubScreenWk && !forceUseWepID)
	{
		// If subscreen is open and we're on inventory menu...
		if (SubScreenWk->open_flag_2C != 0 && SubScreenWk->menu_no_260 == 1)
		{
			// Treat the key-press as weapon binding request
			if (SubScreenWk->puzzlePlayer_2AC)
			{
				// Fetch current highlighted item piece
				itemPiece* ItemPiece = (itemPiece*)SubScreenWk->puzzlePlayer_2AC->ptrPiece(SubScreenWk->puzzlePlayer_2AC->m_p_active_board_30);
				if (ItemPiece && ItemPiece->be_flag_0 == 1)
				{
					if (ItemPiece->pItem_24)
					{
						pConfig->iWeaponHotkeyWepIds[slotIdx] = ItemPiece->pItem_24->id_0;
						pConfig->WriteSettings();
						
						// fall-thru to weapon switch code below, acts as an indicator that binding was set
					}
				}
			}
		}
	}

	// Not binding weapon, must be trying to switch to weapon instead
	int weaponId = pConfig->iWeaponHotkeyWepIds[slotIdx];

	// slotIdx is the actual weaponId if forceUseWepID is true
	if (!forceUseWepID)
		weaponId = pConfig->iWeaponHotkeyWepIds[slotIdx];
	else
		weaponId = slotIdx;

	if (!weaponId)
	{
		static size_t weaponCycleIndex[5] = { 0 };

		std::vector<int>& weaponCycle = pConfig->iWeaponHotkeyCycle[slotIdx];

		size_t desiredIndex = weaponCycleIndex[slotIdx];

		// Search cycle for the current equipped wep, if it contains it, then pick the weapon that comes after it
		// else if it doesn't contain current equipped wep, our desiredIndex will be whatever the previously selected index for this slot was
		if (ItemMgr->m_pWep_C)
		{
			for (size_t i = 0; i < weaponCycle.size(); i++)
				if (ItemMgr->m_pWep_C->id_0 == weaponCycle[i])
				{
					desiredIndex = i + 1;
					break;
				}
		}

		// Check that we have the desired weapon in inventory, continue through all possible weaponCycle values until we find one
		size_t cycleIdx = desiredIndex;
		for (size_t i = 0; i < weaponCycle.size(); i++)
		{
			cycleIdx = (desiredIndex + i) % weaponCycle.size();

			cItem* item = ItemMgr->search(weaponCycle[cycleIdx]);
			if (item)
			{
				weaponId = weaponCycle[cycleIdx];
				break;
			}
		}

		// Save our new index in the cycle
		weaponCycleIndex[slotIdx] = cycleIdx;
	}

	if (weaponId)
	{
		if (ItemMgr->m_pWep_C)
			last_weaponId = ItemMgr->m_pWep_C->id_0;

		cItem* item = ItemMgr->search(weaponId);
		if (ItemMgr->arm(item))
		{
			RequestWeaponChange();
		}
	}
}

void Trainer_ParseKeyCombos()
{
	// NoClip
	{
		KeyComboNoclipToggle.clear();
		KeyComboNoclipToggle = ParseKeyCombo(pConfig->sTrainerNoclipKeyCombo);

		pInput->RegisterHotkey({ []() {
			bool playerCollisionDisabled = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
			FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), !playerCollisionDisabled);
		}, &KeyComboNoclipToggle });
	}

	// Free cam
	{
		KeyComboFreeCamToggle.clear();
		KeyComboFreeCamToggle = ParseKeyCombo(pConfig->sTrainerFreeCamKeyCombo);

		pInput->RegisterHotkey({ []() {
			pConfig->bTrainerEnableFreeCam = !pConfig->bTrainerEnableFreeCam;
		}, &KeyComboFreeCamToggle });
	}

	// SpeedOverride
	{
		KeyComboSpeedOverride.clear();
		KeyComboSpeedOverride = ParseKeyCombo(pConfig->sTrainerSpeedOverrideKeyCombo);

		pInput->RegisterHotkey({ []() {
			pConfig->bTrainerPlayerSpeedOverride = !pConfig->bTrainerPlayerSpeedOverride;
		}, &KeyComboSpeedOverride });
	}

	// Move Ash to player
	{
		KeyComboAshToPlayer.clear();
		KeyComboAshToPlayer = ParseKeyCombo(pConfig->sTrainerMoveAshToPlayerKeyCombo);

		pInput->RegisterHotkey({ []() {
			MoveAshleyToPlayer();
		}, &KeyComboAshToPlayer });
	}

	// WeaponHotkey
	{
		for (int i = 0; i < 5; i++)
		{
			KeyComboWeaponHotkey[i].clear();
			KeyComboWeaponHotkey[i] = ParseKeyCombo(pConfig->sWeaponHotkeys[i]);
		}

		{
			KeyComboLastWeaponHotkey.clear();
			KeyComboLastWeaponHotkey = ParseKeyCombo(pConfig->sLastWeaponHotkey);
		}

		// TODO: make RegisterHotkey use std::function so that we could include `i` in the capture and handle this in the loop above...

		pInput->RegisterHotkey({ []() { HotkeySlotPressed(0); }, &KeyComboWeaponHotkey[0] });
		pInput->RegisterHotkey({ []() { HotkeySlotPressed(1); }, &KeyComboWeaponHotkey[1] });
		pInput->RegisterHotkey({ []() { HotkeySlotPressed(2); }, &KeyComboWeaponHotkey[2] });
		pInput->RegisterHotkey({ []() { HotkeySlotPressed(3); }, &KeyComboWeaponHotkey[3] });
		pInput->RegisterHotkey({ []() { HotkeySlotPressed(4); }, &KeyComboWeaponHotkey[4] });

		pInput->RegisterHotkey({ []() { HotkeySlotPressed(last_weaponId, true); }, &KeyComboLastWeaponHotkey });
	}
}

bool(__cdecl* cameraHitCheck_orig)(Vec *pCross, Vec *pNorm, Vec p0, Vec p1);
bool __cdecl cameraHitCheck_hook(Vec *pCross, Vec *pNorm, Vec p0, Vec p1)
{
	if (pConfig->bTrainerEnableFreeCam)
		return false;
	else
		return cameraHitCheck_orig(pCross, pNorm, p0, p1);
}


std::vector<FlagPatch> flagPatches;
void Trainer_Init()
{
	GLOBAL_WK* globals = GlobalPtr();
	if (!globals)
		return;

	if (!GameVersionIsDebug())
	{
		// Remove bzero call that clears debug flags every frame for some reason
		auto pattern = hook::pattern("83 C0 60 6A 10 50 E8");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(6), 5, true);
	}

	// Game seems to set DBG_NO_DEATH & DBG_INF_BULLET2 flags during gameRoomInit
	// Ordinarilly those get cleared by the bzero mentioned above, but we had to disable that to allow using DBG flags...
	// For now we'll just patch out the code that sets up those flags
	{
		auto pattern = hook::pattern("81 48 68 00 00 80 00");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0), 7, true);
		pattern = hook::pattern("B9 00 00 00 80 09 48 6C");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0), 8, true);
	}

	// gameRoomMemInit clears most DBG flags when loading new room
	// patch out the code responsible so players trainer settings won't get reset between rooms
	{
		auto pattern = hook::pattern("8B 15 ? ? ? ? 89 72 60");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0), 9, true);
		pattern = hook::pattern("A1 ? ? ? ? 89 70 64");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0), 8, true);
	}

	// DBG_NO_DEATH2
	{
		// EmAtkHitCk patch
		auto pattern = hook::pattern("05 88 00 00 00 50 53 57 E8");
		auto EmAtkHitCk_thunk = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(8));
		FlagPatch patch_EmAtkHitCk(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
		patch_EmAtkHitCk.SetPatch((uint8_t*)EmAtkHitCk_thunk.as_int(), { 0x33, 0xC0, 0xC3 });
		flagPatches.push_back(patch_EmAtkHitCk);

		// LifeDownSet2 patch
		pattern = hook::pattern("0F 85 ? ? ? ? A1 ? ? ? ? 66 83 B8 B4 4F 00 00 00 7F");
		auto LifeDownSet2_jg = pattern.count(1).get(0).get<uint8_t>(0x13);
		FlagPatch patch_LifeDownSet2(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
		patch_LifeDownSet2.SetPatch(LifeDownSet2_jg, { 0x90, 0x90 });
		flagPatches.push_back(patch_LifeDownSet2);

		// PlGachaGet patch
		pattern = hook::pattern("A1 ? ? ? ? 8B 15 ? ? ? ? 80 BA 98 4F 00 00 03");
		auto PlGachaGet = pattern.count(1).get(0).get<uint8_t>(0);
		FlagPatch patch_PlGachaGet(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
		patch_PlGachaGet.SetPatch(PlGachaGet, { 0xB8, 0x7F, 0x00, 0x00, 0x00, 0xC3 });
		flagPatches.push_back(patch_PlGachaGet);

		// PlGachaMove patch (prevents button prompt when grabbed)
		pattern = hook::pattern("57 8B 3D ? ? ? ? 6A 00 6A 00 6A 11 6A 0B");
		auto PlGachaMove = pattern.count(1).get(0).get<uint8_t>(0);
		FlagPatch patch_PlGachaMove(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
		patch_PlGachaMove.SetPatch(PlGachaMove, { 0xC3 });
		flagPatches.push_back(patch_PlGachaMove);
	}

	// DBG_EM_WEAK
	{
		// LifeDownSet2 patch
		auto pattern = hook::pattern("0F BF 86 24 03 00 00 3B C7 7D");
		auto LifeDownSet2_jg = pattern.count(1).get(0).get<uint8_t>(9);
		FlagPatch patch_LifeDownSet2(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK));
		patch_LifeDownSet2.SetPatch(LifeDownSet2_jg, { 0x90, 0x90 });
		flagPatches.push_back(patch_LifeDownSet2);
	}

	// DBG_INF_BULLET
	{
		auto pattern = hook::pattern("0B D1 66 89 57 08");
		auto cItemMgr__trigger_0_mov = pattern.count(1).get(0).get<uint8_t>(2);
		FlagPatch patch_cItemMgr__trigger_0(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
		patch_cItemMgr__trigger_0.SetPatch(cItemMgr__trigger_0_mov, { 0x90, 0x90, 0x90, 0x90 });
		flagPatches.push_back(patch_cItemMgr__trigger_0);

		pattern = hook::pattern("4A 66 89 50 02");
		auto cItemMgr__dump_0_mov = pattern.count(1).get(0).get<uint8_t>(1);
		FlagPatch patch_cItemMgr__dump_0(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
		patch_cItemMgr__dump_0.SetPatch(cItemMgr__dump_0_mov, { 0x90, 0x90, 0x90, 0x90 });
		flagPatches.push_back(patch_cItemMgr__dump_0);
	}

	// DBG_EM_NO_ATK
	{
		// TODO: this only affects em10FindCk right now, other Ems also have their own FindCk funcs though, might need to patch those too
		// (unless we can find a func used by all of them - haven't had any luck with that yet...)
		// RouteCk / em10RouteCk also seem involved with searching/routing to player, but patching those can make Em movement buggy...
		auto pattern = hook::pattern("33 D2 8B C6 E8 ? ? ? ? 85 C0 75");
		auto em10FindCk = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(4));

		FlagPatch patch_em10FindCk(globals->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_NO_ATK));
		patch_em10FindCk.SetPatch((uint8_t*)em10FindCk.as_int(), { 0x31, 0xC0, 0xC3 });
		flagPatches.push_back(patch_em10FindCk);
	}

	// Ashley presence
	{
		auto pattern = hook::pattern("F7 80 ? ? ? ? ? ? ? ? 74 22 D9 80 ? ? ? ? 51");
		struct ScenarioRoomInit_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				switch ((AshleyState)AshleyStateOverride) {
				case AshleyState::Present:
					FlagSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_SUB_ASHLEY), true);
					break;
				case AshleyState::NotPresent:
					FlagSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_SUB_ASHLEY), false);
					break;
				case AshleyState::Default:
					break;
				}
				
				// Code we replaced
				if (FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_SUB_ASHLEY)))
					regs.ef &= ~(1 << regs.zero_flag);
				else
					regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<ScenarioRoomInit_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));
	}

	// Enemy HP multiplier
	{
		auto pattern = hook::pattern("66 89 88 ? ? ? ? 0F BF 56 ? 89 55 ? DB 45 ? D8 C9 D9 98");
		struct EmSetFromList_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we repalced
				*(int16_t*)(regs.eax + 0x326) = (int16_t)regs.ecx;

				// Get pointer to cEm
				cEm* Em_ptr = (cEm*)(regs.eax);

				if (pConfig->bTrainerEnemyHPMultiplier)
				{
					float multi = 0.0f;
					if (pConfig->bTrainerRandomHPMultiplier)
						multi = GetRandomFloat(pConfig->fTrainerRandomHPMultiMin, pConfig->fTrainerRandomHPMultiMax);
					else
						multi = pConfig->fTrainerEnemyHPMultiplier;

					Em_ptr->hp_324 = (int16_t)(Em_ptr->hp_324 * multi);
					Em_ptr->hp_max_326 = (int16_t)(Em_ptr->hp_max_326 * multi);
				}
			}
		}; injector::MakeInline<EmSetFromList_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		pattern = hook::pattern("66 89 86 ? ? ? ? 0F BF 57 12 89 55 08 DB 45 08 D8 C9 D9");
		struct EmSetFromList2_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we repalced
				*(int16_t*)(regs.esi + 0x326) = (int16_t)regs.eax;

				// Get pointer to cEm
				cEm* Em_ptr = (cEm*)(regs.esi);

				if (pConfig->bTrainerEnemyHPMultiplier)
				{
					float multi = 0.0f;
					if (pConfig->bTrainerRandomHPMultiplier)
						multi = GetRandomFloat(pConfig->fTrainerRandomHPMultiMin, pConfig->fTrainerRandomHPMultiMax);
					else
						multi = pConfig->fTrainerEnemyHPMultiplier;

					Em_ptr->hp_324 = (int16_t)(Em_ptr->hp_324 * multi);
					Em_ptr->hp_max_326 = (int16_t)(Em_ptr->hp_max_326 * multi);
				}
			}
		}; injector::MakeInline<EmSetFromList2_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		pattern = hook::pattern("66 89 86 ? ? ? ? 0F BF 4F ? 89 4D ? DB 45 ? D8 C9");
		struct EmSetEvent_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we repalced
				*(int16_t*)(regs.esi + 0x326) = (int16_t)regs.eax;

				// Get pointer to cEm
				cEm* Em_ptr = (cEm*)(regs.esi);

				if (pConfig->bTrainerEnemyHPMultiplier)
				{
					float multi = 0.0f;
					if (pConfig->bTrainerRandomHPMultiplier)
						multi = GetRandomFloat(pConfig->fTrainerRandomHPMultiMin, pConfig->fTrainerRandomHPMultiMax);
					else
						multi = pConfig->fTrainerEnemyHPMultiplier;

					Em_ptr->hp_324 = (int16_t)(Em_ptr->hp_324 * multi);
					Em_ptr->hp_max_326 = (int16_t)(Em_ptr->hp_max_326 * multi);
				}
			}
		}; injector::MakeInline<EmSetEvent_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
	}

	// Disable enemy spawn
	{
		auto pattern = hook::pattern("38 8F ? ? ? ? 0F 85 ? ? ? ? 38 87 ? ? ? ? 0F 85 ? ? ? ? 53");
		struct EmSetFromList_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Original cmp we replaced
				bool isEqual = (*(uint8_t*)(regs.edi + 0x4FAD) == (uint8_t)regs.ecx);

				if (!pConfig->bTrainerDisableEnemySpawn && isEqual)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<EmSetFromList_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("8B 15 ? ? ? ? 85 C0 75 22 0F B7 47 18");
		struct EmSetFromList2_hook1
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				regs.edx = (uint32_t)GlobalPtr();

				if (pConfig->bTrainerDisableEnemySpawn)
					regs.eax = 0;
			}
		}; injector::MakeInline<EmSetFromList2_hook1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("38 8a ? ? ? ? 75 ? 38 82 ? ? ? ? 74 ? a1 ? ? ? ? 5f 5b");
		struct EmSetFromList2_hook2
		{
			void operator()(injector::reg_pack& regs)
			{
				// Original cmp we replaced
				bool isEqual = (*(uint8_t*)(regs.edx + 0x4FAD) == (uint8_t)regs.ecx);

				if (!pConfig->bTrainerDisableEnemySpawn && isEqual)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<EmSetFromList2_hook2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Dead bodies never disappear
	{
		auto pattern = hook::pattern("8b 8e ? ? ? ? f7 c1 ? ? ? ? 74 ? b8");
		struct em10_R1_Die_Cramp_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				regs.ecx = *(uint32_t*)(regs.esi + 0x3D0);

				// Dead bodies have to disappear in room 11C, otherwise the game won't advance. Something about enemy recycling?
				bool is11C = (GlobalPtr()->curRoomId_4FAC == 0x11C);

				if (pConfig->bTrainerDeadBodiesNeverDisappear && !is11C)
					regs.eax = 1;
			}
		}; injector::MakeInline<em10_R1_Die_Cramp_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// AllowEnterDoorsWithoutAsh
	{
		auto pattern = hook::pattern("A1 ? ? ? ? 85 C0 74 7A 05 ? ? ? ? 50 A1 ? ? ? ? 05 ? ? ? ? 50 E8");
		struct CheckAshleyActive_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				regs.eax = (uint32_t)AshleyPtr();

				// Make game check for Leon's position against itself, instead of checking against Ashley's position.
				if (pConfig->bTrainerAllowEnterDoorsWithoutAsh)
					regs.eax = (uint32_t)PlayerPtr();
			}
		}; injector::MakeInline<CheckAshleyActive_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
	}

	// Free cam
	{
		// Hook cameraHitCheck to disable hit detection
		auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 20 84 C0 74 2A");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cameraHitCheck_orig);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cameraHitCheck_hook, PATCH_JUMP);

		// Redirect m_direction_ratio_208 and m_depression_ratio_204 when using free cam
		pattern = hook::pattern("D9 83 ? ? ? ? DA E9 89 85 ? ? ? ? DF E0");
		struct calcOffset_direction
		{
			void operator()(injector::reg_pack& regs)
			{
				float direction;

				if (pConfig->bTrainerEnableFreeCam)
					direction = fFreeCam_direction;
				else
					direction = CamCtrl->m_QuasiFPS_278.m_direction_ratio_208;

				__asm {fld direction}
			}
		}; injector::MakeInline<calcOffset_direction>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 83 ? ? ? ? 51 D9 1C ? 8D 85 ? ? ? ? 6A ? 50");
		injector::MakeInline<calcOffset_direction>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 83 ? ? ? ? D9 EE D9 C0 DD EA DF E0 DD D9 F6 C4 ? 7A ? 8B 75");
		struct calcOffset_depression
		{
			void operator()(injector::reg_pack& regs)
			{
				float depression;

				if (pConfig->bTrainerEnableFreeCam)
					depression = fFreeCam_depression;
				else
					depression = CamCtrl->m_QuasiFPS_278.m_depression_ratio_204;

				__asm {fld depression}
			}
		}; injector::MakeInline<calcOffset_depression>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 83 ? ? ? ? D8 D9 DF E0 F6 C4 ? 0F 85");
		injector::MakeInline<calcOffset_depression>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 83 ? ? ? ? 52 D9 9D ? ? ? ? 51 D9 85 ? ? ? ? 8D 45");
		injector::MakeInline<calcOffset_depression>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 83 ? ? ? ? D8 D9 DF E0 F6 C4 ? 0F 8A ? ? ? ? DD D8 8D 95");
		injector::MakeInline<calcOffset_depression>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 83 ? ? ? ? 52 DC 0D ? ? ? ? 51 8D 45 ? D9 9D");
		injector::MakeInline<calcOffset_depression>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
}

void Trainer_Update()
{
	for (auto& flagPatch : flagPatches)
	{
		flagPatch.Update();
	}

	cPlayer* player = PlayerPtr();
	if (!player)
		return;

	// Player speed override handling
	{
		static bool prev_bPlayerSpeedOverride = false;
		static float prev_PlayerSpeed = 0;
		if (prev_bPlayerSpeedOverride != pConfig->bTrainerPlayerSpeedOverride)
		{
			if (pConfig->bTrainerPlayerSpeedOverride)
			{
				// changed from disabled -> enabled, backup previous speed value
				prev_PlayerSpeed = player->Motion_1D8.Seq_speed_C0;
			}
			else if (!pConfig->bTrainerPlayerSpeedOverride)
			{
				// changed from enabled -> disabled, restore previous speed value
				player->Motion_1D8.Seq_speed_C0 = prev_PlayerSpeed;
			}
			prev_bPlayerSpeedOverride = pConfig->bTrainerPlayerSpeedOverride;
		}

		if (pConfig->bTrainerPlayerSpeedOverride)
			player->Motion_1D8.Seq_speed_C0 = pConfig->fTrainerPlayerSpeedOverride;
	}

	// Handle the player's collision
	if (pl_atariInfoFlagSet && 
		!FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT)) &&
		!pConfig->bTrainerEnableFreeCam)
	{
		// Restore collision flags
		player->atari_2B4.m_flag_1A = pl_atariInfoFlagBackup;
		pl_atariInfoFlagSet = false;
	}

	// Handle Ashley's collision
	cPlayer* ashley = AshleyPtr();
	if (ashley && !pConfig->bTrainerEnableFreeCam)
	{
		// Restore collision flags
		if (ash_atariInfoFlagSet)
		{
			ashley->atari_2B4.m_flag_1A = ash_atariInfoFlagBackup;
			ash_atariInfoFlagSet = false;
		}
	}

	if (FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT)))
	{
		// Backup original collision flags and then unset collision-enabled bit
		if (!pl_atariInfoFlagSet)
		{
			pl_atariInfoFlagBackup = player->atari_2B4.m_flag_1A;
			pl_atariInfoFlagSet = true;
		}
		player->atari_2B4.m_flag_1A &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE); // map collision | em collision

		// Handle numpad/mouse wheel movement
		if (pConfig->bTrainerUseNumpadMovement && !pConfig->bTrainerEnableFreeCam)
		{
			// Speed calculation
			float movSpeed = 140.0f * pConfig->fTrainerNumMoveSpeed;

			// Cam directions
			float Sine = CamCtrl->Camera_60.mat_0[2][0];
			float Cosine = CamCtrl->Camera_60.mat_0[0][0];
			float Pitch = fFreeCam_depression;

			Sine *= movSpeed;
			Cosine *= movSpeed;
			Pitch *= movSpeed;

			// Forwards
			if (pInput->is_key_down(0x68)) //VK_NUMPAD_8
			{
				player->pos_94.x += Sine;
				player->pos_94.z -= Cosine;
			}

			// Backwards
			if (pInput->is_key_down(0x62)) //VK_NUMPAD_2
			{
				player->pos_94.x -= Sine;
				player->pos_94.z += Cosine;
			}

			// Left
			if (pInput->is_key_down(0x64)) //VK_NUMPAD_4
			{
				player->pos_94.x -= Cosine;
				player->pos_94.z -= Sine;
			}

			// Right
			if (pInput->is_key_down(0x66)) //VK_NUMPAD_6
			{
				player->pos_94.x += Cosine;
				player->pos_94.z += Sine;
			}

			// Up
			if (pInput->is_key_down(0x67)) //VK_NUMPAD_7
			{
				player->pos_94.y += movSpeed;
			}

			// Down
			if (pInput->is_key_down(0x61)) //VK_NUMPAD_1
			{
				player->pos_94.y -= movSpeed;
			}

			// Handle mouse wheel
			if (pConfig->bTrainerUseMouseWheelUpDown && !ImGuiShouldAcceptInput())
			{
				// Up
				if (pInput->mouse_wheel_delta() > 0)
				{
					movSpeed += pInput->mouse_wheel_delta() * 200.0f;

					player->pos_94.y += movSpeed;
				}

				// Down
				if ((pInput->mouse_wheel_delta() < 0))
				{
					movSpeed -= pInput->mouse_wheel_delta() * 200.0f;

					player->pos_94.y -= movSpeed;
				}
			}
		}
	}

	// Handle free camera
	static bool stopPlFlagSet = false;
	static float CamXYZbackup[3] = {0, 0, 0};
	if (pConfig->bTrainerEnableFreeCam)
	{
		// Camera rotation
		if (isController())
		{
			fFreeCam_direction -= *AnalogRX_8 / 800.0f;
			fFreeCam_depression += *AnalogRY_9 / 800.0f;
		}
		else
		{
			fFreeCam_direction -= *MouseDeltaX / 600.0f;
			fFreeCam_depression += *MouseDeltaY / 600.0f;
		}

		fFreeCam_depression = std::clamp(fFreeCam_depression, -2.33f, 2.33f);

		// Flag and backup
		bool isFlagSet = FlagIsSet(GlobalPtr()->flags_STOP_0_170, uint32_t(Flags_STOP::SPF_PL));

		if (!isFlagSet)
		{
			// Backup cam pos
			CamXYZbackup[0] = CamCtrl->m_QuasiFPS_278.m_pl_mat_178[0][3];
			CamXYZbackup[1] = CamCtrl->m_QuasiFPS_278.m_pl_mat_178[1][3];
			CamXYZbackup[2] = CamCtrl->m_QuasiFPS_278.m_pl_mat_178[2][3];

			// Backup original collision flags and then unset collision-enabled bit
			if (!pl_atariInfoFlagSet)
			{
				pl_atariInfoFlagBackup = player->atari_2B4.m_flag_1A;
				pl_atariInfoFlagSet = true;
			}
			player->atari_2B4.m_flag_1A &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE); // map collision | em collision

			if (ashley)
			{
				if (!ash_atariInfoFlagSet)
				{
					ash_atariInfoFlagBackup = ashley->atari_2B4.m_flag_1A;
					ash_atariInfoFlagSet = true;
				}
				ashley->atari_2B4.m_flag_1A &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE); // map collision | em collision
			}

			// Set pl stop flag
			FlagSet(GlobalPtr()->flags_STOP_0_170, uint32_t(Flags_STOP::SPF_PL), true);
			stopPlFlagSet = true;
		}

		// Input handling
		bool isPressingFwd = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_FORWARD) == (uint64_t)KEY_BTN::KEY_FORWARD);
		bool isPressingBack = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_BACK) == (uint64_t)KEY_BTN::KEY_BACK);
		bool isPressingLeft = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_LEFT) == (uint64_t)KEY_BTN::KEY_LEFT);
		bool isPressingRight = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_RIGHT) == (uint64_t)KEY_BTN::KEY_RIGHT);
		bool isPressingRun = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_RUN) == (uint64_t)KEY_BTN::KEY_RUN);
		bool isPressingUp = false;
		bool isPressingDown = false;

		if (isController())
			isPressingUp = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_KAMAE_KNIFE) == (uint64_t)KEY_BTN::KEY_KAMAE_KNIFE);
		else
		{
			if (!ImGuiShouldAcceptInput())
				isPressingUp = (pInput->mouse_wheel_delta() > 0); // Mouse wheel up
		}

		if (isController())
			isPressingDown = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_KAMAE) == (uint64_t)KEY_BTN::KEY_KAMAE);
		else
		{
			if (!ImGuiShouldAcceptInput())
				isPressingDown = (pInput->mouse_wheel_delta() < 0); // Mouse wheel down
		}

		// Speed calculation
		float movSpeed = 140.0f * pConfig->fTrainerFreeCamSpeed;

		if (isPressingRun)
			movSpeed *= 2.0f;

		// Directions
		float Sine = CamCtrl->Camera_60.mat_0[2][0];
		float Cosine = CamCtrl->Camera_60.mat_0[0][0];
		float Pitch = fFreeCam_depression;

		Sine *= movSpeed;
		Cosine *= movSpeed;
		Pitch *= movSpeed;

		float *X = &CamCtrl->m_QuasiFPS_278.m_pl_mat_178[0][3];
		float *Y = &CamCtrl->m_QuasiFPS_278.m_pl_mat_178[1][3];
		float *Z = &CamCtrl->m_QuasiFPS_278.m_pl_mat_178[2][3];

		// Forwards
		if (isPressingFwd)
		{
			*X += Sine;
			*Y += Pitch;
			*Z -= Cosine;
		}

		// Backwards
		if (isPressingBack)
		{
			*X -= Sine;
			*Y -= Pitch;
			*Z += Cosine;
		}

		// Left
		if (isPressingLeft)
		{
			*X -= Cosine;
			*Z -= Sine;
		}

		// Right
		if (isPressingRight)
		{
			*X += Cosine;
			*Z += Sine;
		}

		// Up
		if (isPressingUp)
		{
			if (isKeyboardMouse())
			{
				movSpeed += pInput->mouse_wheel_delta() * 200.0f;

				if (isPressingRun)
					movSpeed *= 3.0f;
			}

			*Y += movSpeed;
		}

		// Down
		if (isPressingDown)
		{
			if (isKeyboardMouse())
			{
				movSpeed -= pInput->mouse_wheel_delta() * 200.0f;

				if (isPressingRun)
					movSpeed *= 3.0f;
			}

			*Y -= movSpeed;
		}

		// TP if needed
		if (bRequestedPlayerTPtoCam)
		{
			player->pos_94.x = *X;
			player->pos_94.y = *Y;
			player->pos_94.z = *Z;

			// Has to be set to false, or else the game won't read the new position. 
			// Will be automatically be set to true again once the loop reaches this function again.
			FlagSet(GlobalPtr()->flags_STOP_0_170, uint32_t(Flags_STOP::SPF_PL), false);

			bRequestedPlayerTPtoCam = false;
		}

		if (bRequestedAshleyTPtoCam)
		{
			if (ashley)
			{
				ashley->pos_94.x = *X;
				ashley->pos_94.y = *Y;
				ashley->pos_94.z = *Z;
			}

			bRequestedAshleyTPtoCam = false;
		}
	}
	else if (stopPlFlagSet) // Disable Pl stop flag and restore cam backup
	{
		CamCtrl->m_QuasiFPS_278.m_pl_mat_178[0][3] = CamXYZbackup[0];
		CamCtrl->m_QuasiFPS_278.m_pl_mat_178[1][3] = CamXYZbackup[1];
		CamCtrl->m_QuasiFPS_278.m_pl_mat_178[2][3] = CamXYZbackup[2];

		FlagSet(GlobalPtr()->flags_STOP_0_170, uint32_t(Flags_STOP::SPF_PL), false);
		stopPlFlagSet = false;
	}
}

void Trainer_ESP()
{
	if (!(pConfig->bShowSideInfo || pConfig->bShowESP))
		return;

	auto& io = ImGui::GetIO();
	Vec screenpos;

	auto screen_width = io.DisplaySize.x;
	auto screen_height = io.DisplaySize.y;

	cEmMgr& emMgr = *EmMgrPtr();

	GLOBAL_WK* pG = GlobalPtr();
	if (!pG)
		return;

	// Draw side info
	if (pConfig->bShowSideInfo)
	{
		// Drw Em count
		if (pConfig->bSideShowEmCount)
		{
			std::stringstream emCnt;
			emCnt << "Em Count: " << emMgr.count_valid() << " | " << "Max: " << emMgr.count();

			ImGui::GetBackgroundDrawList()->AddText(ImVec2(20, 20), ImColor(255, 255, 255, 255), emCnt.str().c_str());
		}

		// Draw Em list
		if (pConfig->bSideShowEmList)
		{
			std::vector<cEm*> EnemiesVec = cEmMgr::GetVecClosestEms(pConfig->iSideClosestEmsAmount, pConfig->fSideMaxEmDistance, true, true, pConfig->bSideOnlyShowESLSpawned, true);
			std::reverse(EnemiesVec.begin(), EnemiesVec.end());

			float InfoOffsetY = 60.0f;
			for (auto& em : EnemiesVec)
			{
				// Draw Em Name
				std::stringstream EmName;
				EmName << "#" << em->guid_F8 << " " << cEmMgr::EmIdToName(em->id_100, pConfig->bSideShowSimpleNames);
				if (!pConfig->bSideShowSimpleNames)
				{
					EmName << " (type " << (int)em->type_101 << ")";
				}

				ImGui::GetBackgroundDrawList()->AddText(ImVec2(10, screen_height - InfoOffsetY), ImColor(255, 255, 255, 255), EmName.str().c_str());

				if ((pConfig->iSideEmHPMode != 0) && (em->hp_max_326 > 0))
				{
					float hp_percent = 0;
					float hp_max = (float)em->hp_max_326;

					if (hp_max < em->hp_324)
						hp_max = (float)em->hp_324;

					hp_percent = (float)em->hp_324 * 100.0f / hp_max;


					// HP Bar
					if (pConfig->iSideEmHPMode == 1)
					{
						float barScalex = 1.0f;

						ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(30, screen_height - InfoOffsetY + 28.0f), ImVec2(30.0f + 100.0f * barScalex, screen_height - InfoOffsetY + 18.0f), ImColor(105, 105, 105));
						ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(30, screen_height - InfoOffsetY + 28.0f), ImVec2(30.0f + hp_percent * barScalex, screen_height - InfoOffsetY + 18.0f), getHealthColor(hp_percent));

						InfoOffsetY += 12;
					}
					else if (pConfig->iSideEmHPMode == 2) // HP Text
					{
						char CurEmHp[256];
						sprintf(CurEmHp, "Cur HP: %d", em->hp_324);

						char MaxEmHp[256];
						sprintf(MaxEmHp, "Max HP: %d", em->hp_max_326);


						ImGui::GetBackgroundDrawList()->AddText(ImVec2(30, screen_height - InfoOffsetY + 15), getHealthColor(hp_percent), CurEmHp);
						ImGui::GetBackgroundDrawList()->AddText(ImVec2(30, screen_height - InfoOffsetY + 30), getHealthColor(hp_percent), MaxEmHp);

						InfoOffsetY += 30;
					}
				}

				InfoOffsetY += 20; // No HP
			}
		}
	}

	// Draw ESP
	if (pConfig->bShowESP)
	{
		std::vector<cEm*> EmsVector;
		if (pConfig->bEspOnlyShowClosestEms)
			EmsVector = cEmMgr::GetVecClosestEms(pConfig->iEspClosestEmsAmount, pConfig->fEspMaxEmDistance, pConfig->bEspOnlyShowValidEms, pConfig->bEspOnlyShowEnemies, pConfig->bEspOnlyShowESLSpawned, pConfig->bEspOnlyShowAlive);
		else
			EmsVector = cEmMgr::GetVecClosestEms(emMgr.count(), pConfig->fEspMaxEmDistance, pConfig->bEspOnlyShowValidEms, pConfig->bEspOnlyShowEnemies, pConfig->bEspOnlyShowESLSpawned, pConfig->bEspOnlyShowAlive);

		for (auto& em : EmsVector)
		{
			auto coords = em->pos_94;

			if (pConfig->bEspShowInfoOnTop)
			{
				// Replace coords.y with the y from the highest part instead
				float highestPart = -FLT_MAX;
				cParts* part = em->childParts_F4;
				while (part != nullptr)
				{
					if (part->world_70.y > highestPart)
						highestPart = part->world_70.y;

					part = part->nextParts_F4;
				}

				coords.y = highestPart + 200.0f;
				coords.x -= 200.0f;
				coords.z += 200.0f;
			}

			float EspOffsetY = 0.0f;
			if (WorldToScreen(coords, screenpos, pG->Camera_74.v_mat_30, pG->Camera_74.CamPoint_A4.Fovy_1C, screen_width, screen_height))
			{
				// Draw Em Name
				if (pConfig->iEspEmNameMode != 0)
				{
					bool useSimpleNames = (pConfig->iEspEmNameMode == 2);

					std::stringstream EmName;
					EmName << "#" << em->guid_F8 << " " << cEmMgr::EmIdToName(em->id_100, useSimpleNames);
					if (!useSimpleNames)
					{
						EmName << " (type " << (int)em->type_101 << ")";
					}

					ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y), ImColor(255, 255, 255, 255), EmName.str().c_str());
					EspOffsetY += 20.0f;
				}

				// Draw debug info
				if (pConfig->bEspDrawDebugInfo)
				{
					char EmDbgInfo[256];
					sprintf(EmDbgInfo, "0x%p, Rno %d-%d-%d-%d", em, int(em->r_no_0_FC), int(em->r_no_1_FD), int(em->r_no_2_FE), int(em->r_no_3_FF));
					ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + EspOffsetY), ImColor(255, 255, 255), EmDbgInfo);

					EspOffsetY += 20.0f;
				}

				// Draw HP
				if ((pConfig->iEspEmHPMode != 0) && (em->hp_max_326 > 0))
				{
					float hp_percent = 0;
					float hp_max = (float)em->hp_max_326;

					// Workaround for an issue with the HP Multiplier: Some special cases in the game's code can set the curHP/maxHP of Ems
					// after they have been spawned, and after our hook has increased the HP. This can result in situations where the maxHP
					// value is lower than the curHP value. Doesn't seem to be a problem for the game, but it does overflow our HP bar.
					if (hp_max < em->hp_324)
						hp_max = (float)em->hp_324;

					hp_percent = (float)em->hp_324 * 100.0f / hp_max;

					if (hp_percent)
					{
						// HP Bar
						if (pConfig->iEspEmHPMode == 1)
						{
							float barScalex = 1.0f;

							ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(screenpos.x, screenpos.y + EspOffsetY), ImVec2(screenpos.x + 100.0f * barScalex, screenpos.y + EspOffsetY + 10.0f), ImColor(105, 105, 105));
							ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(screenpos.x, screenpos.y + EspOffsetY), ImVec2(screenpos.x + hp_percent * barScalex, screenpos.y + EspOffsetY + 10.0f), getHealthColor(hp_percent));
						
							EspOffsetY += 20.0f;
						}
						else // HP Text
						{
							if (em->hp_max_326 > 0)
							{
								char CurEmHp[256];
								sprintf(CurEmHp, "Cur HP: %d", em->hp_324);

								char MaxEmHp[256];
								sprintf(MaxEmHp, "Max HP: %d", em->hp_max_326);

								ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + EspOffsetY), ImColor(getHealthColor(hp_percent)), CurEmHp);
								ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + EspOffsetY + 15.0f), ImColor(getHealthColor(hp_percent)), MaxEmHp);
							}
						}
					}
				}

				// Draw lines pointing to the Em
				if (pConfig->bEspDrawLines)
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y), ImVec2(screenpos.x, screenpos.y + 5.0f), ImColor(255, 255, 255));
			}
		}
	}
}

void Trainer_RenderUI()
{
	ImColor icn_color = ImColor(230, 15, 95);

	ImColor active = ImColor(150, 10, 40, 255);
	ImColor inactive = ImColor(31, 30, 31, 0);

	auto button_size = ImVec2(135, 31);

	// Patches
	ImGui_TrainerTabButton("##patches", "Patches", active, inactive, TrainerTab::Patches, ICON_FA_DESKTOP, icn_color, IM_COL32_WHITE, button_size);

	// Hotkeys
	ImGui_TrainerTabButton("##hotkeys", "Hotkeys", active, inactive, TrainerTab::Hotkeys, ICON_FA_LAMBDA, icn_color, IM_COL32_WHITE, button_size);

	// FlagEdit
	ImGui_TrainerTabButton("##flagedit", "Flag Editor", active, inactive, TrainerTab::FlagEdit, ICON_FA_FLAG, icn_color, IM_COL32_WHITE, button_size);

	// EmMgr
	ImGui_TrainerTabButton("##emmgr", "Em Manager", active, inactive, TrainerTab::EmMgr, ICON_FA_SNOWMAN, icn_color, IM_COL32_WHITE, button_size);
	
	// EmMgr
	ImGui_TrainerTabButton("##esp", "ESP", active, inactive, TrainerTab::ESP, ICON_FA_SUITCASE, icn_color, IM_COL32_WHITE, button_size);

	// DebugTools
	ImGui_TrainerTabButton("##dbgtools", "Debug Tools", active, inactive, TrainerTab::DebugTools, ICON_FA_VIRUS, icn_color, IM_COL32_WHITE, button_size);

	// Globals
	if (ImGui_TrainerTabButton("##globals", "Globals", active, inactive, TrainerTab::NumTabs, ICON_FA_GLOBE, icn_color, IM_COL32_WHITE, button_size, false))
	{
		UI_NewGlobalsViewer();
	}

	// Last param to ImGui_TrainerTabButton must be false if it is the last button, to avoid the separator below from being SameLine'd as well.
	ImGui_ItemSeparator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::BeginChild("right side", ImVec2(0, 0));

	if (CurTrainerTab == TrainerTab::Patches)
	{
		if (ImGui::BeginTable("TrainerPatches", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// Misc
			{
				ImGui_ColumnSwitch();

				ImGui::Spacing();

				if (ImGui_ButtonSameLine("Heal player", true, -70.0f))
					GlobalPtr()->playerHpCur_4FB4 = GlobalPtr()->playerHpMax_4FB6;

				if (ImGui_ButtonSameLine("Heal Ashley", false))
					GlobalPtr()->subHpCur_4FB8 = GlobalPtr()->subHpMax_4FBA;

				ImGui::Spacing();

				if (ImGui::Button("Move Ashley to player"))
					MoveAshleyToPlayer();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::SetNextItemWidth(210.0f);
				ImGui::InputInt("Pesetas", &GlobalPtr()->goldAmount_4FA8);

				ImGui::Dummy(ImVec2(10, 10));

				// Max HP sliders
				{
					int Plr_maxHP;
					int Ash_maxHP;

					Plr_maxHP = GlobalPtr()->playerHpMax_4FB6;
					Ash_maxHP = GlobalPtr()->subHpMax_4FBA;

					ImGui::SliderInt("Player max HP", &Plr_maxHP, 1, 2400);
					if (ImGui::IsItemEdited())
					{
						GlobalPtr()->playerHpMax_4FB6 = Plr_maxHP;
						GlobalPtr()->playerHpCur_4FB4 = Plr_maxHP;
					}

					ImGui::Spacing();

					ImGui::SliderInt("Ashley max HP", &Ash_maxHP, 1, 1200);
					if (ImGui::IsItemEdited())
					{
						GlobalPtr()->subHpMax_4FBA = Ash_maxHP;
						GlobalPtr()->subHpCur_4FB8 = Ash_maxHP;
					}
				}
				
				ImGui::Dummy(ImVec2(10, 10));

				if (ImGui::Button("Save game"))
					RequestSaveGame();

				if (ImGui::Button("Open Merchant"))
					RequestMerchant();
			}

			// Invincibility
			{
				ImGui_ColumnSwitch();
				bool invincibility = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
				if (ImGui::Checkbox("Invincibility", &invincibility))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2), invincibility);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Prevents taking hits from enemies & automatically skips grabs.");
			}

			// Weak Enemies
			{
				ImGui_ColumnSwitch();

				bool weakEnemies = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK));
				if (ImGui::Checkbox("Weak Enemies", &weakEnemies))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK), weakEnemies);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Makes most enemies die in 1 hit.");
			}

			// Notarget
			{
				ImGui_ColumnSwitch();

				bool noTarget = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_NO_ATK));
				if (ImGui::Checkbox("Disable Enemy Targeting", &noTarget))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_NO_ATK), noTarget);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Prevents enemies from targeting player character, unless player attacks them. (only for enemies using Em10 right now)");
			}

			// Inf Ammo
			{
				ImGui_ColumnSwitch();

				bool infAmmo = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
				if (ImGui::Checkbox("Infinite Ammo", &infAmmo))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET), infAmmo);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Prevents game from removing bullets after firing.");
			}

			// Numpad Movement
			{
				ImGui_ColumnSwitch();

				bool DisablePlayerCollision = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
				if (ImGui::Checkbox("Disable Player Collision", &DisablePlayerCollision))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), DisablePlayerCollision);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Disables collision (no-clip).");

				ImGui::Spacing();

				ImGui::BeginDisabled(!DisablePlayerCollision);
				if (ImGui::Checkbox("Numpad Movement", &pConfig->bTrainerUseNumpadMovement))
					pConfig->HasUnsavedChanges = true;
				ImGui::TextWrapped("Allows noclip movement via numpad.");

				ImGui::Spacing();

				if (ImGui::Checkbox("Use Mouse Wheel", &pConfig->bTrainerUseMouseWheelUpDown))
					pConfig->HasUnsavedChanges = true;
				ImGui::TextWrapped("Allows using the mouse wheel to go up and down.");

				ImGui::Dummy(ImVec2(10, 10));

				if (ImGui::SliderFloat("Speed##numpmov", &pConfig->fTrainerNumMoveSpeed, 0.0f, 10.0f, "%.2f"))
					pConfig->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##numpmov"))
				{
					pConfig->fTrainerNumMoveSpeed = 1.0f;
					pConfig->HasUnsavedChanges = true;
				}
				ImGui::EndDisabled();
			}
			
			// Free camera
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enable Free Camera", &pConfig->bTrainerEnableFreeCam))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Allows flying around with the camera. Controller and Keybard/Mouse are supported.");

				ImGui::Dummy(ImVec2(10, 10));
				ImGui::TextWrapped("Controls:");

				ImGui::Spacing();
				ImGui::TextWrapped("Keyboard/Mouse:");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Movement keys (WASD): Move forwards/backwards/sideways");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Mouse movement: Rotate camera");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Mouse wheel: Move up/down");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Run key: Speedup movement");

				ImGui::Dummy(ImVec2(10, 10));
				ImGui::TextWrapped("Controller:");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Left analog stick: Move forwards/backwards/sideways");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Right analog stick: Rotate camera");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("\"Aim Weapon\" and \"Aim Knife\" keys: Move up/down");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Run key: Speedup movement");

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::BeginDisabled(!pConfig->bTrainerEnableFreeCam);
				if (ImGui::SliderFloat("Speed##freecam", &pConfig->fTrainerFreeCamSpeed, 0.0f, 10.0f, "%.2f"))
					pConfig->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##fcspdoverrid"))
				{
					pConfig->fTrainerFreeCamSpeed = 1.0f;
					pConfig->HasUnsavedChanges = true;
				}

				ImGui::Dummy(ImVec2(10, 10));

				if (ImGui::Button("Teleport player to camera##freecam"))
					bRequestedPlayerTPtoCam = true;

				ImGui::SameLine();

				if (ImGui::Button("Teleport Ashley to camera##freecam"))
					bRequestedAshleyTPtoCam = true;

				ImGui::EndDisabled();
			}

			// Speed override
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enable Player Speed Override", &pConfig->bTrainerPlayerSpeedOverride))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Allows overriding player speed value.");
				ImGui::TextWrapped("Ctrl+Click to input a custom value.");

				ImGui::Spacing();

				ImGui::BeginDisabled(!pConfig->bTrainerPlayerSpeedOverride);
				if (ImGui::SliderFloat("Speed", &pConfig->fTrainerPlayerSpeedOverride, 0.0f, 50.0f, "%.2f"))
					pConfig->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##spdoverrid"))
				{
					pConfig->fTrainerPlayerSpeedOverride = 1.0f;
					pConfig->HasUnsavedChanges = true;
				}
				ImGui::EndDisabled();
			}

			// Ashley presence
			{
				ImGui_ColumnSwitch();

				ImGui::Spacing();
				ImGui::TextWrapped("Override Ashley's Presence");

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::RadioButton("Area Default", &AshleyStateOverride, AshleyState::Default);
				ImGui::RadioButton("Present", &AshleyStateOverride, AshleyState::Present);
				ImGui::RadioButton("Not present", &AshleyStateOverride, AshleyState::NotPresent);
			}

			// Enemy HP multiplier
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enemy HP Multiplier", &pConfig->bTrainerEnemyHPMultiplier))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Allows overriding the HP of enemies.");

				ImGui::BeginDisabled(!pConfig->bTrainerEnemyHPMultiplier || pConfig->bTrainerRandomHPMultiplier);

				ImGui::TextWrapped("The new HP will be whatever their original HP was, multiplied by the value set here.");
				ImGui::TextWrapped("Ctrl+Click to input a custom value.");

				ImGui::Spacing();

				if (ImGui::SliderFloat("HP Multiplier", &pConfig->fTrainerEnemyHPMultiplier, 0.1f, 15.0f, "%.2f"))
					pConfig->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##hpmulti"))
				{
					pConfig->fTrainerEnemyHPMultiplier = 1.0f;
					pConfig->HasUnsavedChanges = true;
				}
				ImGui::EndDisabled();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::BeginDisabled(!pConfig->bTrainerEnemyHPMultiplier);
				pConfig->HasUnsavedChanges |= ImGui::Checkbox("Use random HP multiplier", &pConfig->bTrainerRandomHPMultiplier);
				ImGui::EndDisabled();
				ImGui::BeginDisabled(!pConfig->bTrainerEnemyHPMultiplier || !pConfig->bTrainerRandomHPMultiplier);
				ImGui::TextWrapped("Randomly pick the HP multiplier of each enemy.");
				ImGui::TextWrapped("You can also set the minimum and maximum values that can be generated.");
				ImGui::TextWrapped("Ctrl+Click to input a custom value.");

				ImGui::Spacing();

				if (ImGui::SliderFloat("Random Min", &pConfig->fTrainerRandomHPMultiMin, 0.1f, 15.0f, "%.2f"))
				{
					if (pConfig->fTrainerRandomHPMultiMin > pConfig->fTrainerRandomHPMultiMax)
						pConfig->fTrainerRandomHPMultiMin = pConfig->fTrainerRandomHPMultiMax;

					pConfig->HasUnsavedChanges = true;
				}

				if (ImGui::SliderFloat("Random Max", &pConfig->fTrainerRandomHPMultiMax, 0.1f, 15.0f, "%.2f"))
				{
					if (pConfig->fTrainerRandomHPMultiMax < pConfig->fTrainerRandomHPMultiMin)
						pConfig->fTrainerRandomHPMultiMax = pConfig->fTrainerRandomHPMultiMin;

					pConfig->HasUnsavedChanges = true;
				}

				ImGui::EndDisabled();
			}

			// DisableEnemySpawn
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Disable enemy spawn", &pConfig->bTrainerDisableEnemySpawn))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Make it so enemies don't spawn.");
				ImGui::TextWrapped("May cause crashes during events/cutscenes that expect enemies to be present! Use with caution!");
			}

			// Dead bodies never disappear
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Dead bodies never disappear", &pConfig->bTrainerDeadBodiesNeverDisappear))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Make it so enemies never disappear/despawn when killed.");
			}

			// AllowEnterDoorsWithoutAsh
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Allow entering doors without Ashley", &pConfig->bTrainerAllowEnterDoorsWithoutAsh))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Allows the player to go through doors even if Ashley is far away.");
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	if (CurTrainerTab == TrainerTab::Hotkeys)
	{
		if (ImGui::BeginTable("TrainerHotkeysTop", 1, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui::TableNextColumn();

			static float bgHeight = 0;
			ImGui_ItemBG(bgHeight, itmbgColor);

			ImGui::TextWrapped("Key combinations for trainer-related toggles.");
			ImGui::TextWrapped("Most keys can be combined (requiring multiple to be pressed at the same time). To combine, hold one key and press another at the same time.");
			ImGui::TextWrapped("(Press \"Save\" for changes to take effect.)");

			bgHeight = ImGui::GetCursorPos().y;

			ImGui::EndTable();
		}

		ImGui::Dummy(ImVec2(0.0f, 10.0f));

		if (ImGui::BeginTable("TrainerHotkeys", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// UI Focus 
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to set mouse focus on trainer related UIs (EmManager/Globals)");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(1);
				if (ImGui::Button(pConfig->sTrainerFocusUIKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerFocusUIKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Focus/unfocus trainer UIs");
			}

			// No-clip
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Disable Player Collision\" / no-clip patch.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(2);
				if (ImGui::Button(pConfig->sTrainerNoclipKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerNoclipKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle no-clip");
			}

			// Free cam
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Free Camera\"patch.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(3);
				if (ImGui::Button(pConfig->sTrainerFreeCamKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerFreeCamKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle free cam");
			}

			// Speed override
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Player Speed Override\" patch.");
				ImGui::TextWrapped("(set your player speed override value in the Patches section first)");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(4);
				if (ImGui::Button(pConfig->sTrainerSpeedOverrideKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerSpeedOverrideKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle speed override");
			}

			// Weapon Hotkeys
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enable Weapon Hotkeys", &pConfig->bWeaponHotkeysEnable))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Allows switching weapons via hotkeys rather than needing to use inventory.");
				ImGui::TextWrapped("By default hotkeys will cycle between weapons of different types, they can also be assigned to a weapon directly by highlighting the weapon in inventory and pressing the hotkey on it.");
				ImGui::Spacing();

				ImGui::BeginDisabled(!pConfig->bWeaponHotkeysEnable);

				for (int i = 0; i < 5; i++)
				{
					ImGui::PushID(250 + i);
					if (ImGui::Button(pConfig->sWeaponHotkeys[i].c_str(), ImVec2(150, 0)))
					{
						pConfig->HasUnsavedChanges = true;
						CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sWeaponHotkeys[i], 0, NULL);
					}
					ImGui::PopID();

					ImGui::SameLine();
					std::string text = "Weapon Slot " + std::to_string(i + 1);
					ImGui::TextWrapped(text.c_str());
				}

				ImGui::PushID(200);
				if (ImGui::Button(pConfig->sLastWeaponHotkey.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sLastWeaponHotkey, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();
				ImGui::TextWrapped("Last used weapon");

				ImGui::Text("Assigned hotkey weapon IDs: %d %d %d %d %d",
					pConfig->iWeaponHotkeyWepIds[0], pConfig->iWeaponHotkeyWepIds[1], pConfig->iWeaponHotkeyWepIds[2], pConfig->iWeaponHotkeyWepIds[3], pConfig->iWeaponHotkeyWepIds[4]);

				if (ImGui::Button("Reset assignments (use weapon cycling)"))
				{
					pConfig->iWeaponHotkeyWepIds[0] = pConfig->iWeaponHotkeyWepIds[1] = pConfig->iWeaponHotkeyWepIds[2] = pConfig->iWeaponHotkeyWepIds[3] = pConfig->iWeaponHotkeyWepIds[4] = 0;
					pConfig->HasUnsavedChanges = true;
				}

				ImGui::EndDisabled();
			}


			// Move Ash to Player
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to move Ashley to the player's position.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(5);
				if (ImGui::Button(pConfig->sTrainerMoveAshToPlayerKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerMoveAshToPlayerKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Move Ashley to Player");
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	if (CurTrainerTab == TrainerTab::FlagEdit)
	{
		struct CategoryInfo
		{
			const char* categoryName;
			uint32_t* values;
			const char** valueNames;
			std::unordered_map<int, std::string>* valueDescriptions;
			int numValues;
		};

		// TODO: ROOM, ROOM_SAVE
		CategoryInfo flagCategoryInfo[] = {
			{ "DEBUG", GlobalPtr()->flags_DEBUG_0_60, Flags_DEBUG_Names, &Flags_DEBUG_Descriptions, 128 },
			{ "STOP", GlobalPtr()->flags_STOP_0_170, Flags_STOP_Names, &Flags_STOP_Descriptions, 32 },
			{ "STATUS", GlobalPtr()->flags_STATUS_0_501C, Flags_STATUS_Names, &Flags_STATUS_Descriptions, 128 },
			{ "SYSTEM", GlobalPtr()->Flags_SYSTEM_0_54, Flags_SYSTEM_Names, &Flags_SYSTEM_Descriptions, 32 },
			{ "ITEM_SET", GlobalPtr()->flags_ITEM_SET_0_528C, Flags_ITEM_SET_Names, &Flags_ITEM_SET_Descriptions, 150 },
			{ "SCENARIO", GlobalPtr()->flags_SCENARIO_0_52CC, Flags_SCENARIO_Names, &Flags_SCENARIO_Descriptions, 195 },
			{ "KEY_LOCK", GlobalPtr()->flags_KEY_LOCK_52EC, Flags_KEY_LOCK_Names, &Flags_KEY_LOCK_Descriptions, 150 },
			{ "DISP", GlobalPtr()->Flags_DISP_0_58, Flags_DISP_Names, &Flags_DISP_Descriptions, 32 },
			{ "EXTRA", SystemSavePtr()->flags_EXTRA_4, Flags_EXTRA_Names, &Flags_EXTRA_Descriptions, 21 },
			{ "CONFIG", SystemSavePtr()->flags_CONFIG_0, Flags_CONFIG_Names, &Flags_CONFIG_Descriptions, 6 },
		};

		static bool filter_descriptions_only = true;

		static char searchText[256] = { 0 };
		static int columns = 3;
		static int flagCategory = 0;
		CategoryInfo* curFlagCategory = nullptr;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0f, 5.0f));
		if (ImGui::BeginTable("##flagtoolheader", 2, ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 70)))
		{
			ImGui::TableNextColumn();

			ImGui::PushItemWidth(120.0f * pConfig->fFontSizeScale);
			if (ImGui::BeginCombo("Category", flagCategoryInfo[flagCategory].categoryName))
			{
				for (int i = 0; i < (sizeof(flagCategoryInfo) / sizeof(CategoryInfo)); i++)
				{
					bool selected = false;
					if (ImGui::Selectable(flagCategoryInfo[i].categoryName, &selected))
						if (selected)
							flagCategory = i;
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			curFlagCategory = &flagCategoryInfo[flagCategory];

			ImGui::SameLine();
			ImGui::Checkbox("Functional only", &filter_descriptions_only);

			ImGui::PushItemWidth(220.0f * pConfig->fFontSizeScale);
			ImGui::InputText("Search", searchText, 256);
			ImGui::PopItemWidth();

			ImGui::TableNextColumn();

			ImGui::Text("Columns: %i", columns);

			if (ImGui::Button("Remove column"))
				columns--;

			ImGui::SameLine();

			if (ImGui::Button("Add column"))
				columns++;

			columns = std::clamp(columns, 1, 6);

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		// make search uppercase to make case insensitive search easier...
		std::string searchTextUpper = StrToUpper(searchText);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0f, 5.0f));
		if (ImGui::BeginTable("##flags", columns, ImGuiTableFlags_ScrollY))
		{
			if (curFlagCategory)
				for (int i = 0; i < curFlagCategory->numValues; i++)
				{
					const char* description = nullptr;
					if (curFlagCategory->valueDescriptions->count(i))
						description = (*curFlagCategory->valueDescriptions)[i].c_str();

					bool makeVisible = true;
					if (!searchTextUpper.empty())
					{
						std::string flagNameUpper = StrToUpper(curFlagCategory->valueNames[i]);

						makeVisible = flagNameUpper.find(searchTextUpper) != std::string::npos;

						if (!makeVisible && description)
						{
							std::string descriptionUpper = StrToUpper(description);
							makeVisible = descriptionUpper.find(searchTextUpper) != std::string::npos;
						}
					}

					// Filter flag list to only ones that we've provided descriptions for (meaning they're functional)
					// Except for SCENARIO / ITEM_SET / KEY_LOCK / CONFIG / EXTRA flags
					// (we'll always display them since they're all useful AFAIK)
					if (filter_descriptions_only)
						if (flagCategory != 4 && flagCategory != 5 && flagCategory != 6 && flagCategory != 8 && flagCategory != 9)
							if (!description)
								makeVisible = false;

					if (makeVisible)
					{
						ImGui::TableNextColumn();
						bool selected = FlagIsSet(curFlagCategory->values, i);

						ImU32 col = ImGui::GetColorU32(ImGuiCol_Text);
						if (!description)
							col = ImGui::GetColorU32(ImGuiCol_Text, 0.70f);
						else if (strstr(description, "reimplementation"))
							col = IM_COL32(243, 229, 171, 255); // vanilla gold

						ImGui::PushStyleColor(ImGuiCol_Text, col);
						if (ImGui::Checkbox(curFlagCategory->valueNames[i], &selected))
							FlagSet(curFlagCategory->values, i, selected);
						ImGui::PopStyleColor();

						if (description)
							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
								ImGui::SetTooltip(description);
					}
				}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();
	}

	if (CurTrainerTab == TrainerTab::EmMgr)
	{
		static auto EmMgr = new UI_EmManager();
		EmMgr->Render(false);
	}

	if (CurTrainerTab == TrainerTab::ESP)
	{
		if (ImGui::BeginTable("ESP", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// Screen info
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Show ESP", &pConfig->bShowESP))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::BeginDisabled(!pConfig->bShowESP);

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Displays information on Ems");
				ImGui::Spacing();

				ImGui::Checkbox("Show info on top of Ems", &pConfig->bEspShowInfoOnTop);
				ImGui::Checkbox("Only show enemies", &pConfig->bEspOnlyShowEnemies);
				ImGui::Checkbox("Only show alive", &pConfig->bEspOnlyShowAlive);
				ImGui::Checkbox("Only show valid Ems", &pConfig->bEspOnlyShowValidEms);
				ImGui::Checkbox("Only show ESL-spawned##esp", &pConfig->bEspOnlyShowESLSpawned);
				ImGui::Checkbox("Draw Lines", &pConfig->bEspDrawLines);
				ImGui::Checkbox("Draw Debug Info", &pConfig->bEspDrawDebugInfo);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Name display mode:");
				ImGui::RadioButton("Don't show##nameesp", &pConfig->iEspEmNameMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Normal##esp", &pConfig->iEspEmNameMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Simplified##esp", &pConfig->iEspEmNameMode, 2);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("HP display mode:");
				ImGui::RadioButton("Don't show##hpesp", &pConfig->iEspEmHPMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Bar##esp", &pConfig->iEspEmHPMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Text##esp", &pConfig->iEspEmHPMode, 2);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max distance of Ems").x - 10);
				ImGui::SliderFloat("Max distance of Ems##esp", &pConfig->fEspMaxEmDistance, 0.0f, 200000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::Checkbox("Only show closest Ems", &pConfig->bEspOnlyShowClosestEms);
				ImGui::Spacing();

				ImGui::BeginDisabled(!pConfig->bEspOnlyShowClosestEms);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max amount of Ems").x - 10);
				ImGui::InputInt("Max amount of Ems##esp", &pConfig->iEspClosestEmsAmount);
				ImGui::EndDisabled();

				ImGui::EndDisabled();
			}

			// Side info
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Show side info", &pConfig->bShowSideInfo))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::BeginDisabled(!pConfig->bShowSideInfo);

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Displays information on the left side of the screen.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::Checkbox("Show Em count", &pConfig->bSideShowEmCount);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::Checkbox("Show Em list", &pConfig->bSideShowEmList);
				ImGui::TextWrapped("Ems are listed from bottom to top, with the top one being the closes to the player, and the bottom one being the farthest.");
				ImGui::Dummy(ImVec2(10, 10));
				
				ImGui::BeginDisabled(!pConfig->bSideShowEmList);
				ImGui::Checkbox("Only show ESL-spawned##side", &pConfig->bSideOnlyShowESLSpawned);
				ImGui::Checkbox("Show simplified names##side", &pConfig->bSideShowSimpleNames);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("HP display mode:");
				ImGui::RadioButton("Don't show##side", &pConfig->iSideEmHPMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Bar##side", &pConfig->iSideEmHPMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Text##side", &pConfig->iSideEmHPMode, 2);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max amount of enemies").x - 10);
				ImGui::Spacing();
				ImGui::InputInt("Max amount of enemies##side", &pConfig->iSideClosestEmsAmount);

				ImGui::Spacing();
				ImGui::SliderFloat("Max distance of enemies##side", &pConfig->fSideMaxEmDistance, 0.0f, 200000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();
				ImGui::EndDisabled();

				ImGui::EndDisabled();
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	if (CurTrainerTab == TrainerTab::DebugTools)
	{
		if (ImGui::BeginTable("DebugTools", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// Area jump
			{
				ImGui_ColumnSwitch();

				ImGui::Spacing();
				ImGui::TextWrapped("%s  Area Jump", ICON_FA_SIGN);

				ImGui::SameLine();
				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Open new Area Jump window").x - 15.0f, 0.0f));
				ImGui::SameLine();
				if (ImGui::Button("Open new Area Jump window"))
					UI_NewAreaJump();

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				static auto AreaJmp = new UI_AreaJump();
				AreaJmp->Render(false);
			}
				
			// FilterTool
			{
				ImGui_ColumnSwitch();

				ImGui::Spacing();
				ImGui::TextWrapped("%s  Filter Tool", ICON_FA_CAMERA);

				ImGui::SameLine();
				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Open new Filter Tool window").x - 15.0f, 0.0f));
				ImGui::SameLine();
				if (ImGui::Button("Open new Filter Tool window"))
					UI_NewFilterTool();

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10));

				static auto FilterTool = new UI_FilterTool();
				FilterTool->Render(false);
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	ImGui::EndChild();
}