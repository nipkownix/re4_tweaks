#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "SDK/em09.h"

void SetEmListParamExtensions(cEm* em, EM_LIST* emData)
{
	// Game sometimes uses hardcoded stack-based EM_LIST to spawn certain enemies
	// In those cases, 0xA & 0x1C usually are left untouched, resulting in random stack data being used
	// To protect against that, we'll only allow param extensions from emData that came from pG->emList_5410
	GLOBAL_WK* pG = GlobalPtr();
	if (emData < pG->Em_list_5410 || emData >= &pG->Em_list_5410[256])
		return;

	if ((emData->be_flag_0 & EM_BE_FLAG_MODEXP_SPEEDSCALE) != EM_BE_FLAG_MODEXP_SPEEDSCALE)
		return;

	if (!emData->percentageMotionSpeed_1C && !emData->percentageScale_1E)
		return;

	// let it run the cEm's EmXX_R0_init function before we apply changes
	// (since that has a good chance of overwriting scale value)
	if (em->r_no_0_FC == uint8_t(cEm::Routine0::Init))
		em->move();

	if (emData->percentageMotionSpeed_1C)
		em->Motion_1D8.Seq_speed_C0 = float(double(emData->percentageMotionSpeed_1C) / 100.0f);

	if (emData->percentageScale_1E)
	{
		auto scalePercent = float(double(emData->percentageScale_1E) / 100.0f);
		em->scale_AC.x *= scalePercent;
		em->scale_AC.y *= scalePercent;
		em->scale_AC.z *= scalePercent;

		if (IsGanado(em->id_100))
		{
			// cEm10 holds a seperate scale value that it seems to grow/shrink the actual scale towards each frame
			// make sure we update that as well
			cEm10* em10 = (cEm10*)em;
			em10->Scale_498 = em->scale_AC;
		}
	}
}

cEm* (__cdecl* EmSetEvent)(EM_LIST* emData);
cEm* __cdecl EmSetEvent_Hook(EM_LIST* emData)
{
	cEm* result = EmSetEvent(emData);
	// TODO: check if result == errEm

	SetEmListParamExtensions(result, emData);
	return result;
}

cEm* (__cdecl* EmSetFromList2)(unsigned int em_list_no, int a2);
cEm* __cdecl EmSetFromList2_Hook(unsigned int em_list_no, int a2)
{
	cEm* result = EmSetFromList2(em_list_no, a2);
	SetEmListParamExtensions(result, &GlobalPtr()->Em_list_5410[em_list_no]);

	return result;
}

void __fastcall EmSetFromList_Hook(cEm* em, uint32_t unused)
{
	SetEmListParamExtensions(em, &GlobalPtr()->Em_list_5410[em->emListIndex_3A0]);
	em->move(); // code we patched to jump here
}

void __declspec(naked) EmSetFromList_Hook_Trampoline()
{
	__asm
	{
		mov ecx, eax
		jmp EmSetFromList_Hook
	}
}

BOOL (__fastcall* cEtcTbl__RegistData)(void* thisptr, void* unused, ETS_DATA* pEts, cEm* pPtr);
BOOL __fastcall cEtcTbl__RegistData_Hook(void* thisptr, void* unused, ETS_DATA* pEts, cEm* pPtr)
{
	BOOL ret = cEtcTbl__RegistData(thisptr, unused, pEts, pPtr);

	// update cEm scale value from the ETS data
	// (why doesn't game do this already? ETS data already has a field specifically for scale...)
	if (ret && pPtr)
	{
		Vec scale_model = pEts->scale_4;
		Vec scale_collision = scale_model;

		// Unfortunately some vanilla ETS entries have bad scale values set for some reason, even though game doesn't actually use them
		// Those values usually don't make sense, if our code applies them to the model then it usually won't have a good result
		// So we'll check for flag 0x40 at offset 0x3 into the entry first before applying it
		if ((pEts->expansion_Flag_3 & 0x40) == 0x40 || re4t::cfg->bForceETSApplyScale)
		{
			// Check if "short mode" flag is set
			// If it is, the 12 bytes for scale are treated as two 6-byte "short vectors"
			// First vector controls the scale of model, second one controls collision scale
			// Both "short vector" values are treated as percentages, eg. setting to 200 will scale by 2x
			// If the flag isn't set then both model & collision are scaled by the normal 12-byte scale vector
			// (short mode requires both flag 0x40 and 0x80 to be set at offset 0x3, so 0x3 should be set to 0xC0)
			if ((pEts->expansion_Flag_3 & 0x80) == 0x80)
			{
				scale_model.x = float(double(pEts->expansion_PercentScaleModel_4.x) / 100.0f);
				scale_model.y = float(double(pEts->expansion_PercentScaleModel_4.y) / 100.0f);
				scale_model.z = float(double(pEts->expansion_PercentScaleModel_4.z) / 100.0f);
				scale_collision.x = float(double(pEts->expansion_PercentScaleCollision_A.x) / 100.0f);
				scale_collision.y = float(double(pEts->expansion_PercentScaleCollision_A.y) / 100.0f);
				scale_collision.z = float(double(pEts->expansion_PercentScaleCollision_A.z) / 100.0f);
			}

			// Only apply the scales if they aren't all 0, or the expansion flag is set (showing that the values are intended)
			// (otherwise ForceETSApplyScale could end up applying scale of 0 to things, since some ETS entry scales are 0 already)
			if ((scale_model.x != 0 || scale_model.y != 0 || scale_model.z != 0) || (pEts->expansion_Flag_3 & 0x40) == 0x40)
				pPtr->scale_AC = scale_model;

			if ((scale_collision.x != 0 || scale_collision.y != 0 || scale_collision.z != 0) || (pEts->expansion_Flag_3 & 0x40) == 0x40)
			{
				pPtr->atari_2B4.m_height_14 *= scale_collision.z;

				// Update "rs" values - radius [for] square?
				pPtr->atari_2B4.m_radius_C *= scale_collision.x;
				pPtr->atari_2B4.m_radius_n_1C *= scale_collision.x;

				// Update "ro" values
				pPtr->atari_2B4.m_radius2_10 *= scale_collision.x;
				pPtr->atari_2B4.m_radius2_n_20 *= scale_collision.x;

				// Update "ra" values
				pPtr->atari_2B4.m_radius3_2C *= scale_collision.x;

				// Some models collision seems to use an offset for some reason
				// Multiplying against our scale mostly seems to fix it though
				pPtr->atari_2B4.m_offset_0.x *= scale_collision.x;
				pPtr->atari_2B4.m_offset_0.y *= scale_collision.y;
				pPtr->atari_2B4.m_offset_0.z *= scale_collision.z;
			}

			// Remove flag in case game reads EtcModelId somewhere else...
			pEts->expansion_Flag_3 = 0;
		}
	}
	return ret;
}

struct OSLinkData_mb // TODO: move struct to SDK header
{
	uint32_t rel_filetbl_no_0;
	void(__cdecl* prolog_4)();
	void(__cdecl* epilog_8)();
	void(__cdecl* unresolved_C)();
};
#define OSLINKDATA_ORIG_COUNT 75
OSLinkData_mb g_OSLinkData_Em_Extended[OSLINKDATA_ORIG_COUNT + 1];

BOOL(__fastcall* cEmMgr__construct)(cEmMgr* thisptr, void* unused, cEm* pEm, uint32_t id);
BOOL __fastcall cEmMgr__construct_Hook(cEmMgr* thisptr, void* unused, cEm* pEm, uint32_t id)
{
	BOOL ret = cEmMgr__construct(thisptr, unused, pEm, id);

	// Make Em09 pretend to be Em39 instead, see "Em09 fixing" comments below for why
	if (pEm->id_100 == 9)
		pEm->id_100 = 0x39;

	return ret;
}

void em09_state_fix(cEm09* pEm, bool isWait)
{
	const float maxMeleeDistance = 2250000.0;
	const float startAngerDistance = 25000000.0;
	const float endAngerDistance = 200000000.0;

	if (pEm->l_pl_378 >= maxMeleeDistance) // if player dist is greater than or equals maxMeleeDistance (isn't in melee range)
	{
		// Changed nonsensical "<= 2250000.0" check to "<= endAngerDistance" and combined it with the check inside it
		// Will set R1_Anger if they're far enough (>=startAngerDistance), but not too far (<=endAngerDistance), otherwise will use R1_Walk
		// (pEm->flags_408 & 1) seems to either be a flag that says AI pathing to the player is possible, or that player is in sight of the Em, unsure which atm
		if (pEm->l_pl_378 <= endAngerDistance && pEm->l_pl_378 > startAngerDistance && (pEm->Be_flg_408 & 1) != 0)
			*(DWORD*)&pEm->r_no_0_FC = 0x401;     // set R1_Anger
		else if (isWait) // For some reason setting this after R1_Walk stops it from working, dunno what's going on there
			*(DWORD*)&pEm->r_no_0_FC = 0x101;     // set R1_Walk
	}
	else if (pEm->Pl_rot_mb_5E8 >= 0.7853981852531433)  // If player angle is 90 degrees or greater...
	{
		*(DWORD*)&pEm->r_no_0_FC = 0x901;       // set R1_Turn90_Harai (side attack?)
	}
	else
	{
		*(DWORD*)&pEm->r_no_0_FC = 0x801;       // set R1_Harai (attack)
	}
}

void(__cdecl* em09_R1_Wait)(cEm09* pEm);
void __cdecl em09_R1_Wait_Hook(cEm09* pEm)
{
	em09_R1_Wait(pEm);
	em09_state_fix(pEm, true);
}

void(__cdecl* em09_R1_Walk)(cEm09* pEm);
void __cdecl em09_R1_Walk_Hook(cEm09* pEm)
{
	em09_R1_Walk(pEm);
	em09_state_fix(pEm, false);
}

void re4t::init::ModExpansion()
{
	// Hook ESL-loading functions so we can add extended scale/speed parameters
	auto pattern = hook::pattern("52 66 89 45 E4 66 89 4D F6 E8");
	auto EmSetEvent_thunk = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(9)).as_int();
	ReadCall(EmSetEvent_thunk, EmSetEvent);
	InjectHook(EmSetEvent_thunk, EmSetEvent_Hook, PATCH_JUMP);

	pattern = hook::pattern("6A 01 6A 32 E8 ? ? ? ? 83 C4 20");
	auto EmSetFromList2_thunk = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(4)).as_int();
	ReadCall(EmSetFromList2_thunk, EmSetFromList2);
	InjectHook(EmSetFromList2_thunk, EmSetFromList2_Hook, PATCH_JUMP);

	pattern = hook::pattern("D9 98 7C 03 00 00 8B 42 10");
	auto EmSetFromList_mov = pattern.count(1).get(0).get<uint8_t>(6);
	InjectHook(EmSetFromList_mov, EmSetFromList_Hook_Trampoline, PATCH_CALL);

	// Hook ETS set function so we can update cEm scale value from the ETS data
	// (why doesn't game do this already? ETS data already has a field specifically for scale...)
	pattern = hook::pattern("0F B7 46 02 8D 0C 40 56 8D 0C 8D ? ? ? ? E8 ? ? ? ?");
	auto cEtcTbl__RegistData_thunk = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xF)).as_int();
	ReadCall(cEtcTbl__RegistData_thunk, cEtcTbl__RegistData);
	InjectHook(cEtcTbl__RegistData_thunk, cEtcTbl__RegistData_Hook, PATCH_JUMP);

	// Patch ETS-loading func to read EtcModelNo_2 as byte
	// which gives us a spare byte to use as a flag to change how scale values are read
	// (game only allows EtcModelNo_2 values 0x40 and below anyway, so the byte was pretty much wasted...)
	pattern = hook::pattern("51 0F B7 46 02");
	Patch(pattern.count(1).get(0).get<uint8_t>(2), uint8_t(0xB6)); // change "movzx eax, word ptr" -> movzx eax, byte ptr"

	// Em09 enabling: code & data exists for the Em09 / Tyrant enemy from REmake, code seems to be setup to call into the proper RE4-engine functions fine(!)
	// Unfortunately what doesn't exist is any link for the actual Em id 09 to call into the Em09 code that's available for it
	// Main things missing for it to work:
	// - EmFileTbl defines the link between the ID & the Rel/Dat file number to use (number from there then gets looked up in the FileTbl array)
	//   Em06 - Em0c are setup with the Rel/Dat IDs for Em10, for whatever reason, maybe as placeholders
	//   Would need to change the EmFileTbl[9].Rel/.Dat to point to Em09 entries inside FileTbl, but unfortunately...
	//
	// - FileTbl doesn't contain any entry for the Em09 Rel/Dat
	//   Not a huge deal though, since luckily it does include unused Em06 Rel/Dat entries, which can be easily overwritten
	//
	// - Func that associates DAS files with imagepacks needs updating, would normally set em06.das to use imagepack 6, can just set it to imagepack 9
	//
	// - g_OSLinkData_Em array links Rel IDs up to the actual prolog/epilog/"unresolved" functions
	//   (those are normally loaded/ran from the Rel itself, but UHD merged all the Rel files into the main EXE, so this is how those merged-in functions get mapped)
	//   Sadly there's no Em09 mapping defined there though, and even worse all the mappings that are defined seem to all be for things that are actually used
	//   (with no spare room at the end of it neither, there is one blanked out entry, but seems to be used by the code to signal the end of the array, so can't be overwritten)
	//   Would need to make a copy of the existing array into our own array that has extra space for the new entry
	//
	// ... and hopefully that's it! Then can just edit ESL to use Em id 09 and hopefully it should spawn in fine

	// First let's find the FileTbl:
	struct FileTblEntry
	{
		const char* path;
		int dvdEntryNum;
	};
	pattern = hook::pattern("0F B7 D0 8B 04 D5 ? ? ? ?");
	FileTblEntry* FileTbl = (FileTblEntry*)(*pattern.count(1).get(0).get<uint8_t*>(6) - 4);
	// Patch path inside entries for em06.das/em06.rel to use em09.das/em09.rel instead
	auto& entryDas = FileTbl[0x1C];
	auto& entryRel = FileTbl[0x1D];
	Patch(entryDas.path + 6, '9');
	Patch(entryRel.path + 7, '9');

	// Locate EmFileTbl and patch the Em09 entry to point to the FileTbl entries above
	// TODO: seems each playable char has it's own EmFileTbl for whatever reason, would need to patch those too...
	struct EMD_READ_SET
	{
		uint16_t Dat;
		uint16_t Rel;
		uint16_t LowDat;
		uint16_t Dll;
	};
	pattern = hook::pattern("8D 04 CD ? ? ? ? 89 85 ? ? ? ? 8B 8D ? ? ? ?");
	EMD_READ_SET* EmFileTbl = *pattern.count(1).get(0).get<EMD_READ_SET*>(3);
	EmFileTbl[9].Dat = 0x1C;
	EmFileTbl[9].Rel = 0x1D;

	// Patch func that sets up FileTbl <-> ImagePack mappings, change em06.das = 6 to em09.das = 9
	pattern = hook::pattern("01 C7 05 ? ? ? ? 06 00 00 00 C7 05");
	Patch(pattern.count(1).get(0).get<uint32_t>(7), uint32_t(9));

	// Patch OSLinkData pointer to point to our own array & copy data for it over
	pattern = hook::pattern("8B 45 10 8B 4D 14 C7 45 F0 ? ? ? ?");
	OSLinkData_mb** g_OSLinkData_Em_ptr = pattern.count(1).get(0).get<OSLinkData_mb*>(9);

	// Copy orig data to our extended array
	std::copy_n(*g_OSLinkData_Em_ptr, OSLINKDATA_ORIG_COUNT, g_OSLinkData_Em_Extended);

	// Locate the unused prologEm09 func, easier said than done since nothing ever calls it, and almost all prologEmXX funcs share the same instructions...
	// Fortunately prologEm09 appears to always be the first instance of a prologEmXX func, so we can just search for the prologEmXX pattern and use first result
	pattern = hook::pattern("C7 05 ? ? ? ? ? ? ? ? C3");
	void* prologEm09 = pattern.count(1).get(0).get<uint8_t>(0);

	// Set up the g_OSLinkData entry for Em09
	g_OSLinkData_Em_Extended[74].rel_filetbl_no_0 = 0x1D;
	g_OSLinkData_Em_Extended[74].prolog_4 = (void(*)())prologEm09;
	g_OSLinkData_Em_Extended[74].epilog_8 = g_OSLinkData_Em_Extended[0].epilog_8; // nullsub from [0]
	g_OSLinkData_Em_Extended[74].unresolved_C = g_OSLinkData_Em_Extended[0].epilog_8; // nullsub from [0]

	// End-of-records indicator
	g_OSLinkData_Em_Extended[75] = { 0 };

	// Patch game to use the extended array
	Patch(g_OSLinkData_Em_ptr, &g_OSLinkData_Em_Extended);

	// Em09 fixing: of course enabling & spawning it is only the first step, there's also some issues with the Em itself that need to be fixed.
	// The above lets it spawn in fine, but none of the targeting/hit detection code actually responds to it
	// Seems the detection code checks what Em ID it is and only allows certain IDs to be hit, with ID 9 left out of all those checks...
	// Rather than trying to find & update each of them, we can overwrite the instances cEm::id_100 field to an Em which is known to work
	// (fooling those checks into thinking it's a different Em, while it still runs the Em09 code)
	// 
	// The question is which Em should it masquerade as - some rooms do perform searches for certain Ems and then call functions on them
	// eg. r205 searches for Em2d and tries calling a vtable func on it, which Em09 wouldn't contain, causing a crash...
	// Em10 also seems to crash for some reason, but Em39 appears to work fine, so we'll go with that for now, until someone eventually finds a room that breaks it...

	// Unfortunately easiest way to handle this is hooking cEmMgr::construct, which is only referenced in the cEmMgr vftable, ugh
	// Need to find offset for the vftable and grab addr for construct from it
	// Kinda ugly method which grabs the vftable from the static EmMgr instance
	pattern = hook::pattern("? ? ? ? 00 00 00 00 00 00 00 00 08 0E 00 00 02");
	uint32_t* cEmMgr_vftable = *pattern.count(1).get(0).get<uint32_t*>(0);
	uint32_t cEmMgr_construct_thunk = cEmMgr_vftable[6];
	ReadCall(cEmMgr_construct_thunk, cEmMgr__construct);
	InjectHook(cEmMgr_construct_thunk, cEmMgr__construct_Hook, PATCH_JUMP);

	// Em09 also has two big problems with the AI code:
	// - R1_Walk only ever changes state if player HP is 0, or if the Em receives enough damage it'll run R0_Damage, which sets it back to R1_Wait
	//   Unfortunately R1_Wait also seems to be only func that lets it decide to attack or not
	//   Meaning once it gets stuck in R1_Walk, it'll never try attacking player by itself (only if player attacks it first)
	//
	// - R1_Wait has some nonsensical checks for enabling the R1_Anger state (eg. distance must be greater than /and/ less than N)
	//   Because of that it'll almost always only use R1_Walk, or to one of the attack states if it's close enough
	//   It's likely the nonsensical checks used different distances at some point, and probably got broken during dev
	//
	// Fortunately we can fix both of those by adding our own checks to the end of both R1_Walk & R1_Wait
	// (to add state changes to R1_Walk, or to override whatever state R1_Wait set)
	pattern = hook::pattern("E8 ? ? ? ? 53 56 E8 ? ? ? ? 0F B6 86 FD 00 00 00 8B 0C 85"); // pattern catches a few different emXX_R0_Init funcs, but em09 should hopefully be the first one in all EXEs...
	uint32_t* em09_R1_move_tbl = *pattern.count(1).get(0).get<uint32_t*>(0x16);

	em09_R1_Wait = decltype(em09_R1_Wait)(em09_R1_move_tbl[0]);
	em09_R1_Walk = decltype(em09_R1_Walk)(em09_R1_move_tbl[1]);

	Patch(&em09_R1_move_tbl[0], em09_R1_Wait_Hook);
	Patch(&em09_R1_move_tbl[1], em09_R1_Walk_Hook);
}
