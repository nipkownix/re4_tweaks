#include <iostream>
#include "stdafx.h"
#include "Game.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "Logging/Logging.h"
#include "Patches.h"

static uint32_t* ptrGameFrameRate;

void(__stdcall* setLanguage_Orig)();
void __stdcall setLanguage_Hook()
{
	// Update violence level on game launch / new game
	setLanguage_Orig();
	if (cfg.iViolenceLevelOverride >= 0)
	{
		auto* SystemSave = SystemSavePtr();
		if (SystemSave)
			SystemSave->violenceLevel_9 = uint8_t(cfg.iViolenceLevelOverride);
	}
}

typedef void(__fastcall* cCard__firstCheck10_Fn)(void* thisptr, void* unused);
cCard__firstCheck10_Fn cCard__firstCheck10_Orig;
void __fastcall cCard__firstCheck10_Hook(void* thisptr, void* unused)
{
	// pSys gets overwritten with data from gamesave during first loading screen, so update violence level after reading it
	cCard__firstCheck10_Orig(thisptr, unused);
	if (cfg.iViolenceLevelOverride >= 0)
	{
		auto* SystemSave = SystemSavePtr();
		if (SystemSave)
			SystemSave->violenceLevel_9 = uint8_t(cfg.iViolenceLevelOverride);
	}
}

struct FileMapping
{
	std::string datPath;
	void* data;
	HANDLE hFile;
	HANDLE hMap;
};

struct DatTblEntry
{
	char name_0[48];
	uint8_t flags_30;
	uint8_t unk_31;
	uint16_t unk_32;
	uint8_t* data_34;
	void* unk_38;
};

struct DatTbl
{
	int count_0;
	DatTblEntry* entries_4;
};

std::unordered_map<DatTbl*, std::unordered_map<std::string, FileMapping>> mappedFiles; // key = DatTbl ptr, value = map of paths -> mem-mapped addr
std::unordered_map<std::string, bool> nonExistentFiles; // files that we know aren't on FS, cheaper to keep track of them instead of querying OS each time

bool TryMapFile(DatTbl* datTbl, const char* filePath, void** dataPtr)
{
	std::string path = filePath;

	if (nonExistentFiles.count(path))
		return true; // we know it's non-existent, so lets get outta here

	// Check if file exists at some common paths
	if (GetFileAttributesA(path.c_str()) == 0xFFFFFFFF)
	{
		path = "BIO4\\" + path;
		if (GetFileAttributesA(path.c_str()) == 0xFFFFFFFF)
		{
			path = "..\\" + std::string(filePath);
			if (GetFileAttributesA(path.c_str()) == 0xFFFFFFFF)
			{
				path = "..\\BIO4\\" + std::string(filePath);
				if (GetFileAttributesA(path.c_str()) == 0xFFFFFFFF)
				{
					nonExistentFiles[filePath] = true;
					return true; // file doesn't seem to exist on filesystem, return whatever DatTbl has
				}
			}
		}
	}

	// file exists locally/loosely - map it into memory if we need to, and return ptr to it

	char fullPath[4096];
	GetFullPathNameA(path.c_str(), 4096, fullPath, nullptr);
	path = fullPath;

	if (mappedFiles.count(datTbl) <= 0)
		mappedFiles[datTbl] = std::unordered_map<std::string, FileMapping>();

	if (mappedFiles[datTbl].count(path))
	{
		// File is already mapped in, return ptr to it
		*dataPtr = mappedFiles[datTbl][path].data;
		return true;
	}

	FileMapping mapping;

	mapping.hFile = ::CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!mapping.hFile)
		return true;

	mapping.hMap = ::CreateFileMapping(mapping.hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if (!mapping.hMap)
	{
		::CloseHandle(mapping.hFile);
		return true;
	}

	mapping.data = ::MapViewOfFile(mapping.hMap, FILE_MAP_READ, 0, 0, 0);
	if (!mapping.data)
	{
		::CloseHandle(mapping.hMap);
		::CloseHandle(mapping.hFile);
		return true;
	}

	mapping.datPath = filePath;

	mappedFiles[datTbl][path] = mapping;
	*dataPtr = mapping.data;
	return true;
}

void TryUnmapFile(DatTbl* datTbl, const char* filePath)
{
	if (!mappedFiles.count(datTbl))
		return;

	std::string path = filePath;

	std::string fileKey;
	auto& map = mappedFiles[datTbl];
	for (auto& kvp : map)
	{
		if (kvp.second.datPath != path)
			continue;

		fileKey = kvp.first;

		::UnmapViewOfFile(kvp.second.data);
		::CloseHandle(kvp.second.hMap);
		::CloseHandle(kvp.second.hFile);
	}

	if (!fileKey.empty())
		map.erase(fileKey);
}

bool(__fastcall* DatTbl__GetDat)(DatTbl* thisptr, void* unused, void** dataPtr, uint8_t* a3, const char* filePath, uint32_t* work_no_out);
bool __fastcall DatTbl__GetDat_Hook(DatTbl* thisptr, void* unused, void** dataPtr, uint8_t* a3, const char* filePath, uint32_t* work_no_out)
{
	// Get the dat file normally first - so a3/work_no_out/etc will be set properly
	if (!DatTbl__GetDat(thisptr, unused, dataPtr, a3, filePath, work_no_out))
		return false; // doesn't exist in DAT, possibly corrupt game, return false...

	return TryMapFile(thisptr, filePath, dataPtr);
}

bool(__fastcall* DatTbl__GetDatWkNo)(DatTbl* thisptr, void* unused, void** dataPtr, uint8_t* a3, int work_no);
bool __fastcall DatTbl__GetDatWkNo_Hook(DatTbl* thisptr, void* unused, void** dataPtr, uint8_t* a3, int work_no)
{
	// Get the dat file normally first - so a3 etc will be set properly
	auto ret = DatTbl__GetDatWkNo(thisptr, unused, dataPtr, a3, work_no);
	if (!ret)
		return false; // doesn't exist in DAT, possibly corrupt game, return false...

	if (work_no >= thisptr->count_0)
		return false;

	auto& entry = thisptr->entries_4[work_no];

	return TryMapFile(thisptr, entry.name_0, dataPtr);
}

bool(__fastcall* DatTbl__DelDat)(DatTbl* thisptr, void* unused, const char* filePath);
bool __fastcall DatTbl__DelDat_Hook(DatTbl* thisptr, void* unused, const char* filePath)
{
	TryUnmapFile(thisptr, filePath);

	// Only call DatTbl__DelDatWkNo once we're finished with entry, as that func will clear it...
	return DatTbl__DelDat(thisptr, unused, filePath);
}

bool(__fastcall* DatTbl__DelDatWkNo)(DatTbl* thisptr, void* unused, int work_no);
bool __fastcall DatTbl__DelDatWkNo_Hook(DatTbl* thisptr, void* unused, int work_no)
{
	if (work_no < thisptr->count_0)
	{
		auto& entry = thisptr->entries_4[work_no];
		TryUnmapFile(thisptr, entry.name_0);
	}

	// Only call DatTbl__DelDatWkNo once we're finished with entry, as that func will clear it...
	return DatTbl__DelDatWkNo(thisptr, unused, work_no);
}

bool(__fastcall* DatTbl__DelAll)(DatTbl* thisptr, void* unused, bool a2);
bool __fastcall DatTbl__DelAll_Hook(DatTbl* thisptr, void* unused, bool a2)
{
	auto ret = DatTbl__DelAll(thisptr, unused, a2);

	if (!mappedFiles.count(thisptr))
		return ret;

	auto& map = mappedFiles[thisptr];
	for (auto& kvp : map)
	{
		::UnmapViewOfFile(kvp.second.data);
		::CloseHandle(kvp.second.hMap);
		::CloseHandle(kvp.second.hFile);
	}
	map.clear();
	mappedFiles.erase(thisptr);

	return ret;
}

void Init_Misc()
{
	// Hook DatTbl funcs, to allow side-loading loose files instead
	{
		// DatTbl::GetDat retrieves data pointer from the DAT file - hook it so we can search & read from the local filesystem too
		auto pattern = hook::pattern("52 8D 45 ? 50 83 C1 48 E8 ? ? ? ? 84 C0 75 ? 53");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(8)).as_int(), DatTbl__GetDat);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(8)).as_int(), DatTbl__GetDat_Hook);

		// DatTbl::GetDatWkNo retrieves pointer via DAT table index instead of a file path
		pattern = hook::pattern("52 8D 45 ? 8D 71 48 50 8B CE E8 ? ? ? ? 84 C0 0F");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(10)).as_int(), DatTbl__GetDatWkNo);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(10)).as_int(), DatTbl__GetDatWkNo_Hook);

		// DatTbl::DelDat seems to release the data buffer for a file inside the DAT, hook it so we can release the memory-mapping if needed too
		pattern = hook::pattern("50 83 C1 48 E8 ? ? ? ? 84 C0 75 ? 68");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(4)).as_int(), DatTbl__DelDat);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(4)).as_int(), DatTbl__DelDat_Hook);

		// DatTbl::DelDatWkNo release memory-map by index...
		pattern = hook::pattern("8D 73 48 57 8B CE E8 ? ? ? ? 47 3B 7D");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(6)).as_int(), DatTbl__DelDatWkNo);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(6)).as_int(), DatTbl__DelDatWkNo_Hook);

		// DatTbl::DelAll is called when the game is finished with this DAT, hook it so we can release any memory-mappings we made
		pattern = hook::pattern("32 C0 5E C3 6A 01 E8 ? ? ? ? 8B 46 04 50 E8");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(6)).as_int(), DatTbl__DelAll);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(6)).as_int(), DatTbl__DelAll_Hook);
	}

	// Remove savegame SteamID checks, allows easier save transfers
	{
		auto pattern = hook::pattern("8B 88 40 1E 00 00 3B 4D ? 0F 85 ? ? ? ?");
		Nop(pattern.count(1).get(0).get<uint8_t>(0x9), 6);
		Nop(pattern.count(1).get(0).get<uint8_t>(0x18), 6);
	}

	// Mafia Leon on cutscenes
	if (cfg.bAllowMafiaLeonCutscenes)
	{
		auto pattern = hook::pattern("80 B9 ? ? ? ? 04 6A ? 6A ? 6A ? 6A ? 0F 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), (uint8_t)-1, true); // Allow the correct models to be used

		pattern = hook::pattern("8B 7D 18 75 ? 80 B8 ? ? ? ? 02 75 ? 6A ? 68");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(11), (uint8_t)-1, true); // Allow the correct animations to be used

		Logging::Log() << "AllowMafiaLeonCutscenes enabled";
	}

	// Silence armored Ashley
	if (cfg.bSilenceArmoredAshley)
	{
		auto pattern = hook::pattern("CB 4F 00 00 02 75 ? 83 FF ? 77 ? 0F B6 87 ? ? ? ? FF 24 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(5), (uint8_t)0xEB, true); // jne -> jmp

		Logging::Log() << "SilenceArmoredAshley enabled";
	}

	// Check if the game is running at a valid frame rate
	if (!cfg.bIgnoreFPSWarning)
	{
		// Hook function to read the FPS value from config.ini
		auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
		ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);
		struct ReadFPS
		{
			void operator()(injector::reg_pack& regs)
			{
				int intIniFPS = regs.ecx;
				int intNewFPS;

				#ifdef VERBOSE
				con.AddConcatLog("Config.ini frame rate = ", intIniFPS);
				#endif

				if (intIniFPS != 30 && intIniFPS != 60) {

					// Calculate new fps
					if (intIniFPS > 45)
						intNewFPS = 60;
					else
						intNewFPS = 30;

					// Show warning
					std::string Msg = "ERROR: Invalid frame rate detected\n\nYour game appears to be configured to run at " + std::to_string(intIniFPS) +
						" FPS, which isn't supported and will break multiple aspects of the game.\nThis could mean you edited the game's \"config.ini\" file" +
						" and changed the \"variableframerate\" value to something other than 30 or 60.\n\nTo prevent the problems mentioned, re4_tweaks automatically" +
						" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' .ini file.";

					MessageBoxA(NULL, Msg.c_str(), "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);

					// Use new FPS value
					*(int32_t*)(ptrGameFrameRate) = intNewFPS;
				}
				else {
					// Use .ini FPS value.
					*(int32_t*)(ptrGameFrameRate) = intIniFPS;
				}
			}
		}; injector::MakeInline<ReadFPS>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
	else
		Logging::Log() << "User decided to ignore the FPS warning";

	// Unlock JP-only classic camera angle during Ashley segment
	{
		static uint8_t* pSys = nullptr;
		struct UnlockAshleyJPCameraAngles
		{
			void operator()(injector::reg_pack& regs)
			{
				bool unlock = *(uint32_t*)(pSys + 8) == 0 // pSys->language_8
					|| cfg.bAshleyJPCameraAngles;

				// set zero-flag if we're unlocking the camera, for the jz game uses after this hook
				if (unlock)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag); // clear zero_flag if unlock is false, in case it was set by something previously
			}
		};

		auto pattern = hook::pattern("8B 0D ? ? ? ? 80 79 08 00 75 ? 8A 80 A3 4F 00 00");
		pSys = *pattern.count(1).get(0).get<uint8_t*>(2);

		injector::MakeInline<UnlockAshleyJPCameraAngles>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));

		if (cfg.bAshleyJPCameraAngles)
			Logging::Log() << "AshleyJPCameraAngles enabled";
	}

	// Allow changing games level of violence to users choice
	{
		// find cCard functbl (tbl.1654)
		auto pattern = hook::pattern("0F B6 46 04 8D 14 47 03 D0 8B 04 95 ? ? ? ?");
		uint8_t** tbl_1654 = *pattern.count(1).get(0).get<uint8_t**>(12);

		// Update cCard::firstCheck10 entry to use our hook instead
		cCard__firstCheck10_Orig = (cCard__firstCheck10_Fn)tbl_1654[5];
		injector::WriteMemory(&tbl_1654[5], &cCard__firstCheck10_Hook, true);

		// Hook setLanguage func
		pattern = hook::pattern("A3 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? A9 00 00 00 F0");
		if (!pattern.empty())
		{
			ReadCall(pattern.count(1).get(0).get<uint32_t>(15), setLanguage_Orig);
			InjectHook(pattern.count(1).get(0).get<uint32_t>(15), setLanguage_Hook);
		}
		else
		{
			// JP build is missing whole setLanguage func, hook systemStartInit instead
			pattern = hook::pattern("53 57 E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 8B 35");
			ReadCall(pattern.count(1).get(0).get<uint32_t>(7), setLanguage_Orig);
			InjectHook(pattern.count(1).get(0).get<uint32_t>(7), setLanguage_Hook);
		}

		// Remove stupid check of EXE violence level against savegame
		// (might not be in all versions?)
		// Without this, game decides not to read in savegames for some reason
		pattern = hook::pattern("3A 4A 09 74");
		if (!pattern.empty())
			injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint8_t(0xeb), true);
	}

	// Option to skip the intro logos when starting up the game
	if (cfg.bSkipIntroLogos)
	{
		auto pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);

		Logging::Log() << "SkipIntroLogos enabled";
	}

	// Enable what was leftover from the dev's debug menu (called "ToolMenu")
	if (cfg.bEnableDebugMenu)
	{
		Init_ToolMenu();
		Init_ToolMenuDebug(); // mostly hooks for debug-build tool menu, but also includes hooks to slow down selection cursor

		Logging::Log() << "EnableDebugMenu applied";
	}

	// Add Handgun silencer to merchant sell list
	if (cfg.bAllowSellingHandgunSilencer)
	{
		auto pattern = hook::pattern("DB 00 AC 0D 01 00 FF FF 00 00 00 00 00 00 00 00 00 00 0B 01");

		struct PRICE_INFO // name from PS2/VR
		{
			unsigned short item_id_0;
			unsigned short price_2; // gets multiplied by 100 or 50
			unsigned short valid_4; // set to 1 on valid items? not sure if checked
		};

		// g_item_price_tbl has just enough room at the end to include 1 more item, so we'll add silencer to it there
		PRICE_INFO* g_item_price_tbl_130 = pattern.count(1).get(0).get<PRICE_INFO>(0);
		g_item_price_tbl_130[1].item_id_0 = 0x3F;
		g_item_price_tbl_130[1].price_2 = 3000; // buy price will be 30000, sell price should end up as 15000 - fair for a easter-egg item like this?
		g_item_price_tbl_130[1].valid_4 = 1;
		g_item_price_tbl_130[2].item_id_0 = 0xFFFF; // add new record end marker
		g_item_price_tbl_130[2].price_2 = 0;
		g_item_price_tbl_130[2].valid_4 = 0;

		// Add 1 to price table count
		pattern = hook::pattern("83 00 00 00 78 00 00 00");
		uint32_t* g_item_price_tbl_num = pattern.count(1).get(0).get<uint32_t>(0);
		*g_item_price_tbl_num += 1;

		Logging::Log() << "AllowSellingHandgunSilencer enabled";
	}

	// Allow physics to apply to tactical vest outfit
	// Some reason they specifically excluded that outfit inside the physics functions, maybe there's an issue with it somewhere
	// (Raz0r trainer overwrites this patch every frame for some reason, too bad)
	{
		auto pattern = hook::pattern("80 B8 C9 4F 00 00 02 74");
		injector::MakeNOP(pattern.count(2).get(0).get<uint8_t>(7), 2);
		injector::MakeNOP(pattern.count(2).get(1).get<uint8_t>(7), 2);
	}

	// Patch Ashley suplex glitch back into the game
	// They originally fixed this by adding a check for Ashley player-type inside em10ActEvtSetFS
	// However, the caller of that function (em10_R1_Dm_Small) already has player-type checking inside it, which will demote the suplex to a Kick for certain characters
	// Because of the way they patched it, enemies that are put into Suplex-state won't have any actions at all while playing as Ashley, while other chars are able to Kick
	// So we'll fix this by first removing the patch they added inside em10ActEvtSetFS, then add checks inside em10_R1_Dm_Small instead so that Ashley can Kick
	// (or if user wants we'll leave it with patch removed, so that they can play with the Ashley suplex :)
	// TODO: some reason the below doesn't actually let Ashley use SetKick, might be player-type checks inside it, so right now this just disables Suplex if bAllowAshleySuplex isn't set
	{
		// Remove player-type check from em10ActEvtSetFS
		auto pattern = hook::pattern("0F 8E ? ? ? ? A1 ? ? ? ? 80 B8 C8 4F 00 00 01 0F 84");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0x12), 6, true);

		pattern = hook::pattern("0F B6 81 C8 4F 00 00 83 C0 FE 83 F8 03 77");
		struct SuplexCheckPlayerAshley
		{
			void operator()(injector::reg_pack& regs)
			{
				int playerType = *(uint8_t*)(regs.ecx + 0x4FC8);

				// code we patched over
				regs.eax = playerType;
				regs.eax = regs.eax - 2;

				// Add Ashley player-type check, make it use Ada/Hunk/Wesker case which calls SetKick
				// TODO: some reason SetKick doesn't work for Ashley? might be player-type checks inside it, so right now this just disables Suplex if bAllowAshleySuplex isn't set
				if (!cfg.bAllowAshleySuplex && playerType == 1)
				{
					regs.eax = 0;
				}
			}
		}; injector::MakeInline<SuplexCheckPlayerAshley>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));
	}
}