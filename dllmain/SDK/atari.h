#pragma once
#include "basic_types.h"
#include "cManager.h"
#include "model.h"

struct cSatBlock
{
	Vec m_Pos_0;
	Vec m_Size_C;
	uint16_t m_nFloor_18;
	uint16_t m_nSlope_1A;
	uint16_t m_nWall_1C;
	uint16_t m_Flag_1E;
	cSatBlock* m_pList_20;
};
assert_size(cSatBlock, 0x24);

class cSat : public cUnit
{
public:
	enum FLAG : uint8_t
	{
		FLAG_MEM_VAL = 0x2,
		FLAG_ENABLE_VAL = 0x4,
	};

	Vec* vert_p_C; // originally float(*vert_p_C)[3];
	Vec* norm_p_10;
	Vec* edge_p_14;

	uint16_t(*poly_p_18)[10];
	uint16_t vertex_num_1C;
	uint16_t polygon_num_1E;
	uint16_t floor_num_20;
	uint16_t slope_num_22;
	uint16_t wall_num_24;
	uint16_t bb_num_26;
	uint16_t normal_num_28;
	cSat::FLAG m_Flag_2A;
	uint8_t dummy33_2B;
	uint16_t edge_num_2C;
	uint16_t dummy44_2E;
	cSatBlock* block_p_30;
	float dummyFF[3];
	Vec bound_pos;
	Vec bound_size;
	class cSatFile* pSat_58;
	cSat* pList_5C;
	Mtx mat_60;
	Mtx imat_90;

	inline bool IsValid()
	{
		return (be_flag_4 & 0x201) == 1;
	}
};
assert_size(cSat, 0xC0);

class cSatMgr;
extern cSatMgr* SatMgr; // Game.cpp
extern cSatMgr* EatMgr; // Game.cpp

class cSatMgr : public cManager<cSat>
{
public:
	uint8_t unk_1C[4];
};
assert_size(cSatMgr, 0x20);
