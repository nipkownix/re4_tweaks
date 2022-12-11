#pragma once
#include "basic_types.h"

enum FADE_BE_FLAG : uint16_t
{
	FADE_BE_ALIVE = 0x1,
	FADE_BE_CONTINUE = 0x2,
	FADE_BE_SUPER = 0x4,
};
assert_size(FADE_BE_FLAG, 0x2);

enum FADE_NO
{
	FADE_NO_SYSTEM = 0x0,
	FADE_NO_SCENARIO = 0x1,
	FADE_NO_ROOM = 0x2,
	FADE_NO_ERROR = 0x3,
	FADE_NUM = 0x4,
};
assert_size(FADE_NO, 0x4);

typedef void (__cdecl* j_j_j_FadeSet_Fn)(int no, uint32_t time, uint32_t ctrl_flag);
extern j_j_j_FadeSet_Fn j_j_j_FadeSet;

class FADE_WORK
{
public:
	uint32_t s_col_0;
	uint32_t e_col_4;
	uint32_t col_8;
	Vec pos_C;
	FADE_BE_FLAG be_flg_18;
	uint16_t state_1A;
	uint32_t time_1C;
	float cnt_20;
	float delay_24;

	inline void __cdecl FadeSet(int no, uint32_t time, uint32_t ctrl_flag)
	{
		return j_j_j_FadeSet(no, time, ctrl_flag);
	}
};
assert_size(FADE_WORK, 0x28);