#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"

void SetEmListParamExtensions(cEm* em, EM_LIST* emData)
{
	// Game sometimes uses hardcoded stack-based EM_LIST to spawn certain enemies
	// In those cases, 0xA & 0x1C usually are left untouched, resulting in random stack data being used
	// To protect against that, we'll only allow param extensions from emData that came from pG->emList_5410
	GLOBAL_WK* pG = GlobalPtr();
	if (emData < pG->Em_list_5410 || emData >= &pG->Em_list_5410[256])
		return;

	// check emset_no_A field to see if custom speed/scale params have been set
	// emset_no_A seems to mostly be unused, but the field is defined in the struct, so maybe something has it set
	// hopefully nothing has all upper bits set though
	if ((emData->emset_no_A & 0xF0) != 0xF0)
		return;

	emData->emset_no_A &= ~0xF0; // unset upper bits that we checked

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
			Vec* scale_bk_498 = (Vec*)((uint8_t*)em + 0x498);
			*scale_bk_498 = em->scale_AC;
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

		// Check if "short mode" flag is set
		// If it is, the 12 bytes for scale are treated as two 6-byte "short vectors"
		// First vector controls the scale of model, second one controls collision scale
		// Both "short vector" values are treated as percentages, eg. setting to 200 will scale by 2x
		// If the flag isn't set then both model & collision are scaled by the normal 12-byte scale vector
		if (pEts->expansion_Flag_3 == 0xF0)
		{
			scale_model.x = float(double(pEts->expansion_PercentScaleModel_4.x) / 100.0f);
			scale_model.y = float(double(pEts->expansion_PercentScaleModel_4.y) / 100.0f);
			scale_model.z = float(double(pEts->expansion_PercentScaleModel_4.z) / 100.0f);
			scale_collision.x = float(double(pEts->expansion_PercentScaleCollision_A.x) / 100.0f);
			scale_collision.y = float(double(pEts->expansion_PercentScaleCollision_A.y) / 100.0f);
			scale_collision.z = float(double(pEts->expansion_PercentScaleCollision_A.z) / 100.0f);

			// Remove flag in case game reads EtcModelId somewhere else...
			pEts->expansion_Flag_3 = 0;
		}

		pPtr->scale_AC = scale_model;

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
	return ret;
}

void Init_ModExpansion()
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
}
