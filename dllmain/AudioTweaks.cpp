#include <iostream>
#include "stdafx.h"
#include "Game.h"
#include "dllmain.h"
#include "Settings.h"
#include "AudioTweaks.h"

enum class Snd_vol_type_flg : char
{
	mas_bgm = 1,
	mas_se = 2,
	iss_bgm = 4,
	iss_se = 8,
	str_bgm = 0x10,
	str_se = 0x20
};

struct SND_SYS_VOL // name from PS2 sound driver, might not be correct (values could be part of larger Snd_ctrl_work struct)
{
	int16_t mas_bgm;
	int16_t mas_se;
	int16_t iss_bgm;
	int16_t iss_se;
	int16_t str_bgm;
	int16_t str_se;
};

SND_SYS_VOL* g_Snd_sys_vol;

// track previous volume game tried to set, since it seems to lower volume in certain areas (eg. when subscreen is open)
// ideally we would track all the values game tries setting, but since they usually set all values of certain type (bgm/se) to same value at once, we can cheat here
float prev_vol_bgm = 0;
float prev_vol_se = 0;

void(__cdecl* Snd_set_system_vol)(char flg, int16_t vol);
void __cdecl Snd_set_system_vol_Hook(char flg, int16_t vol)
{
	Snd_vol_type_flg type = Snd_vol_type_flg(flg);

#pragma warning(suppress:26813)
	if (type == Snd_vol_type_flg::mas_bgm || type == Snd_vol_type_flg::iss_bgm || type == Snd_vol_type_flg::str_bgm)
		prev_vol_bgm = float(vol);
	else
		prev_vol_se = float(vol);

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
	if(cfg.fVolumeCutscene > 0)
		new_vol = int(float(vol) / cfg.fVolumeCutscene);

	mwPlySetOutVol(mwply, new_vol);
}

void AudioTweaks_UpdateVolume()
{
	if (!g_Snd_sys_vol || !prev_vol_bgm || !prev_vol_se)
		return;

	// shift value left by 8, as done by Snd_set_system_vol
	g_Snd_sys_vol->str_bgm = g_Snd_sys_vol->iss_bgm = (int16_t(prev_vol_bgm * cfg.fVolumeBGM) << 8);
	g_Snd_sys_vol->iss_se = (int16_t(prev_vol_se * cfg.fVolumeSE) << 8); 
	g_Snd_sys_vol->str_se = (int16_t(prev_vol_se * cfg.fVolumeCutscene) << 8); // str_se seems to mostly get used by cutscenes / merchant dialogue

	if (g_mwply && (
		FlagIsSet(GlobalPtr()->flags_STATUS_501C, uint32_t(Flags_STATUS::STA_MOVIE_ON)) ||
		FlagIsSet(GlobalPtr()->flags_STATUS_501C, uint32_t(Flags_STATUS::STA_MOVIE2_ON))))
	{
		mwPlySetOutVol_Hook(g_mwply, -100); // -100 comes from cSofdec::startApp, hook will adjust it for us
	}
}

void Init_AudioTweaks()
{
	// Hook Snd_set_system_vol so we can retrieve value game is trying to set, and then override it
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
}
