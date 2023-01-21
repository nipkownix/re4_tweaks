#pragma once
#include "basic_types.h"

struct CountDown
{
	uint32_t m_state_0;
	int8_t m_minute_4;
	int8_t m_second_5;
	int8_t m_centisecond_6;
	float m_frame_8;
	uint32_t m_warn_frame_C;
	uint32_t m_counter_10;
};
assert_size(CountDown, 0x14);

struct COUNT_DOWN_SAVE
{
	uint32_t state_0;
	float frame_4;
	bool disp_8;
	uint8_t rev_9;
	int16_t time_A;
	uint32_t warn_frame_C;
};
assert_size(COUNT_DOWN_SAVE, 0x10);

struct ActionButton
{
	uint8_t m_disp_flag_0;
	uint8_t m_disp_flag_old_1;
	uint8_t byte_2;
	uint8_t byte_2_old;
	uint8_t byte_4;
	uint8_t gap5[1];
	uint8_t byte_4_old;
};
assert_size(ActionButton, 0x7);

class LifeMeter;
typedef void(__thiscall* LifeMeter__roomInit_Fn)(LifeMeter* thisptr);
extern LifeMeter__roomInit_Fn LifeMeter__roomInit;

class LifeMeter
{
public:
	cEm* m_pEm_0;
	float m_life_4;
	int8_t m_life_level_8;
	float m_color0_C[4];
	float m_color1_1C[4];
	float m_life_sub_2C;
	int8_t m_life_level_sub_30;
	float m_color0_sub_34[4];
	float m_color1_sub_44[4];
	uint8_t m_state_color0[3][4];
	uint8_t m_state_color1[3][4];

	inline void roomInit()
	{
		return LifeMeter__roomInit(this);
	}
};
assert_size(LifeMeter, 0x6C);

class BulletInfo;
typedef void(__thiscall* BulletInfo__roomInit_Fn)(BulletInfo* thisptr);
extern BulletInfo__roomInit_Fn BulletInfo__roomInit;

class BulletInfo
{
public:
	uint8_t m_full_0;
	uint8_t m_num_1;
	uint8_t m_num_old_2;
	ID_UNIT* pDigit0_4;
	ID_UNIT* pDigit1_8;
	ID_UNIT* pIcon_C;
	int m_mark_old_10;

	inline void roomInit()
	{
		return BulletInfo__roomInit(this);
	}

};
assert_size(BulletInfo, 0x14);

struct Cockpit
{
	LifeMeter m_LifeMeter_0;
	BulletInfo m_BlltInfo_6C;
	Vec m_pos_std_80;
	Vec m_pos_wide_8C;
	CountDown m_CountDown_98;
	COUNT_DOWN_SAVE m_Save_AC;
	uint8_t unk_maybe_part_of_COUNT_DOWN_SAVE_BC;
	ActionButton m_ActBttn_BD;
};
assert_size(Cockpit, 0xC4);

extern Cockpit* Cckpt;
