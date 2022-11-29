#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "SDK/room_jmp.h"
#include <shellapi.h>
#include <optional>

uint32_t paramRoomValue = 0;
int paramLoadSaveSlot = 0;
#if _DEBUG
bool TweaksDevMode = true;
#else
bool TweaksDevMode = false;
#endif

std::optional<Vec> paramPosition;
std::optional<float> paramRotation;
std::optional<GameDifficulty> paramDifficulty;

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

						if (!paramPosition.has_value())
							paramPosition = roomInfo->pos_4;

						if (!paramRotation.has_value())
							paramRotation = roomInfo->r_10;

						if (paramDifficulty.has_value())
							GlobalPtr()->gameDifficulty_847C = *paramDifficulty;

						AreaJump(roomInfo->roomNo_2, *paramPosition, *paramRotation);
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

// Buffers that we can patch game code to point at
std::string g_GameDir_Root = "BIO4\\";
std::string g_GameDir_Text = "BIO4\\text\\";
std::string g_GameDir_TestRocks = "BIO4\\test\\Rocks.tga";
std::string g_GameDir_Gradients = "BIO4\\gradients\\gradient%d.png";
std::string g_GameDir_InputIni = "BIO4/input.ini";

std::string g_GameDir_uvdata_jp = "BIO4/uvdata_jp.t3d";
std::string g_GameDir_uvdata_it = "BIO4/uvdata_it.t3d";
std::string g_GameDir_uvdata_es = "BIO4/uvdata_es.t3d";
std::string g_GameDir_uvdata_fr = "BIO4/uvdata_fr.t3d";
std::string g_GameDir_uvdata_de = "BIO4/uvdata_de.t3d";
std::string g_GameDir_uvdata = "BIO4/uvdata.t3d";

std::string g_GameDir_snd = "BIO4\\snd\\";
std::string g_GameDir_snd_bio4evt_xsb = "BIO4\\snd\\bio4evt.xsb";
std::string g_GameDir_snd_bio4evt_xwb = "BIO4\\snd\\bio4evt.xwb";
std::string g_GameDir_snd_bio4bgm_xsb = "BIO4\\snd\\bio4bgm.xsb";
std::string g_GameDir_snd_bio4bgm_xwb = "BIO4\\snd\\bio4bgm.xwb";
std::string g_GameDir_snd_bio4_project = "BIO4\\snd\\bio4_project.xgs";
std::string g_GameDir_snd_bio4midi_xwb = "BIO4\\snd\\bio4midi.xwb";
std::string g_GameDir_snd_bio4midi_xsb = "BIO4\\snd\\bio4midi.xsb";

void re4t::init::CommandLine()
{
	// Parse command-line params, store values for later
	{
		int nArgs;
		LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

		WCHAR* roomNum = nullptr;
		WCHAR* loadNum = nullptr;
		WCHAR* gameDir = nullptr;
		WCHAR* posX = nullptr;
		WCHAR* posY = nullptr;
		WCHAR* posZ = nullptr;
		WCHAR* posR = nullptr;
		WCHAR* difficulty = nullptr;

		for (int i = 0; i < nArgs; i++)
		{
			WCHAR* arg = szArglist[i];
			if (!wcscmp(arg, L"-dev") || !wcscmp(arg, L"-d"))
				TweaksDevMode = true;

			else if (nArgs <= i + 1)
				break; // no param following this, end it...

			else if (!wcscmp(arg, L"-gamedir") || !wcscmp(arg, L"-g"))
				gameDir = szArglist[i + 1];
			else if (!wcscmp(arg, L"-room") || !wcscmp(arg, L"-r"))
				roomNum = szArglist[i + 1];
			else if (!wcscmp(arg, L"-load") || !wcscmp(arg, L"-l"))
				loadNum = szArglist[i + 1];
			else if (!wcscmp(arg, L"-difficulty") || !wcscmp(arg, L"-df"))
				difficulty = szArglist[i + 1];
			else if (!wcscmp(arg, L"-pos") || !wcscmp(arg, L"-position") || !wcscmp(arg, L"-p"))
			{
				if (i + 3 < nArgs) // do we have 3 params following it?
				{
					posX = szArglist[i + 1];
					posY = szArglist[i + 2];
					posZ = szArglist[i + 3];
				}
			}
			else if (!wcscmp(arg, L"-x"))
				posX = szArglist[i + 1];
			else if (!wcscmp(arg, L"-y"))
				posY = szArglist[i + 1];
			else if (!wcscmp(arg, L"-z"))
				posZ = szArglist[i + 1];
			else if (!wcscmp(arg, L"-angle") || !wcscmp(arg, L"-a"))
				posR = szArglist[i + 1];
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
			catch (std::exception e)
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

		if (difficulty && wcslen(difficulty) > 0)
		{
			GameDifficulty difficultyValue = GameDifficulty::Medium;
			bool difficultyInvalid = false;

			try
			{
				int difficultyLevel = std::stol(difficulty, nullptr, 0);
				difficultyValue = GameDifficulty(difficultyLevel);
			}
			catch (std::exception e)
			{
				// not a number? check for some common difficulties
				if (!_wcsicmp(difficulty, L"veryeasy"))
					difficultyValue = GameDifficulty::VeryEasy;
				else if (!_wcsicmp(difficulty, L"easy"))
					difficultyValue = GameDifficulty::Easy;
				else if (!_wcsicmp(difficulty, L"medium"))
					difficultyValue = GameDifficulty::Medium;
				else if (!_wcsicmp(difficulty, L"pro") || !_wcsicmp(difficulty, L"professional") || !_wcsicmp(difficulty, L"hard"))
					difficultyValue = GameDifficulty::Pro;
				else
					difficultyInvalid = true;
			}

			if (!difficultyInvalid)
				paramDifficulty = difficultyValue;
		}

		if (posX)
		{
			if (!paramPosition.has_value())
				paramPosition = { 0 };
			try
			{
				(*paramPosition).x = std::stof(posX);
			}
			catch (std::exception e)
			{
				(*paramPosition).x = 0;
			}
		}

		if (posY)
		{
			if (!paramPosition.has_value())
				paramPosition = { 0 };
			try
			{
				(*paramPosition).y = std::stof(posY);
			}
			catch (std::exception e)
			{
				(*paramPosition).y = 0;
			}
		}

		if (posZ)
		{
			if (!paramPosition.has_value())
				paramPosition = { 0 };
			try
			{
				(*paramPosition).z = std::stof(posZ);
			}
			catch (std::exception e)
			{
				(*paramPosition).z = 0;
			}
		}

		if (posR)
		{
			try
			{
				paramRotation = std::stof(posR);
			}
			catch (std::exception e)
			{
				paramRotation.reset();
			}
		}

		if (paramRoomValue || paramLoadSaveSlot > 0)
		{
			// User specified a room/save num, they probably want to skip intro logos too, let's give them a hand
			re4t::cfg->bSkipIntroLogos = true;
			re4t::cfg->bSkipMenuFades = true;
		}

		if (gameDir)
		{
			std::string gameDirStr = WstrToStr(gameDir);

			// user specified a custom game dir, need to patch a bunch of addrs to game paths to point at our own instead...
			// none of these strings are modified after this, should be safe to use ptr from c_str

			g_GameDir_Root = gameDirStr + "\\";
			g_GameDir_Text = gameDirStr + "\\text\\";
			g_GameDir_TestRocks = gameDirStr + "\\test\\Rocks.tga";
			g_GameDir_Gradients = gameDirStr + "\\gradients\\gradient%d.png";
			g_GameDir_InputIni = gameDirStr + "\\input.ini";

			g_GameDir_uvdata_jp = gameDirStr + "\\uvdata_jp.t3d";
			g_GameDir_uvdata_it = gameDirStr + "\\uvdata_it.t3d";
			g_GameDir_uvdata_es = gameDirStr + "\\uvdata_es.t3d";
			g_GameDir_uvdata_fr = gameDirStr + "\\uvdata_fr.t3d";
			g_GameDir_uvdata_de = gameDirStr + "\\uvdata_de.t3d";
			g_GameDir_uvdata = gameDirStr + "\\uvdata.t3d";

			g_GameDir_snd = gameDirStr + "\\snd\\";
			g_GameDir_snd_bio4evt_xsb = gameDirStr + "\\snd\\bio4evt.xsb";
			g_GameDir_snd_bio4evt_xwb = gameDirStr + "\\snd\\bio4evt.xwb";
			g_GameDir_snd_bio4bgm_xsb = gameDirStr + "\\snd\\bio4bgm.xsb";
			g_GameDir_snd_bio4bgm_xwb = gameDirStr + "\\snd\\bio4bgm.xwb";
			g_GameDir_snd_bio4_project = gameDirStr + "\\snd\\bio4_project.xgs";
			g_GameDir_snd_bio4midi_xwb = gameDirStr + "\\snd\\bio4midi.xwb";
			g_GameDir_snd_bio4midi_xsb = gameDirStr + "\\snd\\bio4midi.xsb";

			// This func first searches for the string we want to override, then searches for references to the addr of it, then patches them to point to destAddr
			auto PatchCodeRefsToData = [](std::string_view pattern_str, int numReferences, std::string* destAddr, uintptr_t* lastPatchedAddr = nullptr) {
				auto pattern = hook::pattern(pattern_str);
				uint32_t addr = (uint32_t)pattern.count(1).get(0).get<uint8_t>(0);
				uint8_t* addr_bytes = (uint8_t*)&addr;

				char searchStr[256];
				sprintf_s(searchStr, "68 %02X %02X %02X %02X", addr_bytes[0], addr_bytes[1], addr_bytes[2], addr_bytes[3]);

				pattern = hook::pattern(searchStr).count(numReferences);

				for (int i = 0; i < numReferences; i++)
				{
					Patch(pattern.get(i).get<uint8_t>(1), destAddr->c_str());
					if (lastPatchedAddr)
						*lastPatchedAddr = uintptr_t(pattern.get(i).get<uint8_t>(1));
				}

				return addr;
			};

			// BIO4\\0, ref at 9E6EAA
			PatchCodeRefsToData("42 49 4F 34 5C 00", 1, &g_GameDir_Root);

			// BIO4\0, ref at 95AC07 95AC27 95AC85
			{
				uint32_t bio4_addr = PatchCodeRefsToData("42 49 4F 34 00", 4, &g_GameDir_Root);

				// ADXPC funcs need to be patched seperately...

				char searchStr[256];
				uint8_t* addr_bytes = (uint8_t*)&bio4_addr;

				sprintf_s(searchStr, "C7 45 ? %02X %02X %02X %02X", addr_bytes[0], addr_bytes[1], addr_bytes[2], addr_bytes[3]);
				Patch(hook::pattern(searchStr).count(1).get(0).get<uint32_t>(3), g_GameDir_Root.c_str());

				sprintf_s(searchStr, "%02X %02X %02X %02X DB 0F C9 40", addr_bytes[0], addr_bytes[1], addr_bytes[2], addr_bytes[3]);
				Patch(hook::pattern(searchStr).count(1).get(0).get<uint32_t>(0), g_GameDir_Root.c_str());

				// TODO: two spots inside DVDConvertPathToEntrynum use strncmp/strncat with a fixed size of 4 (for BIO4 string)
				// We patch the size here to fix that, but that limits path size to 256... would be better if we can get rid of the fixed size somehow
				// Or reimplement DVDConvertPathToEntrynum entirely?
				auto pattern = hook::pattern("E9 ? ? ? ? 6A 04 68 ? ? ? ? 56 E8 ? ? ? ?");
				Patch(pattern.count(1).get(0).get<uint8_t>(6), uint8_t(g_GameDir_Root.length()));
				pattern = hook::pattern("51 E8 ? ? ? ? 6A 04 8D 95");
				Patch(pattern.count(1).get(0).get<uint8_t>(7), uint8_t(g_GameDir_Root.length()));
			}

			// BIO4\text\, ref at 426575
			{
				uintptr_t patchedAddr = 0;
				PatchCodeRefsToData("42 49 4F 34 5C 74 65 78 74 5C 00", 1, &g_GameDir_Text, &patchedAddr);
				
				// TODO: similar issue inside sub_4263F0 (@ 426573) as the DVDConvertPathToEntrynum issue above, func uses a fixed size of 9
				// We can patch the size to fix it, but that again limits us to path size ~256...
				Patch(patchedAddr - 2, uint8_t(g_GameDir_Text.length()));
			}

			// BIO4\test\Rocks.tga, ref at 93DE46 & 94C04A
			PatchCodeRefsToData("42 49 4F 34 5C 74 65 73 74 5C 52 6F 63 6B 73 2E 74 67 61 00", 2, &g_GameDir_TestRocks);

			// BIO4\gradients\gradient%d.png, ref at 94A767
			PatchCodeRefsToData("42 49 4F 34 5C 67 72 61 64 69 65 6E 74 73 5C 67 72 61 64 69 65 6E 74 25 64 2E 70 6E 67 00", 1, &g_GameDir_Gradients);

			// BIO4/input.ini, ref at 69DDCD 6A00C1 & 6A0F17
			PatchCodeRefsToData("42 49 4F 34 2F 69 6E 70 75 74 2E 69 6E 69 00", 3, &g_GameDir_InputIni);

			// uvdata_XX
			{
				// JP only references uvdata_jp & uvdata, try scanning for jp filename
				auto pattern = hook::pattern("42 49 4F 34 2F 75 76 64 61 74 61 5F 6A 70 2E 74 33 64 00");
				if (pattern.size() > 0)
				{
					PatchCodeRefsToData("42 49 4F 34 2F 75 76 64 61 74 61 5F 6A 70 2E 74 33 64 00", 1, &g_GameDir_uvdata_jp);
				}
				else
				{
					PatchCodeRefsToData("42 49 4F 34 2F 75 76 64 61 74 61 5F 69 74 2E 74 33 64 00", 1, &g_GameDir_uvdata_it);
					PatchCodeRefsToData("42 49 4F 34 2F 75 76 64 61 74 61 5F 65 73 2E 74 33 64 00", 1, &g_GameDir_uvdata_es);
					PatchCodeRefsToData("42 49 4F 34 2F 75 76 64 61 74 61 5F 66 72 2E 74 33 64 00", 1, &g_GameDir_uvdata_fr);
					PatchCodeRefsToData("42 49 4F 34 2F 75 76 64 61 74 61 5F 64 65 2E 74 33 64 00", 1, &g_GameDir_uvdata_de);
				}
				PatchCodeRefsToData("42 49 4F 34 2F 75 76 64 61 74 61 2E 74 33 64 00", 1, &g_GameDir_uvdata);
			}

			// BIO4\snd\ 
			{
				PatchCodeRefsToData("42 49 4F 34 5C 73 6E 64 5C 00", 2, &g_GameDir_snd);

				// TODO: similar issue inside sub_977080 as the DVDConvertPathToEntrynum issue above, func uses a fixed size of 9
				// We can patch the size to fix it, but that again limits us to path size ~256...
				auto pattern = hook::pattern("56 E8 ? ? ? ? 6A 09 68 ? ? ? ?");
				Patch(pattern.count(1).get(0).get<uint8_t>(7), uint8_t(g_GameDir_snd.length()));
			}
			PatchCodeRefsToData("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 5F 70 72 6F 6A 65 63 74 2E 78 67 73 00", 2, &g_GameDir_snd_bio4_project);
			PatchCodeRefsToData("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 6D 69 64 69 2E 78 77 62 00", 1, &g_GameDir_snd_bio4midi_xwb);
			PatchCodeRefsToData("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 6D 69 64 69 2E 78 73 62 00", 1, &g_GameDir_snd_bio4midi_xsb);

			// These four are referenced via some kind of pointer array instead of via code, need to search for & patch it
			// Order is bio4bgm.xwb / bio4bgm.xsb / bio4evt.xwb / bio4evt.xsb
			uint32_t addrs[4];
			addrs[0] = (uint32_t)hook::pattern("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 62 67 6D 2E 78 77 62 00").count(1).get(0).get<uint8_t>(0);
			addrs[1] = (uint32_t)hook::pattern("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 62 67 6D 2E 78 73 62 00").count(1).get(0).get<uint8_t>(0);
			addrs[2] = (uint32_t)hook::pattern("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 65 76 74 2E 78 77 62 00").count(1).get(0).get<uint8_t>(0);
			addrs[3] = (uint32_t)hook::pattern("42 49 4F 34 5C 73 6E 64 5C 62 69 6F 34 65 76 74 2E 78 73 62 00").count(1).get(0).get<uint8_t>(0);

			uint8_t* addr_bytes = (uint8_t*)addrs;
			char searchStr[256];
			sprintf_s(searchStr, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
				addr_bytes[0], addr_bytes[1], addr_bytes[2], addr_bytes[3],
				addr_bytes[4], addr_bytes[5], addr_bytes[6], addr_bytes[7],
				addr_bytes[8], addr_bytes[9], addr_bytes[10], addr_bytes[11],
				addr_bytes[12], addr_bytes[13], addr_bytes[14], addr_bytes[15]);

			auto pattern = hook::pattern(searchStr);
			uint32_t* ptrs = pattern.count(1).get(0).get<uint32_t>(0);
			Patch(&ptrs[0], g_GameDir_snd_bio4bgm_xwb.c_str());
			Patch(&ptrs[1], g_GameDir_snd_bio4bgm_xsb.c_str());
			Patch(&ptrs[2], g_GameDir_snd_bio4evt_xwb.c_str());
			Patch(&ptrs[3], g_GameDir_snd_bio4evt_xsb.c_str());
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
