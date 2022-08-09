#pragma once
#include "basic_types.h"

struct ARQRequest
{
	ARQRequest* next_0;
	uint32_t owner_4;
	uint32_t type_8;
	uint32_t priority_C;
	uint32_t source_10;
	uint32_t dest_14;
	uint32_t length_18;
	void* callback_1C;
};
assert_size(ARQRequest, 0x20);

struct SND_CTRL
{
	uint32_t audio_frame_0;
	uint32_t snd_mode_4;
	uint32_t snd_id_8;
	uint32_t zero_adrs_C;
	uint32_t aram_adrs_10;
	int dvd_err_flag_14;
	uint32_t req_push_idx_18;
	uint32_t req_exec_idx_1C;
	uint16_t random_20;
	uint16_t reset_flag_22;
	uint16_t status_flag_24;
	uint16_t se_ctrl_flag_26;
	int16_t se_fout_time_28;
	int16_t vdown_value_2A;
	int8_t now_srd_type_2C;
	int8_t dummy_1_2D;
	int16_t rnd_pitch_2E;
	bool IsLink_30;
	int16_t pause_blk_32;
	int16_t pause_no_34;
	uint32_t pause_id_38;
	int DVDGetDriveStatus_result_3C;
	int16_t vol_mas_bgm_40;
	int16_t vol_mas_se_42;
	int16_t vol_iss_bgm_44;
	int16_t vol_iss_se_46;
	int16_t vol_str_bgm_48;
	int16_t vol_str_se_4A;
	int8_t prio_4C;
	int8_t pan_4D;
	int8_t span_4E;
	int8_t vol_4F;
	int8_t svol_50;
	int8_t aux_a_51;
	int8_t aux_b_52;
	int8_t lpf_53;
	int8_t srd_type_54;
	int16_t pitch_56;
	int16_t dop_p_58;
	uint16_t req_bit_5A;
	uint16_t para_flag_5C;
	uint32_t seq_adjust_60;
	uint32_t seq_proc_64;
	uint8_t seq_data_68[4];
	uint8_t code_6C;
	uint8_t track_6D;
	uint32_t efx_mem_70;
	uint32_t dsp_cyc_74;
	uint32_t max_cyc_78;
	uint32_t now_cyc_7C;
	uint16_t max_voice_80;
	uint16_t now_voice_82;
	uint16_t max_axv_vo_84;
	uint16_t now_axv_vo_86;
	uint16_t max_str_vo_88;
	uint16_t now_str_vo_8A;
	uint16_t max_syn_vo_8C;
	uint16_t now_syn_vo_8E;
	uint16_t max_total_vo_90;
	uint16_t now_total_vo_92;
	ARQRequest arq_req_94;
	int arq_flag_B4;
};
assert_size(SND_CTRL, 0xB8);

// SND_STR/SND_SEQ seem completely different in PS2, sadly
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
