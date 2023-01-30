#include "dllmain.h"
#include "UI_DebugWindows.h"
#include "Settings.h"
#include "SDK/room_jmp.h"
#include "SDK/filter00.h"
#include "ConsoleWnd.h"
#include <FAhashes.h>

void UI_Window::UpdateWindowTitle()
{
	if (re4t::cfg->sTrainerFocusUIKeyCombo.empty())
		windowTitle = origWindowTitle;
	else
		windowTitle = origWindowTitle + " - " + re4t::cfg->sTrainerFocusUIKeyCombo + " to Focus/Unfocus";
}

std::string UI_EmManager::EmDisplayString(cEm& em, bool showEmPointers)
{
	char tmpBuf[256];
	if (showEmPointers)
		sprintf(tmpBuf, "#%d:0x%x %s (type %x) flags %x", em.guid_F8, (uint32_t)&em, cEmMgr::EmIdToName(em.id_100).c_str(), int(em.type_101), int(em.be_flag_4));
	else
		sprintf(tmpBuf, "#%d %s (type %x) flags %x", em.guid_F8, cEmMgr::EmIdToName(em.id_100).c_str(), int(em.type_101), int(em.be_flag_4));

	return tmpBuf;
}

void ImGui_ItemSeparator(); // cfgMenu.cpp

bool UI_EmManager::Render(bool WindowMode)
{
	bool retVal = true; // set to false on window close
	GLOBAL_WK* pG = GlobalPtr();

	if (WindowMode)
	{
		// Min/Max window sizes
		const float min_x = 320.0f * esHook._cur_monitor_dpi;
		const float min_y = 150.0f * esHook._cur_monitor_dpi;

		const float max_x = 420.0f * esHook._cur_monitor_dpi;
		const float max_y = 630.0f * esHook._cur_monitor_dpi;

		ImGui::SetNextWindowSize(ImVec2(min_x, min_y), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

		ImGui::SetNextWindowBgAlpha(backgroundOpacity); // Background transparency

		ImGuiWindowFlags window_flags = 0;

		if (hideTitleBar) window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (hideScrollBar) window_flags |= ImGuiWindowFlags_NoScrollbar;
		if (hideBackground) window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin(windowTitle.c_str(), &retVal, window_flags);

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Hide title bar", &hideTitleBar);
			ImGui::Checkbox("Hide scrollbar", &hideScrollBar);
			ImGui::Checkbox("Hide background", &hideBackground);

			ImGui::Spacing();
			ImGui::Text("Background opacity:");
			ImGui::SetNextItemWidth(100.0f * esHook._cur_monitor_dpi);
			ImGui::SliderFloat("##bgopacity", &backgroundOpacity, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndPopup();
		}

		// Options button
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
	}

	{
		auto& emMgr = *EmMgrPtr();
		// cEm list
		{
			ImGui::Text("Em Count: %d | Max: %d", emMgr.count_valid(), emMgr.count());
			ImGui::SameLine();

			if (WindowMode)
				ImGui::Checkbox("Show addresses", &showEmPointers);
			else
			{
				ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Open new Em Manager window").x - 25, 0));
				ImGui::SameLine();
				if (ImGui::Button("Open new Em Manager window"))
					UI_NewEmManager();
			}

			std::string currentEmStr = "N/A";
			if (emIdx >= 0)
			{
				cEm* em = emMgr[emIdx];
				if (em)
					currentEmStr = EmDisplayString(*em, showEmPointers);
			}

			if (WindowMode)
			{
				if (emIdx >= 0 && !WindowResized)
				{
					ImGui::SetWindowSize(ImVec2(0, 0)); // Auto-fit
					WindowResized = true;
				}
			}
			else
			{
				ImGui::Checkbox("Only active", &onlyShowValidEms); ImGui::SameLine();
				ImGui::Checkbox("Only ESL-spawned", &onlyShowESLSpawned); ImGui::SameLine();
				ImGui::Checkbox("Show addresses", &showEmPointers);
			}

			if (WindowMode)
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Em").x - 10.0f);
			else
				ImGui::PushItemWidth(360.0f * esHook._cur_monitor_dpi);

			if (ImGui::BeginCombo("Em", currentEmStr.c_str()))
			{
				int i = 0;
				for (auto& em : emMgr)
				{
					if (!onlyShowValidEms || em.IsValid())
					{
						if (!onlyShowESLSpawned || em.IsSpawnedByESL())
						{
							std::string emStr = EmDisplayString(em, showEmPointers);

							const bool is_selected = (emIdx == i);
							if (ImGui::Selectable(emStr.c_str(), is_selected))
								emIdx = i;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
					}

					i++;
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			if (WindowMode)
			{
				ImGui::Checkbox("Only active", &onlyShowValidEms); ImGui::SameLine();
				ImGui::Checkbox("Only ESL-spawned", &onlyShowESLSpawned);
			}
		}

		// cEm info
		{
			if (emIdx >= 0)
			{
				cEm* em = emMgr[emIdx];
				if (em)
				{
					ImGui::Separator();
					ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));

					if (showEmPointers)
						ImGui::Text("#%d:0x%x %s (type %x)", em->guid_F8, em, cEmMgr::EmIdToName(em->id_100).c_str(), int(em->type_101));
					else
						ImGui::Text("#%d %s (type %x)", em->guid_F8, cEmMgr::EmIdToName(em->id_100).c_str(), int(em->type_101));

					static Vec copyPosition = { 0 };
					if (ImGui::Button("Copy position"))
						copyPosition = em->pos_94;

					ImGui::SameLine();

					if (AtariFlagBackupSet)
					{
						em->atari_2B4.m_flag_1A = AtariFlagBackup;
						AtariFlagBackupSet = false;
					}
					if (ImGui::Button("Paste position"))
					{
						em->pos_94 = copyPosition;
						em->pos_old_110 = copyPosition;

						// temporarily disable atariInfo collision, prevents colliding with map
						// TODO: some reason using em->move() isn't enough to bypass collision for some Em's
						// as workaround we let game render another frame and then restore flags above
						AtariFlagBackup = em->atari_2B4.m_flag_1A;
						AtariFlagBackupSet = true;
						em->atari_2B4.m_flag_1A = 0;

						em->matUpdate();
						em->move();
					}

					cPlayer* player = PlayerPtr();
					cPlayer* ashley = AshleyPtr();
					if (em != player)
					{
						if (ImGui::Button("Teleport player"))
						{
							player->pos_94 = em->pos_94;
							player->pos_old_110 = em->pos_94;

							// temporarily disable atariInfo collision, prevents colliding with map
							uint16_t flagBackup = player->atari_2B4.m_flag_1A;
							player->atari_2B4.m_flag_1A = 0;

							player->matUpdate();
							player->move();

							player->atari_2B4.m_flag_1A = flagBackup;
						}
						ImGui::SameLine();

						if (ImGui::Button("Move to player"))
						{
							em->pos_94 = player->pos_94;
							em->pos_old_110 = player->pos_old_110;

							// temporarily disable atariInfo collision, prevents colliding with map
							AtariFlagBackup = em->atari_2B4.m_flag_1A;
							AtariFlagBackupSet = true;
							em->atari_2B4.m_flag_1A = 0;

							em->matUpdate();
							em->move();
						}
					}

					if (WindowMode)
					{
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Flags").x - 60.0f);
						if (ImGui::BeginListBox("Flags"))
						{
							bool collisionEnabled = (em->atari_2B4.m_flag_1A & SAT_SCA_ENABLE) == SAT_SCA_ENABLE;
							if (ImGui::Checkbox("MapCollision", &collisionEnabled))
							{
								if (collisionEnabled)
									em->atari_2B4.m_flag_1A |= SAT_SCA_ENABLE;
								else
									em->atari_2B4.m_flag_1A &= ~SAT_SCA_ENABLE;
							}
							bool emCollisionEnabled = (em->atari_2B4.m_flag_1A & SAT_OBA_ENABLE) == SAT_OBA_ENABLE;
							if (ImGui::Checkbox("EmCollision", &emCollisionEnabled))
							{
								if (emCollisionEnabled)
									em->atari_2B4.m_flag_1A |= SAT_OBA_ENABLE;
								else
									em->atari_2B4.m_flag_1A &= ~SAT_OBA_ENABLE;
							}

							for (int i = 0; i < 32; i++)
							{
								bool bitSet = (em->be_flag_4 & (1 << i)) != 0;

								std::string lbl = cUnit::GetBeFlagName(i);
								if (ImGui::Checkbox(lbl.c_str(), &bitSet))
								{
									if (bitSet)
										em->be_flag_4 = em->be_flag_4 | (1 << i);
									else
										em->be_flag_4 = em->be_flag_4 & ~(1 << i);
								}
							}
							ImGui::EndListBox();
						}
						ImGui::PopItemWidth();

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Position").x - 10.0f);
						if (ImGui::InputFloat3("Position", (float*)&em->pos_94, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
							em->matUpdate();

						if (ImGui::InputFloat3("Rotation", (float*)&em->ang_A0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
							em->matUpdate();

						if (ImGui::InputFloat3("Scale", (float*)&em->scale_AC, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
						{
							if (IsGanado(em->id_100))
							{
								// cEm10 holds a seperate scale value that it seems to grow/shrink the actual scale towards each frame
								// make sure we update that as well
								cEm10* em10 = (cEm10*)em;
								em10->Scale_498 = em->scale_AC;
							}
							em->matUpdate();
						}
						ImGui::PopItemWidth();

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Motion Speed").x - 10.0f);
						ImGui::InputFloat("Motion Speed", &em->Motion_1D8.Seq_speed_C0, 0.1f);
						
						int hpCur = em->hp_324;
						if (em == player)
							hpCur = pG->playerHpCur_4FB4;
						else if (em == ashley)
							hpCur = pG->subHpCur_4FB8;

						if (ImGui::InputInt("HpCur", &hpCur, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							em->hp_324 = hpCur;
							if (em == player)
								pG->playerHpCur_4FB4 = hpCur;
							else if (em == ashley)
								pG->subHpCur_4FB8 = hpCur;
						}

						int hpMax = em->hp_max_326;
						if (em == player)
							hpMax = pG->playerHpMax_4FB6;
						else if (em == ashley)
							hpMax = pG->subHpMax_4FBA;
						if (ImGui::InputInt("HpMax", &hpMax, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
						{
							em->hp_max_326 = hpMax;
							if (em == player)
								pG->playerHpMax_4FB6 = hpMax;
							else if (em == ashley)
								pG->subHpMax_4FBA = hpMax;
						}

						ImGui::PopItemWidth();

						ImGui::Text("Routine: %02X %02X %02X %02X", em->r_no_0_FC, em->r_no_1_FD, em->r_no_2_FE, em->r_no_3_FF);
						ImGui::Text("Parts count: %d", em->PartCount());
						if (em->emListIndex_3A0 != 255)
							ImGui::Text("ESL: %s (%s) @ #%d (offset 0x%x)", GetEmListEnumName(pG->curEmListNumber_4FB3), GetEmListName(pG->curEmListNumber_4FB3), int(em->emListIndex_3A0), int(em->emListIndex_3A0) * sizeof(EM_LIST));

					}
					else
					{
						ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5.0f, 5.0f));
						if (ImGui::BeginTable("##flags", 2, ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 245 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi)))
						{
							ImGui::TableNextColumn();

							ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Position").x - 10.0f);
							if (ImGui::InputFloat3("Position", (float*)&em->pos_94, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
								em->matUpdate();

							if (ImGui::InputFloat3("Rotation", (float*)&em->ang_A0, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
								em->matUpdate();

							if (ImGui::InputFloat3("Scale", (float*)&em->scale_AC, "%.3f", ImGuiInputTextFlags_EnterReturnsTrue))
							{
								if (IsGanado(em->id_100))
								{
									// cEm10 holds a seperate scale value that it seems to grow/shrink the actual scale towards each frame
									// make sure we update that as well
									cEm10* em10 = (cEm10*)em;
									em10->Scale_498 = em->scale_AC;
								}
								em->matUpdate();
							}
							ImGui::PopItemWidth();

							ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Motion Speed").x - 10.0f);
							ImGui::InputFloat("Motion Speed", &em->Motion_1D8.Seq_speed_C0, 0.1f);

							int hpCur = em->hp_324;
							if (em == player)
								hpCur = pG->playerHpCur_4FB4;
							else if (em == ashley)
								hpCur = pG->subHpCur_4FB8;

							if (ImGui::InputInt("HpCur", &hpCur, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								em->hp_324 = hpCur;
								if (em == player)
									pG->playerHpCur_4FB4 = hpCur;
								else if (em == ashley)
									pG->subHpCur_4FB8 = hpCur;
							}

							int hpMax = em->hp_max_326;
							if (em == player)
								hpMax = pG->playerHpMax_4FB6;
							else if (em == ashley)
								hpMax = pG->subHpMax_4FBA;
							if (ImGui::InputInt("HpMax", &hpMax, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
							{
								em->hp_max_326 = hpMax;
								if (em == player)
									pG->playerHpMax_4FB6 = hpMax;
								else if (em == ashley)
									pG->subHpMax_4FBA = hpMax;
							}

							ImGui::PopItemWidth();

							ImGui::Text("Routine: %02X %02X %02X %02X", em->r_no_0_FC, em->r_no_1_FD, em->r_no_2_FE, em->r_no_3_FF);
							ImGui::Text("Parts count: %d", em->PartCount());
							if (em->emListIndex_3A0 != 255)
								ImGui::Text("ESL: %s (%s) @ #%d (offset 0x%x)", GetEmListEnumName(pG->curEmListNumber_4FB3), GetEmListName(pG->curEmListNumber_4FB3), int(em->emListIndex_3A0), int(em->emListIndex_3A0) * sizeof(EM_LIST));

							ImGui::TableNextColumn();

							float flagsWidth = ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Flags").x - 20.0f;
							float flagsHeight = ImGui::GetContentRegionAvail().y - 10.0f;
							if (ImGui::BeginListBox("Flags", ImVec2(flagsWidth, flagsHeight)))
							{
								bool collisionEnabled = (em->atari_2B4.m_flag_1A & SAT_SCA_ENABLE) == SAT_SCA_ENABLE;
								if (ImGui::Checkbox("MapCollision", &collisionEnabled))
								{
									if (collisionEnabled)
										em->atari_2B4.m_flag_1A |= SAT_SCA_ENABLE;
									else
										em->atari_2B4.m_flag_1A &= ~SAT_SCA_ENABLE;
								}
								bool emCollisionEnabled = (em->atari_2B4.m_flag_1A & SAT_OBA_ENABLE) == SAT_OBA_ENABLE;
								if (ImGui::Checkbox("EmCollision", &emCollisionEnabled))
								{
									if (emCollisionEnabled)
										em->atari_2B4.m_flag_1A |= SAT_OBA_ENABLE;
									else
										em->atari_2B4.m_flag_1A &= ~SAT_OBA_ENABLE;
								}

								for (int i = 0; i < 32; i++)
								{
									bool bitSet = (em->be_flag_4 & (1 << i)) != 0;

									std::string lbl = cUnit::GetBeFlagName(i);
									if (ImGui::Checkbox(lbl.c_str(), &bitSet))
									{
										if (bitSet)
											em->be_flag_4 = em->be_flag_4 | (1 << i);
										else
											em->be_flag_4 = em->be_flag_4 & ~(1 << i);
									}
								}
								ImGui::EndListBox();
							}

							ImGui::EndTable();
						}
						ImGui::PopStyleVar();
					}
					
					// works, but unsure what to display atm
					/*
					static int partIdx = -1;
					if (ImGui::BeginListBox("Test"))
					{
						int i = 0;
						cParts* part = em->childParts_F4;
						while (part != nullptr)
						{
							sprintf(tmpBuf, "#%d", i);
							const bool is_selected = (partIdx == i);
							if (ImGui::Selectable(tmpBuf, is_selected))
								partIdx = i;
							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
							i++;
							part = part->nextParts_F4;
						}
						ImGui::EndListBox();
					}*/

					ImGui::Separator();
					ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));

					ImGui::Text("Modification:");

					if (WindowMode)
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("PositionChange").x - 10.0f);
					else
						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("PositionChange").x - 200.0f);

					ImGui::InputFloat3("PositionChange", addPosition, "%.3f");
					ImGui::PopItemWidth();
					if (ImGui::Button("Apply"))
					{
						em->pos_94.x += addPosition[0];
						em->pos_94.y += addPosition[1];
						em->pos_94.z += addPosition[2];
						em->matUpdate();
					}
				}
			}
		}

		if (WindowMode)
			ImGui::End();
	}

	return retVal;
}

bool UI_IDInspector::Init()
{
	GLOBAL_WK* pG = GlobalPtr();
	if (pG)
	{
		// default to IDC_TITLE when opening ID inspector at the title menu
		if (pG->curRoomId_4FAC == 0x120)
			IDClassIdx = 6;
	}
	return true;
}

void UI_IDInspector::DrawTree(int idx)
{
	ID_UNIT* node = unitPtrs[idx];

	std::vector<int> childrenNos = {};
	for (size_t i = idx; i < unitPtrs.size(); ++i)
	{
		if (unitPtrs[i]->parentNo_6 == node->unitNo_1)
			childrenNos.push_back(i);
	}
	bool hasChildren = childrenNos.size() > 0;

	ImGuiTreeNodeFlags treeFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_FramePadding;

	bool selected = selectedId == unitPtrs[idx];
	if (selected)
		treeFlags |= ImGuiTreeNodeFlags_Selected;

	if (hasChildren)
		treeFlags |= ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	else
		treeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	char label[32];
	sprintf(label, node->texId_78 == 0xFF ? "Anchor %d" : "Tex %d", node->unitNo_1);

	ImGui::TableNextRow();

	// unitNo
	ImGui::TableNextColumn();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 0.0f));
	bool open = ImGui::TreeNodeEx(label, treeFlags);
	ImGui::PopStyleVar();

	if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
	{
		selectedId = node;
		undoCopy = *selectedId;;
	}

	// markNo
	ImGui::TableNextColumn();
	if (node->markNo_3 != 0xFF)
		ImGui::Text("%Xh", node->markNo_3);
	else
		ImGui::TextDisabled("--");

	// texId
	ImGui::TableNextColumn();
	if (node->texId_78 != 0xFF)
		ImGui::Text("%d", node->texId_78);
	else
		ImGui::TextDisabled("--");

	// size
	ImGui::TableNextColumn();
	if (node->size0_H_E0 > 0)
		ImGui::Text("%.0f x %.0f", node->size0_W_DC, node->size0_H_E0);
	else
		ImGui::TextDisabled("--");

	if (hasChildren && open)
	{
		for (size_t i = 0; i < childrenNos.size(); ++i)
		{
			DrawTree(childrenNos[i]);
		}

		ImGui::Unindent(3.0f);
		ImGui::TreePop();
		ImGui::Indent(3.0f);
	}
}

bool UI_IDInspector::Render(bool WindowMode)
{
	bool retVal = true; // set to false on window close

	static const char* IDClassNames[] = {
		"IDC_LIFE_METER", "IDC_BLLT_ICON", "IDC_LASER_GAUGE ", "IDC_ACT_BUTTON", "IDC_EVENT", "IDC_EVENT (MERCS)", "IDC_TITLE", "IDC_TITLE_MENU",
		"IDC_PRICE_01|IDC_SSCRN_FAR_0", "IDC_PRICE_02|IDC_TITLE", "IDC_PRICE_00|IDC_ACT_BUTTON",
		"IDC_OPTION", "IDC_OPTION_BG", "IDC_SCOPE", "IDC_BINOCULAR", "IDC_CINESCO", "IDC_COUNT_DOWN", "IDC_EXAMINE",
		"IDC_DATA", "IDC_DEAD", "IDC_CONTINUE", "IDC_MSG_WINDOW", "IDC_WIP", "IDC_SUB_MISSION",
		"IDC_GAUGE", "IDC_CAMERA_BATTERY",
		/*"IDC_SSCRN_MAIN_MENU",*/ "IDC_SSCRN_BACK_GROUND", "IDC_SSCRN_PESETA", "IDC_SSCRN_CONFIRM", "IDC_SSCRN_ETC", "IDC_SSCRN_NEAR_0", "IDC_SSCRN_NEAR_1",
		"IDC_SSCRN_NEAR_2", "IDC_SSCRN_NEAR_3", "IDC_SSCRN_0", "IDC_SSCRN_1", "IDC_SSCRN_2", "IDC_SSCRN_3", "IDC_SSCRN_FAR_0", "IDC_SSCRN_FAR_1",
		"IDC_SSCRN_FAR_2", "IDC_SSCRN_FAR_3", "IDC_SSCRN_CKPT_0", "IDC_SSCRN_CKPT_1", "IDC_SSCRN_CKPT_2", "IDC_SSCRN_CKPT_3",
		"IDC_NUM_00", "IDC_NUM_61", "IDC_PRICE_00", "IDC_PRICE_01", "IDC_PRICE_02", "IDC_PRICE_03", "IDC_PRICE_04", "IDC_TOOL",
		"IDC_NUM_00|IDC_SSCRN_BACK_GROUND", "IDC_NUM_00|IDC_SSCRN_ETC|IDC_SSCRN_CONFIRM",
		"IDC_ANY"
	};

	auto updateUnitPtrs = [&]()
	{
		unitPtrs.clear();

		ID_CLASS idClass = IDC_ANY;
		bool isMercs = false;

		switch (IDClassIdx)
		{
		case  0: idClass = IDC_LIFE_METER; break;
		case  1: idClass = IDC_BLLT_ICON; break;
		case  2: idClass = IDC_LASER_GAUGE; break;
		case  3: idClass = IDC_ACT_BUTTON; break;
		case  4: idClass = IDC_EVENT; break;
		case  5: idClass = IDC_EVENT; isMercs = true; break;
		case  6: idClass = IDC_TITLE; break;
		case  7: idClass = IDC_TITLE_MENU; break;
		case  8: idClass = (ID_CLASS)(IDC_PRICE_01 | IDC_SSCRN_FAR_0); break;
		case  9: idClass = (ID_CLASS)(IDC_PRICE_02 | IDC_TITLE); break;
		case 10: idClass = (ID_CLASS)(IDC_PRICE_00 | IDC_ACT_BUTTON); break;
		case 11: idClass = IDC_OPTION; break;
		case 12: idClass = IDC_OPTION_BG; break;
		case 13: idClass = IDC_SCOPE; break;
		case 14: idClass = IDC_BINOCULAR; break;
		case 15: idClass = IDC_CINESCO; break;
		case 16: idClass = IDC_COUNT_DOWN; break;
		case 17: idClass = IDC_EXAMINE; break;
		case 18: idClass = IDC_DATA; break;
		case 19: idClass = IDC_DEAD; break;
		case 20: idClass = IDC_CONTINUE; break;
		case 21: idClass = IDC_MSG_WINDOW; break;
		case 22: idClass = IDC_WIP; break;
		case 23: idClass = IDC_SUB_MISSION; break;
		case 24: idClass = IDC_GAUGE; break;
		case 25: idClass = IDC_CAMERA_BATTERY_MB; break;
		case 26: idClass = IDC_SSCRN_BACK_GROUND; break;
		case 27: idClass = IDC_SSCRN_PESETA; break;
		case 28: idClass = IDC_SSCRN_CONFIRM; break;
		case 29: idClass = IDC_SSCRN_ETC; break;
		case 30: idClass = IDC_SSCRN_NEAR_0; break;
		case 31: idClass = IDC_SSCRN_NEAR_1; break;
		case 32: idClass = IDC_SSCRN_NEAR_2; break;
		case 33: idClass = IDC_SSCRN_NEAR_3; break;
		case 34: idClass = IDC_SSCRN_0; break;
		case 35: idClass = IDC_SSCRN_1; break;
		case 36: idClass = IDC_SSCRN_2; break;
		case 37: idClass = IDC_SSCRN_3; break;
		case 38: idClass = IDC_SSCRN_FAR_0; break;
		case 39: idClass = IDC_SSCRN_FAR_1; break;
		case 40: idClass = IDC_SSCRN_FAR_2; break;
		case 41: idClass = IDC_SSCRN_FAR_3; break;
		case 42: idClass = IDC_SSCRN_CKPT_0; break;
		case 43: idClass = IDC_SSCRN_CKPT_1; break;
		case 44: idClass = IDC_SSCRN_CKPT_2; break;
		case 45: idClass = IDC_SSCRN_CKPT_3; break;
		case 46: idClass = IDC_NUM_00; break;
		case 47: idClass = IDC_NUM_61; break;
		case 48: idClass = IDC_PRICE_00; break;
		case 49: idClass = IDC_PRICE_01; break;
		case 50: idClass = IDC_PRICE_02; break;
		case 51: idClass = IDC_PRICE_03; break;
		case 52: idClass = IDC_PRICE_04; break;
		case 53: idClass = IDC_TOOL; break;
		case 54: idClass = (ID_CLASS)(IDC_NUM_00 | IDC_SSCRN_BACK_GROUND); break;
		case 55: idClass = (ID_CLASS)(IDC_NUM_00 | IDC_SSCRN_ETC | IDC_SSCRN_CONFIRM); break;
		case 56: idClass = IDC_ANY;
		}

		curIDSystem = isMercs ? &mercIdPtr()->_idSys : IDSystemPtr();
		ID_UNIT* tex = curIDSystem->m_IdUnit_50;

		for (int i = 0; i < curIDSystem->m_maxId_0; ++i)
		{
			if (tex->classNo_2 == idClass)
				unitPtrs.push_back(tex);

			++tex;
		}
	};

	const float min_x = 400.0f * esHook._cur_monitor_dpi;
	const float min_y = 75.0f * esHook._cur_monitor_dpi;

	const float max_x = 400.0f * esHook._cur_monitor_dpi;
	const float max_y = 750.0f * esHook._cur_monitor_dpi;

	ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

	ImGui::SetNextWindowBgAlpha(backgroundOpacity); // Background transparency

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;

	ImGui::Begin(windowTitle.c_str(), &retVal, window_flags);
	{
		updateUnitPtrs();

		ImGui::Separator();

		ImGui::PushItemWidth(225 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);
		if (ImGui::BeginCombo("ID_CLASS", IDClassNames[IDClassIdx], ImGuiComboFlags_HeightLarge))
		{
			for (int i = 0; i < IM_ARRAYSIZE(IDClassNames); ++i)
			{
				bool selected = false;
				if (ImGui::Selectable(IDClassNames[i], &selected))
				{
					if (selected)
					{
						IDClassIdx = i;

						updateUnitPtrs();
						if (!unitPtrs.empty())
						{
							selectedId = unitPtrs[0];
							undoCopy = *selectedId;;
						}
					}
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::IsItemHovered())
		{
			IDClassIdx -= (int)ImGui::GetIO().MouseWheel;
			IDClassIdx = std::clamp(IDClassIdx, 0, IM_ARRAYSIZE(IDClassNames) - 1);
		}
		ImGui::PopItemWidth();

		ImGui::Separator();
		ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));

		if (!unitPtrs.empty())
		{
			if (!selectedId)
			{
				selectedId = unitPtrs[0];
				undoCopy = *selectedId;
			}

			static bool tableView = false;
			static ImVec4 activeCol = ImColor(150, 10, 40, 255);
			static ImVec4 inactiveCol = ImColor(31, 30, 31, 0);
			ImGui::PushStyleColor(ImGuiCol_Button, !tableView ? activeCol : inactiveCol);
			if (ImGui::Button("Tree view"))
				tableView = false;
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Button, tableView ? activeCol : inactiveCol);
			if (ImGui::Button("Table view"))
				tableView = true;
			ImGui::PopStyleColor();

			ImGuiTableFlags tableFlags = ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_ScrollY
				|  ImGuiTableFlags_PadOuterX | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_RowBg;

			ImVec2 outerSize = ImVec2(0.0f, ImGui::GetTextLineHeightWithSpacing() * 14.5f);
			if (!tableView && ImGui::BeginTable("##IDTreeView", 4, tableFlags, outerSize))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("unitNo", ImGuiTableColumnFlags_WidthFixed, 155.0f);
				ImGui::TableSetupColumn("markNo", ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("texId",  ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("size",   ImGuiTableColumnFlags_WidthFixed, 0.0f);
				ImGui::TableHeadersRow();

				for (size_t i = 0; i < unitPtrs.size(); ++i)
				{
					if (unitPtrs[i]->pParent_68 == nullptr || unitPtrs[i]->pParent_68->classNo_2 != unitPtrs[i]->classNo_2)
						DrawTree(i);
				}

				ImGui::EndTable();
			}

			tableFlags |= ImGuiTableFlags_ScrollX | ImGuiTableFlags_Reorderable;
			// todo: add support for sorting by column
			//tableFlags |= ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_SortTristate | ImGuiTableFlags_SortMulti
			if (tableView && ImGui::BeginTable("##IDTable", 11, tableFlags, outerSize))
			{
				ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
				ImGui::TableSetupColumn("unitNo",      ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("markNo",      ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("texId",       ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("parentNo",    ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("parentClass", ImGuiTableColumnFlags_WidthFixed, 70.0f);
				ImGui::TableSetupColumn("levelNo",     ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("rowNo",       ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("type",        ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("texNo",       ImGuiTableColumnFlags_WidthFixed, 50.0f);
				//ImGui::TableSetupColumn("color",       ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("loop_flag",   ImGuiTableColumnFlags_WidthFixed, 50.0f);
				ImGui::TableSetupColumn("size",        ImGuiTableColumnFlags_WidthFixed, 0.0f);
				ImGui::TableHeadersRow();

				for (size_t i = 0; i < unitPtrs.size(); i++)
				{
					bool selected = unitPtrs[i] == selectedId;

					ImGui::TableNextRow();

					// unitNo
					ImGui::TableNextColumn();
					if (ImGui::Selectable(std::to_string(unitPtrs[i]->unitNo_1).c_str(), &selected, ImGuiSelectableFlags_SpanAllColumns))
					{
						if (selected)
						{
							selectedId = unitPtrs[i];
							undoCopy = *selectedId;;
						}
					}

					// markNo
					ImGui::TableNextColumn();
					if (unitPtrs[i]->markNo_3 != 0xFF)
						ImGui::Text("%Xh", unitPtrs[i]->markNo_3);
					else
						ImGui::TextDisabled("--");

					// texId
					ImGui::TableNextColumn();
					if (unitPtrs[i]->texId_78 != 0xFF)
						ImGui::Text("%d", unitPtrs[i]->texId_78);
					else
						ImGui::TextDisabled("--");

					// parentNo
					ImGui::TableNextColumn();
					if (unitPtrs[i]->parentNo_6 != 0xFF)
						ImGui::Text("%d", unitPtrs[i]->parentNo_6);
					else
						ImGui::TextDisabled("--");

					// classNo
					ImGui::TableNextColumn();
					if (unitPtrs[i]->pParent_68)
						ImGui::Text("%d", unitPtrs[i]->pParent_68->classNo_2);
					else
						ImGui::TextDisabled("--");

					// levelNo
					ImGui::TableNextColumn();
					ImGui::Text("%d", unitPtrs[i]->levelNo_5);

					// rowNo
					ImGui::TableNextColumn();
					ImGui::Text("%d", unitPtrs[i]->rowNo_7);

					// type
					ImGui::TableNextColumn();
					ImGui::Text("%d", unitPtrs[i]->type_4);

					// texNo
					ImGui::TableNextColumn();
					ImGui::Text("%d", unitPtrs[i]->texNo_7A);

					// color
					// fixme: has a weird clipping issue with frozen column headers
					//ImGui::TableNextColumn();
					//ImU32 cellBGCol = ImGui::GetColorU32(ImVec4(
					//	unitPtrs[i]->col_R_F4 / 255.0f,
					//	unitPtrs[i]->col_G_F8 / 255.0f,
					//	unitPtrs[i]->col_B_FC / 255.0f,
					//	unitPtrs[i]->col_A_100 / 255.0f));
					//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, cellBGCol);

					// loop_flag
					ImGui::TableNextColumn();
					ImGui::Text("%d", unitPtrs[i]->loop_flag_87);

					// size
					ImGui::TableNextColumn();
					if (unitPtrs[i]->size0_H_E0 > 0)
						ImGui::Text("%.0fx%.0f", unitPtrs[i]->size0_W_DC, unitPtrs[i]->size0_H_E0);
					else
						ImGui::TextDisabled("--");
				}
				ImGui::EndTable();
			}
		
			ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));

			bool visible = selectedId->be_flag_0 & ID_BE_FLAG_VISIBLE;
			if (ImGui::Checkbox("Visible", &visible))
				selectedId->be_flag_0 ^= ID_BE_FLAG_VISIBLE;

			ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("(Hold Ctrl to make faster edits)").x - 10.0f);
			ImGui::Text("(Hold Ctrl to make faster edits)");

			static bool maintainAspectRatio = true;

			ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));

			ImGui::PushItemWidth(140 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);

			ImGui::InputFloat("posX", &selectedId->pos0_94.x, 0.1f, 1.0f, "%.4f");

			ImGui::SameLine(200);
			float origW = selectedId->size0_W_DC;
			if (ImGui::InputFloat("sizeW", &selectedId->size0_W_DC, 0.1f, 1.0f, "%.4f"))
				if (maintainAspectRatio)
					selectedId->size0_H_E0 *= selectedId->size0_W_DC / origW;

			ImGui::InputFloat("posY", &selectedId->pos0_94.y, 0.1f, 1.0f, "%.4f");

			ImGui::SameLine(200);
			float origH = selectedId->size0_H_E0;
			if (ImGui::InputFloat("sizeH", &selectedId->size0_H_E0, 0.1f, 1.0f, "%.4f"))
				if (maintainAspectRatio)
					selectedId->size0_W_DC *= selectedId->size0_H_E0 / origH;

			ImGui::InputFloat("posZ", &selectedId->pos0_94.z, 0.1f, 1.0f, "%.4f");

			ImGui::PopItemWidth();

			ImGui::SameLine(200);
			ImGui::Checkbox("Maintain aspect ratio", &maintainAspectRatio);

			ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));
			if (ImGui::Button("Undo"))
				*selectedId = undoCopy;

			ImGui::SameLine();
			if (ImGui::Button("Reset Time"))
				curIDSystem->setTime(selectedId, 0); // useful for resetting animated textures

			ImGui::PushItemWidth(200 * re4t::cfg->fFontSizeScale * esHook._cur_monitor_dpi);

			ImGui::SameLine(175);
			ImVec4 color0 = ImGui::ColorConvertU32ToFloat4(selectedId->col0_A_EF << 24 | selectedId->col0_B_EE << 16 | selectedId->col0_G_ED << 8 | selectedId->col0_R_EC);
			ImGuiColorEditFlags colorPickerFlags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoDragDrop |
				ImGuiColorEditFlags_AlphaPreviewHalf | ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar;
			if (ImGui::ColorEdit4("##IDcolorpicker0", (float*)&color0, colorPickerFlags))
			{
				uint32_t converted = ImGui::ColorConvertFloat4ToU32(color0);
				selectedId->col0_A_EF = (converted >> 24) & 0xFF;
				selectedId->col0_B_EE = (converted >> 16) & 0xFF;
				selectedId->col0_G_ED = (converted >> 8) & 0xFF;
				selectedId->col0_R_EC = converted & 0xFF;
			}

			// for textures that oscillate between two colors
			// editing color1 without the proper flags causes textures to disappear, so hide this unless we know the texture supports it
			if (selectedId->col1_R_F0 || selectedId->col1_G_F1 || selectedId->col1_B_F2 || selectedId->col1_A_F3)
			{
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 167);
				ImVec4 color1 = ImGui::ColorConvertU32ToFloat4(selectedId->col1_A_F3 << 24 | selectedId->col1_B_F2 << 16 | selectedId->col1_G_F1 << 8 | selectedId->col1_R_F0);
				if (ImGui::ColorEdit4("##IDcolorpicker1", (float*)&color1, colorPickerFlags))
				{
					uint32_t converted = ImGui::ColorConvertFloat4ToU32(color1);
					selectedId->col1_A_F3 = (converted >> 24) & 0xFF;
					selectedId->col1_B_F2 = (converted >> 16) & 0xFF;
					selectedId->col1_G_F1 = (converted >> 8) & 0xFF;
					selectedId->col1_R_F0 = converted & 0xFF;
				}
			}

			ImGui::PopItemWidth();
		}
		else
			selectedId = nullptr;

		ImGui::End();
	}

	return retVal;
}

bool UI_Globals::Render(bool WindowMode)
{
	cRoomJmp_data* test = cRoomJmp_data::get();
	cRoomJmp_stage* stage = test->GetStage(1);
	CRoomInfo* room = stage->GetRoom(0);

	char* room_name = room->getName();
	char* room_person = room->getPerson();
	char* room_person2 = room->getPerson2();

	const float min_x = 250.0f * esHook._cur_monitor_dpi;
	const float min_y = 100.0f * esHook._cur_monitor_dpi;

	const float max_x = 250.0f * esHook._cur_monitor_dpi;
	const float max_y = 550.0f * esHook._cur_monitor_dpi;

	ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

	bool retVal = true; // set to false on window close

	ImGui::SetNextWindowBgAlpha(backgroundOpacity); // Background transparency

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;

	if (hideTitleBar) window_flags |= ImGuiWindowFlags_NoTitleBar;
	if (hideBackground) window_flags |= ImGuiWindowFlags_NoBackground;

	ImGui::Begin(windowTitle.c_str(), &retVal, window_flags);
	{
		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Hide title bar", &hideTitleBar);
			ImGui::Checkbox("Hide background", &hideBackground);

			ImGui::Spacing();
			ImGui::Text("Background opacity:");
			ImGui::SetNextItemWidth(100.0f * esHook._cur_monitor_dpi);
			ImGui::SliderFloat("##bgopacity", &backgroundOpacity, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndPopup();
		}

		// Options button
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");

		auto& emMgr = *EmMgrPtr();
		GLOBAL_WK* globals = GlobalPtr();
		TITLE_WORK* titleWork = TitleWorkPtr();
		if (titleWork)
		{
			ImGui::Text("Menu Routine: %d-%d-%d-%d-%d", int(titleWork->Rno0_0), int(titleWork->Rno1_1), int(titleWork->Rno2_2), int(titleWork->Rno3_3), int(titleWork->titleStart_Rno_8));
			const char* menuRoutine0Names[] = {"Init", "Wait", "Nintendo", "Warning", "Logo", "Main", "Sub", "Exit", "Ada" };
			if(titleWork->Rno0_0 >= int(TITLE_WORK::Routine0::Init) && titleWork->Rno0_0 <= int(TITLE_WORK::Routine0::Ada))
				ImGui::Text("Menu Routine0: %s", menuRoutine0Names[titleWork->Rno0_0]);

			ImGui::Separator();
		}

		// fGPUUsagePtr is almost always 100% or higher, not sure if it's actually valid...
		ImGui::Text("Engine load (CPU / GPU): %.0f%% / %.0f%%", *fCPUUsagePtr * 100.f, *fGPUUsagePtr * 100.f);

		ImGui::Text("Area: R%03x", int(globals->curRoomId_4FAC & 0xFFF));
		
		ImGui::Text("Routine: %d-%d-%d-%d", int(globals->Rno0_20), int(globals->Rno1_21), int(globals->Rno2_22), int(globals->Rno3_23));
		
		// TODO: make this a GLOBAL_WK::Routine0Name() func?
		const char* routine0Names[] = { "Init", "StageInit", "RoomInit", "MainLoop", "Doordemo", "Ending", "Option" };
		if (globals->Rno0_20 >= int(GLOBAL_WK::Routine0::Init) && globals->Rno0_20 <= int(GLOBAL_WK::Routine0::Option))
			ImGui::Text("Routine0: %s", routine0Names[globals->Rno0_20]);

		ImGui::Separator();

		cPlayer* player = PlayerPtr();
		if (!player)
			ImGui::Text("Player Em: N/A");
		else
		{
			int playerIdx = emMgr.indexOf(player);
			ImGui::Text("Player Em: #%d (%p)", playerIdx, player);
			if (playerIdx >= 0)
			{
				ImGui::SameLine();
				if (ImGui::Button("View##viewpl"))
					UI_NewEmManager(playerIdx);
			}
		}

		cPlayer* ashley = AshleyPtr();
		if (!ashley)
			ImGui::Text("Ashley Em: N/A");
		else
		{
			int ashleyIdx = emMgr.indexOf(ashley);
			ImGui::Text("Ashley Em: #%d (%p)", ashleyIdx, ashley);
			if (ashleyIdx >= 0)
			{
				ImGui::SameLine();
				if (ImGui::Button("View##viewsub"))
					UI_NewEmManager(ashleyIdx);
			}
		}

		ImGui::Separator();

		ImGui::Text("Chapter: %d", globals->chapter_4F9A);

		if (player)
			ImGui::Text("Player HP: %d", globals->playerHpCur_4FB4);

		if (ashley)
			ImGui::Text("Ashley HP: %d", globals->subHpCur_4FB8);

		static std::string diff;
		switch (globals->gameDifficulty_847C) {
		case GameDifficulty::VeryEasy:
			diff = "Very Easy";
			break;
		case GameDifficulty::Easy:
			diff = "Easy";
			break;
		case GameDifficulty::Medium:
			diff = "Normal";
			break;
		case GameDifficulty::Pro:
			diff = "Professional";
			break;
		default:
			diff = "unk";
			break;
		}

		ImGui::Text("Game difficulty: %s", diff.c_str());

		ImGui::Text("DynamicDifficulty Level: %d", globals->dynamicDifficultyLevel_4F98);

		ImGui::Text("DynamicDifficulty Points: %d", globals->dynamicDifficultyPoints_4F94);

		ImGui::Text("Pesetas: %d", globals->goldAmount_4FA8);

		ImGui::Text("Kills: %d", globals->g_kill_cnt_8468);

		ImGui::Text("Play time: %d:%02d:%02d", iCurPlayTime[0], iCurPlayTime[1], iCurPlayTime[2]);

		if (ItemMgr->m_pWep_C)
			ImGui::Text("Weapon ID: %d", ItemMgr->m_pWep_C->id_0);

		if (ItemPiecePtr())
		{
			ImGui::Text("itemPiece pItem: %p", ItemPiecePtr()->pItem_24);
			if (ItemPiecePtr()->pItem_24)
				ImGui::Text("itemPiece pItem.id: %d", ItemPiecePtr()->pItem_24->id_0);
		}

		ImGui::Text("Cur bound weapon IDs: %d %d %d %d %d", 
			re4t::cfg->iWeaponHotkeyWepIds[0], re4t::cfg->iWeaponHotkeyWepIds[1], re4t::cfg->iWeaponHotkeyWepIds[2], re4t::cfg->iWeaponHotkeyWepIds[3], re4t::cfg->iWeaponHotkeyWepIds[4]);

		ImGui::End();
	}

	return retVal;
}

std::string UI_AreaJump::RoomDisplayString(int stageIdx, int roomIdx)
{
	cRoomJmp_data* roomJmpData = cRoomJmp_data::get();

	std::string retVal = "N/A";
	if (!roomJmpData)
		return retVal;

	auto* stage = roomJmpData->GetStage(stageIdx);
	if (!stage)
		return retVal;

	auto* room = stage->GetRoom(roomIdx);
	if (!room)
		return retVal;

	char ret[256];
	sprintf_s(ret, "R%03x - %s", int(room->roomNo_2 & 0xFFF), room->getName());
	return ret;
}

void UI_AreaJump::UpdateRoomInfo()
{
	cRoomJmp_data* roomJmpData = cRoomJmp_data::get();
	cRoomJmp_stage* stage = roomJmpData->GetStage(curStage);
	CRoomInfo* room = stage->GetRoom(curRoomIdx);
	//sprintf_s(curRoomNo, "%03X", int(room->roomNo_2 & 0xFFF));
	if ((room->flag_0 & 1) != 0)
	{
		curRoomPosition = room->pos_4;
		curRoomRotation = room->r_10;
	}
	else
	{
		curRoomPosition = { 0 };
		curRoomRotation = 0;
	};

	roomPosRotNeedsUpdate = false;
}

bool UI_AreaJump::Init()
{
	GLOBAL_WK* pG = GlobalPtr();
	if (pG)
	{
		// Init current stage to whatever stage player is currently on
		curStage = (pG->curRoomId_4FAC & 0xFF00) >> 8;
		// ...but don't let us init it to the boring 0 stage
		if (curStage <= 0)
			curStage = 1;

		if (pG->curRoomId_4FAC == 0x120)
		{
			// On main menu or the boring r120 intro stage, try using their previous area jump indexes
			curStage = re4t::cfg->iTrainerLastAreaJumpStage;
			curRoomIdx = re4t::cfg->iTrainerLastAreaJumpRoomIdx;
		}
	}
	return true;
}

bool UI_AreaJump::Render(bool WindowMode)
{
	bool retVal = true; // set to false on window close

	if (WindowMode)
	{
		const float min_x = 300.0f * esHook._cur_monitor_dpi;
		const float min_y = 130.0f * esHook._cur_monitor_dpi;

		const float max_x = 300.0f * esHook._cur_monitor_dpi;
		const float max_y = 350.0f * esHook._cur_monitor_dpi;

		ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

		ImGui::SetNextWindowBgAlpha(backgroundOpacity); // Background transparency

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar;

		if (hideTitleBar) window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (hideBackground) window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin(windowTitle.c_str(), &retVal, window_flags);

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Hide title bar", &hideTitleBar);
			ImGui::Checkbox("Hide background", &hideBackground);

			ImGui::Spacing();
			ImGui::Text("Background opacity:");
			ImGui::SetNextItemWidth(100.0f * esHook._cur_monitor_dpi);
			ImGui::SliderFloat("##bgopacity", &backgroundOpacity, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndPopup();
		}

		// Options button
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
	}

	{
		cRoomJmp_data* roomJmpData = cRoomJmp_data::get();

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Stage").x - 10.0f);
		if (ImGui::InputInt("Stage", &curStage))
		{
			if (curStage < 0)
				curStage = AREAJUMP_MAX_STAGE;
			if (curStage > AREAJUMP_MAX_STAGE)
				curStage = 0;

			curRoomIdx = 0;
			roomPosRotNeedsUpdate = true;
		}

		cRoomJmp_stage* stage = roomJmpData->GetStage(curStage);
		if (!stage)
			curStage = 1;
		else
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Room").x - 10.0f);
			std::string curRoomStr = RoomDisplayString(curStage, curRoomIdx);
			if (ImGui::BeginCombo("Room", curRoomStr.c_str(), ImGuiComboFlags_HeightLarge))
			{
				for (int i = 0; i < stage->nData_0; i++)
				{
					std::string roomStr = RoomDisplayString(curStage, i);

					bool is_selected = (curRoomIdx == i);
					if (ImGui::Selectable(roomStr.c_str(), is_selected))
					{
						curRoomIdx = i;
						roomPosRotNeedsUpdate = true;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			CRoomInfo* roomData = stage->GetRoom(curRoomIdx);
			if (!roomData)
				curRoomIdx = 0;
			else
			{
				if (roomPosRotNeedsUpdate)
					UpdateRoomInfo();

				char* person1 = roomData->getPerson();
				char* person2 = roomData->getPerson2();

				bool hasPerson1 = person1 && strlen(person1) > 0;
				bool hasPerson2 = person2 && strlen(person2) > 0;

				ImGui::BeginDisabled(true);
				if (hasPerson1)
					ImGui::Text("Scenario: %s", person1);
				if (hasPerson2)
				{
					if (hasPerson1)
						ImGui::SameLine();

					ImGui::Text("Program: %s", person2);
				}
				ImGui::EndDisabled();

				ImGui::Separator();
				ImGui::Dummy(ImVec2(10, 5 * esHook._cur_monitor_dpi));

				//ImGui::InputText("Room", curRoomNo, 256); // some reason this breaks combo box?

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Position").x - 10.0f);
				ImGui::InputFloat3("Position", (float*)&curRoomPosition);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Rotation").x - 10.0f);
				ImGui::InputFloat("Rotation", &curRoomRotation);

				ImGui::BeginDisabled(!bio4::CardCheckDone());
				if (ImGui::Button("Jump!"))
				{
					// Save users "last area jump stage/room" settings to INI
					re4t::cfg->iTrainerLastAreaJumpStage = curStage;
					re4t::cfg->iTrainerLastAreaJumpRoomIdx = curRoomIdx;
					re4t::cfg->WriteSettings(true);

					GlobalPtr()->JumpPoint_4FAF = curRoomIdx;
					AreaJump(roomData->roomNo_2, curRoomPosition, curRoomRotation);
				}

				if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !bio4::CardCheckDone())
					ImGui::SetTooltip("Area jump is unavailable during intro/'PRESS ANY KEY' screens.");

				ImGui::EndDisabled();
			}
		}
		
		if (WindowMode)
			ImGui::End();
	}

	return retVal;
}

bool UI_FilterTool::Render(bool WindowMode)
{
	bool retVal = true; // set to false on window close

	if (WindowMode)
	{
		// Min/Max window sizes
		const float min_x = 300.0f * esHook._cur_monitor_dpi;
		const float min_y = 100.0f * esHook._cur_monitor_dpi;

		const float max_x = 300.0f * esHook._cur_monitor_dpi;
		const float max_y = 440.0f * esHook._cur_monitor_dpi;

		ImGui::SetNextWindowSize(ImVec2(max_x, max_y), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

		ImGui::SetNextWindowBgAlpha(backgroundOpacity); // Background transparency

		ImGuiWindowFlags window_flags = 0;

		if (hideTitleBar) window_flags |= ImGuiWindowFlags_NoTitleBar;
		if (hideScrollBar) window_flags |= ImGuiWindowFlags_NoScrollbar;
		if (hideBackground) window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::Begin(windowTitle.c_str(), &retVal, window_flags);

		// Options menu
		if (ImGui::BeginPopup("Options"))
		{
			ImGui::Checkbox("Hide title bar", &hideTitleBar);
			ImGui::Checkbox("Hide scrollbar", &hideScrollBar);
			ImGui::Checkbox("Hide background", &hideBackground);

			ImGui::Spacing();
			ImGui::Text("Background opacity:");
			ImGui::SetNextItemWidth(100.0f * esHook._cur_monitor_dpi);
			ImGui::SliderFloat("##bgopacity", &backgroundOpacity, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

			ImGui::EndPopup();
		}

		// Options button
		if (ImGui::Button("Options"))
			ImGui::OpenPopup("Options");
	}

	{
		cLightEnv* LightEnv = &LightMgr->LightEnv_20;

		bool env_changed = false;

		ImGui::Text("DoF / Filter01:");

		ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("ContrastLevel").x - 10.0f);

		if (ImGui::SliderInt("Distance", &LightEnv->FocusZ_28, 0, 65535))
			env_changed = true;

		int alphaLvl = LightEnv->FocusLevel_2D;
		if (ImGui::SliderInt("AlphaLvl", &alphaLvl, 0, 10))
		{
			env_changed = true;
			LightEnv->FocusLevel_2D = uint8_t(alphaLvl);
		}

		const char* DoFModeNames[] = { "NEAR", "FAR", "FollowPL NEAR", "FollowPL FAR" };
		const char* curMode = DoFModeNames[LightEnv->FocusMode_2E];
		if (ImGui::BeginCombo("Mode", curMode))
		{
			for (int i = 0; i < 4; i++)
			{
				bool selected = false;
				if (ImGui::Selectable(DoFModeNames[i], &selected))
					if (selected)
					{
						env_changed = true;
						LightEnv->FocusMode_2E = uint8_t(i);
					}
			}
			ImGui::EndCombo();
		}

		ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi));

		ImGui::Text("Blur / Filter00:");

		cFilter00Params* params = cFilter00Params::get();
		cFilter00Params2* params2 = cFilter00Params2::get();

		const char* BlurTypeNames[] = { "NORMAL", "SPREAD", "ADD", "SUBTRACT" };
		const char* curType = BlurTypeNames[LightEnv->blur_type_F0];
		if (ImGui::BeginCombo("Type", curType))
		{
			for (int i = 0; i < 4; i++)
			{
				bool selected = false;
				if (ImGui::Selectable(BlurTypeNames[i], &selected))
					if (selected)
					{
						env_changed = true;
						LightEnv->blur_type_F0 = uint8_t(i);
					}
			}
			ImGui::EndCombo();
		}

		int blurRate = LightEnv->blur_rate_2F;
		if (ImGui::SliderInt("Rate", &blurRate, 0, 255))
		{
			env_changed = true;
			LightEnv->blur_rate_2F = uint8_t(blurRate);
		}

		int blurPower = LightEnv->blur_power_F1;
		if (ImGui::SliderInt("Power", &blurPower, 0, 255))
		{
			env_changed = true;
			LightEnv->blur_power_F1 = uint8_t(blurPower);
		}

		int contLevel = LightEnv->contrast_level_F5;
		if (ImGui::SliderInt("ContrastLevel", &contLevel, 0, 255))
		{
			env_changed = true;
			LightEnv->contrast_level_F5 = uint8_t(contLevel);
		}

		int contPow = LightEnv->contrast_pow_F6;
		if (ImGui::SliderInt("ContrastPow", &contPow, 0, 255))
		{
			env_changed = true;
			LightEnv->contrast_pow_F6 = uint8_t(contPow);
		}

		int contBias = LightEnv->contrast_bias_F7;
		if (ImGui::SliderInt("ContrastBias", &contBias, 0, 255))
		{
			env_changed = true;
			LightEnv->contrast_bias_F7 = uint8_t(contBias);
		}

		int effRate = params->eff_blur_rate;
		if (ImGui::SliderInt("EffBlurRate", &effRate, 0, 255))
			params->eff_blur_rate = uint8_t(effRate);

		ImGui::SliderFloat("EffSpreadNum", &params->eff_spread_num, 0, 255);

		if (env_changed)
			LightMgr->setEnv(LightEnv, LightMgr->m_Hokan_178);

		ImGui::PopItemWidth();

		if (WindowMode)
			ImGui::End();
	}

	return retVal;
}
