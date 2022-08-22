#pragma once
#include "basic_types.h"
#include "item.h"

class pzlPiece
{
public:
	uint8_t be_flag_0;
	struct pieceData* m_p_data_4;
	uint8_t m_state;
	float m_center_x_C;
	float m_center_y_10;
	float m_pos_x_14;
	float m_pos_y_18;
	float m_pos_z_1C;
	int8_t m_orientation_20;
	uint8_t m_place_21;
};

class itemPiece : public pzlPiece
{
public:
	cItem* pItem_24;
	struct cMap* pModel_28;
};

itemPiece* ItemPiecePtr(); // Game.cpp
