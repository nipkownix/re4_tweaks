#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"

static uint32_t* ptrGameFrameRate;

void(__stdcall* setLanguage_Orig)();
void __stdcall setLanguage_Hook()
{
	// Update violence level on game launch / new game
	setLanguage_Orig();
	if (pConfig->iViolenceLevelOverride >= 0)
	{
		auto* SystemSave = SystemSavePtr();
		if (SystemSave)
			SystemSave->violenceLevel_9 = uint8_t(pConfig->iViolenceLevelOverride);
	}
}

typedef void(__fastcall* cCard__firstCheck10_Fn)(void* thisptr, void* unused);
cCard__firstCheck10_Fn cCard__firstCheck10_Orig;
void __fastcall cCard__firstCheck10_Hook(void* thisptr, void* unused)
{
	// pSys gets overwritten with data from gamesave during first loading screen, so update violence level after reading it
	cCard__firstCheck10_Orig(thisptr, unused);
	if (pConfig->iViolenceLevelOverride >= 0)
	{
		auto* SystemSave = SystemSavePtr();
		if (SystemSave)
			SystemSave->violenceLevel_9 = uint8_t(pConfig->iViolenceLevelOverride);
	}
}

struct FileData
{
	std::string filePath;
	void* data;

	FileData& operator=(FileData&& o)
	{
		filePath = std::move(o.filePath);

		// transfer ownership of data
		data = o.data;

		o.data = nullptr;

		return *this;
	}

	~FileData()
	{
		if (data)
			free(data);
		data = nullptr;
	}

	bool load(std::string_view path)
	{
		HANDLE hFile = ::CreateFileA(path.data(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (!hFile)
			return false;

		DWORD high = 0;
		auto size = GetFileSize(hFile, &high);
		data = malloc(size);
		if (!data)
			return false;

		DWORD read = 0;
		bool success = ReadFile(hFile, data, size, &read, nullptr) 
			&& read == size;

		if (success)
			filePath = path;

		CloseHandle(hFile);
		return success;
	}
};

std::unordered_map<DatTbl*, std::unordered_map<std::string, FileData>> loadedFiles; // key = DatTbl ptr, value = local-path:data
std::unordered_map<std::string, bool> nonExistentFiles; // files that we know aren't on FS, cheaper to keep track of them instead of querying OS each time

std::string looseFilePath(const char* filePath)
{
	std::string path = filePath;
	if (nonExistentFiles.count(path))
		return ""; // we know it's non-existent already, no need to check again

	// Check if file exists at some common paths
	std::string origPath = path;
	if (GetFileAttributesA(path.c_str()) == 0xFFFFFFFF)
	{
		path = rootPath + "re4_tweaks\\sideload\\" + origPath;
		con.AddLogChar(path.c_str());
		if (GetFileAttributesA(path.c_str()) == 0xFFFFFFFF)
		{
			nonExistentFiles[origPath] = true;
			return "";
		}
	}

	char fullPath[4096];
	GetFullPathNameA(path.c_str(), 4096, fullPath, nullptr);
	return fullPath;
}

bool DatTbl_TryLoadFile(DatTbl* datTbl, const char* filePath, void** dataPtr)
{
	std::string path = looseFilePath(filePath);

	if (path.empty())
		return true; // not found in any loose paths...

	// file exists locally/loosely - load it into memory if we need to, and return ptr to it

	if (loadedFiles.count(datTbl) <= 0)
		loadedFiles[datTbl] = std::unordered_map<std::string, FileData>();

	if (!loadedFiles[datTbl].count(path))
	{
		// File isn't loaded in yet
		FileData file;

		if (!file.load(path))
			return true;

		loadedFiles[datTbl][path] = std::move(file);
	}

	*dataPtr = loadedFiles[datTbl][path].data;
	return true;
}

void DatTbl_TryUnloadFile(DatTbl* datTbl, const char* filePath)
{
	if (!loadedFiles.count(datTbl))
		return;

	std::string path = filePath;

	std::string fileKey;
	auto& map = loadedFiles[datTbl];
	for (auto& kvp : map)
	{
		if (kvp.second.filePath != path)
			continue;

		fileKey = kvp.first;
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

	return DatTbl_TryLoadFile(thisptr, filePath, dataPtr);
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

	return DatTbl_TryLoadFile(thisptr, entry.name_0, dataPtr);
}

bool(__fastcall* DatTbl__DelDat)(DatTbl* thisptr, void* unused, const char* filePath);
bool __fastcall DatTbl__DelDat_Hook(DatTbl* thisptr, void* unused, const char* filePath)
{
	DatTbl_TryUnloadFile(thisptr, filePath);

	// Only call DatTbl__DelDatWkNo once we're finished with entry, as that func will clear it...
	return DatTbl__DelDat(thisptr, unused, filePath);
}

bool(__fastcall* DatTbl__DelDatWkNo)(DatTbl* thisptr, void* unused, int work_no);
bool __fastcall DatTbl__DelDatWkNo_Hook(DatTbl* thisptr, void* unused, int work_no)
{
	if (work_no < thisptr->count_0)
	{
		auto& entry = thisptr->entries_4[work_no];
		DatTbl_TryUnloadFile(thisptr, entry.name_0);
	}

	// Only call DatTbl__DelDatWkNo once we're finished with entry, as that func will clear it...
	return DatTbl__DelDatWkNo(thisptr, unused, work_no);
}

bool(__fastcall* DatTbl__DelAll)(DatTbl* thisptr, void* unused, bool a2);
bool __fastcall DatTbl__DelAll_Hook(DatTbl* thisptr, void* unused, bool a2)
{
	auto ret = DatTbl__DelAll(thisptr, unused, a2);

	if (!loadedFiles.count(thisptr))
		return ret;

	auto& map = loadedFiles[thisptr];
	map.clear();
	loadedFiles.erase(thisptr);

	return ret;
}

std::unordered_map<int, FileData> SsTermMain_files;
void** SsTermMain_MdtPtr = nullptr; // set by SsTermMain::OpeMdtSetNo, seems to be the actual mdt data ptr...

void(__fastcall* SsTermMain__OpeMdtSetNo)(uint8_t* thisptr, void* unused, int mdtSetNo);
void __fastcall SsTermMain__OpeMdtSetNo_Hook(uint8_t* thisptr, void* unused, int mdtSetNo)
{
	SsTermMain__OpeMdtSetNo(thisptr, unused, mdtSetNo);

	// Check if we can override/side-load the requested MDT with a loose version
	if (mdtSetNo >= 0x18)
		return; // invalid

	if (!SsTermMain_files.count(mdtSetNo))
	{
		// gotta load it
		int chapter = (GlobalPtr()->curRoomId_4FAC >> 8) & 0xF;

		int realSetNo = mdtSetNo;

		const int MessageCount_op01 = 13;
		const int MessageCount_op02 = 6;
		const int MessageCount_op03 = 5;

		// fixups for chapters 2/3 (game code is only really setup to read op01, looks like they hacked in support for op02/op03)
		if (chapter == 2 && realSetNo >= MessageCount_op01)
			realSetNo -= MessageCount_op01;
		else if (chapter == 3 && realSetNo >= (MessageCount_op01 + MessageCount_op02))
			realSetNo -= (MessageCount_op01 + MessageCount_op02);

		std::stringstream path;

		path << "op\\unpacked\\op";
		path << std::setfill('0') << std::setw(2) << chapter;
		path << "_";
		path << std::setfill('0') << std::setw(2) << realSetNo;
		path << ".mdt";

		if (path.str().empty())
			return; // no loose file to load...

		FileData data;
		if (!data.load(path.str()))
			return;

		SsTermMain_files[mdtSetNo] = std::move(data);
	}

	*(void**)(thisptr + 0x44) = SsTermMain_files[mdtSetNo].data;
	*SsTermMain_MdtPtr = SsTermMain_files[mdtSetNo].data;
}

void(__cdecl* SndCall)(void* a1, void* a2, void* a3, void* a4, void* a5);
void SsTermMain__quit_SndCall_hook(void* a1, void* a2, void* a3, void* a4, void* a5)
{
	// called once game is finished with the MDT, now we can unload it
	// have to hook the SndCall call inside SsTermMain::quit because it was only accessible by vftable
	// and all the stack manipulating opcodes broke MakeInline...
	SndCall(a1, a2, a3, a4, a5);

	SsTermMain_files.clear();
}

BYTE(__stdcall *j_PlSetCostume_Orig)();
BYTE __stdcall j_PlSetCostume_Hook()
{
	BYTE val = j_PlSetCostume_Orig();

	if (!pConfig->bOverrideCostumes)
		return val;

	CharacterID curPlType = (CharacterID)GlobalPtr()->curPlType_4FC8;

	switch (curPlType)
	{
	case CharacterID::Leon:
		GlobalPtr()->plCostume_4FC9 = (uint8_t)pConfig->CostumeOverride.Leon;
		GlobalPtr()->Costume_subchar_4FCB = (uint8_t)pConfig->CostumeOverride.Ashley;
		break;
	case CharacterID::Ashley:
		// GlobalPtr()->plCostume_4FC9 = (uint8_t)pConfig->CostumeOverride.Ashley; <- Disabled for being very unreliable. Crashed on me many times, and when it didn't Ashley had parts of her body invisible. TODO: Fix this oe day.
		break;
	case CharacterID::Ada:
	{
		// ID number 2 seems to be the exact same outfit as ID number 0, for some reason, so we increase the ID here to use the actual next costume
		uint8_t costumeID = (uint8_t)pConfig->CostumeOverride.Ada;
		if (costumeID == 2)
			costumeID++;

		GlobalPtr()->plCostume_4FC9 = costumeID;
		break;
	}
	default:
		break; // HUNK, Krauser and Wesker only have one costume
	}

	#ifdef VERBOSE
	con.AddConcatLog("Player type: ", GlobalPtr()->curPlType_4FC8);
	con.AddConcatLog("Player costume: ", GlobalPtr()->plCostume_4FC9);
	con.AddConcatLog("Subchar costume: ", GlobalPtr()->Costume_subchar_4FCB);
	#endif

	return val;
}

typedef void(__cdecl* wepXX_routine)(cPlayer* a1);
wepXX_routine wep17_r3_ready00 = nullptr;
wepXX_routine wep17_r3_ready10 = nullptr;
wepXX_routine wep02_r3_ready10 = nullptr;

float(__cdecl* CameraControl__getCameraDirection)();
void __cdecl wep17_r3_ready00_Hook(cPlayer* a1)
{
	// Update weapon direction to match camera if allowing quickturn
	if(pConfig->bAllowMatildaQuickturn)
		a1->plWep_7D8->field_30 = CameraControl__getCameraDirection();

	wep17_r3_ready00(a1);
}

void __cdecl wep17_r3_ready10_Hook(cPlayer* a1)
{
	// Jump to wep02_r3_ready10 to allow Mathilda to quickturn character
	if (pConfig->bAllowMatildaQuickturn)
		wep02_r3_ready10(a1);
	else
		wep17_r3_ready10(a1);
}

void(__fastcall* cPlayer__weaponInit)(cPlayer* thisptr, void* unused);
void __fastcall cPlayer__weaponInit_Hook(cPlayer* thisptr, void* unused)
{
	// Fix Ditman glitch by resetting player anim speed to 1 when weapon changing (weaponInit is called during change)
	// Ditman glitch seems to be caused by changing weapon while in-between different weapon states
	// The Striker has unique code inside wep07_r3_ready00 state which increases this speed var to 1.4
	// (maybe as a workaround to increase the anim speed without redoing anims, no other weps have similar code)
	// Normally this would then be nearly-instantly reverted back to 1.0 by the wep07_r3_ready10 state, but changing weapons can interrupt that
	// We fix that here by resetting speed to 1 whenever weapon change is occurring, pretty simple fix

	if (pConfig->bFixDitmanGlitch)
		thisptr->MotInfo_1D8.speed_C0 = 1.0f;

	cPlayer__weaponInit(thisptr, unused);
}

void Install_LangLogHook()
{
	static char* game_lang = nullptr;

	auto pattern = hook::pattern("8A ? 08 8B ? ? ? ? ? 88 ? ? ? ? ? C3 8B FF");
	struct GameLangLog
	{
		void operator()(injector::reg_pack& regs)
		{
			if (GameVersion() == "1.1.0")
				*(uint8_t*)(regs.ecx + 0x4FA3) = (uint8_t)regs.eax;
			else
				*(uint8_t*)(regs.edx + 0x4FA3) = (uint8_t)regs.ecx;

			switch (GameVersion() == "1.1.0" ? (uint8_t)regs.eax : (uint8_t)regs.ecx)
			{
			case 2:
				game_lang = "English";
				break;
			case 3:
				game_lang = "German";
				break;
			case 4:
				game_lang = "French";
				break;
			case 5:
				game_lang = "Spanish";
				break;
			case 6:
				game_lang = GameVersion() == "1.1.0" ? "Traditional Chinese" : "Italian";
				break;
			case 7:
				game_lang = "Simplified Chinese";
				break;
			case 8:
				game_lang = "Italian";
				break;
			default:
				game_lang = "Unknown"; // We should never reach this.
			}

			#ifdef VERBOSE
			con.AddConcatLog("Game language: ", game_lang);
			#endif

			spd::log()->info("Game language: {}", game_lang);
		}
	};

	// Japanese exe doesn't have the setLanguage function, so we end up hooking nothing
	if (pattern.size() != 1)
	{
		#ifdef VERBOSE
		con.AddLogChar("Game language: Japanese");
		#endif

		spd::log()->info("Game language: Japanese");
	}
	else
		injector::MakeInline<GameLangLog>(pattern.count(1).get(0).get<uint32_t>(9), pattern.count(1).get(0).get<uint32_t>(15));
}

bool *PlReloadDirect;
bool cPlayer__keyReload_hook()
{
	bool isAiming = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_KAMAE) == (uint64_t)KEY_BTN::KEY_KAMAE);
	KEY_BTN key = KEY_BTN::KEY_B; // Default key: (Xinput A)
	bool bReloadWithoutZoom = false;

	switch (LastUsedDevice()) {
		case InputDevices::DinputController:
		case InputDevices::XinputController:
			if (pConfig->bAllowReloadWithoutAiming_controller)
			{
				// Change controller reload key, since the default is also used for sprinting when not aiming
				switch (SystemSavePtr()->keyConfigType_B) {
					case keyConfigTypes::TypeI:
						key = KEY_BTN::KEY_CANCEL; // Xinput B
						break;
					case keyConfigTypes::TypeII:
					case keyConfigTypes::TypeIII:
						key = KEY_BTN::KEY_Y; // Xinput X
						break;
				}

				bReloadWithoutZoom = (pConfig->bReloadWithoutZoom_controller && !isAiming);

				isAiming = true; // Pass aim check
			}
			break;
		case InputDevices::Keyboard:
		case InputDevices::Mouse:
			if (pConfig->bAllowReloadWithoutAiming_kbm)
			{
				bReloadWithoutZoom = (pConfig->bReloadWithoutZoom_kbm && !isAiming);
				isAiming = true; // Pass aim check
			}
			break;
	}

	*PlReloadDirect = bReloadWithoutZoom;

	bool hasPressedReload = ((Key_btn_trg() & (uint64_t)key) == (uint64_t)key);

	return (isAiming && hasPressedReload);
}

void Init_Misc()
{
	// Check if the exe is LAA
	if (!laa.GameIsLargeAddressAware())
	{
		spd::log()->info("Non-LAA executable detected!");
		laa.LAA_State = LAADialogState::Showing; // Show LAA patch prompt
	}

	// Hook cPlayer::keyReload to allow reloading without aiming
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 84 C0 74 3A 8B 86 ? ? ? ? 39 58 ? 74 ? 8B 40");

		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cPlayer__keyReload_hook, PATCH_JUMP);

		// PlReloadDirect is used to skip the camera change when you press the aim button (checked in cPlayer::isKamae)
		pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? 3C ? 74 ? 3C ? 74 ? B8 ? ? ? ? C3");
		PlReloadDirect = (bool*)*pattern.count(1).get(0).get<uint32_t*>(2);
	}

	// Hook cPlayer::weaponInit so we can add code to fix ditman glitch
	{
		auto pattern = hook::pattern("83 C4 0C E8 ? ? ? ? D9 EE 8B 06 D9 9E 44 05 00 00");
		
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(3)).as_int(), cPlayer__weaponInit);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(3)).as_int(), cPlayer__weaponInit_Hook, PATCH_JUMP);
	}
	// Hooks to allow quickturning character when wielding Matilda
	// (credits to qingsheng8848 for finding out this method!)
	{
		// Get pointer to wep02_r2_ready func table
		auto pattern = hook::pattern("89 45 FC 53 56 8B 75 08 0F B6 86 FF 00 00 00 8B 0C 85");
		uint32_t* wep02_r2_ready_funcTbl = *pattern.count(1).get(0).get<uint32_t*>(0x12);

		wep02_r3_ready10 = (wepXX_routine)wep02_r2_ready_funcTbl[1];

		// Get pointer to wep17_r2_ready (mathilda) func table
		pattern = hook::pattern("C6 40 24 01 0F B6 86 FF 00 00 00 8B 0C 85 ? ? ? ? 56 FF D1 83 C4 04 83 3D");
		uint32_t* wep17_r2_ready_funcTbl = *pattern.count(1).get(0).get<uint32_t*>(0xE);

		wep17_r3_ready00 = (wepXX_routine)wep17_r2_ready_funcTbl[0];
		wep17_r3_ready10 = (wepXX_routine)wep17_r2_ready_funcTbl[1];

		// Hook wep17_r3_ready00 & ready10
		wep17_r2_ready_funcTbl[0] = (uint32_t)&wep17_r3_ready00_Hook;
		wep17_r2_ready_funcTbl[1] = (uint32_t)&wep17_r3_ready10_Hook;

		// Fetch CameraControl::getCameraDirection addr
		pattern = hook::pattern("E8 ? ? ? ? 8B 96 D8 07 00 00 D9 5A 30");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), CameraControl__getCameraDirection);
	}
	// Hook SsTermMain MDT reading functions so we can load loose file instead
	{
		auto pattern = hook::pattern("8B 41 44 A3");
		SsTermMain_MdtPtr = *pattern.count(2).get(0).get<void**>(4);

		pattern = hook::pattern("50 8B CB E8 ? ? ? ? 5F 5E 5B 5D C2");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(3)).as_int(), SsTermMain__OpeMdtSetNo);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(3)).as_int(), SsTermMain__OpeMdtSetNo_Hook);

		pattern = hook::pattern("6A 00 6A 00 6A 00 6A 15 6A 00 E8");
		ReadCall(pattern.count(1).get(0).get<uint8_t>(0xA), SndCall);
		InjectHook(pattern.count(1).get(0).get<uint8_t>(0xA), SsTermMain__quit_SndCall_hook);
	}
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

	// Allow subtitles when using English language
	// (but only if we find a replacement for the placeholder English subs in the game folder)
	{
		bool hasSubs =
			GetFileAttributesA((rootPath + "/re4_tweaks/sideload/event/r100/s03/etc/r100s03.mdt").c_str()) != 0xFFFFFFFF;

		if (hasSubs)
		{
			// Patch graphics_menu to enable subtitle option
			auto pattern = hook::pattern("E8 ? ? ? ? B1 01 3A C1 0F");
			if (pattern.size() > 0) // JP exe doesn't contain code for english lang...
			{
				injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(9), uint16_t(0xE990), true);

				// patch Event::MesSet to allow English subs to be drawn
				pattern = hook::pattern("8A 40 08 3C 02 74 ? 3C 01");
				injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(5), 2, true);

				// Patch second language check inside graphics_menu, unsure what it's doing...
				pattern = hook::pattern("C7 85 ? ? ? ? FF FF FF FF E8 ? ? ? ? 33 D2 3C 01");
				uint8_t xorEax[] = { 0x31, 0xC0, 0x90, 0x90, 0x90 };
				injector::WriteMemoryRaw(pattern.count(1).get(0).get<uint8_t>(0xA), xorEax, 5, true);

				// Fix R245EventS00/R22EEventS00 accidentally calling EvtReadExec with a do-not-free-after-use (0x40) flag
				// (without this fix, those cutscenes will leak memory whenever they're played...)
				pattern = hook::pattern("6A 50 53 68 ? ? ? ? B9 ? ? ? ? E8");
				injector::WriteMemory(pattern.count(2).get(0).get<uint8_t>(1), uint8_t(0x10), true);
				injector::WriteMemory(pattern.count(2).get(1).get<uint8_t>(1), uint8_t(0x10), true);

				spd::log()->info("{} -> English subtitles unlocked", __FUNCTION__);
			}
		}
	}

	// Remove savegame SteamID checks, allows easier save transfers
	{
		auto pattern = hook::pattern("8B 88 40 1E 00 00 3B 4D ? 0F 85 ? ? ? ?");
		Nop(pattern.count(1).get(0).get<uint8_t>(0x9), 6);
		Nop(pattern.count(1).get(0).get<uint8_t>(0x18), 6);
	}

	// Log game language once setLanguage is done
	{
		static char* game_lang = nullptr;

		auto pattern = hook::pattern("8A ? 08 8B ? ? ? ? ? 88 ? ? ? ? ? C3 8B FF");
		struct GameLangLog
		{
			void operator()(injector::reg_pack& regs)
			{
				if (GameVersion() == "1.1.0")
					*(uint8_t*)(regs.ecx + 0x4FA3) = (uint8_t)regs.eax;
				else
					*(uint8_t*)(regs.edx + 0x4FA3) = (uint8_t)regs.ecx;

				switch (GameVersion() == "1.1.0" ? (uint8_t)regs.eax : (uint8_t)regs.ecx)
				{
				case 2:
					game_lang = "English";
					break;
				case 3:
					game_lang = "German";
					break;
				case 4:
					game_lang = "French";
					break;
				case 5:
					game_lang = "Spanish";
					break;
				case 6:
					game_lang = GameVersion() == "1.1.0" ? "Traditional Chinese" : "Italian";
					break;
				case 7:
					game_lang = "Simplified Chinese";
					break;
				case 8:
					game_lang = "Italian";
					break;
				default:
					game_lang = "Unknown"; // We should never reach this.
				}

				#ifdef VERBOSE
				con.AddConcatLog("Game language: ", game_lang);
				#endif

				spd::log()->info("Game language: {}", game_lang);
			}
		};

		// Japanese exe doesn't have the setLanguage function, so we end up hooking nothing
		if (pattern.size() != 1)
		{
			#ifdef VERBOSE
			con.AddLogChar("Game language: Japanese");
			#endif

			spd::log()->info("Game language: Japanese");
		}
		else
			injector::MakeInline<GameLangLog>(pattern.count(1).get(0).get<uint32_t>(9), pattern.count(1).get(0).get<uint32_t>(15));
	}

	// Hook PlSetCostume to allow custom costume combos
	{
		auto pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 66 83 88 ? ? ? ? ? 5B");
		ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), j_PlSetCostume_Orig);
		InjectHook(injector::GetBranchDestination(pattern.get_first()).as_int(), j_PlSetCostume_Hook);

		// Separate Ways and Assignment Ada don't call PlSetCostume. Instead, the costume is set at titleAda and titleSub.
		struct AdaCosStruct
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bOverrideCostumes)
					GlobalPtr()->plCostume_4FC9 = (uint8_t)AdaCostumes::Normal;
				else
				{
					// ID number 2 seems to be the exact same outfit as ID number 0, for some reason, so we increase the ID here to use the actual next costume
					uint8_t costumeID = (uint8_t)pConfig->CostumeOverride.Ada;
					if (costumeID == 2)
						costumeID++;

					GlobalPtr()->plCostume_4FC9 = costumeID;

					#ifdef VERBOSE
					con.AddConcatLog("Player type: ", GlobalPtr()->curPlType_4FC8);
					con.AddConcatLog("Player costume: ", GlobalPtr()->plCostume_4FC9);
					#endif
				}
			}
		}; 
		
		// titleAda
		pattern = hook::pattern("C6 81 ? ? ? ? ? 8B 15 ? ? ? ? 6A ? C6 82 ? ? ? ? ? A1 ? ? ? ? 80 78");
		injector::MakeInline<AdaCosStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// titleSub
		pattern = hook::pattern("C6 ? ? ? ? ? ? E8 ? ? ? ? 6A ? 68 ? ? ? ? 6A ? 89 46 ? E8 ? ? ? ? 8B 15");
		injector::MakeInline<AdaCosStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// The Mercenaries also sets costumes using titleSub.
		pattern = hook::pattern("C6 46 ? ? 88 4E ? E8 ? ? ? ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? E8");
		struct MercsTitleSubCos
		{
			void operator()(injector::reg_pack& regs)
			{

				*(uint8_t*)(regs.esi + 0x01) = 0x0A;
				*(uint8_t*)(regs.esi + 0x6E) = (uint8_t)regs.ecx & 0xFF;

				if (pConfig->bOverrideCostumes)
				{
					CharacterID curPlType = (CharacterID)GlobalPtr()->curPlType_4FC8;

					switch (curPlType)
					{
					case CharacterID::Leon:
						GlobalPtr()->plCostume_4FC9 = (uint8_t)pConfig->CostumeOverride.Leon;
						break;
					case CharacterID::Ada:
					{
						// ID number 2 seems to be the exact same outfit as ID number 0, for some reason, so we increase the ID here to use the actual next costume
						uint8_t costumeID = (uint8_t)pConfig->CostumeOverride.Ada;
						if (costumeID == 2)
							costumeID++;

						GlobalPtr()->plCostume_4FC9 = costumeID;
						break;
					}
					default:
						GlobalPtr()->plCostume_4FC9 = (uint8_t)0; // HUNK, Krauser and Wesker only have one costume
					}

					#ifdef VERBOSE
					con.AddConcatLog("Player type: ", GlobalPtr()->curPlType_4FC8);
					con.AddConcatLog("Player costume: ", GlobalPtr()->plCostume_4FC9);
					#endif
				}
			}
		}; injector::MakeInline<MercsTitleSubCos>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// Fix instructions that are checking for Ashley's armor costume instead of Leon's mafia costume, which can a bunch of issues
		//
		// PlClothSetLeon -- Issue: Would not apply jacket physics if Ashley isn't armored
		pattern = hook::pattern("80 B8 ? ? ? ? 02 75 ? 6A ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 66 85 ? 75 ? A1");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		// Chicago Typewriter -- Issue: if Ashley is armored, Leon's Chicago Typewriter reload animation would always be the special Mafia one, regardless of Leon's costume
		auto pattern_wep11_r2_reload_1 = hook::pattern("80 B8 ? ? ? ? ? 0F 85 ? ? ? ? 38 98 ? ? ? ? 0F 85 ? ? ? ? 80 B8 ? ? ? ? ? 0F 85 ? ? ? ? 8B 96");
		auto pattern_wep11_r2_reload_2to7 = hook::pattern("80 B8 ? ? ? ? 02 75 ? 38 98 ? ? ? ? 75 ? 80 B8 ? ? ? ? 0C");
		auto pattern_wep11_r2_reload_8 = hook::pattern("80 B8 ? ? ? ? ? D9 EE 0F 85 ? ? ? ? 38 98 ? ? ? ? 0F 85 ? ? ? ? 80 B8 ? ? ? ? ? 0F 85"); 

		auto pattern_ReadWepData = hook::pattern("80 B9 ? ? ? ? ? 75 ? BB ? ? ? ? EB ? BB ? ? ? ? 8D 3C DD");
		auto pattern_cObjTompson__setMotion = hook::pattern("80 B8 ? ? ? ? ? 75 ? 38 88 ? ? ? ? 75 ? 8B 96 ? ? ? ? 89 8A ? ? ? ? 8B 86 ? ? ? ? 89");
		auto pattern_cPlayer__seqSeCtrl = hook::pattern("80 B8 ? ? ? ? ? 75 ? 80 B8 ? ? ? ? ? 75 ? 83 FB ? 77 ? 0F B6 8B ? ? ? ? FF 24 8D");
		auto pattern_cObjTompson__moveReload = hook::pattern("80 B9 ? ? ? ? ? 75 ? 80 B9 ? ? ? ? ? 0F 84 ? ? ? ? 80 BE ? ? ? ? ? 0F B6 81");
	
		injector::WriteMemory(pattern_wep11_r2_reload_1.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern_wep11_r2_reload_1.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		for (size_t i = 0; i < pattern_wep11_r2_reload_2to7.size(); ++i)
		{
			injector::WriteMemory(pattern_wep11_r2_reload_2to7.count(6).get(i).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_wep11_r2_reload_2to7.count(6).get(i).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04
		}

		injector::WriteMemory(pattern_wep11_r2_reload_8.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern_wep11_r2_reload_8.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		injector::WriteMemory(pattern_ReadWepData.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern_ReadWepData.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		injector::WriteMemory(pattern_cObjTompson__setMotion.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern_cObjTompson__setMotion.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		injector::WriteMemory(pattern_cPlayer__seqSeCtrl.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern_cPlayer__seqSeCtrl.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		injector::WriteMemory(pattern_cObjTompson__moveReload.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
		injector::WriteMemory(pattern_cObjTompson__moveReload.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostumes::Mafia, true); // 02 -> 04

		spd::log()->info("OverrideCostumes applied");
	}

	// Mafia Leon on cutscenes
	if (pConfig->bAllowMafiaLeonCutscenes)
	{
		auto pattern = hook::pattern("80 B9 ? ? ? ? 04 6A ? 6A ? 6A ? 6A ? 0F 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), (uint8_t)-1, true); // Allow the correct models to be used

		pattern = hook::pattern("8B 7D 18 75 ? 80 B8 ? ? ? ? 02 75 ? 6A ? 68");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(11), (uint8_t)-1, true); // Allow the correct animations to be used

		spd::log()->info("AllowMafiaLeonCutscenes enabled");
	}

	// Silence armored Ashley
	{
		auto pattern = hook::pattern("83 C4 ? 80 BA ? ? ? ? 02 75 ? 83 FF ? 77 ? 0F B6 87 ? ? ? ? FF 24 85 ? ? ? ? BE");
		struct ClankClanklHook
		{
			void operator()(injector::reg_pack& regs)
			{
				int AshleyCostumeID = GlobalPtr()->Costume_subchar_4FCB;

				// Mimic what CMP does, since we're overwriting it.
				if (AshleyCostumeID > 2)
				{
					// Clear both flags
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}
				else if (AshleyCostumeID < 2)
				{
					// ZF = 0, CF = 1
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef |= (1 << regs.carry_flag);
				}
				else if (AshleyCostumeID == 2)
				{
					// ZF = 1, CF = 0
					regs.ef |= (1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}

				if (pConfig->bSilenceArmoredAshley)
				{
					// Make the game think Ashley isn't using the clanky costume
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef |= (1 << regs.carry_flag);
				}
			}
		}; injector::MakeInline<ClankClanklHook>(pattern.count(1).get(0).get<uint32_t>(3), pattern.count(1).get(0).get<uint32_t>(10));

		spd::log()->info("SilenceArmoredAshley applied");
	}

	// Check if the game is running at a valid frame rate
	if (!pConfig->bIgnoreFPSWarning)
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
		spd::log()->info("User decided to ignore the FPS warning!");

	// Unlock JP-only classic camera angle during Ashley segment
	{
		static uint8_t* pSys = nullptr;
		struct UnlockAshleyJPCameraAngles
		{
			void operator()(injector::reg_pack& regs)
			{
				bool unlock = *(uint32_t*)(pSys + 8) == 0 // pSys->language_8
					|| pConfig->bAshleyJPCameraAngles;

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

		if (pConfig->bAshleyJPCameraAngles)
			spd::log()->info("AshleyJPCameraAngles enabled");
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
	if (pConfig->bSkipIntroLogos)
	{
		auto pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);

		spd::log()->info("SkipIntroLogos enabled");
	}

	// Enable what was leftover from the dev's debug menu (called "ToolMenu")
	if (pConfig->bEnableDebugMenu)
	{
		Init_ToolMenu();
		Init_ToolMenuDebug(); // mostly hooks for debug-build tool menu, but also includes hooks to slow down selection cursor

		spd::log()->info("EnableDebugMenu applied");
	}

	// Add Handgun silencer to merchant sell list
	if (pConfig->bAllowSellingHandgunSilencer)
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

		spd::log()->info("AllowSellingHandgunSilencer enabled");
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
				if (!pConfig->bAllowAshleySuplex && playerType == 1)
				{
					regs.eax = 0;
				}
			}
		}; injector::MakeInline<SuplexCheckPlayerAshley>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));
	}
}