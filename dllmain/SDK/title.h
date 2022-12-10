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

struct ID_UNIT
{
	uint8_t be_flag_0;
	uint8_t unitNo_1;
	uint8_t classNo_2;
	uint8_t markNo_3;
	uint8_t type_4;
	uint8_t levelNo_5;
	uint8_t parentNo_6;
	uint8_t rowNo_7;
	Mtx mat_8;
	Mtx l_mat_38;
	ID_UNIT* pParent_68;
	ID_UNIT* pChild_6C;
	ID_UNIT* pPrev_70;
	ID_UNIT* pNext_74;
	uint8_t texId_78;
	uint8_t maskId_79;
	uint8_t texNo_7A;
	uint8_t maskNo_7B;
	uint8_t tex_ptn_no_7C;
	uint8_t mask_ptn_no_7D;
	int16_t pos_time_7E;
	int16_t size_time_80;
	int16_t col_time_82;
	int16_t rot_time_84;
	uint8_t pos_flag_86;
	uint8_t loop_flag_87;
	uint8_t size_flag_88;
	uint8_t rot_flag_89;
	uint8_t rev_flag_8A;
	uint8_t tex_flag_8B;
	uint8_t otType_8C;
	uint8_t otNo_8D;
	uint8_t trans_type_8E;
	uint8_t pow_8F;
	uint8_t blend_type_90;
	uint8_t anima_state_91;
	uint8_t pad_92[2];
	Vec pos0_94;
	Vec pos_A0;
	Vec ver_AC[4];
	float size0_W_DC;
	float size0_H_E0;
	float size_W_E4;
	float size_H_E8;
	uint8_t col0_R_EC;
	uint8_t col0_G_ED;
	uint8_t col0_B_EE;
	uint8_t col0_A_EF;
	uint8_t col1_R_F0;
	uint8_t col1_G_F1;
	uint8_t col1_B_F2;
	uint8_t col1_A_F3;
	float col_R_F4;
	float col_G_F8;
	float col_B_FC;
	float col_A_100;
	Vec rot0_104;
	Vec rot_110;
	float u0_11C;
	float u1_120;
	float v0_124;
	float v1_128;
	struct FUNC_PATH_PTR* pPath_12C;
	struct B_SPLINE_3_PTR* pBSpline_130;
	struct HERMITE_1_PTR* pScurve_134;
	struct HERMITE_1_PTR* pSizeCurve_138;
	struct HERMITE_1_PTR* pAlphaCurve_13C;
	struct HERMITE_1_PTR* pRotCurve_140;
};
assert_size(ID_UNIT, 0x144);

struct IDSystem
{
	int m_maxId_0;
	int m_nId_4;
	int m_levelMax_8;
	uint32_t m_set_flag_C[8];
	uint32_t m_disp_off_2C[8];
	ID_UNIT* m_pRoot_4C;
	ID_UNIT* m_IdUnit_50;
	void* ptr_54;
};
assert_size(IDSystem, 0x58);

enum ID_CLASS
{
	IDC_SSCRN_MAIN_MENU = 0x0,
	IDC_SSCRN_BACK_GROUND = 0x1,
	IDC_SSCRN_PESETA = 0x2,
	IDC_SSCRN_CONFIRM = 0x3,
	IDC_SSCRN_ETC = 0x4,
	IDC_SSCRN_NEAR_0 = 0x10,
	IDC_SSCRN_NEAR_1 = 0x11,
	IDC_SSCRN_NEAR_2 = 0x12,
	IDC_SSCRN_NEAR_3 = 0x13,
	IDC_SSCRN_0 = 0x14,
	IDC_SSCRN_1 = 0x15,
	IDC_SSCRN_2 = 0x16,
	IDC_SSCRN_3 = 0x17,
	IDC_SSCRN_FAR_0 = 0x18,
	IDC_SSCRN_FAR_1 = 0x19,
	IDC_SSCRN_FAR_2 = 0x1A,
	IDC_SSCRN_FAR_3 = 0x1B,
	IDC_SSCRN_CKPT_0 = 0x1C,
	IDC_SSCRN_CKPT_1 = 0x1D,
	IDC_SSCRN_CKPT_2 = 0x1E,
	IDC_SSCRN_CKPT_3 = 0x1F,
	IDC_ACT_BUTTON = 0x20,
	IDC_LIFE_METER = 0x21,
	IDC_GAUGE = 0x22,
	IDC_COUNT_DOWN = 0x23,
	IDC_BINOCULAR = 0x24,
	IDC_SCOPE = 0x25,
	IDC_EXAMINE = 0x26,
	IDC_DATA = 0x27,
	IDC_TITLE = 0x28,
	IDC_TITLE_MENU = 0x29,
	IDC_OPTION = 0x2A,
	IDC_OPTION_BG = 0x2B,
	IDC_EVENT = 0x2C,
	IDC_DEAD = 0x2D,
	IDC_CONTINUE = 0x2E,
	IDC_MSG_WINDOW = 0x2F,
	IDC_CINESCO = 0x30,
	IDC_WIP = 0x31,
	IDC_BLLT_ICON = 0x32,
	IDC_SUB_MISSION = 0x33,
	IDC_LASER_GAUGE = 0x34,
	IDC_CAMERA_BATTERY_MB = 0x35,
	IDC_NUM_00 = 0x40,
	IDC_NUM_61 = 0x7D,
	IDC_PRICE_00 = 0x80,
	IDC_PRICE_01 = 0x81,
	IDC_PRICE_02 = 0x82,
	IDC_PRICE_03 = 0x83,
	IDC_PRICE_04 = 0x84,
	IDC_TOOL = 0xFE,
	IDC_ANY = 0xFF,
};
