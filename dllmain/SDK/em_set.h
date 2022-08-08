#pragma once
#include "basic_types.h"

enum EM_BE_FLAG : uint8_t
{
	EM_BE_FLAG_ALIVE = 0x1,
	EM_BE_FLAG_SET = 0x2,
	EM_BE_FLAG_DIE = 0x80,
};

struct EM_LIST
{
	EM_BE_FLAG be_flag_0;
	char id_1;
	char type_2;
	char set_3;
	uint32_t flag_4;
	int16_t hp_8;
	uint8_t emset_no_A;
	char Character_B;
	SVEC s_pos_C;
	SVEC s_ang_12;
	uint16_t room_18;
	int16_t Guard_r_1A;

	// extended params added by re4_tweaks, normally field is Dummy / unused
	uint16_t percentageMotionSpeed_1C;
	uint16_t percentageScale_1E;
};
assert_size(EM_LIST, 0x20);
