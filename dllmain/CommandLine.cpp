#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "SDK/room_jmp.h"
#include <shellapi.h>

uint32_t paramRoomValue = 0;
int paramLoadSaveSlot = 0;

int loadSaveSlot = 0; // copy value for our dataSelect hook to use

void(__cdecl* titleMain)(TITLE_WORK* pT);
void __cdecl titleMain_Hook(TITLE_WORK* pT)
{
	// Wait till Rno1_1 == 1 before acting, to ensure system save data is loaded in
	// (makes it a little slower to load in though, maybe a -fast option would be useful, or some way to speed up the fade-ins)
	if (pT->Rno1_1 == 1)
	{
		// -load & -room should hopefully both work together
		// you can use -load to pick a save with the chara/inventory/etc you want
		// and then use -room to pick the room to load into
		// (some rooms are only designed to work with certain charas though, hopefully can be fixed in future...)

		if (paramLoadSaveSlot > 0) // "-load #" argument
		{
			loadSaveSlot = paramLoadSaveSlot;

			if (!bio4::CardLoad(0))
				FlagSet(GlobalPtr()->Flags_SYSTEM_0_54, uint32_t(Flags_SYSTEM::SYS_SOFT_RESET), true);

			// These need to be set otherwise game will hang after loading
			// (copied from after titleMain CardLoad call)
			pT->Rno1_1 = 4;
			FlagSet(GlobalPtr()->Flags_SYSTEM_0_54, uint32_t(Flags_SYSTEM::SYS_LOAD_GAME), true);

			paramLoadSaveSlot = 0; // only run this once
		}

		if (paramRoomValue > 0) // "-room rXXX" argument
		{
			int stageNum = (paramRoomValue & 0xFF00) >> 8;
			if (stageNum >= 0 && stageNum <= 7)
			{
				// Search for a CRoomInfo instance with the same room number
				cRoomJmp_data* roomJmpData = cRoomJmp_data::get();
				if (roomJmpData)
				{
					cRoomJmp_stage* stage = roomJmpData->GetStage(stageNum);

					CRoomInfo* roomInfo = nullptr;
					int roomInfoIdx = 0;
					for (uint32_t i = 0; i < stage->nData_0; i++)
					{
						CRoomInfo* curRoomInfo = stage->GetRoom(i);
						if (curRoomInfo->roomNo_2 == paramRoomValue)
						{
							roomInfo = curRoomInfo;
							roomInfoIdx = i;
							break;
						}
					}

					// Found roominfo for the requested room, let's jump to it
					if (roomInfo)
					{
						GlobalPtr()->JumpPoint_4FAF = roomInfoIdx;
						AreaJump(roomInfo->roomNo_2, roomInfo->pos_4, roomInfo->r_10);
					}
				}
			}
			paramRoomValue = 0; // only run this once
		}
	}

	titleMain(pT);
}

// cCard::dataSelect hook: lets us set the save slot to load & then immediately try loading it
// TODO: cCard code has a bunch of timer/fade stuff which slows it down, need to see if it could be temporarily removed...
void(__fastcall* cCard__dataSelect)(cCard* thisptr, void* unused);
void __fastcall cCard__dataSelect_Hook(cCard* thisptr, void* unused)
{
	if (loadSaveSlot > 0)
	{
		thisptr->m_SaveNo_1D = loadSaveSlot - 1; // actual slots are 0 indexed, but are shown in save menu as 1 indexed
		thisptr->m_Rno0_4 = cCard::Routine0::LoadSave;
		loadSaveSlot = 0;
		return;
	}

	cCard__dataSelect(thisptr, unused);
}

void Init_CommandLine()
{
	// Parse command-line params, store values for later
	{
		int nArgs;
		LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

		WCHAR* roomNum = nullptr;
		WCHAR* loadNum = nullptr;

		for (int i = 0; i < nArgs; i++)
		{
			if (nArgs <= i + 1)
				break; // no param following this, end it...

			WCHAR* arg = szArglist[i];
			if (!wcscmp(arg, L"-room"))
			{
				roomNum = szArglist[i + 1];
			}
			else if (!wcscmp(arg, L"-load"))
			{
				loadNum = szArglist[i + 1];
			}
		}

		if (roomNum && wcslen(roomNum) > 0)
		{
			std::wstring roomNumStr = roomNum;
			if (roomNumStr[0] == L'R' || roomNumStr[0] == L'r')
				roomNumStr = roomNumStr.substr(1);

			try
			{
				paramRoomValue = std::stoul(roomNumStr, nullptr, 16);
			}
			catch(std::exception e)
			{
				paramRoomValue = 0;
			}
		}

		if (loadNum && wcslen(loadNum) > 0)
		{
			try
			{
				paramLoadSaveSlot = std::stol(loadNum, nullptr, 0);
			}
			catch (std::exception e)
			{
				paramLoadSaveSlot = 0;
			}

			if (paramLoadSaveSlot > MAX_SAVE_SLOTS || paramLoadSaveSlot <= 0)
				paramLoadSaveSlot = 0;
		}

		if (paramRoomValue || paramLoadSaveSlot > 0)
		{
			// User specified a room/save num, they probably want to skip intro logos too, let's give them a hand
			pConfig->bSkipIntroLogos = true;
		}
	}

	// Hook titleMain so we can areaJump once title screen starts running it
	auto pattern = hook::pattern("0F BE 06 8B 0C 85 ? ? ? ? 56 FF D1 6A 01 E8");
	uint32_t* titleFuncTbl = *pattern.count(1).get(0).get<uint32_t*>(6);
	uint32_t* titleMain_ptr = &titleFuncTbl[int(TITLE_WORK::Routine0::Main)];
	titleMain = (decltype(titleMain))*titleMain_ptr;
	Patch(titleMain_ptr, titleMain_Hook);

	// cCard::dataSelect hook so we can make it load directly into the save
	if (paramLoadSaveSlot > 0)
	{
		pattern = hook::pattern("0F B6 46 04 8D 14 47");
		cCard_MainLoop_tbl* cCardLoopTbl = *pattern.count(1).get(0).get<cCard_MainLoop_tbl*>(0xC);
		cCard_MainLoop_tbl* dataSelect_ptr = &cCardLoopTbl[int(cCard::Routine0::DataSelect)];
		cCard__dataSelect = dataSelect_ptr->load_0;
		Patch(dataSelect_ptr, cCard__dataSelect_Hook);
	}
}
