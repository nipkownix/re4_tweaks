#include "dllmain.h"
#include "Trainer.h"
#include "UI_Utility.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include <ImGuiFileDialog.h>
#include <nlohmann/json.hpp>
#include <FAhashes.h>

ITEM_ID wepAdded_ID = 0;
int wepAdded_FirePower = 1;
int wepAdded_FiringSpeed = 1;
int wepAdded_ReloadSpeed = 1;
int wepAdded_Capacity = 1;

void ItemMgr_Update()
{
	// Apply weapon upgrades from the item adder
	if (wepAdded_ID)
	{
		auto wepItmPtr = ItemMgr->search(wepAdded_ID);

		// Game seems to be adding wep id 33 (FN57) when you request id 64 (Punisher), so ItemMgr->search never finds it in the inventory.
		// Workaround that by changing the wepAdded_ID if wepItmPtr is null.
		if (!wepItmPtr && (EItemId(wepAdded_ID) == EItemId::Punisher))
			wepAdded_ID = int(EItemId::FN57);

		// Wait until the game has actually created the weapon
		if (wepItmPtr)
		{
			wepItmPtr->setFirePower(wepAdded_FirePower);
			wepItmPtr->setFiringSpeed(wepAdded_FiringSpeed);
			wepItmPtr->setReloadSpeed(wepAdded_ReloadSpeed);
			wepItmPtr->setCapacity(wepAdded_Capacity);

			// Reset upgrades
			wepAdded_ID = 0;
			wepAdded_FirePower = 1;
			wepAdded_FiringSpeed = 1;
			wepAdded_ReloadSpeed = 1;
			wepAdded_Capacity = 1;
		}
	}
}

void ItemMgr_Render()
{
	// Inventory Item Manager
	{
		static bool show_ALL = true;
		static bool show_WEAPON = true;
		static bool show_AMMO = true;
		static bool show_TREASURE = true;
		static bool show_CONSUMABLE = true;
		static bool show_KEY_ITEM = true;
		static bool show_WEAPON_MOD = true;
		static bool show_FILE = true;
		static bool show_TREASURE_MAP = true;
		static bool show_TREASURE_GEM = true;
		static bool show_BOTTLECAP = true;

		ImGui::Text("Right-click on an item to make changes.");

		ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

		ImGui::Text("Filter categories:");

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.f, 2.f));
		if (ImGui::BeginTable("itmmgrfilter", 6))
		{
			ImGui::TableNextColumn();
			ImGui::PushID(10101);
			if (ImGui::Checkbox("Show All", &show_ALL))
			{
				show_WEAPON = show_ALL;
				show_AMMO = show_ALL;
				show_TREASURE = show_ALL;
				show_CONSUMABLE = show_ALL;
				show_KEY_ITEM = show_ALL;
				show_WEAPON_MOD = show_ALL;
				show_FILE = show_ALL;
				show_TREASURE_MAP = show_ALL;
				show_TREASURE_GEM = show_ALL;
				show_BOTTLECAP = show_ALL;
			}

			ImGui::PopID();

			ImGui::BeginDisabled(show_ALL);
			ImGui::TableNextColumn(); ImGui::PushID(10102); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_WEAPON], &show_WEAPON); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10103); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_AMMO], &show_AMMO); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10106); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_TREASURE], &show_TREASURE); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10107); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_CONSUMABLE], &show_CONSUMABLE); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10108); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_KEY_ITEM], &show_KEY_ITEM); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10110); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_WEAPON_MOD], &show_WEAPON_MOD); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10111); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_FILE], &show_FILE); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10112); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_TREASURE_MAP], &show_TREASURE_MAP); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10113); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_TREASURE_GEM], &show_TREASURE_GEM); ImGui::PopID();
			ImGui::TableNextColumn(); ImGui::PushID(10114); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_BOTTLECAP], &show_BOTTLECAP); ImGui::PopID();
			ImGui::EndDisabled();

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		static char searchText[256] = { 0 };
		static bool alwaysShowInventory = true;
		static int columns = 3;

		ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.f, 2.f));
		if (ImGui::BeginTable("##itmmgrsearch", 2))
		{
			ImGui::TableNextColumn();

			ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
			ImGui::InputText("Search", searchText, 256);
			ImGui::PopItemWidth();

			ImGui::SameLine();
			if (ImGui::SmallButton(ICON_FA_BACKSPACE))
				strcpy(searchText, "");

			ImGui::TableNextColumn();

			ImGui::Dummy(ImVec2((ImGui::GetContentRegionAvail().x - (270 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi)), 1));

			ImGui::SameLine();

			ImGui::Text("Columns: %i", columns);

			ImGui::SameLine();

			if (ImGui::Button("Remove column"))
				columns--;

			ImGui::SameLine();

			if (ImGui::Button("Add column"))
				columns++;

			columns = std::clamp(columns, 1, 6);

			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		std::string searchTextUpper = StrToUpper(searchText);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.f, 5.f));

		if (ImGui::BeginTable("##itmmgrlist", columns, ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
			ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - (55 * esHook._cur_monitor_dpi))))
		{
			std::vector<ITEM_TYPE_mb> chosenTypes;

			if (show_WEAPON) { chosenTypes.push_back(ITEM_TYPE_WEAPON); chosenTypes.push_back(ITEM_TYPE_GRENADE); }
			if (show_AMMO) chosenTypes.push_back(ITEM_TYPE_AMMO);
			if (show_TREASURE) { chosenTypes.push_back(ITEM_TYPE_TREASURE); chosenTypes.push_back(ITEM_TYPE_TREASURE_MERCS); }
			if (show_CONSUMABLE) chosenTypes.push_back(ITEM_TYPE_CONSUMABLE);
			if (show_KEY_ITEM) { chosenTypes.push_back(ITEM_TYPE_KEY_ITEM); chosenTypes.push_back(ITEM_TYPE_IMPORTANT); }
			if (show_WEAPON_MOD) chosenTypes.push_back(ITEM_TYPE_WEAPON_MOD);
			if (show_FILE) chosenTypes.push_back(ITEM_TYPE_FILE);
			if (show_TREASURE_MAP) chosenTypes.push_back(ITEM_TYPE_TREASURE_MAP);
			if (show_TREASURE_GEM) chosenTypes.push_back(ITEM_TYPE_TREASURE_GEM);
			if (show_BOTTLECAP) chosenTypes.push_back(ITEM_TYPE_BOTTLECAP);

			for (auto& type : chosenTypes)
			{
				cItem* itmPtr = ItemMgr->m_pItem_14;

				int m_array_num_1C = ItemMgr->m_array_num_1C;
				for (int loopcnt = 0; loopcnt <= m_array_num_1C; loopcnt++)
				{
					itmPtr++;

					if ((itmPtr->be_flag_4 & 1) == 0)
						continue;

					if (itmPtr->chr_5 != ItemMgr->m_char_13)
						continue;

					int item_id = itmPtr->id_0;

					ITEM_INFO curInfo;
					curInfo.id_0 = ITEM_ID(item_id);
					bio4::itemInfo(ITEM_ID(item_id), &curInfo);

					if (curInfo.type_2 != type)
						continue;

					bool stackable = curInfo.maxNum_4 > 1;

					std::string name;

					if (EItemId_Names[int(item_id)])
						name = std::string(ITEM_TYPE_Names[curInfo.type_2]) + ": " + EItemId_Names[int(item_id)];
					else
						name = std::string(ITEM_TYPE_Names[curInfo.type_2]) + ": " + std::to_string(int(item_id)) + " (missing name)";

					if (stackable)
						name += " (" + std::to_string(itmPtr->num_2) + ")";

					bool makeVisible = true;
					if (!searchTextUpper.empty())
					{
						std::string itemNameUpper = StrToUpper(name);

						makeVisible = itemNameUpper.find(searchTextUpper) != std::string::npos;
					}

					if (makeVisible)
					{
						ImGui::TableNextColumn();

						ImGui::PushID((int)itmPtr);
						ImGui::Selectable(name.c_str());
						ImGui::PopID();

						// Context menu for items
						if (ImGui::BeginPopupContextItem()) // <-- use last item id as popup id
						{
							ImGui::Text("\"%s\":", EItemId_Names[int(item_id)]);

							ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

							// Disable button if no player character, or game wouldn't allow player to open inventory, or inventory is already opened
							bool disable =
								!PlayerPtr() ||
								!PlayerPtr()->subScrCheck() ||
								SubScreenWk->open_flag_2C != 0 ||
								OptionOpenFlag();

							ImGui::BeginDisabled(disable);
							ImGui::BeginGroup();

							// Manage weapon upgrades
							if (curInfo.type_2 == ITEM_TYPE_WEAPON)
							{
								// Manage weapon upgrades
								{
									static int customFirePower = 1;
									static int customFiringSpeed = 1;
									static int customReloadSpeed = 1;
									static int customCapacity = 1;

									int maxFirePower = bio4::WeaponId2MaxLevel(curInfo.id_0, 0) + extra_upgrades[EItemId(curInfo.id_0)].firePower;
									int maxFiringSpeed = bio4::WeaponId2MaxLevel(curInfo.id_0, 1) + extra_upgrades[EItemId(curInfo.id_0)].firingSpeed;
									int maxReloadSpeed = bio4::WeaponId2MaxLevel(curInfo.id_0, 2) + extra_upgrades[EItemId(curInfo.id_0)].reloadSpeed;
									int maxCapacity = bio4::WeaponId2MaxLevel(curInfo.id_0, 3) + extra_upgrades[EItemId(curInfo.id_0)].Capacity;

									// Disable if there's nothing to upgrade
									bool disable = ((maxFirePower == 1) && (maxFiringSpeed == 1) && (maxReloadSpeed == 1) && (maxCapacity == 1));

									ImGui::BeginDisabled(disable);
									if (ImGui::Button("Adjust upgrades"))
									{
										// Get wep's current values
										customFirePower = itmPtr->getFirePower() + 1;
										customFiringSpeed = itmPtr->getFiringSpeed() + 1;
										customReloadSpeed = itmPtr->getReloadSpeed() + 1;
										customCapacity = itmPtr->getCapacity() + 1;

										ImGui::OpenPopup("Adjust upgrades");
									}
									ImGui::EndDisabled();

									// Always center this window when appearing
									ImVec2 center = ImGui::GetMainViewport()->GetCenter();
									ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

									if (ImGui::BeginPopupModal("Adjust upgrades", NULL, ImGuiWindowFlags_AlwaysAutoResize))
									{
										ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);

										ImGui::BeginDisabled(maxFirePower == 1);
										ImGui::SliderInt("Fire Power", &customFirePower, 1, maxFirePower, "%d", ImGuiSliderFlags_NoInput);
										ImGui::EndDisabled();

										ImGui::BeginDisabled(maxFiringSpeed == 1);
										ImGui::SliderInt("Firing Speed", &customFiringSpeed, 1, maxFiringSpeed, "%d", ImGuiSliderFlags_NoInput);
										ImGui::EndDisabled();

										ImGui::BeginDisabled(maxReloadSpeed == 1);
										ImGui::SliderInt("Reload Speed", &customReloadSpeed, 1, maxReloadSpeed, "%d", ImGuiSliderFlags_NoInput);
										ImGui::EndDisabled();

										ImGui::BeginDisabled(maxCapacity == 1);
										ImGui::SliderInt("Capacity", &customCapacity, 1, maxCapacity, "%d", ImGuiSliderFlags_NoInput);
										ImGui::EndDisabled();

										ImGui::PopItemWidth();

										ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

										ImGui::Separator();

										if (ImGui::Button("OK", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
										{
											itmPtr->setFirePower(customFirePower);
											itmPtr->setFiringSpeed(customFiringSpeed);
											itmPtr->setReloadSpeed(customReloadSpeed);
											itmPtr->setCapacity(customCapacity);

											// update weapon power globals if this is the currently equipped weapon
											if (itmPtr == ItemMgr->m_pWep_C)
												ItemMgr->arm(itmPtr);

											ImGui::CloseCurrentPopup();
										}

										ImGui::SameLine();

										if (ImGui::Button("Cancel", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
										{
											ImGui::CloseCurrentPopup();
										}
										ImGui::EndPopup();
									}
								}

								// Change ammo count
								{
									static int newAmmo = 1;
									static bool ignoreMaxAmmo = false;
									int maxAmmo = itmPtr->getMaxAmmo();

									ImGui::BeginDisabled(maxAmmo < 2);
									if (ImGui::Button("Change ammo count"))
									{
										newAmmo = itmPtr->getAmmo();
										ignoreMaxAmmo = false;
										ImGui::OpenPopup("Change ammo count");
									}
									ImGui::EndDisabled();

									// Always center this window when appearing
									ImVec2 center = ImGui::GetMainViewport()->GetCenter();
									ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

									if (ImGui::BeginPopupModal("Change ammo count", NULL, ImGuiWindowFlags_AlwaysAutoResize))
									{
										ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
										ImGui::Checkbox("Ignore ammo limit", &ignoreMaxAmmo);
										ImGui::PopStyleVar();

										ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
										ImGui::SliderInt("Ammo count", &newAmmo, 0, ignoreMaxAmmo ? 999 : maxAmmo);
										ImGui::PopItemWidth();

										ImGui::Separator();

										if (ImGui::Button("OK", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
										{
											itmPtr->setAmmo(newAmmo);

											ImGui::CloseCurrentPopup();
										}

										ImGui::SameLine();

										if (ImGui::Button("Cancel", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
										{
											ImGui::CloseCurrentPopup();
										}
										ImGui::EndPopup();
									}
								}
							}

							// Change stack amount
							if (stackable)
							{
								static int newAmount = 1;

								if (ImGui::Button("Change stack amount"))
								{
									newAmount = itmPtr->num_2;
									ImGui::OpenPopup("Change stack amount");
								}

								// Always center this window when appearing
								ImVec2 center = ImGui::GetMainViewport()->GetCenter();
								ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

								if (ImGui::BeginPopupModal("Change stack amount", NULL, ImGuiWindowFlags_AlwaysAutoResize))
								{
									ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
									ImGui::SliderInt("Stack count", &newAmount, 1, curInfo.maxNum_4);
									ImGui::PopItemWidth();

									ImGui::Separator();

									if (ImGui::Button("OK", ImVec2(120, 0)))
									{
										itmPtr->num_2 = newAmount;

										ImGui::CloseCurrentPopup();
									}

									ImGui::SameLine();

									if (ImGui::Button("Cancel", ImVec2(120, 0)))
									{
										ImGui::CloseCurrentPopup();
									}
									ImGui::EndPopup();
								}
							}

							// Erase items
							{
								static bool DontAskAgain = false;

								ImGui::BeginDisabled(!itmPtr);
								if (ImGui::Button("Delete item"))
								{
									if (!DontAskAgain)
										ImGui::OpenPopup("Delete item");
									else
										ItemMgr->erase(itmPtr);
								}

								ImGui::EndDisabled();
								// Always center this window when appearing
								ImVec2 center = ImGui::GetMainViewport()->GetCenter();
								ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

								if (ImGui::BeginPopupModal("Delete item", NULL, ImGuiWindowFlags_AlwaysAutoResize))
								{
									ImGui::Text("Are you sure you want to delete this item?\n\n");
									ImGui::Separator();

									ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
									ImGui::Checkbox("Don't ask next time", &DontAskAgain);
									ImGui::PopStyleVar();

									if (ImGui::Button("Yes", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
									{
										ItemMgr->erase(itmPtr);
										ImGui::CloseCurrentPopup();
									}

									ImGui::SameLine();
									if (ImGui::Button("Cancel", ImVec2(120 * esHook._cur_monitor_dpi, 0))) { ImGui::CloseCurrentPopup(); }
									ImGui::EndPopup();
								}
							}

							ImGui::EndGroup();
							ImGui::EndDisabled();

							if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && disable)
								ImGui::SetTooltip("Items can only be edited while outside of menus.");

							if (ImGui::Button("Close"))
								ImGui::CloseCurrentPopup();

							ImGui::EndPopup();
						}
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Right-click to open popup");
					}
				}
			}
			ImGui::EndTable();
		}
		ImGui::PopStyleVar();

		ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

		// Disable button if no player character, or game wouldn't allow player to open inventory, or inventory is already opened
		bool disable =
			!PlayerPtr() ||
			!PlayerPtr()->subScrCheck() ||
			SubScreenWk->open_flag_2C != 0 ||
			OptionOpenFlag();

		// Item adder popup
		{
			ImGui::BeginDisabled(disable);
			if (ImGui::Button("Add items to inventory"))
				ImGui::OpenPopup("Inventory Item Adder");
			ImGui::EndDisabled();

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && disable)
				ImGui::SetTooltip("Items can only be added while outside of menus.");

			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			// Min/Max window sizes
			const float min_x = 840.0f * esHook._cur_monitor_dpi;
			const float min_y = 540.0f * esHook._cur_monitor_dpi;

			const float max_x = 1920.0f * esHook._cur_monitor_dpi;
			const float max_y = 1080.0f * esHook._cur_monitor_dpi;

			ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

			if (ImGui::BeginPopupModal("Inventory Item Adder", NULL))
			{
				// Inventory Item Adder
				static bool show_ALL = true;
				static bool show_WEAPON = true;
				static bool show_AMMO = true;
				static bool show_TREASURE = true;
				static bool show_CONSUMABLE = true;
				static bool show_KEY_ITEM = true;
				static bool show_WEAPON_MOD = true;
				static bool show_FILE = true;
				static bool show_TREASURE_MAP = true;
				static bool show_TREASURE_GEM = true;
				static bool show_BOTTLECAP = true;

				ImGui::Text("Filter categories:");

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.f, 2.f));
				if (ImGui::BeginTable("itmadderfilter", 6))
				{
					ImGui::TableNextColumn();
					ImGui::PushID(10001);
					if (ImGui::Checkbox("Show All", &show_ALL))
					{
						show_WEAPON = show_ALL;
						show_AMMO = show_ALL;
						show_TREASURE = show_ALL;
						show_CONSUMABLE = show_ALL;
						show_KEY_ITEM = show_ALL;
						show_WEAPON_MOD = show_ALL;
						show_FILE = show_ALL;
						show_TREASURE_MAP = show_ALL;
						show_TREASURE_GEM = show_ALL;
						show_BOTTLECAP = show_ALL;
					}

					ImGui::PopID();

					ImGui::BeginDisabled(show_ALL);
					ImGui::TableNextColumn(); ImGui::PushID(10002); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_WEAPON], &show_WEAPON); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10003); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_AMMO], &show_AMMO); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10006); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_TREASURE], &show_TREASURE); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10007); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_CONSUMABLE], &show_CONSUMABLE); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10008); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_KEY_ITEM], &show_KEY_ITEM); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10010); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_WEAPON_MOD], &show_WEAPON_MOD); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10011); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_FILE], &show_FILE); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10012); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_TREASURE_MAP], &show_TREASURE_MAP); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10013); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_TREASURE_GEM], &show_TREASURE_GEM); ImGui::PopID();
					ImGui::TableNextColumn(); ImGui::PushID(10014); ImGui::Checkbox(ITEM_TYPE_Names[ITEM_TYPE_BOTTLECAP], &show_BOTTLECAP); ImGui::PopID();
					ImGui::EndDisabled();

					ImGui::EndTable();
				}
				ImGui::PopStyleVar();

				static char searchText[256] = { 0 };
				static int columns = 3;

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.f, 2.f));
				if (ImGui::BeginTable("##itmaddersearch", 2))
				{
					ImGui::TableNextColumn();

					ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
					ImGui::InputText("Search", searchText, 256);
					ImGui::PopItemWidth();

					ImGui::SameLine();
					if (ImGui::SmallButton(ICON_FA_BACKSPACE))
						strcpy(searchText, "");

					ImGui::TableNextColumn();

					ImGui::Dummy(ImVec2((ImGui::GetContentRegionAvail().x - (270 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi)), 1));

					ImGui::SameLine();

					ImGui::Text("Columns: %i", columns);

					ImGui::SameLine();

					if (ImGui::Button("Remove column"))
						columns--;

					ImGui::SameLine();

					if (ImGui::Button("Add column"))
						columns++;

					columns = std::clamp(columns, 1, 6);

					ImGui::EndTable();
				}
				ImGui::PopStyleVar();

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

					// These get added to key items, but freeze game when examining
					EItemId::Krauser_Knife,
					EItemId::aaa,
					EItemId::Punisher_KeyItem,
					EItemId::Handgun_KeyItem,
					EItemId::Shotgun_KeyItem,
					EItemId::MineThrower_KeyItem,
					EItemId::Handcannon_KeyItem,
					EItemId::MineThrowerwScope_KeyItem,

					EItemId::Ada_New_Weapon, // gets added to key items as "Killer7 w/ Silencer", no icon, examine shows the model, but it's otherwise pretty useless
											 // maybe this can be made equippable if "piece_info" data is added for it though...

					// These don't seem to have any effect when added, probably requires some struct to be updated too..
					EItemId::Any
				};

				static EItemId itemId = EItemId::Bullet_45in_H;
				static int stackCount = 100;

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.f, 5.f));

				if (ImGui::BeginTable("##itmadderlist", columns, ImGuiTableFlags_BordersInner | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - (195 * esHook._cur_monitor_dpi))))
				{
					std::vector<ITEM_TYPE_mb> chosenTypes;

					if (show_WEAPON) { chosenTypes.push_back(ITEM_TYPE_WEAPON); chosenTypes.push_back(ITEM_TYPE_GRENADE); }
					if (show_AMMO) chosenTypes.push_back(ITEM_TYPE_AMMO);
					if (show_TREASURE) { chosenTypes.push_back(ITEM_TYPE_TREASURE); chosenTypes.push_back(ITEM_TYPE_TREASURE_MERCS); }
					if (show_CONSUMABLE) chosenTypes.push_back(ITEM_TYPE_CONSUMABLE);
					if (show_KEY_ITEM) { chosenTypes.push_back(ITEM_TYPE_KEY_ITEM); chosenTypes.push_back(ITEM_TYPE_IMPORTANT); }
					if (show_WEAPON_MOD) chosenTypes.push_back(ITEM_TYPE_WEAPON_MOD);
					if (show_FILE) chosenTypes.push_back(ITEM_TYPE_FILE);
					if (show_TREASURE_MAP) chosenTypes.push_back(ITEM_TYPE_TREASURE_MAP);
					if (show_TREASURE_GEM) chosenTypes.push_back(ITEM_TYPE_TREASURE_GEM);
					if (show_BOTTLECAP) chosenTypes.push_back(ITEM_TYPE_BOTTLECAP);

					for (auto& type : chosenTypes)
					{
						for (int item_id = 0; item_id < 272; item_id++)
						{
							auto size = badItems.size();

							if (std::find(badItems.begin(), badItems.end(), EItemId(item_id)) != badItems.end())
								continue;

							ITEM_INFO curInfo;
							curInfo.id_0 = ITEM_ID(item_id);
							bio4::itemInfo(ITEM_ID(item_id), &curInfo);

							if (curInfo.type_2 != type)
								continue;

							std::string name = std::string(ITEM_TYPE_Names[curInfo.type_2]) + ": " + EItemId_Names[int(item_id)];

							bool makeVisible = true;
							if (!searchTextUpper.empty())
							{
								std::string itemNameUpper = StrToUpper(name);

								makeVisible = itemNameUpper.find(searchTextUpper) != std::string::npos;
							}

							if (makeVisible)
							{
								ImGui::TableNextColumn();

								bool selected = itemId == EItemId(item_id);

								ImGui::BeginDisabled(wepAdded_ID != 0); // Disable if there's a pending weapon upgrade
								if (ImGui::Selectable(name.c_str(), &selected))
								{
									if (selected)
									{
										itemId = EItemId(item_id);
										if (curInfo.maxNum_4 > 1 && curInfo.type_2 == ITEM_TYPE_AMMO)
											stackCount = curInfo.maxNum_4; // help user by setting stack count to max for this item

										wepAdded_FirePower = 1;
										wepAdded_FiringSpeed = 1;
										wepAdded_ReloadSpeed = 1;
										wepAdded_Capacity = 1;
									}
								}
								ImGui::EndDisabled();
							}
						}
					}

					ImGui::EndTable();
				}
				ImGui::PopStyleVar();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ITEM_INFO info;
				info.id_0 = ITEM_ID(itemId);
				bio4::itemInfo(ITEM_ID(itemId), &info);
				if (TweaksDevMode)
					ImGui::Text("selected: id %d type %d def %d, max %d", info.id_0, info.type_2, info.defNum_3, info.maxNum_4);

				int maxFirePower = 1;
				int maxFiringSpeed = 1;
				int maxReloadSpeed = 1;
				int maxCapacity = 1;

				if (info.type_2 == ITEM_TYPE_WEAPON)
				{
					maxFirePower = bio4::WeaponId2MaxLevel(info.id_0, 0) + extra_upgrades[EItemId(info.id_0)].firePower;
					maxFiringSpeed = bio4::WeaponId2MaxLevel(info.id_0, 1) + extra_upgrades[EItemId(info.id_0)].firingSpeed;
					maxReloadSpeed = bio4::WeaponId2MaxLevel(info.id_0, 2) + extra_upgrades[EItemId(info.id_0)].reloadSpeed;
					maxCapacity = bio4::WeaponId2MaxLevel(info.id_0, 3) + extra_upgrades[EItemId(info.id_0)].Capacity;
				}

				ImGui::BeginDisabled(info.type_2 != ITEM_TYPE_WEAPON || wepAdded_ID != 0);
				ImGui::Text("Weapon Upgrades:");
				ImGui::EndDisabled();

				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4.f, 2.f));
				if (ImGui::BeginTable("##wepupgrades", 4, ImGuiTableFlags_BordersInnerV))
				{
					float sliderwidth = 15.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi;

					ImGui::TableNextColumn();

					ImGui::BeginDisabled(info.type_2 != ITEM_TYPE_WEAPON || wepAdded_ID != 0);

					ImGui::BeginDisabled(maxFirePower == 1);
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Fire Power").x - sliderwidth);
					ImGui::SliderInt("Fire Power", &wepAdded_FirePower, 1, maxFirePower, "%d", ImGuiSliderFlags_NoInput);
					ImGui::PopItemWidth();
					ImGui::EndDisabled();
					ImGui::TableNextColumn();

					ImGui::BeginDisabled(maxFiringSpeed == 1);
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Firing Speed").x - sliderwidth);
					ImGui::SliderInt("Firing Speed", &wepAdded_FiringSpeed, 1, maxFiringSpeed, "%d", ImGuiSliderFlags_NoInput);
					ImGui::PopItemWidth();
					ImGui::EndDisabled();
					ImGui::TableNextColumn();

					ImGui::BeginDisabled(maxReloadSpeed == 1);
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Reload Speed").x - sliderwidth);
					ImGui::SliderInt("Reload Speed", &wepAdded_ReloadSpeed, 1, maxReloadSpeed, "%d", ImGuiSliderFlags_NoInput);
					ImGui::PopItemWidth();
					ImGui::EndDisabled();
					ImGui::TableNextColumn();

					ImGui::BeginDisabled(maxCapacity == 1);
					ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Capacity").x - sliderwidth);
					ImGui::SliderInt("Capacity", &wepAdded_Capacity, 1, maxCapacity, "%d", ImGuiSliderFlags_NoInput);
					ImGui::PopItemWidth();
					ImGui::EndDisabled();

					ImGui::EndDisabled();

					ImGui::EndTable();
				}
				ImGui::PopStyleVar();

				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				bool stackable = info.maxNum_4 > 1;
				bool showsInInventory = bio4::itemShowsInInventory(info.type_2);

				if (!stackable)
					stackCount = 1;
				else
				{
					if (stackCount > info.maxNum_4)
						stackCount = info.maxNum_4;
				}

				ImGui::PushItemWidth(220.0f * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
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
				ImGui::PopItemWidth();

				ImGui_ItemSeparator();
				ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

				ImGui::BeginDisabled(!showsInInventory);
				if (ImGui::Checkbox("Open inventory after adding", &re4t::cfg->bTrainerOpenInventoryOnItemAdd))
					re4t::cfg->WriteSettings(true);

				if (!showsInInventory && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
					ImGui::SetTooltip("The selected item will appear on treasure/file tab instead of inventory.");
				ImGui::EndDisabled();

				// Disable button if no player character, or game wouldn't allow player to open inventory, or inventory is already opened
				bool disable =
					!PlayerPtr() ||
					!PlayerPtr()->subScrCheck() ||
					SubScreenWk->open_flag_2C != 0 ||
					OptionOpenFlag();

				ImGui::BeginDisabled(disable || wepAdded_ID != 0);
				if (ImGui::Button("Add Item", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
				{
					if (info.type_2 == ITEM_TYPE_WEAPON)
					{
						// Save info about the added wep so we can apply upgrades later
						wepAdded_ID = ITEM_ID(itemId);
					}

					if (re4t::cfg->bTrainerOpenInventoryOnItemAdd)
					{
						bCfgMenuOpen = false;
					}

					// Gotta unpause everything so the item actually gets added
					PauseGame(false);

					EItemId lambda_itemId = itemId;
					int lambda_stackCount = stackCount;
					bool lambda_alwaysShowInventory = re4t::cfg->bTrainerOpenInventoryOnItemAdd;
					bool lambda_handle_attache_case = true;
					Game_ScheduleInMainThread([lambda_itemId, lambda_stackCount, lambda_alwaysShowInventory, lambda_handle_attache_case]()
						{
							// Add the new item
							InventoryItemAdd(ITEM_ID(lambda_itemId), lambda_stackCount, lambda_alwaysShowInventory, lambda_handle_attache_case);

							// Pause the game again if needed
							PauseGame(bCfgMenuOpen && bPauseGameWhileInCfgMenu);
						});
				}
				ImGui::EndDisabled();

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && disable)
					ImGui::SetTooltip("Items can only be added while outside of menus.");

				ImGui::SameLine();

				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Close").x - 97.0f, 0.0f));
				ImGui::SameLine();
				if (ImGui::Button("Close", ImVec2(120 * esHook._cur_monitor_dpi, 0))) { ImGui::CloseCurrentPopup(); }

				ImGui::EndPopup();
			}
		}

		ImGui::SameLine();

		// Clear inventory
		{
			ImGui::BeginDisabled(disable);
			if (ImGui::Button("Clear inventory"))
				ImGui::OpenPopup("Clear inventory");
			ImGui::EndDisabled();

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && disable)
				ImGui::SetTooltip("Items can only be removed while outside of menus.");

			// Always center this window when appearing
			ImVec2 center = ImGui::GetMainViewport()->GetCenter();
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

			if (ImGui::BeginPopupModal("Clear inventory", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Are you sure you want to delete ALL items?\n\n");
				ImGui::Separator();

				if (ImGui::Button("Yes", ImVec2(120 * esHook._cur_monitor_dpi, 0)))
				{
					// Disarm current wep
					ItemMgr->arm(0);
					Game_ScheduleInMainThread([]() { bio4::WeaponChange(); });

					ItemMgr->eraseAll();

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(120 * esHook._cur_monitor_dpi, 0))) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
		}

		ImGui::SameLine();

		// Save/Load buttons
		{
			static float saveBtnWidth = 0;
			static float loadBtnWidth = 0;

			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - saveBtnWidth - loadBtnWidth - 25.0f, 0.0f));
			ImGui::SameLine();

			// Save json
			{
				if (ImGui::Button("Save inventory to .json"))
				{
					char timestamp[256];
					__time64_t time;
					struct tm ltime;
					_time64(&time);
					_localtime64_s(&ltime, &time);
					strftime(timestamp, _countof(timestamp), "inventory_%Y-%m-%d_%H-%M-%S", &ltime);

					ImGuiFileDialog::Instance()->OpenDialog("SaveJson", " Choose a File to Save", ".json", ".", timestamp, 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
					ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".json", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
				}

				saveBtnWidth = ImGui::GetItemRectSize().x;

				// Show file picker
				const float min_x = 840.0f * esHook._cur_monitor_dpi;
				const float min_y = 540.0f * esHook._cur_monitor_dpi;
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.f, 5.f));
				if (ImGuiFileDialog::Instance()->Display("SaveJson", ImGuiWindowFlags_NoCollapse, ImVec2(min_x, min_y)))
				{
					// If user pressed Ok
					if (ImGuiFileDialog::Instance()->IsOk())
					{
						using json = nlohmann::ordered_json;
						json js;

						cItem* itmPtr = ItemMgr->m_pItem_14;

						// Save the Attache Case regardless if the user has or not an attache case item in their inventory, as this is needed
						// to make sure the items will fit when loaded later
						js["player"]["attache_size"] = SubScreenWk->board_size_2AA;
						js["player"]["equipped_wep_id"] = ItemMgr->m_pWep_C ? ItemMgr->m_pWep_C->id_0 : -1;

						int m_array_num_1C = ItemMgr->m_array_num_1C;
						for (int loopcnt = 0; loopcnt <= m_array_num_1C; loopcnt++)
						{
							itmPtr++;

							if ((itmPtr->be_flag_4 & 1) == 0)
								continue;

							if (itmPtr->chr_5 != ItemMgr->m_char_13)
								continue;

							ITEM_INFO curInfo;
							curInfo.id_0 = ITEM_ID(itmPtr->id_0);
							bio4::itemInfo(ITEM_ID(itmPtr->id_0), &curInfo);

							std::string itemName = EItemId_Names[curInfo.id_0] + std::string(" - ") + IntToHexStr((int)itmPtr);

							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["id"] = itmPtr->id_0;
							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["pos_x"] = itmPtr->pos_x_A;
							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["pos_y"] = itmPtr->pos_y_B;
							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["orientation"] = itmPtr->orientation_C;
							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["num"] = itmPtr->num_2;
							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["be_flag"] = itmPtr->be_flag_4;
							js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["chr"] = itmPtr->chr_5;

							if (curInfo.type_2 == ITEM_TYPE_WEAPON)
							{
								// Upgrades
								js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["weapon0"]["firePower"] = itmPtr->getFirePower() + 1;
								js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["weapon0"]["firingSpeed"] = itmPtr->getFiringSpeed() + 1;
								js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["weapon0"]["reloadSpeed"] = itmPtr->getReloadSpeed() + 1;
								js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["weapon0"]["Capacity"] = itmPtr->getCapacity() + 1;

								// Ammo
								js["items"][ITEM_TYPE_Names[curInfo.type_2]][itemName]["weapon1"]["ammo"] = itmPtr->getAmmo();
							}
						}

						std::ofstream o(ImGuiFileDialog::Instance()->GetFilePathName());
						o << std::setw(4) << js << std::endl;
					}

					ImGuiFileDialog::Instance()->Close();
				}
				ImGui::PopStyleVar();
			}

			ImGui::SameLine();

			// Load json
			{
				ImGui::BeginDisabled(disable);
				if (ImGui::Button("Load inventory from .json"))
				{
					ImGuiFileDialog::Instance()->OpenDialog("LoadJson", " Choose a File to Load", ".json", ".", "", 1, nullptr);
					ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".json", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
				}
				ImGui::EndDisabled();

				loadBtnWidth = ImGui::GetItemRectSize().x;

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && disable)
					ImGui::SetTooltip("Items can only be added while outside of menus.");

				// Show file picker
				const float min_x = 840.0f * esHook._cur_monitor_dpi;
				const float min_y = 540.0f * esHook._cur_monitor_dpi;
				ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.f, 5.f));
				if (ImGuiFileDialog::Instance()->Display("LoadJson", ImGuiWindowFlags_NoCollapse, ImVec2(min_x, min_y)))
				{
					// If user pressed Ok
					if (ImGuiFileDialog::Instance()->IsOk())
					{
						using json = nlohmann::ordered_json;

						std::ifstream f(ImGuiFileDialog::Instance()->GetFilePathName());
						json js;

						try
						{
							js = json::parse(f);

							if (!js.is_null() && !js.empty())
							{
								std::vector<int> editedItms;

								// Clear inventory
								ItemMgr->eraseAll();

								// Disarm current wep
								ItemMgr->arm(0);
								Game_ScheduleInMainThread([]() { bio4::WeaponChange(); });

								// Setup proper board size to fit everything from the json
								SubScreenWk->board_size_2AA = js["player"]["attache_size"];
								SubScreenWk->board_next_2AB = js["player"]["attache_size"];

								// Add items from json
								for (auto& types : js["items"].items())
								{
									for (const auto& items : types.value())
									{
										// Add item to inventory
										ITEM_ID itemId = ITEM_ID(items["id"]);
										int itemNum = items["num"];

										// Use our cSceSys__scheduler_Hook for the Assault_Jacket, otherwise the game crashes
										if (EItemId(itemId) == EItemId::Assault_Jacket)
											Game_ScheduleInMainThread([itemId, itemNum]() { InventoryItemAdd(itemId, itemNum, false, false); });
										else
											InventoryItemAdd(itemId, itemNum, false, false);

										// Loop through all the items to find the one we just added, excluding items that have already been modified by us before.
										// We do this so we can restore the stats of all items, including ones that have the same ITEM_ID in the inventory 
										// (such as multiple First-Aid sprays), which would fail with ItemMgr->search since that func only returns the first instance it finds.
										// (Is there a less weird way to do this?)
										cItem* itmPtr = ItemMgr->m_pItem_14;

										int m_array_num_1C = ItemMgr->m_array_num_1C;
										for (int loopcnt = 0; loopcnt <= m_array_num_1C; loopcnt++)
										{
											itmPtr++;

											if ((itmPtr->be_flag_4 & 1) == 0)
												continue;

											if (itmPtr->chr_5 != ItemMgr->m_char_13)
												continue;

											if (itmPtr->id_0 == itemId)
											{
												if (std::find(editedItms.begin(), editedItms.end(), (int)itmPtr) != editedItms.end())
													continue;
												else
													break;
											}
										}

										// Apply the stats from the json
										if (itmPtr)
										{
											editedItms.push_back((int)itmPtr);

											itmPtr->pos_x_A = items["pos_x"];
											itmPtr->pos_y_B = items["pos_y"];
											itmPtr->orientation_C = items["orientation"];
											itmPtr->num_2 = items["num"];
											itmPtr->be_flag_4 = items["be_flag"];
											itmPtr->chr_5 = items["chr"];

											ITEM_INFO curInfo;
											curInfo.id_0 = ITEM_ID(itmPtr->id_0);
											bio4::itemInfo(ITEM_ID(itmPtr->id_0), &curInfo);

											if (curInfo.type_2 == ITEM_TYPE_WEAPON)
											{
												itmPtr->setFirePower(items["weapon0"]["firePower"]);
												itmPtr->setFiringSpeed(items["weapon0"]["firingSpeed"]);
												itmPtr->setReloadSpeed(items["weapon0"]["reloadSpeed"]);
												itmPtr->setCapacity(items["weapon0"]["Capacity"]);

												itmPtr->setAmmo(items["weapon1"]["ammo"]);
											}
										}
									}
								}

								// Try equipping weapon if set
								int equipped_id = js["player"]["equipped_wep_id"];
								if (equipped_id != -1)
								{
									cItem* item = ItemMgr->search(equipped_id);
									if (ItemMgr->arm(item))
									{
										Game_ScheduleInMainThread([]() { bio4::WeaponChange(); });
									}
								}
							}
						}
						catch (json::parse_error&)
						{
							#ifdef VERBOSE
							con.log("Failed to parse inventory .json!");
							#endif

							spd::log()->info("{} -> Failed to parse inventory .json!", __FUNCTION__);
						}
					}
					ImGuiFileDialog::Instance()->Close();
				}
				ImGui::PopStyleVar();
			}
		}
	}
}