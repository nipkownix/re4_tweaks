#include <iostream>
#include "stdafx.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"

struct SND_SYS_VOL // name from PS2 sound driver, might not be correct (values could be part of larger Snd_ctrl_work struct)
{
	int16_t mas_bgm;
	int16_t mas_se;
	int16_t iss_bgm;
	int16_t iss_se;
	int16_t str_bgm;
	int16_t str_se;
};
static_assert(sizeof(SND_SYS_VOL) == 0xC, "sizeof(SND_SYS_VOL)");

struct SND_STR
{
	uint32_t sound_idx_0; // idx into Snd_str_work? used to fetch IXACT3Cue ptr
	uint8_t unk_4[4];
	uint32_t field_8;
	uint32_t field_C;
	uint8_t unk_10[0x18];
	uint32_t field_28;
	uint8_t unk_2C[0x10];
	int vol_decibels_3C;
	uint16_t vol_transformed_40;
	uint8_t unk_42[0x4];
	uint16_t status_flags_46;
	uint8_t unk_48[8];
	int str_type_50; // checked by Snd_str_work_calc_ax_vol, 4 is treated as SE, otherwise BGM
	int snd_id_54;
	uint8_t unk_58[0x4];
};
static_assert(sizeof(SND_STR) == 0x5C, "sizeof(SND_STR)");

struct SND_SEQ
{
	uint32_t sound_idx_0; // idx into Snd_str_work? used to fetch IXACT3Cue ptr
	uint8_t unk_4[0x10];
	uint16_t field_14;
	uint8_t unk_16[0x36];
	uint8_t status_flags_4C; // maybe uint32, game only checks lowest byte tho...
	uint8_t unk_4D[0x7];
	uint8_t seq_type_54; // checked by Snd_seq_work_calc_ax_vol, 2 is treated as SE, otherwise BGM
	uint8_t unk_55[0xF];
	int vol_decibels_64;
	uint8_t unk_68[0x24];
};
static_assert(sizeof(SND_SEQ) == 0x8C, "sizeof(SND_SEQ)");

SND_SYS_VOL* g_Snd_sys_vol;

void(__cdecl* Snd_set_system_vol)(char flg, int16_t vol);
void __cdecl Snd_set_system_vol_Hook(char flg, int16_t vol)
{
	Snd_set_system_vol(flg, vol);
	AudioTweaks_UpdateVolume();
}

void* g_mwply = nullptr;

void(__cdecl* mwPlySetOutVol)(void* mwply, int vol);
void __cdecl mwPlySetOutVol_Hook(void* mwply, int vol)
{
	g_mwply = mwply;

	// volume of 0 would cause divide by zero (which doesn't crash, but does make volume extremely loud)
	// -960 seems to be minimum volume allowed by Criware, so we'll set to that (not completely sure if it's muted or not though)
	int new_vol = -960;
	if (pConfig->iVolumeCutscene > 0)
		new_vol = int(float(vol) / ((pConfig->iVolumeCutscene / 100.0f) * (pConfig->iVolumeMaster / 100.0f)));

	mwPlySetOutVol(mwply, new_vol);
}

SND_STR* str_work = nullptr; // 0x127B6B8 in 1.1.0
void(__cdecl* Snd_str_work_calc_ax_vol)(SND_STR* str); // 0x979E70 in 1.1.0
void(__cdecl* SYNSetMasterVolume)(uint32_t a1, uint16_t a2, float a3); // 0x97AB90 in 1.1.0, GC vers names it SYNSetMasterVolume, likely different name on PC

SND_SEQ* (__cdecl* Snd_search_seq_work_seq_no)(int seq_no); // 0x978070 in 1.1.0, guessed name
void(__cdecl* Snd_seq_work_calc_ax_vol)(SND_SEQ* a1); // 0x978150 in 1.1.0

void AudioTweaks_UpdateVolume()
{
	if (!g_Snd_sys_vol)
		return;

	// shift value left by 8, as done by Snd_set_system_vol

	const float vol_max = 127;

	g_Snd_sys_vol->str_bgm = g_Snd_sys_vol->iss_bgm = (int16_t(vol_max * (pConfig->iVolumeBGM / 100.0f) * (pConfig->iVolumeMaster / 100.0f)) << 8);
	g_Snd_sys_vol->iss_se = (int16_t(vol_max * (pConfig->iVolumeSE / 100.0f) * (pConfig->iVolumeMaster / 100.0f)) << 8);
	g_Snd_sys_vol->str_se = (int16_t(vol_max * (pConfig->iVolumeCutscene / 100.0f) * (pConfig->iVolumeMaster / 100.0f)) << 8); // str_se seems to mostly get used by cutscenes / merchant dialogue

	if (g_mwply && (
		FlagIsSet(GlobalPtr()->flags_STATUS_501C, uint32_t(Flags_STATUS::STA_MOVIE_ON)) ||
		FlagIsSet(GlobalPtr()->flags_STATUS_501C, uint32_t(Flags_STATUS::STA_MOVIE2_ON))))
	{
		mwPlySetOutVol_Hook(g_mwply, -100); // -100 comes from cSofdec::startApp, hook will adjust it for us
	}

	// Update volume of any playing str sounds
	for (int i = 0; i < 4; i++)
	{
		SND_STR* cur_str = &str_work[i];
		if (cur_str->field_C != 1)
			continue;

		// Use Snd_str_work_calc_ax_vol to update volume of the SND_STR
		Snd_str_work_calc_ax_vol(cur_str);

		// Use SYNSetMasterVolume to update XAudio volume of the sound
		SYNSetMasterVolume(cur_str->sound_idx_0, cur_str->str_type_50 /* gets truncated to uint16 for some reason? */, float(cur_str->vol_decibels_3C));
	}

	// Update volume of any playing seq sounds
	for (int i = 0; i < 8; i++)
	{
		SND_SEQ* cur_seq = Snd_search_seq_work_seq_no(i);
		if ((cur_seq->status_flags_4C & 0x10) == 0)
			continue;

		// Update volume of the seq with our updated values
		Snd_seq_work_calc_ax_vol(cur_seq);

		// Use SYNSetMasterVolume to update XAudio volume of the sound
		SYNSetMasterVolume(cur_seq->sound_idx_0, cur_seq->field_14, float(cur_seq->vol_decibels_64));
	}
}

void Init_AudioTweaks()
{
	// Hook Snd_set_system_vol so we can override volume values with our own after game updates them
	auto pattern = hook::pattern("B8 10 00 00 00 0F B6 55 ? 52 50 E8 ? ? ? ? 83 C4");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xB)).as_int(), Snd_set_system_vol);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xB)).as_int(), Snd_set_system_vol_Hook);

	// Fetch pointer to volume table
	pattern = hook::pattern("F6 C1 01 74 06 66 A3 ? ? ? ? F6 C1 02");
	g_Snd_sys_vol = *pattern.count(1).get(0).get<SND_SYS_VOL*>(7);

	// Hook mwPlySetOutVol so we can override FMV volume, and store mwply handle to allow updating volume during runtime
	pattern = hook::pattern("6A 9C 56 E8 ? ? ? ?");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(3), mwPlySetOutVol);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(3), mwPlySetOutVol_Hook, PATCH_CALL);

	// Fetch addr of SND_STR structs
	pattern = hook::pattern("6B C0 5C 05 ? ? ? ? 5D C3");
	str_work = *pattern.count(2).get(0).get<SND_STR*>(4);

	// Find Snd_str_work_calc_ax_vol so we can update volume of SND_STR correctly
	pattern = hook::pattern("8A 46 4C A8 01 74 ? A8 04 75 ? 56 E8 ? ? ? ? 83 C4 04");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(0xC), Snd_str_work_calc_ax_vol);

	// Find SYNSetMasterVolume so we can tell XAudio to update volume
	pattern = hook::pattern("D9 1C ? 52 50 E8 ? ? ? ? 8B 07 83 C4 ? 5F");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(0x5), SYNSetMasterVolume);

	// Find Snd_search_seq_work_seq_no
	pattern = hook::pattern("56 E8 ? ? ? ? 83 C4 ? 66 83 78 4C 00 74");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(0x1), Snd_search_seq_work_seq_no);

	// Find Snd_seq_work_calc_ax_vol
	pattern = hook::pattern("F6 46 56 01 74 ? 56 E8 ? ? ? ? DB 46 64");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(0x7), Snd_seq_work_calc_ax_vol);
}
