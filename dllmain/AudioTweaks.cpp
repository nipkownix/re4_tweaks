#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "AudioTweaks.h"
#include <mutex>
#include <cstddef>

void(__cdecl* Snd_set_system_vol)(char flg, int16_t vol);
void __cdecl Snd_set_system_vol_Hook(char flg, int16_t vol)
{
	Snd_set_system_vol(flg, vol);
	re4t::AudioTweaks::UpdateVolume();
}

void* g_mwply = nullptr;

void(__cdecl* mwPlySetOutVol)(void* mwply, int vol);
void __cdecl mwPlySetOutVol_Hook(void* mwply, int vol)
{
	g_mwply = mwply;

	// volume of 0 would cause divide by zero (which doesn't crash, but does make volume extremely loud)
	// -960 seems to be minimum volume allowed by Criware, so we'll set to that (not completely sure if it's muted or not though)
	int new_vol = -960;
	if (re4t::cfg->iVolumeCutscene > 0)
		new_vol = int(float(vol) / ((re4t::cfg->iVolumeCutscene / 100.0f) * (re4t::cfg->iVolumeMaster / 100.0f)));

	mwPlySetOutVol(mwply, new_vol);
}

SND_STR* str_work = nullptr; // 0x127B6B8 in 1.1.0
void(__cdecl* Snd_str_work_calc_ax_vol)(SND_STR* str); // 0x979E70 in 1.1.0
void(__cdecl* SYNSetMasterVolume)(uint32_t a1, uint16_t a2, float a3); // 0x97AB90 in 1.1.0, GC vers names it SYNSetMasterVolume, likely different name on PC

SND_SEQ* (__cdecl* Snd_search_seq_work_seq_no)(int seq_no); // 0x978070 in 1.1.0, guessed name
void(__cdecl* Snd_seq_work_calc_ax_vol)(SND_SEQ* a1); // 0x978150 in 1.1.0

// Serializes access to the game's XACT cue table between the audio thread's tick, sound bank releases (room loads) and
// our own volume updates. See InstallXactCueUseAfterFreeFix.
static std::recursive_mutex s_xactCueMutex;

void re4t::AudioTweaks::UpdateVolume()
{
	if (!Snd_ctrl_work)
		return;

	// shift value left by 8, as done by Snd_set_system_vol

	const float vol_max = 127;

	Snd_ctrl_work->vol_str_bgm_48 = Snd_ctrl_work->vol_iss_bgm_44 = (int16_t(vol_max * (re4t::cfg->iVolumeBGM / 100.0f) * (re4t::cfg->iVolumeMaster / 100.0f)) << 8);
	Snd_ctrl_work->vol_iss_se_46 = (int16_t(vol_max * (re4t::cfg->iVolumeSE / 100.0f) * (re4t::cfg->iVolumeMaster / 100.0f)) << 8);
	Snd_ctrl_work->vol_str_se_4A = (int16_t(vol_max * (re4t::cfg->iVolumeCutscene / 100.0f) * (re4t::cfg->iVolumeMaster / 100.0f)) << 8); // str_se seems to mostly get used by cutscenes / merchant dialogue

	if (g_mwply && (
		FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_MOVIE_ON)) ||
		FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_MOVIE2_ON))))
	{
		mwPlySetOutVol_Hook(g_mwply, -100); // -100 comes from cSofdec::startApp, hook will adjust it for us
	}

	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);

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

uint32_t __cdecl knife_r3_fire10_SndCall_Hook(uint16_t blk, uint16_t call_no, Vec* pos, uint8_t id, uint32_t flag, cModel* pMod)
{
	if (re4t::cfg->bRestoreGCSoundEffects)
		call_no = 85; // use original GC sound effect

	return bio4::SndCall(blk, call_no, pos, id, flag, pMod);
}

// ---- XACT cue use-after-free when sound banks are released
//
// The crash: cb_audio_frame -> Snd_iss_manager -> Snd_axv_work_close_check ->
// XAudio2_XACTCUE_GetState -> IXACT3Cue::GetState on a cue that had already been freed.
//
// Cause: on room loads the game's loader thread releases and reloads the room's SE blocks (SndFileToLoad / SndSeObjInit): it destroys
// the block's wave bank and sound bank (sub_97B270 / sub_97B2C0 in 1.1.0). Destroying an XACT sound bank also destroys
// every cue still alive from it, but the game never removes those cues from its cue table (g_xact_cue_mb), and nothing
// synchronizes this with the audio thread, which keeps polling the table every few ms. If a sound was still playing, 
// the audio thread calls into the freed cue, crashing the game.
//
// Fix: right before a SE block's banks are destroyed, destroy that block's cues ourselves (exactly like
// XAudio2_XACTCUE_Destroy does, while the bank is still alive) so their table entries are empty. The audio thread then
// sees an empty entry (state -1) and closes everything normally.

// IXACT3Cue vtable (not COM, no IUnknown): 0 Play, 1 Stop, 2 GetState, 3 Destroy, ...
struct XactCue { void** vtbl; };

// Game's cue table entry (SND_XACT_CUE)
struct XactCueEntry
{
	uint32_t work_id_0;
	uint16_t blk_no_4;
	uint16_t pad_6;
	XactCue* cue_8;
	uint32_t in_use_C;
	uint16_t snd_id_10;
	uint16_t pad_12;
	uint32_t field_14;
};
static_assert(sizeof(XactCueEntry) == 0x18, "sizeof(XactCueEntry)");

static const int XACT_CUE_COUNT = 0x60;        // g_xact_cue_mb[96]
static XactCueEntry* g_xactCues = nullptr;     // g_xact_cue_mb, 0x127B890 in 1.1.0

// SE block bank table (0x12784E8 in 1.1.0, g_seBlockTable): per block { wave bank file (size, mem, IXACT3WaveBank*),
// sound bank file (size, mem, IXACT3SoundBank*) }. The cue table entries' blk_no is the index into this table
// (SndSePlay checks the block's sound bank from it with the same index it compares blk_no against).
static const int SE_BLOCK_COUNT = 14;
static const uintptr_t SE_BLOCK_STRIDE = 0x18;
static const uintptr_t SE_BLOCK_WAVEBANK_OFS = 0x0;
static const uintptr_t SE_BLOCK_SOUNDBANK_OFS = 0xC;
static uint8_t* g_seBlockTable = nullptr;

static void XactCueDestroy(XactCueEntry& e)
{
	// Same as the game's XAudio2_XACTCUE_Destroy
	if (e.cue_8)
	{
		reinterpret_cast<HRESULT(__stdcall*)(XactCue*)>(e.cue_8->vtbl[3])(e.cue_8);
		e.cue_8 = nullptr;
	}
	e.blk_no_4 = 0xFFFF;
	e.work_id_0 = 0xFFFFFFFF;
	e.snd_id_10 = 0;
	e.in_use_C = 0;
	e.field_14 = 0xFFFFFFFF;
}

// Called with the bank file struct the release function got. Only acts for SE block banks.
static void DestroySeBlockCues(uintptr_t bankFile, uintptr_t ofsInBlock)
{
	if (!g_seBlockTable || !g_xactCues)
		return;

	const intptr_t rel = intptr_t(bankFile) - intptr_t(uintptr_t(g_seBlockTable) + ofsInBlock);
	if (rel < 0 || (rel % SE_BLOCK_STRIDE) != 0 || rel / SE_BLOCK_STRIDE >= SE_BLOCK_COUNT)
		return; // Not one of the SE block banks

	const uint16_t blk = uint16_t(rel / SE_BLOCK_STRIDE);
	for (int i = 0; i < XACT_CUE_COUNT; i++)
	{
		XactCueEntry& e = g_xactCues[i];
		if (e.cue_8 && e.blk_no_4 == blk)
		{
			if (re4t::cfg->bVerboseLog)
				spd::log()->info("{} -> SE block {} is being released while cue {} (snd id {}) is alive; destroying it first",
					__FUNCTION__, blk, i, e.snd_id_10);
			XactCueDestroy(e);
		}
	}
}

// Audio thread tick: cb_audio_frame's loop starts with five calls in a row (0x97B815 in 1.1.0), then
// IXACT3Engine::DoWork and Sleep(3):
//   +0  sub_40E214  <- (broken, NOPed by AudioTweaks, thanks @QLOC)
//   +5  sub_403E27  <- AudioTickBegin: lock, then the original call
//   +10 sub_4016D6  <- (unsure; kept inside the lock in case it touches cues)
//   +15 Snd_iss_manager
//   +20 Snd_midi_sequencer <- AudioTickEnd: calls the original call, then unlock
// DoWork (XACT's own update, internally synchronized) stays outside the lock.
static uint8_t* g_audioTickCalls = nullptr;
static void(__cdecl* AudioTickFirstCall)();
static void(__cdecl* AudioTickLastCall)();

static void __cdecl AudioTickBegin()
{
	s_xactCueMutex.lock(); // Released in AudioTickEnd, a few calls later on the same thread
	AudioTickFirstCall();
}

static void __cdecl AudioTickEnd()
{
	AudioTickLastCall();
	s_xactCueMutex.unlock();
}

// SE block bank release functions (1.1.0: sub_97B270 wave bank, sub_97B2C0 sound bank; called through thunks from
// SndFileToLoad, SndSeObjInit, SndSeObjInit_0 and the release-all loop). Wrapped so the block's cues are destroyed first
// (while the bank still exists), and so the bank's Destroy (which frees any cue still alive from it) can't overlap the
// audio tick.
static void(__cdecl* SeWaveBankRelease_orig)(uint8_t* bankFile);
static void(__cdecl* SeSoundBankRelease_orig)(uint8_t* bankFile);

static void __cdecl SeWaveBankRelease_Hook(uint8_t* bankFile)
{
	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);
	DestroySeBlockCues(uintptr_t(bankFile), SE_BLOCK_WAVEBANK_OFS);
	SeWaveBankRelease_orig(bankFile);
}

static void __cdecl SeSoundBankRelease_Hook(uint8_t* bankFile)
{
	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);
	DestroySeBlockCues(uintptr_t(bankFile), SE_BLOCK_SOUNDBANK_OFS);
	SeSoundBankRelease_orig(bankFile);
}

// Game-thread cue users
static int(__cdecl* XAudio2_XACTCUE_GetState_0_orig)(int snd_id);
static int(__cdecl* XAudio2_XACTCUE_GetState_1_orig)(int snd_id, int blk_no);
static int(__cdecl* XAudio2_XACTCUE_IsEnded_orig)(int cue_no); // sub_97AED0
static int(__cdecl* Snd_str_req_orig)(int a1, int a2, int16_t a3, uint16_t a4);

static int __cdecl XAudio2_XACTCUE_GetState_0_Hook(int snd_id)
{
	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);
	return XAudio2_XACTCUE_GetState_0_orig(snd_id);
}

static int __cdecl XAudio2_XACTCUE_GetState_1_Hook(int snd_id, int blk_no)
{
	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);
	return XAudio2_XACTCUE_GetState_1_orig(snd_id, blk_no);
}

static int __cdecl XAudio2_XACTCUE_IsEnded_Hook(int cue_no)
{
	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);
	return XAudio2_XACTCUE_IsEnded_orig(cue_no);
}

static int __cdecl Snd_str_req_Hook(int a1, int a2, int16_t a3, uint16_t a4)
{
	std::lock_guard<std::recursive_mutex> lock(s_xactCueMutex);
	return Snd_str_req_orig(a1, a2, a3, a4);
}

static void InstallXactCueUseAfterFreeFix()
{
	// Get g_xact_cue_mb from XAudio2_XACTCUE_GetState
	auto pattern = hook::pattern("8B 8C 00 ? ? ? ? 03 C0 85 C9 74 30 83 B8 ? ? ? ? ? 75 09 83 B8");
	g_xactCues = reinterpret_cast<XactCueEntry*>(*pattern.count(1).get(0).get<uint8_t*>(3) - offsetof(XactCueEntry, cue_8));

	// Get the SE block table from SndSePlay
	pattern = hook::pattern("83 3C C5 ? ? ? ? ? 74 24 0F B7 4E 14 8B 16 51 52 E8");
	g_seBlockTable = *pattern.count(1).get(0).get<uint8_t*>(3) - 0x10;

	// Get bank release funcs from SndFileToLoad
	pattern = hook::pattern("E8 ? ? ? ? 53 E8 ? ? ? ? 56 E8 ? ? ? ? 56 68 ? ? ? ? E8 ? ? ? ? 8B 87");
	ReadCall(injector::GetBranchDestination(pattern.get(0).get<uint32_t>(0)).as_int(), SeWaveBankRelease_orig);
	InjectHook(injector::GetBranchDestination(pattern.get(0).get<uint32_t>(0)).as_int(), SeWaveBankRelease_Hook);
	ReadCall(injector::GetBranchDestination(pattern.get(0).get<uint32_t>(6)).as_int(), SeSoundBankRelease_orig);
	InjectHook(injector::GetBranchDestination(pattern.get(0).get<uint32_t>(6)).as_int(), SeSoundBankRelease_Hook);

	// Audio tick: lock before its first sound call, unlock after its last (see AudioTickBegin/AudioTickEnd)
	// The loop's first call (+0, sub_40E214) is skipped, since AudioTweaks NOPs it unconditionally (see there).
	ReadCall(g_audioTickCalls + 5, AudioTickFirstCall);
	InjectHook(g_audioTickCalls + 5, AudioTickBegin, HookType::Call);
	ReadCall(g_audioTickCalls + 20, AudioTickLastCall);
	InjectHook(g_audioTickCalls + 20, AudioTickEnd, HookType::Call);

	// Get XAudio2_XACTCUE_GetState_0 from SndEndCheck_0.
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 85 C0 0F 94 C0 5D C3 CC");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), XAudio2_XACTCUE_GetState_0_orig);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), XAudio2_XACTCUE_GetState_0_Hook);

	// Get XAudio2_XACTCUE_GetState_1 from SndEndCheck.
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 08 85 C0 0F 94 C0 5D C3 CC");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), XAudio2_XACTCUE_GetState_1_orig);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), XAudio2_XACTCUE_GetState_1_Hook);

	// Get XAudio2_XACTCUE_IsEnded from se_ctrl_pause_on2.
	pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 85 C0 75 0E 66 83 4E ? ? B9 ? ? ? ? 66 09 4E 2E 47");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), XAudio2_XACTCUE_IsEnded_orig);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), XAudio2_XACTCUE_IsEnded_Hook);

	// Get Snd_str_req from SndAllStopMovie.
	pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? 83 C4 10 83 C7 10 83 FF 40 7C A7");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), Snd_str_req_orig);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), Snd_str_req_Hook);

	spd::log()->info("{} -> Applied", __FUNCTION__);
}

void re4t::init::AudioTweaks()
{
	// Hook Snd_set_system_vol so we can override volume values with our own after game updates them
	auto pattern = hook::pattern("B8 10 00 00 00 0F B6 55 ? 52 50 E8 ? ? ? ? 83 C4");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xB)).as_int(), Snd_set_system_vol);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0xB)).as_int(), Snd_set_system_vol_Hook);

	// Hook mwPlySetOutVol so we can override FMV volume, and store mwply handle to allow updating volume during runtime
	pattern = hook::pattern("6A 9C 56 E8 ? ? ? ?");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(3), mwPlySetOutVol);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(3), mwPlySetOutVol_Hook, HookType::Call);

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

	// Hook SndCall call inside knife_r3_fire10 to allow restoring original GC knife sound effect
	pattern = hook::pattern("83 C0 ? 50 6A 03 6A 01 E8");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(8), knife_r3_fire10_SndCall_Hook, HookType::Call);

	// Nop out a bugged function call that has a small chance of causing audio issues.
	// 
	// Called function pushes ECX to stack then branches based on that stack variable, but the caller never sets ECX first.
	// ECX is just leftover garbage data from either `VISetPostRetraceCallback` or Win32 `Sleep`.
	// 
	// Code inside the func seems to pause all active AVX tracks, in testing it never seemed to actually run.
	// but since ECX is essentially random data there's still a chance it could trigger and cause audio issues.
	// (forcing the code to run caused load screen music to stop playing, likely affects other music in the game too)
	// 
	// Possibly an uninitialized local variable (or one only assigned in debug builds)
	// which MSVC optimized into a `push ecx` for stack allocation, leaving the comparison to read garbage?
	// (@QLOC may want to take notice of C4700 warnings in future!)
	pattern = hook::pattern("E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 8B");
	g_audioTickCalls = pattern.count(1).get(0).get<uint8_t>(); // cb_audio_frame's loop, also used by InstallXactCueUseAfterFreeFix
	Nop(g_audioTickCalls, 5);

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

				if (re4t::cfg->bSilenceArmoredAshley)
				{
					// Make the game think Ashley isn't using the clanky costume
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef |= (1 << regs.carry_flag);
				}
			}
		}; injector::MakeInline<ClankClanklHook>(pattern.count(1).get(0).get<uint32_t>(3), pattern.count(1).get(0).get<uint32_t>(10));

		spd::log()->info("SilenceArmoredAshley applied");
	}

	InstallXactCueUseAfterFreeFix();
}
