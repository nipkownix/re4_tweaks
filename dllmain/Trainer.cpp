#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"
#include "imgui.h"
#include <FAhashes.h>
#include "UI_DebugWindows.h"
#include "Trainer.h"
#include <DirectXMath.h>

int AshleyStateOverride;
int last_weaponId;

// Side info
bool ShowSideInfo = false;
bool SideShowEmCount = true;
bool SideShowEmList = true;
bool SideOnlyShowESLSpawned = false;
int SideClosestEmsAmount = 5;
float SideMaxEmDistance = 30000.0f;
int SideInfoEmHPMode = 1;

// ESP
bool ShowESP = false;
bool EspOnlyShowEnemies = true;
bool EspOnlyShowValidEms = true;
bool EspOnlyShowESLSpawned = false;
bool EspOnlyShowAlive = true;
bool EspOnlyShowClosestEms = false;
int EspClosestEmsAmount = 3;
float EspMaxEmDistance = 30000.0f;
bool EspDrawLines = false;
int EspEmHPMode = 1;
bool EspDrawDebugInfo = false;

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

uint16_t ash_atariInfoFlagBackup = 0;
bool ash_atariInfoFlagSet = false;
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
std::vector<uint32_t> KeyComboSpeedOverride;
std::vector<uint32_t> KeyComboAshToPlayer;

std::vector<uint32_t> KeyComboWeaponHotkey[5];
std::vector<uint32_t> KeyComboLastWeaponHotkey;

void HotkeySlotPressed(int slotIdx, bool forceUseWepID = false)
{
	if (!pConfig->bWeaponHotkeysEnable)
		return;

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

				con.AddLogChar("emlist2");

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

	// Handle Ashley's collision
	cPlayer* ashley = AshleyPtr();
	if (ashley)
	{
		// Restore collision flags
		if (ash_atariInfoFlagSet)
		{
			ashley->atari_2B4.m_flag_1A = ash_atariInfoFlagBackup;
			ash_atariInfoFlagSet = false;
		}
	}

	static uint16_t atariInfoFlagBackup = 0;
	static bool atariInfoFlagSet = false;
	if (FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT)))
	{
		// Backup original collision flags and then unset collision-enabled bit
		if (!atariInfoFlagSet)
		{
			atariInfoFlagBackup = player->atari_2B4.m_flag_1A;
			atariInfoFlagSet = true;
		}
		player->atari_2B4.m_flag_1A &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE); // map collision | em collision

		Vec positionMod{ 0 };
		bool positionModded = false;

		// Handle numpad-movement
		if (pConfig->bTrainerUseNumpadMovement)
		{
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
		}

		// Handle mouse wheel
		if (pConfig->bTrainerUseMouseWheelUpDown)
		{
			if (pInput->mouse_wheel_delta() > 0) // Mouse wheel up
			{
				positionMod.y = pInput->mouse_wheel_delta() * 200.0f;
				positionModded = true;
			}

			if (pInput->mouse_wheel_delta() < 0) // Mouse wheel down
			{
				positionMod.y = -pInput->mouse_wheel_delta() * -200.0f;
				positionModded = true;
			}
		}

		if (positionModded)
		{
			player->pos_94.x += positionMod.x;
			player->pos_94.y += positionMod.y;
			player->pos_94.z += positionMod.z;
		}
	}
	else if (atariInfoFlagSet) // Restore collision flags
	{
		player->atari_2B4.m_flag_1A = atariInfoFlagBackup;
		atariInfoFlagSet = false;
	}
}

void Trainer_ESP()
{
	if (!(ShowSideInfo || ShowESP))
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
	if (ShowSideInfo)
	{
		// Drw Em count
		if (SideShowEmCount)
		{
			std::stringstream emCnt;
			emCnt << "Em Count: " << emMgr.count_valid() << " | " << "Max: " << emMgr.count();

			ImGui::GetBackgroundDrawList()->AddText(ImVec2(20, 20), ImColor(255, 255, 255, 255), emCnt.str().c_str());
		}

		// Draw Em list
		if (SideShowEmList)
		{
			std::vector<cEm*> EnemiesVec = cEmMgr::GetVecClosestEms(SideClosestEmsAmount, SideMaxEmDistance, true, true, SideOnlyShowESLSpawned, true);
			std::reverse(EnemiesVec.begin(), EnemiesVec.end());

			float InfoOffsetY = 60.0f;
			for (auto& em : EnemiesVec)
			{
				char EmName[256];
				sprintf(EmName, "#%d %s (type %x)", em->guid_F8, cEmMgr::EmIdToName(em->id_100).c_str(), int(em->type_101));

				ImGui::GetBackgroundDrawList()->AddText(ImVec2(10, screen_height - InfoOffsetY), ImColor(255, 255, 255, 255), EmName);

				if ((SideInfoEmHPMode != 0) && (em->hp_max_326 > 0))
				{
					float hp_percent = 0;
					float hp_max = (float)em->hp_max_326;

					if (hp_max < em->hp_324)
						hp_max = (float)em->hp_324;

					hp_percent = (float)em->hp_324 * 100.0f / hp_max;


					// HP Bar
					if (SideInfoEmHPMode == 1)
					{
						float barScalex = 1.0f;

						ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(30, screen_height - InfoOffsetY + 28.0f), ImVec2(30.0f + 100.0f * barScalex, screen_height - InfoOffsetY + 18.0f), ImColor(105, 105, 105));
						ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(30, screen_height - InfoOffsetY + 28.0f), ImVec2(30.0f + hp_percent * barScalex, screen_height - InfoOffsetY + 18.0f), getHealthColor(hp_percent));

						InfoOffsetY += 12;
					}
					else if (SideInfoEmHPMode == 2) // HP Text
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
	if (ShowESP)
	{
		std::vector<cEm*> EmsVector;
		if (EspOnlyShowClosestEms)
			EmsVector = cEmMgr::GetVecClosestEms(EspClosestEmsAmount, EspMaxEmDistance, EspOnlyShowValidEms, EspOnlyShowEnemies, EspOnlyShowESLSpawned, EspOnlyShowAlive);
		else
			EmsVector = cEmMgr::GetVecClosestEms(emMgr.count(), EspMaxEmDistance, EspOnlyShowValidEms, EspOnlyShowEnemies, EspOnlyShowESLSpawned, EspOnlyShowAlive);

		for (auto& em : EmsVector)
		{
			auto coords = em->pos_94;

			if (WorldToScreen(coords, screenpos, pG->Camera_74.v_mat_30, pG->Camera_74.CamPoint_A4.Fovy_1C, screen_width, screen_height))
			{
				// Draw Em Name
				char EmName[256];
				sprintf(EmName, "#%d %s (type %x)", em->guid_F8, cEmMgr::EmIdToName(em->id_100).c_str(), int(em->type_101));

				ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y), ImColor(255, 255, 255, 255), EmName);

				if ((EspEmHPMode != 0) && (em->hp_max_326 > 0))
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
						if (EspEmHPMode == 1)
						{
							float barScalex = 1.0f;

							ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(screenpos.x, screenpos.y + 20.0f), ImVec2(screenpos.x + 100.0f * barScalex, screenpos.y + 30.0f), ImColor(105, 105, 105));
							ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(screenpos.x, screenpos.y + 20.0f), ImVec2(screenpos.x + hp_percent * barScalex, screenpos.y + 30.0f), getHealthColor(hp_percent));
						}
						else // HP Text
						{
							if (em->hp_max_326 > 0)
							{
								char CurEmHp[256];
								sprintf(CurEmHp, "Cur HP: %d", em->hp_324);

								char MaxEmHp[256];
								sprintf(MaxEmHp, "Max HP: %d", em->hp_max_326);

								ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + 15), ImColor(getHealthColor(hp_percent)), CurEmHp);
								ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + 30), ImColor(getHealthColor(hp_percent)), MaxEmHp);
							}
						}
					}
				}

				if (EspDrawDebugInfo)
				{
					sprintf(EmName, "0x%p, Rno %d-%d-%d-%d", em, int(em->r_no_0_FC), int(em->r_no_1_FD), int(em->r_no_2_FE), int(em->r_no_3_FF));
					ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + 30), ImColor(255, 255, 255), EmName);
				}

				// Draw lines pointing to the Em
				if (EspDrawLines)
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y), ImVec2(screenpos.x, screenpos.y), ImColor(255, 255, 255));
			}
		}
	}
}

enum class TrainerTab
{
	Patches,
	Hotkeys,
	FlagEdit,
	EmMgr,
	ESP,
	DebugTools,
	NumTabs
};
TrainerTab CurTrainerTab = TrainerTab::Patches;

// Helper funcs from cfgMenu.cpp
// TODO: move these to seperate cpp/h to help make cfgMenu less crowded?
void ImGui_ItemSeparator();
void ImGui_ColumnInit();
void ImGui_ColumnSwitch();
void ImGui_ColumnFinish();
void ImGui_ItemBG(float RowSize, ImColor bgCol);
extern ImColor itmbgColor;
void SetHotkeyComboThread(std::string* cfgHotkey);

bool ImGui_ButtonSameLine(const char* label, bool samelinecheck = true, float offset = 0.0f, const ImVec2 size = ImVec2(0, 0))
{
	bool ret = ImGui::Button(label, size);

	if (samelinecheck)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		float window_visible_x2 = ImGui::GetCursorScreenPos().x + offset + ImGui::GetContentRegionAvail().x;
		float last_button_x2;
		float next_button_x2;

		last_button_x2 = ImGui::GetItemRectMax().x;
		next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x; // Expected position if next button was on same line
		if (next_button_x2 < window_visible_x2)
			ImGui::SameLine();
	}

	return ret;
}

bool ImGui_TrainerTabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, TrainerTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size = ImVec2(0, 0), const bool samelinecheck = true)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::PushStyleColor(ImGuiCol_Button, CurTrainerTab == tabID ? activeCol : inactiveCol);
	bool ret = ImGui_ButtonSameLine(btnID, samelinecheck, 0.0f, size);
	ImGui::PopStyleColor();

	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();

	float textWidth = ImGui::CalcTextSize(text).x;
	float textHeight = ImGui::CalcTextSize(text).y;
	float iconWidth = ImGui::CalcTextSize(icon).x;

	float x_text_offset = iconWidth + ((size.x - textWidth) * 0.5f);
	float y_text_offset = (size.y - textHeight) * 0.5f;

	float x_icon_offset = x_text_offset - iconWidth;
	float y_icon_offset = y_text_offset + 2.0f;

	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + x_icon_offset - 14.0f, p0.y + y_icon_offset), iconColor, icon, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + x_text_offset - 7.0f, p0.y + y_text_offset), IM_COL32_WHITE, text, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));

	if (ret && tabID != TrainerTab::NumTabs)
		CurTrainerTab = tabID;

	return ret;
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

				/* Not working?
				if (ImGui::Button("Save game"))
					RequestSaveGame();
				*/

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
					ImGui::PushID(2 + i);
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

				ImGui::PushID(25);
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

			// No-clip
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Disable Player Collision\" / no-clip patch.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(7);
				if (ImGui::Button(pConfig->sTrainerNoclipKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerNoclipKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle no-clip");
			}

			// Speed override
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Player Speed Override\" patch.");
				ImGui::TextWrapped("(set your player speed override value in the Patches section first)");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(8);
				if (ImGui::Button(pConfig->sTrainerSpeedOverrideKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerSpeedOverrideKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle speed override");
			}

			// Move Ash to Player
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to move Ashley to the player's position.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(9);
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
			int numValues;
		};

		// TODO: ROOM, ROOM_SAVE
		CategoryInfo flagCategoryInfo[] = {
			{ "DEBUG", GlobalPtr()->flags_DEBUG_0_60, Flags_DEBUG_Names, 128 },
			{ "STOP", GlobalPtr()->flags_STOP_0_170, Flags_STOP_Names, 32 },
			{ "STATUS", GlobalPtr()->flags_STATUS_0_501C, Flags_STATUS_Names, 128 },
			{ "SYSTEM", GlobalPtr()->Flags_SYSTEM_0_54, Flags_SYSTEM_Names, 32 },
			{ "ITEM_SET", GlobalPtr()->flags_ITEM_SET_0_528C, Flags_ITEM_SET_Names, 150 },
			{ "SCENARIO", GlobalPtr()->flags_SCENARIO_0_52CC, Flags_SCENARIO_Names, 195 },
			{ "KEY_LOCK", GlobalPtr()->flags_KEY_LOCK_52EC, Flags_KEY_LOCK_Names, 150 },
			{ "DISP", GlobalPtr()->Flags_DISP_0_58, Flags_DISP_Names, 32 },
			{ "EXTRA", SystemSavePtr()->flags_EXTRA_4, Flags_EXTRA_Names, 21 },
			{ "CONFIG", SystemSavePtr()->flags_CONFIG_0, Flags_CONFIG_Names, 6 },
		};

		static char searchText[256] = { 0 };
		static int columns = 3;
		CategoryInfo* curFlagCategory = nullptr;
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0f, 5.0f));
		if (ImGui::BeginTable("##flagtoolheader", 2, ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 70)))
		{
			ImGui::TableNextColumn();

			static int flagCategory = 0;
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
					bool makeVisible = true;
					if (!searchTextUpper.empty())
					{
						std::string flagNameUpper = StrToUpper(curFlagCategory->valueNames[i]);

						makeVisible = flagNameUpper.find(searchTextUpper) != std::string::npos;
					}

					if (makeVisible)
					{
						ImGui::TableNextColumn();
						bool selected = FlagIsSet(curFlagCategory->values, i);
						if (ImGui::Checkbox(curFlagCategory->valueNames[i], &selected))
							FlagSet(curFlagCategory->values, i, selected);
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
		if (ImGui::BeginTable("DebugTools", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// Screen info
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Show ESP", &ShowESP))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::BeginDisabled(!ShowESP);

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Displays information on Ems");
				ImGui::Spacing();

				ImGui::Checkbox("Only show enemies", &EspOnlyShowEnemies);
				ImGui::Checkbox("Only show alive", &EspOnlyShowAlive);
				ImGui::Checkbox("Only show valid Ems", &EspOnlyShowValidEms);
				ImGui::Checkbox("Only show ESL-spawned##esp", &EspOnlyShowESLSpawned);
				ImGui::Checkbox("Draw Lines", &EspDrawLines);
				ImGui::Checkbox("Draw Debug Info", &EspDrawDebugInfo);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("HP display mode:");
				ImGui::RadioButton("Don't show##side", &EspEmHPMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Bar##side", &EspEmHPMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Text##side", &EspEmHPMode, 2);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::Checkbox("Only show closest Ems", &EspOnlyShowClosestEms);
				ImGui::Spacing();

				ImGui::BeginDisabled(!EspOnlyShowClosestEms);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max ammount of Ems").x - 10);
				ImGui::InputInt("Max ammount of Ems##side", &EspClosestEmsAmount);
				ImGui::Spacing();

				ImGui::SliderFloat("Max distance of Ems##side", &EspMaxEmDistance, 0.0f, 200000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::PopItemWidth();
				ImGui::EndDisabled();

				ImGui::EndDisabled();
			}

			// Side info
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Show side info", &ShowSideInfo))
					pConfig->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::BeginDisabled(!ShowSideInfo);

				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("Displays information on the left side of the screen.");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::Checkbox("Show Em count", &SideShowEmCount);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::Checkbox("Show Em list", &SideShowEmList);
				ImGui::TextWrapped("Ems are listed from bottom to top, with the top one being the closes to the player, and the bottom one being the farthest.");
				ImGui::Dummy(ImVec2(10, 10));
				
				ImGui::BeginDisabled(!SideShowEmList);
				ImGui::Checkbox("Only show ESL-spawned##side", &SideOnlyShowESLSpawned);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::TextWrapped("HP display mode:");
				ImGui::RadioButton("Don't show##side", &SideInfoEmHPMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Bar##side", &SideInfoEmHPMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Text##side", &SideInfoEmHPMode, 2);
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max ammount of enemies").x - 10);
				ImGui::Spacing();
				ImGui::InputInt("Max ammount of enemies##side", &SideClosestEmsAmount);

				ImGui::Spacing();
				ImGui::SliderFloat("Max distance of enemies##side", &SideMaxEmDistance, 0.0f, 200000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
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