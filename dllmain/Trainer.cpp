#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"
#include "imgui.h"
#include <FAhashes.h>
#include "UI_DebugWindows.h"

bool bUseNumpadMovement = false;
bool bUseMouseWheelUPDOWN = false;

bool bPlayerSpeedOverride = false;
float fPlayerSpeedOverride = 1.0f;

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

std::vector<uint32_t> KeyComboNoclipToggle;
std::vector<uint32_t> KeyComboSpeedOverride;
void Trainer_ParseKeyCombos()
{
	KeyComboNoclipToggle.clear();
	KeyComboNoclipToggle = ParseKeyCombo(pConfig->sTrainerNoclipKeyCombo);

	KeyComboSpeedOverride.clear();
	KeyComboSpeedOverride = ParseKeyCombo(pConfig->sTrainerSpeedOverrideKeyCombo);
}

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
}

void Trainer_Update()
{
	if (pInput->is_combo_pressed(&KeyComboNoclipToggle))
	{
		bool playerCollisionDisabled = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
		FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), !playerCollisionDisabled);
	}

	if (pInput->is_combo_pressed(&KeyComboSpeedOverride))
	{
		bPlayerSpeedOverride = !bPlayerSpeedOverride;
	}

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
		if (prev_bPlayerSpeedOverride != bPlayerSpeedOverride)
		{
			if (bPlayerSpeedOverride)
			{
				// changed from disabled -> enabled, backup previous speed value
				prev_PlayerSpeed = player->Motion_1D8.Seq_speed_C0;
			}
			else if (!bPlayerSpeedOverride)
			{
				// changed from enabled -> disabled, restore previous speed value
				player->Motion_1D8.Seq_speed_C0 = prev_PlayerSpeed;
			}
			prev_bPlayerSpeedOverride = bPlayerSpeedOverride;
		}

		if (bPlayerSpeedOverride)
			player->Motion_1D8.Seq_speed_C0 = fPlayerSpeedOverride;
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
		player->atari_2B4.m_flag_1A &= ~0x100;

		Vec positionMod{ 0 };
		bool positionModded = false;

		// Handle numpad-movement
		if (bUseNumpadMovement)
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
		if (bUseMouseWheelUPDOWN)
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

enum class TrainerTab
{
	Patches,
	Hotkeys,
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

bool ImGui_TrainerTabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, TrainerTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size = ImVec2(0, 0))
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::PushStyleColor(ImGuiCol_Button, CurTrainerTab == tabID ? activeCol : inactiveCol);
	bool ret = ImGui::Button(btnID, ImVec2(172, 31));
	ImGui::PopStyleColor();

	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 35.0f, p0.y + 10.0f), iconColor, icon, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 65.0f, p0.y + 8.0f), textColor, text, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));

	if (ret && tabID != TrainerTab::NumTabs)
		CurTrainerTab = tabID;

	return ret;
}

void Trainer_RenderUI()
{
	ImColor icn_color = ImColor(230, 15, 95);

	ImColor active = ImColor(150, 10, 40, 255);
	ImColor inactive = ImColor(31, 30, 31, 0);

	// Patches
	ImGui_TrainerTabButton("##patches", "Patches", active, inactive, TrainerTab::Patches, ICON_FA_DESKTOP, icn_color, IM_COL32_WHITE, ImVec2(172, 31));

	// Hotkeys
	ImGui::SameLine();
	ImGui_TrainerTabButton("##hotkeys", "Hotkeys", active, inactive, TrainerTab::Hotkeys, ICON_FA_LAMBDA, icn_color, IM_COL32_WHITE, ImVec2(172, 31));

	// EmMgr
	ImGui::SameLine();
	if (ImGui_TrainerTabButton("##emmgr", "Em Manager", active, inactive, TrainerTab::NumTabs, ICON_FA_SNOWMAN, icn_color, IM_COL32_WHITE, ImVec2(172, 31)))
	{
		UI_NewEmManager();
	}

	// Globals
	ImGui::SameLine();
	if (ImGui_TrainerTabButton("##globals", "Globals", active, inactive, TrainerTab::NumTabs, ICON_FA_HEADPHONES, icn_color, IM_COL32_WHITE, ImVec2(172, 31)))
	{
		UI_NewGlobalsViewer();
	}

	ImGui_ItemSeparator();

	ImGui::Dummy(ImVec2(0.0f, 12.0f));

	if (CurTrainerTab == TrainerTab::Patches)
	{
		if (ImGui::BeginTable("TrainerPatches", 2, ImGuiTableFlags_PadOuterX, ImVec2(ImGui::GetItemRectSize().x - 12, 0)))
		{
			ImGui_ColumnInit();

			// Invincibility
			{
				ImGui_ColumnSwitch();
				bool invincibility = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2));
				if (ImGui::Checkbox("Invincibility", &invincibility))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_NO_DEATH2), invincibility);

				ImGui::TextWrapped("Prevents taking hits from enemies & automatically skips grabs.");
			}

			// Weak Enemies
			{
				ImGui_ColumnSwitch();

				bool weakEnemies = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK));
				if (ImGui::Checkbox("Weak Enemies", &weakEnemies))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EM_WEAK), weakEnemies);

				ImGui::TextWrapped("Makes most enemies die in 1 hit.");
			}

			// Inf Ammo
			{
				ImGui_ColumnSwitch();

				bool infAmmo = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET));
				if (ImGui::Checkbox("Infinite Ammo", &infAmmo))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_INF_BULLET), infAmmo);

				ImGui::TextWrapped("Prevents game from removing bullets after firing.");
			}

			// Numpad Movement
			{
				ImGui_ColumnSwitch();

				bool DisablePlayerCollision = FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT));
				if (ImGui::Checkbox("Disable Player Collision", &DisablePlayerCollision))
					FlagSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_PL_NOHIT), DisablePlayerCollision);

				ImGui::TextWrapped("Disables collision (no-clip).");

				ImGui::Spacing();

				ImGui::BeginDisabled(!DisablePlayerCollision);
				ImGui::Checkbox("Numpad Movement", &bUseNumpadMovement);
				ImGui::TextWrapped("Allows noclip movement via numpad.");

				ImGui::Spacing();

				ImGui::Checkbox("Use Mouse Wheel", &bUseMouseWheelUPDOWN);
				ImGui::TextWrapped("Allows using the mouse wheel to go up and down.");
				ImGui::EndDisabled();
			}

			// Speed override
			{
				ImGui_ColumnSwitch();

				ImGui::Checkbox("Enable Player Speed Override", &bPlayerSpeedOverride);
				ImGui::TextWrapped("Allows overriding player speed value.");

				ImGui::Spacing();

				ImGui::BeginDisabled(!bPlayerSpeedOverride);
				ImGui::InputFloat("Speed", &fPlayerSpeedOverride);
				ImGui::EndDisabled();
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}
	}

	if (CurTrainerTab == TrainerTab::Hotkeys)
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.51f, 0.00f, 0.14f, 1.00f));

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

			// Noclip
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

			// Speed override
			{
				ImGui_ColumnSwitch();

				ImGui::TextWrapped("Key combination to toggle the \"Player Speed Override\" patch.");
				ImGui::TextWrapped("(set your player speed override value in the Patches section first)");
				ImGui::Dummy(ImVec2(10, 10));

				ImGui::PushID(2);
				if (ImGui::Button(pConfig->sTrainerSpeedOverrideKeyCombo.c_str(), ImVec2(150, 0)))
				{
					pConfig->HasUnsavedChanges = true;
					CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&SetHotkeyComboThread, &pConfig->sTrainerSpeedOverrideKeyCombo, 0, NULL);
				}
				ImGui::PopID();

				ImGui::SameLine();

				ImGui::TextWrapped("Toggle speed override");
			}

			ImGui_ColumnFinish();
			ImGui::EndTable();
		}

		ImGui::PopStyleColor();
	}
}