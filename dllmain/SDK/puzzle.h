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
assert_size(pzlPiece, 0x24);

class itemPiece : public pzlPiece
{
public:
	cItem* pItem_24;
	struct cMap* pModel_28;
};
assert_size(itemPiece, 0x2C);

itemPiece* ItemPiecePtr(); // Game.cpp

struct pzlBoard
{
	uint8_t* m_cell_0;
	int8_t m_size_x_4;
	int8_t m_size_y_5;
	uint8_t m_piece_max_6;
	pzlPiece** m_p_piece_8;
	Mtx m_mat_C;
	int8_t m_cur_x_3C;
	int8_t m_cur_y_3D;
	int8_t m_wall_miss_flag_3E;
	int8_t m_out_miss_flag_3F;
};
assert_size(pzlBoard, 0x40);

class pzlPlayer;
typedef pzlPiece*(__fastcall* pzlPlayer__ptrPiece_Fn)(pzlPlayer* thisptr, void* unused, pzlBoard* p_board);
extern pzlPlayer__ptrPiece_Fn pzlPlayer__ptrPiece;

class pzlPlayer
{
public:
	pzlBoard* m_board_0;
	pzlBoard* m_space_4;
	itemPiece* m_piece_8;
	uint8_t m_piece_max_C;
	pzlPiece* m_inhand_10;
	pzlPiece* m_extra_14;
	float m_piece_bak_pos_x_18;
	float m_piece_bak_pos_y_1C;
	int8_t m_piece_bak_orientation_20;
	pzlBoard* m_piece_bak_board_24;
	pzlBoard* m_board_sav_28;
	int8_t m_cur_x_sav_2C;
	int8_t m_cur_y_sav_2D;
	pzlBoard* m_p_active_board_30;

	inline pzlPiece* ptrPiece(pzlBoard* p_board)
	{
		return pzlPlayer__ptrPiece(this, nullptr, p_board);
	}
};
assert_size(pzlPlayer, 0x34);
