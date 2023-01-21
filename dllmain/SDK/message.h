#pragma once
#include "basic_types.h"

struct MSG_QUE
{
	int16_t pos_X_0;
	int16_t pos_Y_2;
	uint16_t glyphId_4;
	uint8_t width_6;
	uint8_t height_7;
	uint32_t color_8;
	struct MessageFont* m_p_font_C;
};
assert_size(MSG_QUE, 0x10);

struct Message
{
	void* __vftable;
	uint32_t backup_Flags_STOP_4;
	uint32_t be_flag_8;
	uint8_t r_no_0_C;
	uint8_t r_no_1_D;
	uint8_t r_no_2_E;
	uint8_t r_no_3_F;
	uint32_t m_state_10;
	float m_scale_w_14;
	float m_scale_h_18;
	int8_t m_font_w_1C;
	int8_t m_font_h_1D;
	uint16_t m_item_no_1E;
	uint16_t m_ot_type_20;
	uint16_t m_ot_no_22;
	struct MessageFont* m_pFont_24;
	int16_t m_screenPos_X_28;
	int16_t m_screenPos_Y_2A;
	int16_t m_pos0_x_2C[16];
	int16_t m_pos0_y_4C;
	int16_t m_width_4E[16];
	int16_t m_width_max_6E;
	int16_t m_height_70;
	int16_t m_bttn_wait_72;
	int8_t m_btn_74;
	int8_t m_evt_no_75;
	int8_t m_lines_76;
	uint8_t unk_77[1];
	int16_t m_number_width_78;
	int16_t m_line_gap_7A;
	int16_t m_char_gap_7C;
	WORD word7E;
	uint32_t m_col_80;
	uint32_t m_attr_84;
	int16_t m_spd_88;
	int16_t m_spd_cnt_8A;
	float float8C;
	int16_t m_spd_old_90;
	int16_t m_spd_flag_92;
	int16_t padpadpad_94;
	int16_t m_wait_cnt_96;
	uint16_t m_jump_mes_98[3];
	int8_t m_jump_idx_9E;
	int8_t m_jump_max_9F;
	uint16_t* m_pMes_A0;
	uint16_t* m_pRetAddr_A4;
	struct Message::MessageFont* m_pRetFont_A8;
	uint32_t m_number_AC[2];
	uint32_t m_div_B4[2];
	uint32_t m_number_bak_BC[2];
	uint32_t m_div_bak_C4[2];
	uint16_t m_se_blk_CC;
	uint16_t m_se_no_CE;
	MSG_QUE* m_queue_D0;
	MSG_QUE* m_pMque_D4;
	MSG_QUE* m_selTbl_D8[8];
	char m_selTbl_size_F8;
	char m_sel_F9;
	char m_cur_FA;
	char m_cursol_time_FB;
	char m_who_FC;
	char m_turn_FD;
	BYTE field_FE;
	BYTE field_FF;
};
assert_size(Message, 0x100);

struct	GXTlutObj { uint32_t dummy_0[3]; };
assert_size(GXTlutObj, 0xC);

struct FONT_TEX
{
	GXTexObj texobj_0;
	GXTlutObj tlutobj_20;
	float mat[2][4];
	uint8_t unk_4C[16];
	uint32_t field_5C;
	void* data_60;
};
assert_size(FONT_TEX, 0x64);

enum LAYOUT_TYPE
{
	LAYOUT_CAPTION = 0x0,
	LAYOUT_ACT_BTN = 0x1,
	LAYOUT_SUBSCRN = 0x2,
	LAYOUT_MEMCARD = 0x3,
	LAYOUT_OPERATOR = 0x4,
	LAYOUT_SYSTEM = 0x5,
	LAYOUT_SHOP_LIST = 0x6,
	LAYOUT_FILE = 0x7,
	LAYOUT_MANUAL = 0x8,
	LAYOUT_NUM = 0x9,
};

class MessageControl;
typedef void(__thiscall* MessageControl__setFontSize_Fn)(MessageControl* thisptr, int wk, int16_t font_w, int16_t font_h);
typedef void(__thiscall* MessageControl__setLayout_Fn)(MessageControl* thisptr, int wk, LAYOUT_TYPE type);
extern MessageControl__setFontSize_Fn MessageControl__setFontSize;
extern MessageControl__setLayout_Fn MessageControl__setLayout;

class MessageControl
{
public:
	void* __vftable;
	int8_t m_sel_sav_4;
	struct Message m_Msg_8[16];
	uint8_t* m_font_addr_1008[4];
	uint32_t m_state_1018;
	FONT_TEX m_mTex[4][2];
	float m_stop_133C;

	inline void setFontSize(int wk, int16_t font_w, int16_t font_h)
	{
		return MessageControl__setFontSize(this, wk, font_w, font_h);
	}

	inline void setLayout(int wk, LAYOUT_TYPE type)
	{
		return MessageControl__setLayout(this, wk, type);
	}
};
assert_size(MessageControl, 0x1340);

extern MessageControl* cMes;
