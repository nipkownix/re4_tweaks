#pragma once
#include "basic_types.h"

class cModel;
struct cAtariInfo
{
	Vec m_offset_0;
	float m_radius_C;
	float m_radius2_10;
	float m_height_14;
	int16_t m_parts_no_18;
	uint16_t m_flag_1A;
	float m_radius_n_1C;
	float m_radius2_n_20;
	uint16_t m_hokan_24;
	uint16_t m_stat_26;
	cModel* m_pMod_28;
	float m_radius3_2C;
	Vec m_Pos_30;
	Vec m_oldPos_3C;
	cAtariInfo* m_pList_48;
};
assert_size(cAtariInfo, 0x4C);
