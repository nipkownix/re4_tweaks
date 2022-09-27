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
			SystemSave->eff_country_9 = uint8_t(pConfig->iViolenceLevelOverride);
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
			SystemSave->eff_country_9 = uint8_t(pConfig->iViolenceLevelOverride);
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

	if (work_no >= thisptr->NumDatTbl_0)
		return false;

	auto& entry = thisptr->PDatTbl_4[work_no];

	return DatTbl_TryLoadFile(thisptr, entry.Name_0, dataPtr);
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
	if (work_no < thisptr->NumDatTbl_0)
	{
		auto& entry = thisptr->PDatTbl_4[work_no];
		DatTbl_TryUnloadFile(thisptr, entry.Name_0);
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

		std::stringstream pathBuf;

		pathBuf << "op\\unpacked\\op";
		pathBuf << std::setfill('0') << std::setw(2) << chapter;
		pathBuf << "_";
		pathBuf << std::setfill('0') << std::setw(2) << realSetNo;
		pathBuf << ".mdt";

		std::string path = looseFilePath(pathBuf.str().c_str());
		if (path.empty())
			return; // no loose file to load...

		FileData data;
		if (!data.load(path))
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

	PlayerCharacter curPlType = GlobalPtr()->pl_type_4FC8;

	switch (curPlType)
	{
	case PlayerCharacter::Leon:
		GlobalPtr()->plCostume_4FC9.Leon = (LeonCostume)(uint8_t)pConfig->CostumeOverride.Leon;
		GlobalPtr()->subCostume_4FCB = (AshleyCostume)(uint8_t)pConfig->CostumeOverride.Ashley;
		break;
	case PlayerCharacter::Ashley: // When playing as Ashley in the castle section
		// GlobalPtr()->plCostume_4FC9 = (uint8_t)pConfig->CostumeOverride.Ashley; <- Disabled for being very unreliable. Crashed on me many times, and when it didn't Ashley had parts of her body invisible. TODO: Fix this oe day.
		break;
	case PlayerCharacter::Ada:
	{
		GlobalPtr()->plCostume_4FC9.Ada = (AdaCostume)(uint8_t)pConfig->CostumeOverride.Ada;
		break;
	}
	default:
		break; // HUNK, Krauser and Wesker only have one costume
	}

	#ifdef VERBOSE
	con.AddConcatLog("Player type: ", int(GlobalPtr()->pl_type_4FC8));
	con.AddConcatLog("Player costume: ", int(GlobalPtr()->plCostume_4FC9.Leon));
	con.AddConcatLog("Subchar costume: ", int(GlobalPtr()->subCostume_4FCB));
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
		a1->Wep_7D8->m_CamAdjY_30 = CameraControl__getCameraDirection();

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
		thisptr->Motion_1D8.Seq_speed_C0 = 1.0f;

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

bool bShouldReload = false;
BOOL (*joyKamae_orig)();
BOOL joyKamae_Hook()
{
	// Run the original function first
	BOOL ret = joyKamae_orig();

	KEY_BTN reloadKey{};

	bool reloadKeyCheck = false;

	if (pConfig->bAllowReloadWithoutAiming_kbm && isKeyboardMouse())
	{
		reloadKeyCheck = true;
		reloadKey = KEY_BTN::KEY_RELOCKON; // Default key: R
	}
	else if (pConfig->bAllowReloadWithoutAiming_controller && isController())
	{
		reloadKeyCheck = true;

		// Change controller reload key, since the default is also used for sprinting when not aiming
		switch (SystemSavePtr()->pad_type_B) {
		case keyConfigTypes::TypeI:
			reloadKey = KEY_BTN::KEY_CANCEL; // Xinput B
			break;
		case keyConfigTypes::TypeII:
		case keyConfigTypes::TypeIII:
			reloadKey = KEY_BTN::KEY_Y; // Xinput X
			break;
		}
	}

	if (reloadKeyCheck)
	{
		// Check if the reload key has been pressed
		bool hasPressedReload = ((Key_btn_on() & (uint64_t)reloadKey) == (uint64_t)reloadKey);

		auto wepPtr = PlayerPtr()->Wep_7D8;
		if (wepPtr->m_pWep_34)
		{
			if (wepPtr->m_pWep_34->reloadable() && hasPressedReload)
			{
				bShouldReload = true; // Tell cPlayer__keyReload_hook to reload
				ret = TRUE; // Tell the game to initiate the aiming routine
			}
			else
				bShouldReload = false;
		}
	}

	return ret;
}

bool* PlReloadDirect;
bool (*cPlayer__keyReload_orig)();
bool cPlayer__keyReload_hook()
{
	bool ret = false;

	if ((pConfig->bAllowReloadWithoutAiming_kbm && isKeyboardMouse()) || 
		(pConfig->bAllowReloadWithoutAiming_controller && isController()))
	{
		bool isAiming = ((Key_btn_on() & (uint64_t)KEY_BTN::KEY_KAMAE) == (uint64_t)KEY_BTN::KEY_KAMAE);
		bool reloadWithoutZoom = false;

		switch (LastUsedDevice()) {
		case InputDevices::DinputController:
		case InputDevices::XinputController:
			if (pConfig->bAllowReloadWithoutAiming_controller)
				ret = bShouldReload;

			if (pConfig->bReloadWithoutZoom_controller)
				reloadWithoutZoom = (pConfig->bReloadWithoutZoom_controller && !isAiming);
			break;
		case InputDevices::Keyboard:
		case InputDevices::Mouse:
			if (pConfig->bAllowReloadWithoutAiming_kbm)
				ret = bShouldReload;

			if (pConfig->bReloadWithoutZoom_kbm)
				reloadWithoutZoom = (pConfig->bReloadWithoutZoom_kbm && !isAiming);
			break;
		}

		*PlReloadDirect = reloadWithoutZoom;
	}
	else
	{
		ret = cPlayer__keyReload_orig();
	}

	return ret;
}

// Small fixup for GetEtcFlgPtr - R6xx / R7xx rooms call into this, but those rooms don't have StX_data_tbl[n].save_flg set
// making GetEtcFlgPtr return NULL for those, causing a crash since the caller doesn't seem to handle NULL returns
// We could just patch the save_flg, but that might affect savegames in some way, so this way is probably safest
// (RE4 PS2 actually works very similarly to this, but only seems to use this for R0XX rooms, not sure if R6XX/R7XX would even run on there...)
uint16_t dmy_flg;
uint16_t* (__cdecl* GetEtcFlgPtr)(uint32_t etc_no, uint16_t room_no);
uint16_t* GetEtcFlgPtr_Hook(uint32_t etc_no, uint16_t room_no)
{
	uint16_t* ret = GetEtcFlgPtr(etc_no, room_no);
	if (ret)
		return ret;

	int stageNum = (GlobalPtr()->curRoomId_4FAC & 0xFF00) >> 8;

	// Reimplement PS2s stage == 0 check, which provides pointer to a dummy flag to prevent crashing
	if (!stageNum)
		return &dmy_flg;

	// Add our own stage number checks that also provide dummy pointer if necessary
	// Originally this was only meant to affect stage 6/7, as those are the only levels in vanilla game that had this crash issue
	// However some modders have found ways to store maps under later stage numbers, which can run into the same issue of crashing due to missing save_flg
	// (eg. has been noticed to happen when renaming r102 to ra02 & trying to jump there - should be able to load fine now with this)
	if (stageNum >= 6)
		return &dmy_flg;

	return ret;
}

void Init_Misc()
{
	// Hooks to allow reloading without aiming
	{
		// joyKamae -> Tells the game if the player is aiming or not.
		// We originally only hooked cPlayer::keyReload, and that worked for the most part, but under some circustances,
		// some type of race condition could occur if the player pressed reload first, and then held the aim button after reloading began. This would lead to
		// the game skipping the wep**_r3_ready** funcs, which are responsible to set up a bunch of stuff related to aiming, leading to a broken state. Now we hook
		// this function and, if the player wants to reload without aiming, we first pretend the player is aiming. This makes everything get set up properly. Then,
		// we tell cPlayer__keyReload_hook to go into the reload routine. Kinda messy, but it seems to fix the issue we had before.
		auto pattern = hook::pattern("E8 ? ? ? ? 85 C0 74 ? 81 A6 ? ? ? ? ? ? ? ? C7 86 ? ? ? ? ? ? ? ? 5E");

		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), joyKamae_orig);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), joyKamae_Hook, PATCH_JUMP);

		// cPlayer::keyReload -> Makes the game enter the reload routine if a condition is met.
		pattern = hook::pattern("E8 ? ? ? ? 84 C0 74 3A 8B 86 ? ? ? ? 39 58 ? 74 ? 8B 40");

		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cPlayer__keyReload_orig);
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
		// titleAda/Separate Ways
		pattern = hook::pattern("C6 81 ? ? ? ? ? 8B 15 ? ? ? ? 6A ? C6 82 ? ? ? ? ? A1 ? ? ? ? 80 78");
		struct AdaCosStructSW
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bOverrideCostumes)
					GlobalPtr()->plCostume_4FC9.Ada = AdaCostume::Normal;
				else
				{
					GlobalPtr()->plCostume_4FC9.Ada = pConfig->CostumeOverride.Ada;

					#ifdef VERBOSE
					con.AddConcatLog("Player type: ", int(GlobalPtr()->pl_type_4FC8));
					con.AddConcatLog("Player costume: ", int(GlobalPtr()->plCostume_4FC9.Ada));
					#endif
				}
			}
		}; injector::MakeInline<AdaCosStructSW>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// titleSub/Assignment Ada
		pattern = hook::pattern("C6 ? ? ? ? ? ? E8 ? ? ? ? 6A ? 68 ? ? ? ? 6A ? 89 46 ? E8 ? ? ? ? 8B 15");
		struct AdaCosStructAA
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!pConfig->bOverrideCostumes)
					GlobalPtr()->plCostume_4FC9.Ada = AdaCostume::Spy;
				else
				{
					GlobalPtr()->plCostume_4FC9.Ada = pConfig->CostumeOverride.Ada;

					#ifdef VERBOSE
					con.AddConcatLog("Player type: ", int(GlobalPtr()->pl_type_4FC8));
					con.AddConcatLog("Player costume: ", int(GlobalPtr()->plCostume_4FC9.Ada));
					#endif
				}
			}
		}; injector::MakeInline<AdaCosStructAA>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

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
					PlayerCharacter curPlType = GlobalPtr()->pl_type_4FC8;

					switch (curPlType)
					{
					case PlayerCharacter::Leon:
						GlobalPtr()->plCostume_4FC9.Leon = pConfig->CostumeOverride.Leon;
						break;
					case PlayerCharacter::Ada:
					{
						// ID number 2 seems to be the exact same outfit as ID number 0, for some reason, so we increase the ID here to use the actual next costume
						uint8_t costumeID = (uint8_t)pConfig->CostumeOverride.Ada;
						if (costumeID == 2)
							costumeID++;

						GlobalPtr()->plCostume_4FC9.Ada = AdaCostume(costumeID);
						break;
					}
					default:
						GlobalPtr()->plCostume_4FC9.Leon = (LeonCostume)0; // HUNK, Krauser and Wesker only have one costume
					}

					#ifdef VERBOSE
					con.AddConcatLog("Player type: ", int(GlobalPtr()->pl_type_4FC8));
					con.AddConcatLog("Player costume: ", int(GlobalPtr()->plCostume_4FC9.Leon));
					#endif
				}
			}
		}; injector::MakeInline<MercsTitleSubCos>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// Fix instructions that are checking for Ashley's armor costume instead of Leon's mafia costume, which can cause a bunch of issues
		{
			// PlClothSetLeon -- Issue: Would not apply jacket physics if Ashley isn't armored
			auto pattern = hook::pattern("80 B8 ? ? ? ? 02 75 ? 6A ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 66 85 ? 75 ? A1");
			injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04

			// Chicago Typewriter -- Issue: if Ashley is armored, Leon's Chicago Typewriter reload animation would always be the special Mafia one, regardless of Leon's costume
			auto pattern_wep11_r2_reload_1 = hook::pattern("80 B8 ? ? ? ? ? 0F 85 ? ? ? ? 38 98 ? ? ? ? 0F 85 ? ? ? ? 80 B8 ? ? ? ? ? 0F 85 ? ? ? ? 8B 96");
			auto pattern_wep11_r2_reload_2to7 = hook::pattern("80 B8 ? ? ? ? 02 75 ? 38 98 ? ? ? ? 75 ? 80 B8 ? ? ? ? 0C");
			auto pattern_wep11_r2_reload_8 = hook::pattern("80 B8 ? ? ? ? ? D9 EE 0F 85 ? ? ? ? 38 98 ? ? ? ? 0F 85 ? ? ? ? 80 B8 ? ? ? ? ? 0F 85"); 

			auto pattern_ReadWepData = hook::pattern("80 B9 ? ? ? ? ? 75 ? BB ? ? ? ? EB ? BB ? ? ? ? 8D 3C DD");
			auto pattern_cObjTompson__setMotion = hook::pattern("80 B8 ? ? ? ? ? 75 ? 38 88 ? ? ? ? 75 ? 8B 96 ? ? ? ? 89 8A ? ? ? ? 8B 86 ? ? ? ? 89");
			auto pattern_cPlayer__seqSeCtrl = hook::pattern("80 B8 ? ? ? ? ? 75 ? 80 B8 ? ? ? ? ? 75 ? 83 FB ? 77 ? 0F B6 8B ? ? ? ? FF 24 8D");
			auto pattern_cObjTompson__moveReload = hook::pattern("80 B9 ? ? ? ? ? 75 ? 80 B9 ? ? ? ? ? 0F 84 ? ? ? ? 80 BE ? ? ? ? ? 0F B6 81");
	
			injector::WriteMemory(pattern_wep11_r2_reload_1.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_wep11_r2_reload_1.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04

			for (size_t i = 0; i < pattern_wep11_r2_reload_2to7.size(); ++i)
			{
				injector::WriteMemory(pattern_wep11_r2_reload_2to7.count(6).get(i).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
				injector::WriteMemory(pattern_wep11_r2_reload_2to7.count(6).get(i).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04
			}

			injector::WriteMemory(pattern_wep11_r2_reload_8.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_wep11_r2_reload_8.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04

			injector::WriteMemory(pattern_ReadWepData.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_ReadWepData.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04

			injector::WriteMemory(pattern_cObjTompson__setMotion.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_cObjTompson__setMotion.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04

			injector::WriteMemory(pattern_cPlayer__seqSeCtrl.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_cPlayer__seqSeCtrl.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04

			injector::WriteMemory(pattern_cObjTompson__moveReload.count(1).get(0).get<uint32_t>(2), (uint8_t)0xC9, true); // +00004FCB -> +00004FC9
			injector::WriteMemory(pattern_cObjTompson__moveReload.count(1).get(0).get<uint32_t>(6), (uint8_t)LeonCostume::Mafia, true); // 02 -> 04
		}

		// Workaround crashes caused by some Ada costumes not working with certain weapons. Only seems to affect the inventory screen.
		{
			auto pattern = hook::pattern("0F B7 0D ? ? ? ? 51 E8 ? ? ? ? 0F B6 ? 50 8D 4D ? 51 E8 ? ? ? ? 8B");
			static uint16_t* m_wep_id_10 = (uint16_t*)*pattern.count(1).get(0).get<uint32_t>(3);

			// SubScreenTask -> Make game think the player's hand is empty instead of having the problematic weapons.
			struct SubScreenTask_hook
			{
				void operator()(injector::reg_pack& regs)
				{
					// Code we replaced
					regs.ecx = *m_wep_id_10;

					PlayerCharacter curPlType = GlobalPtr()->pl_type_4FC8;

					if (curPlType == PlayerCharacter::Ada)
					{
						AdaCostume curCostume = GlobalPtr()->plCostume_4FC9.Ada;

						switch (curCostume) {
						case AdaCostume::Spy:
							if (regs.ecx == 0x10) // 0x10 = Bowgun
								regs.ecx = 0x800; // 0x800 = Empty hand
							break;
						case AdaCostume::RE2:
						case AdaCostume::RE2_d:
							if (regs.ecx == 0x53) // 0x53 = Chicago Typewriter
								regs.ecx = 0x800; // 0x800 = Empty hand
						}
					}
				}
			}; injector::MakeInline<SubScreenTask_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

			// playerModelInit -> Same as before.
			pattern = hook::pattern("0F B7 05 ? ? ? ? 53 56 57 50 E8 ? ? ? ? 0F B7 15 ? ? ? ? 52 0F B6 ? E8");
			struct playerModelInit_hook
			{
				void operator()(injector::reg_pack& regs)
				{
					// Code we replaced
					regs.eax = *m_wep_id_10;

					PlayerCharacter curPlType = GlobalPtr()->pl_type_4FC8;

					if (curPlType == PlayerCharacter::Ada)
					{
						AdaCostume curCostume = GlobalPtr()->plCostume_4FC9.Ada;

						switch (curCostume) {
						case AdaCostume::Spy:
							if (regs.eax == 0x10) // 0x10 = Bowgun
								regs.eax = 0x800; // 0x800 = Empty hand
							break;
						case AdaCostume::RE2:
						case AdaCostume::RE2_d:
							if (regs.eax == 0x53) // 0x53 = Chicago Typewriter
								regs.eax = 0x800; // 0x800 = Empty hand
						}
					}
				}
			}; injector::MakeInline<playerModelInit_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

			// SsPzzlMain::move -> Same as before, but for when changing weapons in the inventory.
			pattern = hook::pattern("0F B7 05 ? ? ? ? 50 E8 ? ? ? ? 0F B7 15 ? ? ? ? 52 0F B6 ? E8 ? ? ? ? 0F B6 ? 50");
			struct SsPzzlMain__move_hook
			{
				void operator()(injector::reg_pack& regs)
				{
					// Code we replaced
					regs.eax = *m_wep_id_10;
					PlayerCharacter curPlType = GlobalPtr()->pl_type_4FC8;

					if (curPlType == PlayerCharacter::Ada)
					{
						AdaCostume curCostume = GlobalPtr()->plCostume_4FC9.Ada;

						switch (curCostume) {
						case AdaCostume::Spy:
							if (regs.eax == 0x10) // 0x10 = Bowgun
								regs.eax = 0x800; // 0x800 = Empty hand
							break;
						case AdaCostume::RE2:
						case AdaCostume::RE2_d:
							if (regs.eax == 0x53) // 0x53 = Chicago Typewriter
								regs.eax = 0x800; // 0x800 = Empty hand
						}
					}
				}
			}; injector::MakeInline<SsPzzlMain__move_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
		}

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
				int AshleyCostumeID = int(GlobalPtr()->subCostume_4FCB);

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

		// Skip titleWarning fade-in
		pattern = hook::pattern("DD D8 0F BE 46 01 83 E8 00 0F");
		uint8_t* titleWarningCode = pattern.count(1).get(0).get<uint8_t>(2);
		Patch(titleWarningCode + 0x7, uint16_t(0x9090));
		Patch(titleWarningCode + 0x9, uint32_t(0x90909090));
		Patch(titleWarningCode + 0xE, uint16_t(0x9090));
		Patch(titleWarningCode + 0x11, uint16_t(0x9090));
		Patch(titleWarningCode + 0x13, uint32_t(0x90909090));

		// Skip logo fade-in
		pattern = hook::pattern("3B 05 ? ? ? ? 7E ? 3B 05 ? ? ? ? 7D ? 68 00 20 00 00 68 00 10 00 C0");
		uint8_t* titleLogoCode = pattern.count(1).get(0).get<uint8_t>(0);
		Patch(titleLogoCode + 0x6, uint16_t(0x9090));
		Patch(titleLogoCode + 0xE, uint16_t(0x9090));
		Patch(titleLogoCode + 0x24, uint16_t(0x9090));

		// Skip key checks on "PRESS ANY KEY" screen
		if (pConfig->bSkipMenuLogo)
		{
			static uint8_t* anyKeyPressedBool = nullptr;

			auto pattern = hook::pattern("E8 ? ? ? ? 80 3D ? ? ? ? 00 0F 84 ? ? ? ? 6A 00 68 00 00 00 40");
			anyKeyPressedBool = *pattern.count(1).get(0).get<uint8_t*>(7);

			static bool hasBeenSkipped = false;
			struct SkipMenuLogo
			{
				void operator()(injector::reg_pack& regs)
				{
					bool noKeyPressed = *anyKeyPressedBool == 0;

					// Only skip the initial logo screen, after that user might back out to logo screen again, which we don't want to skip
					if (!hasBeenSkipped)
					{
						noKeyPressed = false;
						hasBeenSkipped = true;
					}

					if (noKeyPressed)
						regs.ef |= (1 << regs.zero_flag);
					else
						regs.ef &= ~(1 << regs.zero_flag);
				}
			}; injector::MakeInline<SkipMenuLogo>(pattern.count(1).get(0).get<uint32_t>(5), pattern.count(1).get(0).get<uint32_t>(12));
		}

		// Remove delays from cCard::loadMain
		pattern = hook::pattern("D9 E8 5E DE E1 DC 05 ? ? ? ? D9 9B FC 04 00 00");
		uint8_t* addr1 = pattern.count(2).get(0).get<uint8_t>(0);
		uint8_t* addr2 = pattern.count(2).get(1).get<uint8_t>(0);
		// fld1 -> fldz
		Patch(addr1 + 1, uint8_t(0xEE));
		Patch(addr2 + 1, uint8_t(0xEE));
		// nop out insn that adds 15 to count
		injector::MakeNOP(addr1 + 5, 6, true);
		injector::MakeNOP(addr2 + 5, 6, true);

		// cCard: patch FadeSet time to speed up entering/exiting save menu
		pattern = hook::pattern("F6 41 04 80 75 ? 6A 00 6A 00 6A 0A C7 45");
		Patch(pattern.count(1).get(0).get<uint8_t>(0xB), uint8_t(0)); // cCard::exit
		pattern = hook::pattern("F6 40 04 08 75 ? 53 53 6A 0A C7 45");
		Patch(pattern.count(1).get(0).get<uint8_t>(0x9), uint8_t(0)); // cCard::initialize

		// cCard::firstCheck10: skip timer check, speeds up initial "Loading, please wait..." text
		pattern = hook::pattern("8D 8B FC 04 00 00 6A 00 E8 ? ? ? ? 84 C0 74");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0xF), 2, true);

		/* the following lets us speed up cCard::saveMain a lot
		but it might be confusing for users since "save successful!" message won't appear
		if we could make it so just the "successful!" message shows for a small period then this might be useful
		for now leaving it disabled here... */
#if 0
		// cCard::saveMain
		pattern = hook::pattern("D9 E8 DE E1 DC 05 ? ? ? ? D9 9E FC 04 00 00");
		uint8_t* addr3 = pattern.count(2).get(0).get<uint8_t>(0);
		uint8_t* addr4 = pattern.count(2).get(1).get<uint8_t>(0);
		Patch(addr3 + 1, uint8_t(0xEE));
		Patch(addr4 + 1, uint8_t(0xEE));
		injector::MakeNOP(addr3 + 4, 6, true);
		injector::MakeNOP(addr4 + 4, 6, true);
		// zero cCard::saveMain tick count check, related to the typewriter animation
		pattern = hook::pattern("3D B8 0B 00 00");
		Patch(pattern.count(3).get(0).get<uint8_t>(1), uint32_t(0));
		Patch(pattern.count(3).get(1).get<uint8_t>(1), uint32_t(0));
		Patch(pattern.count(3).get(2).get<uint8_t>(1), uint32_t(0));
		// nop key check to let save screen exit by itself (no "successful message!")
		pattern = hook::pattern("0F 84 ? ? ? ? 33 FF 8D A4 24 00 00 00 00 57");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0), 6, true);
#endif

#if 0
		// Patch FadeSet to automatically skip all fading animations
		// TODO: check if everything still works fine with this
		// could be worth adding as a [DEBUG] option since it might be able to speed up a lot of places
		pattern = hook::pattern("BA 01 00 00 00 85 C9 78 ? BA 03 00 00 00");
		uint8_t* addr = pattern.count(1).get(0).get<uint8_t>(0);
		Patch(addr + 1, uint32_t(0)); // remove FADE_BE_ALIVE flag
		Patch(addr + 0xA, uint32_t(0)); // remove FADE_BE_ALIVE & FADE_BE_CONTINUE flags
#endif
		// TODO: skip JP warning screen

		spd::log()->info("SkipIntroLogos enabled");
	}

	// Enable what was leftover from the dev's debug menu (called "ToolMenu")
	Init_ToolMenu();
	if (pConfig->bEnableDebugMenu)
	{
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

	// Fix GetEtcFlgPtr crashes for R6XX/R7XX rooms (see comment above GetEtcFlgPtr_Hook)
	{
		auto pattern = hook::pattern("0F B6 8E 56 06 00 00 83 C4 ? 50 51 E8");

		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xC)).as_int(), GetEtcFlgPtr);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xC)).as_int(), GetEtcFlgPtr_Hook, PATCH_JUMP);

		spd::log()->info("GetEtcFlgPtr hook applied");
	}

	// Patch reference to non-existent st7_0.rel to point to st6_0.rel instead, allows loading into R7XX rooms
	{
		auto pattern = hook::pattern("73 74 37 5F 30 2E 72 65");
		Patch(pattern.count(1).get(0).get<uint8_t>(2), uint8_t('6'));

		spd::log()->info("st7_0 -> st6_0 patch applied");
	}

	// Allow merchant item list to skip to top/bottom when going past bounds of the list
	{
		// listRangeCheck handles preventing item selector from going below 0 / above max item count, and positioning of the selector
		// We'll change the selected idx before listRangeCheck is ran on it, which allows position to be updated for us correctly
		auto pattern = hook::pattern("8B 80 10 03 00 00");
		struct listRangeCheck_AllowSkipToBottomFix_SellMenu
		{
			void operator()(injector::reg_pack& regs)
			{
				SUB_SCREEN* sscrn = (SUB_SCREEN*)regs.eax;
				SSCRN_SHOP* shop = sscrn->shop_310;

				regs.eax = (uint32_t)shop; // orig code we patched

				if (shop->_list_num_0)
				{
					if (shop->_list_no_8 < 0)
						shop->_list_no_8 = shop->_list_num_0 - 1;
					else if (shop->_list_no_8 >= shop->_list_num_0)
							shop->_list_no_8 = 0;
				}
			}
		}; injector::MakeInline<listRangeCheck_AllowSkipToBottomFix_SellMenu>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(0x6));

		// Buy menu & upgrade menu had listRangeCheck inlined into it, grr...
		// We'll replace the logic that overwrites _list_no_8 with our own
		// (since there's not really a good way to hook it, and our change makes it obsolete anyway)
		pattern = hook::pattern("8B 46 08 85 C0 79");
		struct listRangeCheck_AllowSkipToBottomFix_BuyMenu
		{
			void operator()(injector::reg_pack& regs)
			{
				SSCRN_SHOP* shop = (SSCRN_SHOP*)regs.esi;

				if (shop->_list_num_0)
				{
					if (shop->_list_no_8 < 0)
						shop->_list_no_8 = shop->_list_num_0 - 1;
					else if (shop->_list_no_8 >= shop->_list_num_0)
						shop->_list_no_8 = 0;
				}
				else
					shop->_list_no_8 = 0;

				regs.eax = shop->_list_no_8;
			}
		}; injector::MakeInline<listRangeCheck_AllowSkipToBottomFix_BuyMenu>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(0x14));

		pattern = hook::pattern("8B 47 08 85 C0 79");
		struct listRangeCheck_AllowSkipToBottomFix_LvUpMenu
		{
			void operator()(injector::reg_pack& regs)
			{
				SSCRN_SHOP* shop = (SSCRN_SHOP*)regs.edi;

				if (shop->_list_num_0)
				{
					if (shop->_list_no_8 < 0)
						shop->_list_no_8 = shop->_list_num_0 - 1;
					else if (shop->_list_no_8 >= shop->_list_num_0)
						shop->_list_no_8 = 0;
				}
				else
					shop->_list_no_8 = 0;

				regs.eax = shop->_list_no_8;
			}
		}; injector::MakeInline<listRangeCheck_AllowSkipToBottomFix_LvUpMenu>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(0x14));
	}
}