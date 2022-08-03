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
	GLOBALS* pG = GlobalPtr();
	if (emData < pG->emList_5410 || emData >= &pG->emList_5410[256])
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
		em->Motion_1D8.speed_C0 = float(double(emData->percentageMotionSpeed_1C) / 100.0f);

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
	SetEmListParamExtensions(result, &GlobalPtr()->emList_5410[em_list_no]);

	return result;
}

void __fastcall EmSetFromList_Hook(cEm* em, uint32_t unused)
{
	SetEmListParamExtensions(em, &GlobalPtr()->emList_5410[em->emListIndex_3A0]);
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
}
