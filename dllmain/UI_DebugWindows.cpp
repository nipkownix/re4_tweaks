#include "dllmain.h"
#include "UI_DebugWindows.h"
#include "Game.h"

std::string UI_EmManager::EmDisplayString(int i, cEm& em)
{
	char tmpBuf[256];
	if (showEmPointers)
		sprintf(tmpBuf, "#%d:0x%x c%s (type %x) flags %x", i, (uint32_t)&em, cEmMgr::EmIdToName(em.id_100).c_str(), int(em.type_101), int(em.be_flag_4));
	else
		sprintf(tmpBuf, "#%d c%s (type %x) flags %x", i, cEmMgr::EmIdToName(em.id_100).c_str(), int(em.type_101), int(em.be_flag_4));

	return tmpBuf;
}

bool UI_EmManager::Render()
{
	//ImGui::SetNextWindowSizeConstraints(ImVec2(940, 640), ImVec2(1280, 720));

	bool retVal = true; // set to false on window close

	// TODO: find how to get rid of this string, seems imgui needs each UI to have unique ID, EmManager## doesn't work?
	std::string id = std::string("EmManager") + std::to_string(windowId);
	ImGui::Begin(id.c_str(), &retVal, ImGuiWindowFlags_NoCollapse);
	{
		auto pos = ImGui::GetWindowPos();
		auto draw = ImGui::GetWindowDrawList();

		auto& emMgr = *EmMgrPtr();
		// cEm list
		{
			ImGui::Text("Em Count: %d | Max: %d", emMgr.count_valid(), emMgr.count());
			ImGui::SameLine();
			ImGui::Checkbox("Show addresses", &showEmPointers);

			std::string currentEmStr = "N/A";
			if (emIdx >= 0)
			{
				cEm* em = emMgr[emIdx];
				if (em)
					currentEmStr = EmDisplayString(emIdx, *em);
			}

			if (ImGui::BeginCombo("CurrentEm", currentEmStr.c_str()))
			{
				int i = 0;
				for (auto& em : emMgr)
				{
					if (!onlyShowValidEms || em.IsValid())
					{
						if (!onlyShowESLSpawned || em.IsSpawnedByESL())
						{
							std::string emStr = EmDisplayString(i, em);

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

			ImGui::Checkbox("Active", &onlyShowValidEms); ImGui::SameLine();
			ImGui::Checkbox("ESL-spawned", &onlyShowESLSpawned);
		}

		// cEm info
		{
			if (emIdx >= 0)
			{
				cEm* em = emMgr[emIdx];
				if (em)
				{
					if (showEmPointers)
						ImGui::Text("#%d:0x%x c%s (type %x)", emIdx, em, cEmMgr::EmIdToName(em->id_100).c_str(), int(em->type_101));
					else
						ImGui::Text("#%d c%s (type %x)", emIdx, cEmMgr::EmIdToName(em->id_100).c_str(), int(em->type_101));

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

					if (ImGui::BeginListBox("Flags"))
					{
						bool collisionEnabled = (em->atari_2B4.m_flag_1A & 0x100) == 0x100;
						if (ImGui::Checkbox("MapCollision", &collisionEnabled))
						{
							if (collisionEnabled)
								em->atari_2B4.m_flag_1A |= 0x100;
							else
								em->atari_2B4.m_flag_1A &= ~0x100;
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
							Vec* scale_bk_498 = (Vec*)((uint8_t*)em + 0x498);
							*scale_bk_498 = em->scale_AC;
						}
						em->matUpdate();
					}

					ImGui::InputFloat("MotInfo.Speed", &em->Motion_1D8.Seq_speed_C0, 0.1f);

					int hpCur = em->hp_324;
					if (ImGui::InputInt("HpCur", &hpCur, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
						em->hp_324 = hpCur;

					int hpMax = em->hp_max_326;
					if (ImGui::InputInt("HpMax", &hpMax, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue))
						em->hp_max_326 = hpMax;

					ImGui::Text("Routine: %02X %02X %02X %02X", em->r_no_0_FC, em->r_no_1_FD, em->r_no_2_FE, em->r_no_3_FF);
					//ImGui::Text("Parts count: %d", em->PartCount());
					//if (em->emListIndex_3A0 != 255)
					//	ImGui::Text("ESL: %s @ #%d (offset 0x%x)", getEmListName(GlobalPtr()->curEmListNumber_4FB3), int(em->emListIndex_3A0), int(em->emListIndex_3A0) * sizeof(EM_LIST));

					ImGui::Dummy(ImVec2(10, 25));

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

					ImGui::Dummy(ImVec2(10, 25));

					ImGui::Text("Modification:");
					ImGui::InputFloat3("PositionChange", addPosition, "%.3f");
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

		ImGui::End();
	}

	return retVal;
}
