#include "dllmain.h"
#include "UI_DebugWindows.h"
#include "Settings.h"
#include "SDK/room_jmp.h"
#include "SDK/filter00.h"

void UI_Window::UpdateWindowTitle()
{
	if (pConfig->sTrainerFocusUIKeyCombo.empty())
		windowTitle = origWindowTitle;
	else
		windowTitle = origWindowTitle + " - " + pConfig->sTrainerFocusUIKeyCombo + " to Focus/Unfocus";
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
		ImGui::SetNextWindowSize(ImVec2(320, 120), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(320, 120), ImVec2(420, 630));

		ImGui::Begin(windowTitle.c_str(), &retVal);
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
				ImGui::PushItemWidth((ImGui::GetContentRegionAvail().x - 375.0f) * pConfig->fFontSizeScale);

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
					ImGui::Dummy(ImVec2(10, 5));

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
						if (ImGui::BeginTable("##flags", 2, ImGuiTableFlags_ScrollY, ImVec2(ImGui::GetContentRegionAvail().x, 245 * pConfig->fFontSizeScale)))
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
					ImGui::Dummy(ImVec2(10, 5));

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

bool UI_Globals::Render(bool WindowMode)
{
	cRoomJmp_data* test = cRoomJmp_data::get();
	cRoomJmp_stage* stage = test->GetStage(1);
	CRoomInfo* room = stage->GetRoom(0);

	char* room_name = room->getName();
	char* room_person = room->getPerson();
	char* room_person2 = room->getPerson2();

	ImGui::SetNextWindowSizeConstraints(ImVec2(250, 100), ImVec2(250, 550));

	bool retVal = true; // set to false on window close
	ImGui::Begin(windowTitle.c_str(), &retVal, ImGuiWindowFlags_AlwaysAutoResize);
	{
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
			pConfig->iWeaponHotkeyWepIds[0], pConfig->iWeaponHotkeyWepIds[1], pConfig->iWeaponHotkeyWepIds[2], pConfig->iWeaponHotkeyWepIds[3], pConfig->iWeaponHotkeyWepIds[4]);

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
	}
	return true;
}

bool UI_AreaJump::Render(bool WindowMode)
{
	bool retVal = true; // set to false on window close

	if (WindowMode)
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(300, 350));

		ImGui::Begin(windowTitle.c_str(), &retVal, ImGuiWindowFlags_AlwaysAutoResize);
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
			UpdateRoomInfo();
		}

		cRoomJmp_stage* stage = roomJmpData->GetStage(curStage);
		if (stage)
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Room").x - 10.0f);
			std::string curRoomStr = RoomDisplayString(curStage, curRoomIdx);
			if (ImGui::BeginCombo("Room", curRoomStr.c_str()))
			{
				for (uint32_t i = 0; i < stage->nData_0; i++)
				{
					std::string roomStr = RoomDisplayString(curStage, i);

					bool is_selected = (curRoomIdx == i);
					if (ImGui::Selectable(roomStr.c_str(), is_selected))
					{
						curRoomIdx = i;
						UpdateRoomInfo();
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();

			CRoomInfo* roomData = stage->GetRoom(curRoomIdx);
			if(roomData)
			{
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
				ImGui::Dummy(ImVec2(10, 5));

				//ImGui::InputText("Room", curRoomNo, 256); // some reason this breaks combo box?

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Position").x - 10.0f);
				ImGui::InputFloat3("Position", (float*)&curRoomPosition);
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Rotation").x - 10.0f);
				ImGui::InputFloat("Rotation", &curRoomRotation);

				ImGui::BeginDisabled(!bio4::CardCheckDone());
				if (ImGui::Button("Jump!"))
				{
					GlobalPtr()->JumpPoint_4FAF = curRoomIdx;
					AreaJump(roomData->roomNo_2, curRoomPosition, curRoomRotation);
				}
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
		ImGui::SetNextWindowSize(ImVec2(300, 410), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(300, 100), ImVec2(300, 410));

		ImGui::Begin(windowTitle.c_str(), &retVal);
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

		ImGui::Dummy(ImVec2(10, 10));

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
