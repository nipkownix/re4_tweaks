#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "SDK/room_jmp.h"
#include <shellapi.h>

uint32_t paramRoomValue = 0;

typedef void(__cdecl* titleMain_Fn)(TITLE_WORK* pT);
titleMain_Fn titleMain;
void __cdecl titleMain_Hook(TITLE_WORK* pT)
{
	// Wait till Rno1_1 == 1 before jumping, to ensure system save data is loaded in
	// (makes it a little slower to load in though, maybe a -fast option would be useful, or some way to speed up the fade-ins)
	if (paramRoomValue > 0 && pT->Rno1_1 == 1)
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

	titleMain(pT);
}

void Init_CommandLine()
{
	// Parse command-line params, store values for later
	{
		int nArgs;
		LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

		WCHAR* roomNum = nullptr;

		for (int i = 0; i < nArgs; i++)
		{
			WCHAR* arg = szArglist[i];
			if (!wcscmp(arg, L"-room"))
			{
				if (nArgs > i + 1)
				{
					roomNum = szArglist[i + 1];
				}
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

			if (paramRoomValue)
			{
				// User specified a room num, they probably want to skip intro logos too, let's give them a hand
				pConfig->bSkipIntroLogos = true;
			}
		}
	}

	// Hook titleMain so we can areaJump once title screen starts running it
	auto pattern = hook::pattern("0F BE 06 8B 0C 85 ? ? ? ? 56 FF D1 6A 01 E8");
	uint32_t* titleFuncTbl = *pattern.count(1).get(0).get<uint32_t*>(6);
	uint32_t* titleMain_ptr = &titleFuncTbl[int(TITLE_WORK::Routine0::Main)];
	titleMain = (titleMain_Fn)*titleMain_ptr;
	Patch(titleMain_ptr, titleMain_Hook);
}
