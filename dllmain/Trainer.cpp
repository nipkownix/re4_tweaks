#include <iostream>
#include <optional>
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

std::optional<uint32_t> FlagsExtraValue;

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
std::vector<uint32_t> KeyComboDebugTrg;

std::vector<uint32_t> KeyComboWeaponHotkey[5];
std::vector<uint32_t> KeyComboLastWeaponHotkey;

void HotkeySlotPressed(int slotIdx, bool forceUseWepID = false)
{
	if (!re4t::cfg->bWeaponHotkeysEnable)
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

	// Function used by game to determine whether inventory/subscreens can be opened
	// Swapping weapons would normally require going through inventory, so we should probably make use of this too
	// (fixes issues with swapping weapons during certain actions like jumping, which would interrupt the actions anim and usually place player outside of collision)
	if (!player->subScrCheck())
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
						re4t::cfg->iWeaponHotkeyWepIds[slotIdx] = ItemPiece->pItem_24->id_0;
						re4t::cfg->WriteSettings();
						
						// fall-thru to weapon switch code below, acts as an indicator that binding was set
					}
				}
			}
		}
	}

	// Not binding weapon, must be trying to switch to weapon instead
	int weaponId = re4t::cfg->iWeaponHotkeyWepIds[slotIdx];

	// slotIdx is the actual weaponId if forceUseWepID is true
	if (!forceUseWepID)
		weaponId = re4t::cfg->iWeaponHotkeyWepIds[slotIdx];
	else
		weaponId = slotIdx;

	if (!weaponId)
	{
		static size_t weaponCycleIndex[5] = { 0 };

		std::vector<int>& weaponCycle = re4t::cfg->iWeaponHotkeyCycle[slotIdx];

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
		KeyComboNoclipToggle = re4t::cfg->ParseKeyCombo(re4t::cfg->sTrainerNoclipKeyCombo);

		pInput->RegisterHotkey({ []() {
			bool playerCollisionDisabled = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
			FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), !playerCollisionDisabled);
		}, &KeyComboNoclipToggle });
	}

	// Free cam
	{
		KeyComboFreeCamToggle.clear();
		KeyComboFreeCamToggle = re4t::cfg->ParseKeyCombo(re4t::cfg->sTrainerFreeCamKeyCombo);

		pInput->RegisterHotkey({ []() {
			re4t::cfg->bTrainerEnableFreeCam = !re4t::cfg->bTrainerEnableFreeCam;
		}, &KeyComboFreeCamToggle });
	}

	// SpeedOverride
	{
		KeyComboSpeedOverride.clear();
		KeyComboSpeedOverride = re4t::cfg->ParseKeyCombo(re4t::cfg->sTrainerSpeedOverrideKeyCombo);

		pInput->RegisterHotkey({ []() {
			re4t::cfg->bTrainerPlayerSpeedOverride = !re4t::cfg->bTrainerPlayerSpeedOverride;
		}, &KeyComboSpeedOverride });
	}

	// Move Ash to player
	{
		KeyComboAshToPlayer.clear();
		KeyComboAshToPlayer = re4t::cfg->ParseKeyCombo(re4t::cfg->sTrainerMoveAshToPlayerKeyCombo);

		pInput->RegisterHotkey({ []() {
			MoveAshleyToPlayer();
		}, &KeyComboAshToPlayer });
	}

	// DebugTrg
	{
		KeyComboDebugTrg.clear();
		KeyComboDebugTrg = re4t::cfg->ParseKeyCombo(re4t::cfg->sTrainerDebugTrgKeyCombo);
	}

	// WeaponHotkey
	{
		for (int i = 0; i < 5; i++)
		{
			KeyComboWeaponHotkey[i].clear();
			KeyComboWeaponHotkey[i] = re4t::cfg->ParseKeyCombo(re4t::cfg->sWeaponHotkeys[i]);
		}

		{
			KeyComboLastWeaponHotkey.clear();
			KeyComboLastWeaponHotkey = re4t::cfg->ParseKeyCombo(re4t::cfg->sLastWeaponHotkey);
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

void(__cdecl* j_GameAddPoint_orig)(int type);
void __cdecl j_GameAddPoint_Hook(int type)
{
	if (re4t::cfg->bTrainerOverrideDynamicDifficulty)
	{
		GlobalPtr()->dynamicDifficultyPoints_4F94 = re4t::cfg->iTrainerDynamicDifficultyLevel * 1000 + 500;
		GlobalPtr()->dynamicDifficultyLevel_4F98 = re4t::cfg->iTrainerDynamicDifficultyLevel;
		return;
	}

	j_GameAddPoint_orig(type);
}

bool(__cdecl* cameraHitCheck_orig)(Vec *pCross, Vec *pNorm, Vec p0, Vec p1);
bool __cdecl cameraHitCheck_hook(Vec *pCross, Vec *pNorm, Vec p0, Vec p1)
{
	if (re4t::cfg->bTrainerEnableFreeCam)
		return false;
	else
		return cameraHitCheck_orig(pCross, pNorm, p0, p1);
}

bool ShowDebugTrgHint = false;

// DebugTrg reimpl based on 2007 port, thank you SourceNext!
// (no other publicly available builds appear to include this, even the debug builds...)
// Gamepad combo is LB + B + X
bool __cdecl DebugTrg_hook(uint32_t flag)
{
	if (!re4t::cfg->bTrainerEnableDebugTrg)
		return false;

	if (FlagIsSet(GlobalPtr()->Flags_SYSTEM_0_54, uint32_t(Flags_SYSTEM::SYS_PUBLICITY_VER)))
		return false; // disallowed if SYS_PUBLICITY_VER set, probably set on E3/TGS/trade show builds

	ShowDebugTrgHint = true;

	// if flag is set, then caller func seems to allow checking for previously held buttons
	// otherwise, buttons must have only been pressed this frame

	if (flag && pInput->is_combo_down(&KeyComboDebugTrg))
		return true;
	if (!flag && pInput->is_combo_pressed(&KeyComboDebugTrg))
		return true;

	JOY* Joy = JoyPtr();

	if ((Joy[0].on_14 & JOY_START) != 0)
		return false; // if start pressed, always return false

	if (flag)
	{
		if ((Joy[0].on_14 & JOY_LB) == 0 || (Joy[0].on_14 & JOY_B) == 0)
			return false; // LB or B not pressed/held, return false
		return ((Joy[0].on_14 & JOY_X) != 0); // return true if X also pressed/held
	}

	// If flag == 0, the same LB/B checks are used as above, but the X check is performed on Joy.trg instead of Joy.on
	// Seems trg shows buttons that were newly pressed on this frame, then gets cleared on subsequent frame even if held down
	// I guess to help ensure the function only runs once, on the frame when button was first pressed
	// (and not on any subsequent frames where it might still be held down)

	if ((Joy[0].on_14 & JOY_LB) == 0 || (Joy[0].on_14 & JOY_B) == 0)
		return false; // LB or B not pressed/held, return false
	return ((Joy[0].trg_18 & JOY_X) != 0); // return true if X was newly pressed
}

void Trainer_DrawDebugTrgHint()
{
	if (!bCfgMenuOpen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(ImVec2(20, viewport->Pos.y + 20));
		ImGui::SetNextWindowBgAlpha(0.5);

		ImGui::Begin("debugTrgHint", nullptr, ImGuiWindowFlags_NoDecoration |
			ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing);

		std::string tooltip = std::string("DebugTrg available!");

		ImGui::TextUnformatted(tooltip.data());

		ImGui::End();
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

				if (re4t::cfg->bTrainerEnemyHPMultiplier)
				{
					float multi = 0.0f;
					if (re4t::cfg->bTrainerRandomHPMultiplier)
						multi = GetRandomFloat(re4t::cfg->fTrainerRandomHPMultiMin, re4t::cfg->fTrainerRandomHPMultiMax);
					else
						multi = re4t::cfg->fTrainerEnemyHPMultiplier;

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

				if (re4t::cfg->bTrainerEnemyHPMultiplier)
				{
					float multi = 0.0f;
					if (re4t::cfg->bTrainerRandomHPMultiplier)
						multi = GetRandomFloat(re4t::cfg->fTrainerRandomHPMultiMin, re4t::cfg->fTrainerRandomHPMultiMax);
					else
						multi = re4t::cfg->fTrainerEnemyHPMultiplier;

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

				if (re4t::cfg->bTrainerEnemyHPMultiplier)
				{
					float multi = 0.0f;
					if (re4t::cfg->bTrainerRandomHPMultiplier)
						multi = GetRandomFloat(re4t::cfg->fTrainerRandomHPMultiMin, re4t::cfg->fTrainerRandomHPMultiMax);
					else
						multi = re4t::cfg->fTrainerEnemyHPMultiplier;

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

				if (!re4t::cfg->bTrainerDisableEnemySpawn && isEqual)
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

				if (re4t::cfg->bTrainerDisableEnemySpawn)
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

				if (!re4t::cfg->bTrainerDisableEnemySpawn && isEqual)
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

				if (re4t::cfg->bTrainerDeadBodiesNeverDisappear && !is11C)
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
				if (re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh)
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

				if (re4t::cfg->bTrainerEnableFreeCam)
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

				if (re4t::cfg->bTrainerEnableFreeCam)
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

	// Hook DebugTrg stub to use our reimplemented version
	{
		auto pattern = hook::pattern("83 C4 18 6A 01 E8 ? ? ? ? 83 C4 04 3C 01");
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0x5)).as_int(), DebugTrg_hook, PATCH_JUMP);
	}

	// Hook j_GameAddPoint to override the Dynamic Difficulty Level
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 E8 ? ? ? ? 0F B6 C8");
		ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), j_GameAddPoint_orig);
		InjectHook(injector::GetBranchDestination(pattern.get_first()).as_int(), j_GameAddPoint_Hook);
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
		if (prev_bPlayerSpeedOverride != re4t::cfg->bTrainerPlayerSpeedOverride)
		{
			if (re4t::cfg->bTrainerPlayerSpeedOverride)
			{
				// changed from disabled -> enabled, backup previous speed value
				prev_PlayerSpeed = player->Motion_1D8.Seq_speed_C0;
			}
			else if (!re4t::cfg->bTrainerPlayerSpeedOverride)
			{
				// changed from enabled -> disabled, restore previous speed value
				player->Motion_1D8.Seq_speed_C0 = prev_PlayerSpeed;
			}
			prev_bPlayerSpeedOverride = re4t::cfg->bTrainerPlayerSpeedOverride;
		}

		if (re4t::cfg->bTrainerPlayerSpeedOverride)
			player->Motion_1D8.Seq_speed_C0 = re4t::cfg->fTrainerPlayerSpeedOverride;
	}

	// Handle the player's collision
	if (pl_atariInfoFlagSet && 
		!FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT)) &&
		!re4t::cfg->bTrainerEnableFreeCam)
	{
		// Restore collision flags
		player->atari_2B4.m_flag_1A = pl_atariInfoFlagBackup;
		pl_atariInfoFlagSet = false;
	}

	// Handle Ashley's collision
	cPlayer* ashley = AshleyPtr();
	if (ashley && !re4t::cfg->bTrainerEnableFreeCam)
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
		if (re4t::cfg->bTrainerUseNumpadMovement && !re4t::cfg->bTrainerEnableFreeCam)
		{
			// Speed calculation
			float movSpeed = 140.0f * re4t::cfg->fTrainerNumMoveSpeed;

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
			if (re4t::cfg->bTrainerUseMouseWheelUpDown && !ImGuiShouldAcceptInput())
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
	if (re4t::cfg->bTrainerEnableFreeCam)
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
		float movSpeed = 140.0f * re4t::cfg->fTrainerFreeCamSpeed;

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
	if (!(re4t::cfg->bShowSideInfo || re4t::cfg->bShowESP))
		return;

	ImGui::PushFont(esHook.ESP_font);

	auto& io = ImGui::GetIO();
	Vec screenpos;

	auto screen_width = io.DisplaySize.x;
	auto screen_height = io.DisplaySize.y;

	cEmMgr& emMgr = *EmMgrPtr();

	GLOBAL_WK* pG = GlobalPtr();
	if (!pG)
		return;

	// Draw side info
	if (re4t::cfg->bShowSideInfo)
	{
		// Drw Em count
		if (re4t::cfg->bSideShowEmCount)
		{
			std::stringstream emCnt;
			emCnt << "Em Count: " << emMgr.count_valid() << " | " << "Max: " << emMgr.count();

			ImGui::GetBackgroundDrawList()->AddText(ImVec2(20, 20), ImColor(255, 255, 255, 255), emCnt.str().c_str());
		}

		// Draw Em list
		if (re4t::cfg->bSideShowEmList)
		{
			std::vector<cEm*> EnemiesVec = cEmMgr::GetVecClosestEms(re4t::cfg->iSideClosestEmsAmount, re4t::cfg->fSideMaxEmDistance, true, true, re4t::cfg->bSideOnlyShowESLSpawned, true);
			std::reverse(EnemiesVec.begin(), EnemiesVec.end());

			float InfoOffsetY = 60.0f;
			for (auto& em : EnemiesVec)
			{
				// Draw Em Name
				std::stringstream EmName;
				EmName << "#" << em->guid_F8 << " " << cEmMgr::EmIdToName(em->id_100, re4t::cfg->bSideShowSimpleNames);
				if (!re4t::cfg->bSideShowSimpleNames)
				{
					EmName << " (type " << (int)em->type_101 << ")";
				}

				ImGui::GetBackgroundDrawList()->AddText(ImVec2(10, screen_height - InfoOffsetY), ImColor(255, 255, 255, 255), EmName.str().c_str());

				if ((re4t::cfg->iSideEmHPMode != 0) && (em->hp_max_326 > 0))
				{
					float hp_percent = 0;
					float hp_max = (float)em->hp_max_326;

					if (hp_max < em->hp_324)
						hp_max = (float)em->hp_324;

					hp_percent = (float)em->hp_324 * 100.0f / hp_max;

					// HP Bar
					if (re4t::cfg->iSideEmHPMode == 1)
					{
						float barScalex = 1.0f;

						ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(30, screen_height - InfoOffsetY + 28.0f), ImVec2(30.0f + 100.0f * barScalex, screen_height - InfoOffsetY + 18.0f), ImColor(105, 105, 105));
						ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(30, screen_height - InfoOffsetY + 28.0f), ImVec2(30.0f + hp_percent * barScalex, screen_height - InfoOffsetY + 18.0f), getHealthColor(hp_percent));

						InfoOffsetY += 12;
					}
					else if (re4t::cfg->iSideEmHPMode == 2) // HP Text
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
	if (re4t::cfg->bShowESP)
	{
		std::vector<cEm*> EmsVector;
		if (re4t::cfg->bEspOnlyShowClosestEms)
			EmsVector = cEmMgr::GetVecClosestEms(re4t::cfg->iEspClosestEmsAmount, re4t::cfg->fEspMaxEmDistance, re4t::cfg->bEspOnlyShowValidEms, re4t::cfg->bEspOnlyShowEnemies, re4t::cfg->bEspOnlyShowESLSpawned, re4t::cfg->bEspOnlyShowAlive);
		else
			EmsVector = cEmMgr::GetVecClosestEms(emMgr.count(), re4t::cfg->fEspMaxEmDistance, re4t::cfg->bEspOnlyShowValidEms, re4t::cfg->bEspOnlyShowEnemies, re4t::cfg->bEspOnlyShowESLSpawned, re4t::cfg->bEspOnlyShowAlive);

		for (auto& em : EmsVector)
		{
			auto coords = em->pos_94;

			if (re4t::cfg->bEspShowInfoOnTop)
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
				if (re4t::cfg->iEspEmNameMode != 0)
				{
					bool useSimpleNames = (re4t::cfg->iEspEmNameMode == 2);

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
				if (re4t::cfg->bEspDrawDebugInfo)
				{
					char EmDbgInfo[256];
					sprintf(EmDbgInfo, "0x%p, Rno %d-%d-%d-%d", em, int(em->r_no_0_FC), int(em->r_no_1_FD), int(em->r_no_2_FE), int(em->r_no_3_FF));
					ImGui::GetBackgroundDrawList()->AddText(ImVec2(screenpos.x, screenpos.y + EspOffsetY), ImColor(255, 255, 255), EmDbgInfo);

					EspOffsetY += 20.0f;
				}

				// Draw HP
				if ((re4t::cfg->iEspEmHPMode != 0) && (em->hp_max_326 > 0))
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
						if (re4t::cfg->iEspEmHPMode == 1)
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
				if (re4t::cfg->bEspDrawLines)
					ImGui::GetBackgroundDrawList()->AddLine(ImVec2(io.DisplaySize.x / 2, io.DisplaySize.y), ImVec2(screenpos.x, screenpos.y + 5.0f), ImColor(255, 255, 255));
			}
		}
	}

	ImGui::PopFont();
}

void Trainer_RenderUI(int columnCount)
{
	ImColor icn_color = ImColor(230, 15, 95);

	ImColor active = ImColor(150, 10, 40, 255);
	ImColor inactive = ImColor(31, 30, 31, 0);

	ImVec2 btn_size = ImVec2(135 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi, 31 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);

	// Patches
	ImGui_TrainerTabButton("##patches", "Patches", active, inactive, TrainerTab::Patches, ICON_FA_DESKTOP, icn_color, IM_COL32_WHITE, btn_size);

	// Hotkeys
	ImGui_TrainerTabButton("##hotkeys", "Hotkeys", active, inactive, TrainerTab::Hotkeys, ICON_FA_LAMBDA, icn_color, IM_COL32_WHITE, btn_size);

	// FlagEdit
	ImGui_TrainerTabButton("##flagedit", "Flag Editor", active, inactive, TrainerTab::FlagEdit, ICON_FA_FLAG, icn_color, IM_COL32_WHITE, btn_size);

	// EmMgr
	ImGui_TrainerTabButton("##emmgr", "Em Manager", active, inactive, TrainerTab::EmMgr, ICON_FA_SNOWMAN, icn_color, IM_COL32_WHITE, btn_size);
	
	// EmMgr
	ImGui_TrainerTabButton("##esp", "ESP", active, inactive, TrainerTab::ESP, ICON_FA_SUITCASE, icn_color, IM_COL32_WHITE, btn_size);

	// DebugTools
	ImGui_TrainerTabButton("##dbgtools", "Debug Tools", active, inactive, TrainerTab::DebugTools, ICON_FA_VIRUS, icn_color, IM_COL32_WHITE, btn_size);

	// Globals
	if (ImGui_TrainerTabButton("##globals", "Globals", active, inactive, TrainerTab::NumTabs, ICON_FA_GLOBE, icn_color, IM_COL32_WHITE, btn_size, false))
	{
		UI_NewGlobalsViewer();
	}

	// Last param to ImGui_TrainerTabButton must be false if it is the last button, to avoid the separator below from being SameLine'd as well.
	ImGui_ItemSeparator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	ImGui::BeginChild("right side", ImVec2(0, 0));

	if (CurTrainerTab == TrainerTab::Patches)
	{
		if (ImGui::BeginTable("TrainerPatches", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
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

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::SetNextItemWidth(210.0f * esHook._cur_monitor_dpi);
				ImGui::InputInt("Pesetas", &GlobalPtr()->goldAmount_4FA8);

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

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
				
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

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

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Prevents taking hits from enemies & automatically skips grabs.");
			}

			// Weak Enemies
			{
				ImGui_ColumnSwitch();

				bool weakEnemies = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK));
				if (ImGui::Checkbox("Weak Enemies", &weakEnemies))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK), weakEnemies);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Makes most enemies die in 1 hit.");
			}

			// Notarget
			{
				ImGui_ColumnSwitch();

				bool noTarget = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_NO_ATK));
				if (ImGui::Checkbox("Disable Enemy Targeting", &noTarget))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_NO_ATK), noTarget);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Prevents enemies from targeting player character, unless player attacks them. (only for enemies using Em10 right now)");
			}

			// Inf Ammo
			{
				ImGui_ColumnSwitch();

				bool infAmmo = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
				if (ImGui::Checkbox("Infinite Ammo", &infAmmo))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET), infAmmo);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Prevents game from removing bullets after firing.");
			}

			// Numpad Movement
			{
				ImGui_ColumnSwitch();

				bool DisablePlayerCollision = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
				if (ImGui::Checkbox("Disable Player Collision", &DisablePlayerCollision))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), DisablePlayerCollision);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Disables collision (no-clip).");

				ImGui::Spacing();

				ImGui::BeginDisabled(!DisablePlayerCollision);
				if (ImGui::Checkbox("Numpad Movement", &re4t::cfg->bTrainerUseNumpadMovement))
					re4t::cfg->HasUnsavedChanges = true;
				ImGui::TextWrapped("Allows noclip movement via numpad.");

				ImGui::Spacing();

				if (ImGui::Checkbox("Use Mouse Wheel", &re4t::cfg->bTrainerUseMouseWheelUpDown))
					re4t::cfg->HasUnsavedChanges = true;
				ImGui::TextWrapped("Allows using the mouse wheel to go up and down.");

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				if (ImGui::SliderFloat("Speed##numpmov", &re4t::cfg->fTrainerNumMoveSpeed, 0.0f, 10.0f, "%.2f"))
					re4t::cfg->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##numpmov"))
				{
					re4t::cfg->fTrainerNumMoveSpeed = 1.0f;
					re4t::cfg->HasUnsavedChanges = true;
				}
				ImGui::EndDisabled();
			}
			
			// Free camera
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enable Free Camera", &re4t::cfg->bTrainerEnableFreeCam))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Allows flying around with the camera. Controller and Keybard/Mouse are supported.");

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
				ImGui::TextWrapped("Controls:");

				ImGui::Spacing();
				ImGui::TextWrapped("Keyboard/Mouse:");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Movement keys (WASD): Move forwards/backwards/sideways");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Mouse movement: Rotate camera");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Mouse wheel: Move up/down");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Run key: Speedup movement");

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
				ImGui::TextWrapped("Controller:");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Left analog stick: Move forwards/backwards/sideways");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Right analog stick: Rotate camera");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("\"Aim Weapon\" and \"Aim Knife\" keys: Move up/down");
				ImGui::Bullet(); ImGui::SameLine(); ImGui::TextWrapped("Run key: Speedup movement");

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::BeginDisabled(!re4t::cfg->bTrainerEnableFreeCam);
				if (ImGui::SliderFloat("Speed##freecam", &re4t::cfg->fTrainerFreeCamSpeed, 0.0f, 10.0f, "%.2f"))
					re4t::cfg->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##fcspdoverrid"))
				{
					re4t::cfg->fTrainerFreeCamSpeed = 1.0f;
					re4t::cfg->HasUnsavedChanges = true;
				}

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

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

				if (ImGui::Checkbox("Enable Player Speed Override", &re4t::cfg->bTrainerPlayerSpeedOverride))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Allows overriding player speed value.");
				ImGui::TextWrapped("Ctrl+Click to input a custom value.");

				ImGui::Spacing();

				ImGui::BeginDisabled(!re4t::cfg->bTrainerPlayerSpeedOverride);
				if (ImGui::SliderFloat("Speed", &re4t::cfg->fTrainerPlayerSpeedOverride, 0.0f, 50.0f, "%.2f"))
					re4t::cfg->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##spdoverrid"))
				{
					re4t::cfg->fTrainerPlayerSpeedOverride = 1.0f;
					re4t::cfg->HasUnsavedChanges = true;
				}
				ImGui::EndDisabled();
			}

			// Dynamic Difficulty Slider
			{
				ImGui_ColumnSwitch();

				ImGui::Checkbox("Enable Difficulty Level Override", &re4t::cfg->bTrainerOverrideDynamicDifficulty);

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Allows overriding the dynamic difficulty level.");
				ImGui::TextWrapped("(Affects enemy health, damage, speed, and aggression)");

				ImGui::Spacing();
				re4t::cfg->iTrainerDynamicDifficultyLevel = GlobalPtr()->dynamicDifficultyLevel_4F98;
				ImGui::BeginDisabled(!re4t::cfg->bTrainerOverrideDynamicDifficulty);
				ImGui::SliderInt("", &re4t::cfg->iTrainerDynamicDifficultyLevel, 1, 10);
				ImGui::EndDisabled();

				if (ImGui::IsItemEdited())
				{
					GlobalPtr()->dynamicDifficultyPoints_4F94 = re4t::cfg->iTrainerDynamicDifficultyLevel * 1000 + 500;
					GlobalPtr()->dynamicDifficultyLevel_4F98 = re4t::cfg->iTrainerDynamicDifficultyLevel;
				}
			}

			// Ashley presence
			{
				ImGui_ColumnSwitch();

				ImGui::Spacing();
				ImGui::TextWrapped("Override Ashley's Presence");

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::RadioButton("Area Default", &AshleyStateOverride, AshleyState::Default);
				ImGui::RadioButton("Present", &AshleyStateOverride, AshleyState::Present);
				ImGui::RadioButton("Not present", &AshleyStateOverride, AshleyState::NotPresent);
			}

			// Enemy HP multiplier
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enemy HP Multiplier", &re4t::cfg->bTrainerEnemyHPMultiplier))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Allows overriding the HP of enemies.");

				ImGui::BeginDisabled(!re4t::cfg->bTrainerEnemyHPMultiplier || re4t::cfg->bTrainerRandomHPMultiplier);

				ImGui::TextWrapped("The new HP will be whatever their original HP was, multiplied by the value set here.");
				ImGui::TextWrapped("Ctrl+Click to input a custom value.");

				ImGui::Spacing();

				if (ImGui::SliderFloat("HP Multiplier", &re4t::cfg->fTrainerEnemyHPMultiplier, 0.1f, 15.0f, "%.2f"))
					re4t::cfg->HasUnsavedChanges = true;

				if (ImGui::Button("Reset##hpmulti"))
				{
					re4t::cfg->fTrainerEnemyHPMultiplier = 1.0f;
					re4t::cfg->HasUnsavedChanges = true;
				}
				ImGui::EndDisabled();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::BeginDisabled(!re4t::cfg->bTrainerEnemyHPMultiplier);
				re4t::cfg->HasUnsavedChanges |= ImGui::Checkbox("Use random HP multiplier", &re4t::cfg->bTrainerRandomHPMultiplier);
				ImGui::EndDisabled();
				ImGui::BeginDisabled(!re4t::cfg->bTrainerEnemyHPMultiplier || !re4t::cfg->bTrainerRandomHPMultiplier);
				ImGui::TextWrapped("Randomly pick the HP multiplier of each enemy.");
				ImGui::TextWrapped("You can also set the minimum and maximum values that can be generated.");
				ImGui::TextWrapped("Ctrl+Click to input a custom value.");

				ImGui::Spacing();

				if (ImGui::SliderFloat("Random Min", &re4t::cfg->fTrainerRandomHPMultiMin, 0.1f, 15.0f, "%.2f"))
				{
					if (re4t::cfg->fTrainerRandomHPMultiMin > re4t::cfg->fTrainerRandomHPMultiMax)
						re4t::cfg->fTrainerRandomHPMultiMin = re4t::cfg->fTrainerRandomHPMultiMax;

					re4t::cfg->HasUnsavedChanges = true;
				}

				if (ImGui::SliderFloat("Random Max", &re4t::cfg->fTrainerRandomHPMultiMax, 0.1f, 15.0f, "%.2f"))
				{
					if (re4t::cfg->fTrainerRandomHPMultiMax < re4t::cfg->fTrainerRandomHPMultiMin)
						re4t::cfg->fTrainerRandomHPMultiMax = re4t::cfg->fTrainerRandomHPMultiMin;

					re4t::cfg->HasUnsavedChanges = true;
				}

				ImGui::EndDisabled();
			}

			// DisableEnemySpawn
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Disable enemy spawn", &re4t::cfg->bTrainerDisableEnemySpawn))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Make it so enemies don't spawn.");
				ImGui::TextWrapped("May cause crashes during events/cutscenes that expect enemies to be present! Use with caution!");
			}

			// Dead bodies never disappear
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Dead bodies never disappear", &re4t::cfg->bTrainerDeadBodiesNeverDisappear))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Make it so enemies never disappear/despawn when killed.");
			}

			// AllowEnterDoorsWithoutAsh
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Allow entering doors without Ashley", &re4t::cfg->bTrainerAllowEnterDoorsWithoutAsh))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Allows the player to go through doors even if Ashley is far away.");
			}

			// EnableDebugTrg
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enable DebugTrg function", &re4t::cfg->bTrainerEnableDebugTrg))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Reimplements the games DebugTrg function, usually allowing certain sections of rooms/events to be skipped.");
				ImGui::TextWrapped("Can be triggered by pressing LB + X + B together on controller, a hotkey can also be bound in the hotkeys page.");

				ImGui::BeginDisabled(!re4t::cfg->bTrainerEnableDebugTrg);
				if (ImGui::Checkbox("Show DebugTrg hint on screen", &re4t::cfg->bTrainerShowDebugTrgHintText))
					re4t::cfg->HasUnsavedChanges = true;
				ImGui::TextWrapped("Will display \"DebugTrg available!\" text on top-left whenever DebugTrg is being actively checked by the game.");
				ImGui::EndDisabled();
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	if (CurTrainerTab == TrainerTab::Hotkeys)
	{
		float btn_size_x = 150 * esHook._cur_monitor_dpi;

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

		if (ImGui::BeginTable("TrainerHotkeys", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// UI Focus 
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to set mouse focus on trainer related UIs (EmManager/Globals)");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushID(1);
				if (ImGui::Button(re4t::cfg->sTrainerFocusUIKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sTrainerFocusUIKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Focus/unfocus trainer UIs");
			}

			// No-clip
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Disable Player Collision\" / no-clip patch.");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushID(2);
				if (ImGui::Button(re4t::cfg->sTrainerNoclipKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sTrainerNoclipKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle no-clip");
			}

			// Free cam
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Free Camera\"patch.");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushID(3);
				if (ImGui::Button(re4t::cfg->sTrainerFreeCamKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sTrainerFreeCamKeyCombo, 0, NULL);
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
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushID(4);
				if (ImGui::Button(re4t::cfg->sTrainerSpeedOverrideKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sTrainerSpeedOverrideKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle speed override");
			}

			// Weapon Hotkeys
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Enable Weapon Hotkeys", &re4t::cfg->bWeaponHotkeysEnable))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Allows switching weapons via hotkeys rather than needing to use inventory.");
				ImGui::TextWrapped("By default hotkeys will cycle between weapons of different types, they can also be assigned to a weapon directly by highlighting the weapon in inventory and pressing the hotkey on it.");
				ImGui::Spacing();

				ImGui::BeginDisabled(!re4t::cfg->bWeaponHotkeysEnable);

				for (int i = 0; i < 5; i++)
				{
					ImGui::PushID(250 + i);
					if (ImGui::Button(re4t::cfg->sWeaponHotkeys[i].c_str(), ImVec2(btn_size_x, 0)))
					{
						re4t::cfg->HasUnsavedChanges = true;
						CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sWeaponHotkeys[i], 0, NULL);
					}
					ImGui::PopID();

					ImGui::SameLine();
					std::string text = "Weapon Slot " + std::to_string(i + 1);
					ImGui::TextWrapped(text.c_str());
				}

				ImGui::PushID(200);
				if (ImGui::Button(re4t::cfg->sLastWeaponHotkey.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sLastWeaponHotkey, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();
				ImGui::TextWrapped("Last used weapon");

				ImGui::Text("Assigned hotkey weapon IDs: %d %d %d %d %d",
					re4t::cfg->iWeaponHotkeyWepIds[0], re4t::cfg->iWeaponHotkeyWepIds[1], re4t::cfg->iWeaponHotkeyWepIds[2], re4t::cfg->iWeaponHotkeyWepIds[3], re4t::cfg->iWeaponHotkeyWepIds[4]);

				if (ImGui::Button("Reset assignments (use weapon cycling)"))
				{
					re4t::cfg->iWeaponHotkeyWepIds[0] = re4t::cfg->iWeaponHotkeyWepIds[1] = re4t::cfg->iWeaponHotkeyWepIds[2] = re4t::cfg->iWeaponHotkeyWepIds[3] = re4t::cfg->iWeaponHotkeyWepIds[4] = 0;
					re4t::cfg->HasUnsavedChanges = true;
				}

				ImGui::EndDisabled();
			}

			// Move Ash to Player
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to move Ashley to the player's position.");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushID(5);
				if (ImGui::Button(re4t::cfg->sTrainerMoveAshToPlayerKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sTrainerMoveAshToPlayerKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Move Ashley to Player");
			}

			// DebugTrg trigger
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("DebugTrg combination, requires EnableDebugTrg to be enabled.");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushID(5);
				if (ImGui::Button(re4t::cfg->sTrainerDebugTrgKeyCombo.c_str(), ImVec2(btn_size_x, 0)))
				{
					re4t::cfg->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &re4t::cfg->sTrainerDebugTrgKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("DebugTrg");
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

		// Make sure to keep order of this in sync with CategoryInfoIdx enum below
		std::vector<CategoryInfo> flagCategoryInfo = {
			{ "DEBUG", GlobalPtr()->flags_DEBUG_0_60, Flags_DEBUG_Names, &Flags_DEBUG_Descriptions, 128 },
			{ "STOP", GlobalPtr()->flags_STOP_0_170, Flags_STOP_Names, &Flags_STOP_Descriptions, 32 },
			{ "STATUS", GlobalPtr()->flags_STATUS_0_501C, Flags_STATUS_Names, &Flags_STATUS_Descriptions, 128 },
			{ "SYSTEM", GlobalPtr()->Flags_SYSTEM_0_54, Flags_SYSTEM_Names, &Flags_SYSTEM_Descriptions, 32 },
			{ "ITEM_SET", GlobalPtr()->flags_ITEM_SET_0_528C, Flags_ITEM_SET_Names, &Flags_ITEM_SET_Descriptions, 150 },
			{ "SCENARIO", GlobalPtr()->flags_SCENARIO_0_52CC, Flags_SCENARIO_Names, &Flags_SCENARIO_Descriptions, 195 },
			{ "KEY_LOCK", GlobalPtr()->flags_KEY_LOCK_52EC, Flags_KEY_LOCK_Names, &Flags_KEY_LOCK_Descriptions, 150 },
			{ "DISP", GlobalPtr()->Flags_DISP_0_58, Flags_DISP_Names, &Flags_DISP_Descriptions, 32 },
			{ "EXTRA", SystemSavePtr()->flags_EXTRA_4, Flags_EXTRA_Names, &Flags_EXTRA_Descriptions, 16 },
			{ "CONFIG", SystemSavePtr()->flags_CONFIG_0, Flags_CONFIG_Names, &Flags_CONFIG_Descriptions, 6 },
		};

		// cRoomData::getRoomSavePtr returns nullptr for certain rooms like r120 (or maybe just returns that in main menu), check it's not null before adding:
		ROOM_SAVE_DATA* roomSaveData = RoomData->getRoomSavePtr(GlobalPtr()->curRoomId_4FAC);
		if (roomSaveData)
		{
			// We have flag names for r226, so display them:
			if (GlobalPtr()->curRoomId_4FAC == 0x226)
			{
				flagCategoryInfo.push_back({ "ROOM_SAVE", roomSaveData->save_flg_4, Flags_ROOM_SAVE_r226_Names, &Flags_ROOM_SAVE_Descriptions, 32 });
				flagCategoryInfo.push_back({ "ROOM", GlobalPtr()->flags_ROOM_0_174, Flags_ROOM_r226_Names, &Flags_ROOM_Descriptions, 128 });
			}
			else
			{
				flagCategoryInfo.push_back({ "ROOM_SAVE", roomSaveData->save_flg_4, Flags_ROOM_SAVE_Names, &Flags_ROOM_SAVE_Descriptions, 32 });
				flagCategoryInfo.push_back({ "ROOM", GlobalPtr()->flags_ROOM_0_174, Flags_ROOM_Names, &Flags_ROOM_Descriptions, 128 });
			}
		}

		// Make sure to keep order of this in sync with flagCategoryInfo vector above
		enum class CategoryInfoIdx
		{
			DEBUG,
			STOP,
			STATUS,
			SYSTEM,
			ITEM_SET,
			SCENARIO,
			KEY_LOCK,
			DISP,
			EXTRA,
			CONFIG,
			ROOM_SAVE,
			ROOM
		};

		static bool filter_descriptions_only = true;

		static char searchText[256] = { 0 };
		static int columns = 3;
		static int flagCategory = 0;
		CategoryInfo* curFlagCategory = nullptr;
		bool curFlagCategoryIsExtra = false;

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0f, 5.0f));
		if (ImGui::BeginTable("##flagtoolheader", 2, ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 70 * esHook._cur_monitor_dpi)))
		{
			ImGui::TableNextColumn();

			ImGui::PushItemWidth(120.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
			if (ImGui::BeginCombo("Category", flagCategoryInfo[flagCategory].categoryName))
			{
				for (size_t i = 0; i < flagCategoryInfo.size(); i++)
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
			curFlagCategoryIsExtra = curFlagCategory->values == SystemSavePtr()->flags_EXTRA_4;

			ImGui::SameLine();
			ImGui::Checkbox("Functional only", &filter_descriptions_only);
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip("Only display flags that are known to have an effect.\n(note that some working flags may be missing!)");

			ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
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

		CategoryInfoIdx flagCategoryIdx = CategoryInfoIdx(flagCategory);
		const char* helpText = nullptr;
		if (flagCategoryIdx == CategoryInfoIdx::EXTRA)
		{
			if (!bio4::CardCheckDone())
				helpText = "EXTRA flags disabled: flags can only be modified after the intro/'PRESS ANY KEY' screens";
			else if (FlagsExtraValue.has_value())
				helpText = "EXTRA flags changed: exit to main menu & back out to the game logo/'PRESS ANY KEY' screen for them to take effect!";
			else
				helpText = ""; // reserve space for the text above
		}
		else if (flagCategoryIdx == CategoryInfoIdx::ROOM_SAVE)
			helpText = "ROOM_SAVE: state of the current room which is saved between rooms; reload area after changing to take effect.";
		else if (flagCategoryIdx == CategoryInfoIdx::ROOM)
			helpText = "ROOM: temporary flags set by the current room to keep track of the current state, will be reset on room load.";

		if (helpText)
			ImGui::TextWrapped(helpText);

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
					// Except for ITEM_SET / SCENARIO / KEY_LOCK / CONFIG / EXTRA / ROOM / ROOM_SAVE flags
					// (we'll always display them since they're all useful AFAIK)
					if (filter_descriptions_only)
						if (flagCategoryIdx != CategoryInfoIdx::ITEM_SET && flagCategoryIdx != CategoryInfoIdx::SCENARIO && flagCategoryIdx != CategoryInfoIdx::KEY_LOCK &&
							flagCategoryIdx != CategoryInfoIdx::EXTRA && flagCategoryIdx != CategoryInfoIdx::CONFIG && flagCategoryIdx != CategoryInfoIdx::ROOM && flagCategoryIdx != CategoryInfoIdx::ROOM_SAVE)
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

						ImGui::BeginDisabled(curFlagCategoryIsExtra && !bio4::CardCheckDone());
						ImGui::PushStyleColor(ImGuiCol_Text, col);
						if (ImGui::Checkbox(curFlagCategory->valueNames[i], &selected))
						{
							FlagSet(curFlagCategory->values, i, selected);
							if (curFlagCategoryIsExtra)
							{
								FlagsExtraValue = *curFlagCategory->values;
							}
						}
						ImGui::EndDisabled();
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

				if (ImGui::Checkbox("Show ESP", &re4t::cfg->bShowESP))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::BeginDisabled(!re4t::cfg->bShowESP);

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Displays information on Ems");
				ImGui::Spacing();

				ImGui::Checkbox("Show info on top of Ems", &re4t::cfg->bEspShowInfoOnTop);
				ImGui::Checkbox("Only show enemies", &re4t::cfg->bEspOnlyShowEnemies);
				ImGui::Checkbox("Only show alive", &re4t::cfg->bEspOnlyShowAlive);
				ImGui::Checkbox("Only show valid Ems", &re4t::cfg->bEspOnlyShowValidEms);
				ImGui::Checkbox("Only show ESL-spawned##esp", &re4t::cfg->bEspOnlyShowESLSpawned);
				ImGui::Checkbox("Draw Lines", &re4t::cfg->bEspDrawLines);
				ImGui::Checkbox("Draw Debug Info", &re4t::cfg->bEspDrawDebugInfo);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Name display mode:");
				ImGui::RadioButton("Don't show##nameesp", &re4t::cfg->iEspEmNameMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Normal##esp", &re4t::cfg->iEspEmNameMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Simplified##esp", &re4t::cfg->iEspEmNameMode, 2);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("HP display mode:");
				ImGui::RadioButton("Don't show##hpesp", &re4t::cfg->iEspEmHPMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Bar##esp", &re4t::cfg->iEspEmHPMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Text##esp", &re4t::cfg->iEspEmHPMode, 2);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max distance of Ems").x - 10);
				ImGui::SliderFloat("Max distance of Ems##esp", &re4t::cfg->fEspMaxEmDistance, 0.0f, 200000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::Checkbox("Only show closest Ems", &re4t::cfg->bEspOnlyShowClosestEms);
				ImGui::Spacing();

				ImGui::BeginDisabled(!re4t::cfg->bEspOnlyShowClosestEms);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max amount of Ems").x - 10);
				ImGui::InputInt("Max amount of Ems##esp", &re4t::cfg->iEspClosestEmsAmount);
				ImGui::EndDisabled();

				ImGui::EndDisabled();
			}

			// Side info
			{
				ImGui_ColumnSwitch();

				if (ImGui::Checkbox("Show side info", &re4t::cfg->bShowSideInfo))
					re4t::cfg->HasUnsavedChanges = true;

				ImGui_ItemSeparator();

				ImGui::BeginDisabled(!re4t::cfg->bShowSideInfo);

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("Displays information on the left side of the screen.");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::Checkbox("Show Em count", &re4t::cfg->bSideShowEmCount);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::Checkbox("Show Em list", &re4t::cfg->bSideShowEmList);
				ImGui::TextWrapped("Ems are listed from bottom to top, with the top one being the closes to the player, and the bottom one being the farthest.");
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));
				
				ImGui::BeginDisabled(!re4t::cfg->bSideShowEmList);
				ImGui::Checkbox("Only show ESL-spawned##side", &re4t::cfg->bSideOnlyShowESLSpawned);
				ImGui::Checkbox("Show simplified names##side", &re4t::cfg->bSideShowSimpleNames);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::TextWrapped("HP display mode:");
				ImGui::RadioButton("Don't show##side", &re4t::cfg->iSideEmHPMode, 0); ImGui::SameLine();
				ImGui::RadioButton("Bar##side", &re4t::cfg->iSideEmHPMode, 1); ImGui::SameLine();
				ImGui::RadioButton("Text##side", &re4t::cfg->iSideEmHPMode, 2);
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Max amount of enemies").x - 10);
				ImGui::Spacing();
				ImGui::InputInt("Max amount of enemies##side", &re4t::cfg->iSideClosestEmsAmount);

				ImGui::Spacing();
				ImGui::SliderFloat("Max distance of enemies##side", &re4t::cfg->fSideMaxEmDistance, 0.0f, 200000.0f, "%.0f", ImGuiSliderFlags_AlwaysClamp);
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
		if (ImGui::BeginTable("DebugTools", columnCount, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
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

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

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

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				static auto FilterTool = new UI_FilterTool();
				FilterTool->Render(false);
			}

			// Inventory Item Adder
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Add item to inventory");

				ImGui_ItemSeparator();
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				static char searchText[256] = { 0 };
				static bool alwaysShowInventory = true;

				ImGui::InputText("Search", searchText, 256);

				std::string searchTextUpper = StrToUpper(searchText);

				static std::vector<EItemId> badItems = {
					// These use ITEM_TYPE_WEAPON but don't have any "piece_info" data in the game for them
					// would cause crash if added, so filter them out instead
					EItemId::Ruger_SA,
					EItemId::Mauser_ST,
					EItemId::New_Weapon_SC,
					EItemId::Styer_St,
					EItemId::HK_Sniper_Thermo,
					EItemId::S_Field_Sc,
					EItemId::HK_Sniper_Sc,
					EItemId::S_Field_Thermo,
					EItemId::Mine_SC,

					EItemId::Krauser_Knife, // gets added to key items, but freezes game when examining
					EItemId::Ada_New_Weapon, // gets added to key items as "Killer7 w/ Silencer", no icon, examine shows the model, but it's otherwise pretty useless
					                         // maybe this can be made equippable if "piece_info" data is added for it though...

					// These don't seem to have any effect when added, probably requires some struct to be updated too..
					EItemId::Attache_Case_S,
					EItemId::Attache_Case_M,
					EItemId::Attache_Case_L,
					EItemId::Attache_Case_O,
					EItemId::Assault_Jacket,
					EItemId::Any
				};

				static EItemId itemId = EItemId::Bullet_45in_H;
				static int stackCount = 100;

				if (ImGui::BeginListBox("ItemId"))
				{
					for(int item_id = 0; item_id < 255; item_id++)
					{
						if (std::find(badItems.begin(), badItems.end(), EItemId(item_id)) != badItems.end())
							continue;

						ITEM_INFO curInfo;
						curInfo.id_0 = ITEM_ID(item_id);
						bio4::itemInfo(ITEM_ID(item_id), &curInfo);

						std::string name = std::string(ITEM_TYPE_Names[curInfo.type_2]) + ": " + EItemId_Names[int(item_id)];

						bool makeVisible = true;
						if (!searchTextUpper.empty())
						{
							std::string itemNameUpper = StrToUpper(name);

							makeVisible = itemNameUpper.find(searchTextUpper) != std::string::npos;
						}

						if (makeVisible)
						{
							bool selected = itemId == EItemId(item_id);
							if (ImGui::Selectable(name.c_str(), &selected))
								if (selected)
								{
									itemId = EItemId(item_id);
									if (curInfo.maxNum_4 > 1)
										stackCount = curInfo.maxNum_4; // help user by setting stack count to max for this item
								}
						}
					}
					ImGui::EndListBox();
				}

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ITEM_INFO info;
				info.id_0 = ITEM_ID(itemId);
				bio4::itemInfo(ITEM_ID(itemId), &info);
				if (TweaksDevMode)
					ImGui::Text("selected: id %d type %d def %d, max %d", info.id_0, info.type_2, info.defNum_3, info.maxNum_4);

				bool stackable = info.maxNum_4 > 1;
				bool showsInInventory = bio4::itemShowsInInventory(info.type_2);

				if (!stackable)
					stackCount = 1;
				else
				{
					if (stackCount > info.maxNum_4)
						stackCount = info.maxNum_4;
				}

				ImGui::BeginDisabled(!stackable);
				ImGui::InputInt("Stack count", &stackCount);
				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				{
					if (!stackable)
						ImGui::SetTooltip("The selected item is unable to be stacked.");
					else
						ImGui::SetTooltip("Number of items in the stack (max for this item: %d)", info.maxNum_4);
				}
				ImGui::EndDisabled();

				// Disable button if no player character, or game wouldn't allow player to open inventory, or inventory is already opened
				// TODO: pause menu, checks below don't seem to catch it...
				bool disable =
					!PlayerPtr() ||
					!PlayerPtr()->subScrCheck() ||
					SubScreenWk->open_flag_2C != 0 ||
					FlagIsSet(GlobalPtr()->flags_STOP_0_170, uint32_t(Flags_STOP::SPF_PL)); // disallow if player stop flag is set (eg. during pause screen)

				ImGui::BeginDisabled(disable);
				if (ImGui::Button("Add Item"))
					RequestInventoryAdd(ITEM_ID(itemId), stackCount, alwaysShowInventory);

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && disable)
					ImGui::SetTooltip("Items can only be added while outside of menus.");

				ImGui::EndDisabled();

				ImGui_ItemSeparator();
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::BeginDisabled(!showsInInventory);
				ImGui::Checkbox("Open inventory after adding", &alwaysShowInventory);
				if (!showsInInventory && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("The selected item will appear on treasure/file tab instead of inventory.");
				ImGui::EndDisabled();
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	ImGui::EndChild();
}