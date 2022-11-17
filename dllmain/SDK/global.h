#pragma once
#include "basic_types.h"
#include "camera.h"
#include "em_set.h"
#include "pad.h"

struct TRANS
{
	uint32_t tev_stage_0;
	uint32_t texmap_4;
	uint32_t tex_coord_8;
	Mtx screen_mat_C[248];
	GXTexObj TexObj_2E8C[248];
	uint8_t* pbptr_4D8C;
	float max_pbptr_4D90;
};
assert_size(TRANS, 0x4D94);

struct EMINFO_WK
{
	uint8_t WorkType_0;
	uint8_t Work0_1;
	uint8_t Work1_2;
	uint8_t Work2_3;
	Vec Pos_4;
	float Dir_mb_10;
	float Dir_mb_14;
	uint8_t Dummy_18[40];
};
assert_size(EMINFO_WK, 0x40);

struct EMINFO_DATA
{
	uint32_t Num_0;
	uint32_t Id_4;
	EMINFO_WK Data_8[256];
};
assert_size(EMINFO_DATA, 0x4008);

struct EM_SE_INFO
{
	Vec pos_0;
	uint8_t type_C;
	uint8_t dummy11_D;
	uint8_t dummy12_E;
	uint8_t dummy13_F;
};
assert_size(EM_SE_INFO, 0x10);

enum class PlayerCharacter : uint8_t
{
	Leon,
	Ashley,
	Ada,
	HUNK,
	Krauser,
	Wesker,
	LeonAshley
};

enum class GameDifficulty : uint8_t
{
	Unk0 = 0x0,
	VeryEasy = 0x1,
	Unk2 = 0x2,
	Easy = 0x3,
	Unk4 = 0x4,
	Medium = 0x5,
	Pro = 0x6,
};

enum class LeonCostume : uint8_t
{
	Jacket,
	Normal,
	Vest,
	RPD,
	Mafia
};

enum class AshleyCostume : uint8_t
{
	Normal,
	Popstar,
	Armor
};

enum class AdaCostume : uint8_t
{
	RE2,
	Spy,
	RE2_d, // duplicated?
	Normal
};

union PlayerCostume
{
	LeonCostume Leon;
	AshleyCostume Ashley;
	AdaCostume Ada;
};
assert_size(PlayerCostume, 1);

struct ITEM_SAVE_WORK
{
	uint8_t item_type_0;
	uint8_t item_at_1;
	int8_t item_eff_2;
	uint8_t padd0_3;
	uint16_t room_no_4;
	uint16_t item_id_6;
	uint16_t item_num_8;
	int16_t item_pos_x_A;
	int16_t item_pos_y_C;
	int16_t item_pos_z_E;
};
assert_size(ITEM_SAVE_WORK, 0x10);

struct OperatorSaveWk
{
	uint32_t SaveFlag_0[1];
	int8_t OwType_4;
	uint8_t dummy8_5[3];
	uint32_t OcFlag_8[3];
	uint32_t OwFlag_14[1];
	uint32_t MdtNo_18;
	uint32_t dummy32_1C[10];
};
assert_size(OperatorSaveWk, 0x44);

// Pointed to by pG
struct __declspec(align(4)) GLOBAL_WK
{
	enum class Routine0 : uint8_t
	{
		Init,
		StageInit,
		RoomInit,
		MainLoop,
		Doordemo, // set by tool menu areajump, maybe showed door animation at earlier point
		Ending,
		Option
	};

	bool IsDevConsole_0;
	char shooting_mode_1;
	char CardLastSelNo_2;
	uint8_t glunk_3[1];
	uint32_t CardStatus_4;
	uint32_t CardLastSaveNo_mb_8;
	uint32_t glunk_field_C;
	void* ptr_romFont_10;
	bool IsMessageInit_14;
	uint8_t glunk_field_15;
	uint8_t glunk_field_16;
	uint8_t wideMode_17;
	uint32_t glunk_flags_18;
	uint32_t glunk_field_1C;
	uint8_t Rno0_20;
	uint8_t Rno1_21;
	uint8_t Rno2_22;
	uint8_t Rno3_23;
	uint32_t DblBufIdx_24;
	uint16_t RoomNo_next;
	uint8_t Part_next_2A;
	uint8_t glunk_2B[1];
	Vec nextRoomPosition_2C;
	float nextRoomRotationY_38;
	uint32_t* pStFnt_3C;
	uint32_t* pRoom_40;
	uint32_t* pWep_44;
	uint32_t* pCore_48;
	uint32_t* pOption_4C;
	uint32_t* pOption_50;
	uint32_t Flags_SYSTEM_0_54[1];
	uint32_t Flags_DISP_0_58[1];
	uint32_t curGameTime_5C;
	uint32_t flags_DEBUG_0_60[1];
	uint32_t flags_DEBUG_1_64;
	uint32_t flags_DEBUG_2_68;
	uint32_t flags_DEBUG_3_6C;
	float deltaTime_70;
	CAMERA Camera_74;
	uint32_t room_start_addr[1];
	uint32_t flags_STOP_0_170[1];
	uint32_t flags_ROOM_0_174[4];
	TRANS Trans_184;
	uint8_t* pPrim_4F18;
	int nPrim_4F1C;
	uint32_t* RoomMes_4F20;
	void* pCamCore_4F24;
	void* pCamRoom_4F28;
	struct GLOBAL_WK::RTP* Rtp_4F2C;
	EMINFO_DATA* pEmi_4F30;
	uint32_t* pOsd_4F34;
	int8_t AreaNo_4F38;
	uint8_t glunk_4F39[3];
	EM_SE_INFO SeInfo_4F3C;
	uint32_t Room_act_tbl_4F4C[8];
	uint32_t MotTbl_4F6C[1];
	uint32_t room_end_addr_4F70[1];
	Vec Pl_adjust_4F74;
	uint8_t weapon_no_old_4F80;
	uint8_t door_se_4F81;
	uint16_t time_bonus_4F82;
	Vec coin_pos_4F84;
	uint32_t save_data_start_addr_4F90;
	int32_t dynamicDifficultyPoints_4F94;
	uint8_t dynamicDifficultyLevel_4F98;
	uint8_t glunk_field_4F99;
	uint8_t chapter_4F9A;
	uint8_t glunk_field_4F9B;
	uint16_t save_cnt_4F9C;
	uint16_t game_cnt_4F9E;
	uint16_t r_continue_cnt_4FA0;
	uint8_t terminal_no_4FA2;
	uint8_t languageId_4FA3;
	int playTime_4FA4;
	int goldAmount_4FA8;
	uint16_t curRoomId_4FAC;
	uint8_t Part_4FAE;
	uint8_t JumpPoint_4FAF;
	uint16_t prevRoomId_4FB0;
	uint8_t Part_old_4FB2;
	char curEmListNumber_4FB3;
	int16_t playerHpCur_4FB4;
	int16_t playerHpMax_4FB6;
	int16_t subHpCur_4FB8;
	int16_t subHpMax_4FBA;
	int16_t pl_dummy30_4FBC;
	int16_t pl_dummy32_4FBE;
	uint8_t weapon_no_4FC0;
	uint8_t weapon_type_4FC1;
	uint8_t bullet_type_4FC2;
	uint8_t weapon_lv_power_4FC3;
	uint8_t weapon_lv_speed_4FC4;
	uint8_t weapon_lv_blt_4FC5;
	uint8_t pl_dummy52_4FC6;
	uint8_t pl_dummy53_4FC7;
	PlayerCharacter pl_type_4FC8;
	PlayerCostume plCostume_4FC9;
	uint8_t weapon_lv_reload_4FCA;
	AshleyCostume subCostume_4FCB;
	uint16_t pl_dummy60_4FCC;
	uint16_t pl_flag_4FCE;
	Vec gameSave_position_4FD0;
	float gameSave_rotationY_4FDC;
	uint32_t pl_dummy90_4FE0[15];
	uint32_t flags_STATUS_0_501C[1];
	uint32_t flags_STATUS_1_5020[1];
	uint32_t flags_STATUS_2_5024[1];
	uint32_t flags_STATUS_3_5028[1];
	uint32_t Em_flg_502C[19][8];
	uint32_t flags_ITEM_SET_0_528C[8];
	uint32_t Item_find_flg_52AC[8];
	uint32_t flags_SCENARIO_0_52CC[1];
	uint32_t flags_SCENARIO_1_52D0[1];
	uint32_t flags_SCENARIO_2_52D4[6];
	uint32_t flags_KEY_LOCK_52EC[8];
	uint32_t frameCounter_530C;
	uint32_t save_free_work_5310[64];
	EM_LIST Em_list_5410[256];
	ITEM_SAVE_WORK item_save_7410[256];
	OperatorSaveWk OpeSaveWK_8410;
	int32_t peseta_bak_8454;
	int16_t shootingScore_8458[4];
	uint16_t c_continue_cnt_8460;
	uint16_t g_continue_cnt_8462;
	uint32_t c_kill_cnt_8464;
	uint32_t g_kill_cnt_8468;
	uint32_t c_hit_cnt_846C;
	uint32_t g_hit_cnt_8470;
	uint32_t c_shot_cnt_8474;
	uint32_t g_shot_cnt_8478;
	GameDifficulty gameDifficulty_847C;
	uint8_t save_dummy8_847D[3];
	int32_t SaveKind_8480;
	uint8_t glunk_8484[516];
	int8_t PrintNo_8688;
	int8_t PrintNo_bak_8689;
	uint32_t glunk_field_868C_maybePadding;

	inline void SetRoutine(Routine0 Rno0, uint8_t Rno1, uint8_t Rno2, uint8_t Rno3)
	{
		Rno0_20 = uint8_t(Rno0);
		Rno1_21 = Rno1;
		Rno2_22 = Rno2;
		Rno3_23 = Rno3;
	}
};
assert_size(GLOBAL_WK, 0x8690);

struct SYSTEM_SAVE_WORK
{
	uint32_t flags_CONFIG_0[1];
	uint32_t flags_EXTRA_4[1];
	uint8_t language_8;
	uint8_t eff_country_9; // determines level of violence
	uint8_t brightness_A;
	keyConfigTypes pad_type_B;
	uint8_t SndMode_C;
	uint8_t adasReportNo_D;
	uint8_t field_E;
	uint8_t field_F;
	uint32_t field_10;
	uint32_t field_14;
	uint32_t field_18;
	uint32_t field_1C;
	uint32_t field_20;
	uint32_t field_24;
	uint32_t field_28;
	uint8_t struct_2C[80];
	uint32_t field_7C;
	uint32_t field_80;
	uint32_t field_84;
	uint32_t field_88;
	uint32_t field_8C;
	uint32_t field_90;
	uint8_t unk_94[8];
};
assert_size(SYSTEM_SAVE_WORK, 0x9C);
