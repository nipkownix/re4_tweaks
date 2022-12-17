#pragma once
#include "basic_types.h"

struct __declspec(align(4)) TITLE_WORK
{
	enum class Routine0 : int8_t
	{
		Init,
		Wait,
		Nintendo,
		Warning,
		Logo,
		Main,
		Sub,
		Exit,
		Ada
	};
	int8_t Rno0_0;
	int8_t Rno1_1;
	int8_t Rno2_2;
	int8_t Rno3_3;
	int8_t TITLE_WORK_field_4;
	int8_t TITLE_WORK_field_5;
	int8_t TITLE_WORK_field_6;
	int8_t TITLE_WORK_field_7;
	int8_t titleStart_Rno_8; // guessed name
	uint8_t TITLE_WORK_field_9[3];
	uint32_t TITLE_WORK_field_timer_mb_C;
	char TITLE_WORK_field_10;
	uint8_t TITLE_WORK_field_11;
	uint8_t TITLE_WORK_field_12;
	uint8_t TITLE_WORK_unk_13[5];
	uint32_t read_id_18;
	uint32_t se_id_1C;
	uint32_t* pIdDat_20;
	int32_t counter_24;
	int32_t menu_num_28;
	int32_t menu_no_2C;
	int32_t level_no_30;
	int32_t lang_no_34;
	int32_t extra_no_38;
	struct ID_UNIT* p_menu_3C[7];
	bool scroll_58;
	float scroll_add_5C;
	int32_t dbg_mode_60;
	uint8_t* data_addr_64;
	uint32_t data_size_68;
	int8_t omk_menu_no_6C;
	int8_t omk_char_no_6D;
	int8_t omk_stage_no_6E;
	int8_t Rno1_bak_6F;
	int8_t Rno2_bak_70;
	int8_t Rno3_bak_71;
	int32_t counter_bak_74;
	int8_t Stage_78;
	uint32_t TITLE_WORK_field_7C;
	uint8_t TITLE_WORK_field_r_no_1_bk_mb_80;
	uint8_t TITLE_WORK_unk_84[56];

	inline void SetRoutine(Routine0 Rno0, int8_t Rno1, int8_t Rno2, int8_t Rno3)
	{
		Rno0_0 = int8_t(Rno0);
		Rno1_1 = Rno1;
		Rno2_2 = Rno2;
		Rno3_3 = Rno3;
	}
};
assert_size(TITLE_WORK, 0xBC);

enum TITLE_COMMAND
{
	TTL_CMD_NULL = 0,
	TTL_CMD_START = 1,
	TTL_CMD_LOAD = 2,
	TTL_CMD_OPTION = 3,
	TTL_CMD_LEVEL = 4,
	TTL_CMD_LANGUAGE = 5,
	TTL_CMD_EXTRA = 6,
	TTL_CMD_LEADERBOARDS_mb = 7,
	TTL_CMD_CREDITS_mb = 8,
	TTL_CMD_QUITGAME_mb = 9
};


