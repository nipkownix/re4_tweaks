#pragma once
#include "basic_types.h"

class cModel;

enum SAT_TYPE : uint16_t
{
	SAT_TOP_WK = 0x1,
	SAT_SHAPE_RECTANGLE = 0x2,
	SAT_OBJ_ADJ_CANCEL0 = 0,
	SAT_OBJ_ADJ_CANCEL = 4,
	SAT_OBJ_ADJ_CANCEL1 = 0x4,
	SAT_OBJ_ADJ_CANCEL2 = 0x8,
	SAT_OBJ_ADJ_CANCEL3 = 0x10,
	SAT_EDGE_CANCEL = 0x20,
	SAT_CK_ALL = 0,
	SAT_CK_FLOOR = 0x40,
	SAT_CK_WALL = 0x80,
	SAT_SCA_ENABLE = 0x100, // map collision? SCA = SCene Atari?
	SAT_OBA_ENABLE = 0x200, // em collision? OBA = OBject Atari?
	SAT_TYPE_NONE = 0,
	SAT_TYPE_MIDDLE = 0x400,
	SAT_TYPE_SMALL = 0x800,
	SAT_TYPE_PL = 0x1000,
	SAT_TYPE_EM = 0x2000,
	SAT_TYPE_ROUTE = 0x4000,
	SAT_TYPE_SEE = 0x8000,
};

struct cAtariInfo
{
	Vec m_offset_0;
	float m_radius_C;
	float m_radius2_10;
	float m_height_14;
	int16_t m_parts_no_18;
	uint16_t m_flag_1A; // SAT_TYPE
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
