#pragma once
#include "basic_types.h"

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

enum ID_BE_FLAG
{
	ID_BE_FLAG_VISIBLE = 0x8
};

class IDSystem;
struct ID_FILE_HEADER;
enum ID_CLASS;
typedef void(__thiscall* IDSystem__set_Fn)(IDSystem* thisptr, ID_FILE_HEADER* pData, uint8_t markNo, ID_CLASS classNo, uint8_t otType, uint8_t otNo, uint32_t Attr);
typedef ID_UNIT* (__thiscall* IDSystem__unitPtr_Fn)(IDSystem* thisptr, uint8_t markNo, ID_CLASS classNo);
typedef void (__thiscall* IDSystem__kill_Fn)(IDSystem* thisptr, uint8_t markNo, ID_CLASS classNo);
typedef void(__thiscall* IDSystem__setTime_Fn)(IDSystem* thisptr, ID_UNIT* pParent, __int16 time);
extern IDSystem__set_Fn IDSystem__set;
extern IDSystem__unitPtr_Fn IDSystem__unitPtr;
extern IDSystem__kill_Fn IDSystem__kill;
extern IDSystem__setTime_Fn IDSystem__setTime;

class IDSystem
{
public:
	int m_maxId_0;
	int m_nId_4;
	int m_levelMax_8;
	uint32_t m_set_flag_C[8];
	uint32_t m_disp_off_2C[8];
	ID_UNIT* m_pRoot_4C;
	ID_UNIT* m_IdUnit_50;
	void* ptr_54;

	inline void set(ID_FILE_HEADER* pData, uint8_t markNo, ID_CLASS classNo, uint8_t otType, uint8_t otNo, uint32_t Attr)
	{
		return IDSystem__set(this, pData, markNo, classNo, otType, otNo, Attr);
	}

	inline ID_UNIT* unitPtr(uint8_t markNo, ID_CLASS classNo)
	{
		return IDSystem__unitPtr(this, markNo, classNo);
	}

	inline void kill(uint8_t markNo, ID_CLASS classNo)
	{
		return IDSystem__kill(this, markNo, classNo);
	}

	inline void setTime(ID_UNIT* pParent, __int16 time)
	{
		return IDSystem__setTime(this, pParent, time);
	}
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
	IDC_GAUGE = 0x22, // mercs score timer
	IDC_COUNT_DOWN = 0x23,
	IDC_BINOCULAR = 0x24,
	IDC_SCOPE = 0x25,
	IDC_EXAMINE = 0x26,
	IDC_DATA = 0x27,
	IDC_TITLE = 0x28,
	IDC_TITLE_MENU_0 = 0x29,
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
	IDC_LASER_GAUGE = 0x34, // PRL gauge
	IDC_CAMERA_BATTERY_MB = 0x35,
	IDC_NUM_00 = 0x40,
	IDC_UNK_41 = 0x41,
	IDC_UNK_47 = 0x47,
	IDC_NUM_61 = 0x7D,
	IDC_PRICE_00 = 0x80,
	IDC_PRICE_01 = 0x81,
	IDC_PRICE_02 = 0x82,
	IDC_PRICE_03 = 0x83,
	IDC_PRICE_04 = 0x84,
	IDC_TITLE_MENU_1 = 0x99,
	IDC_UNK_A0 = 0xA0,
	IDC_UNK_AA = 0xAA,
	IDC_TOOL = 0xFE,
	IDC_ANY = 0xFF,
};

struct IDApplication { struct IDApplication_vtbl* __vftable /*VFT*/; };
assert_size(IDApplication, 0x4);

struct IdScope : IDApplication
{
	uint32_t m_pos_time_sav_4;
	uint32_t m_size_time_sav_8;
	bool m_zoom_disp_C;
};
assert_size(IdScope, 0x10);

struct MercID
{
	uint32_t _eff_0;
	uint32_t _uwf_4[3];
	uint32_t* _addr_10;
	IDSystem _idSys;
};
assert_size(MercID, 0x6C);

extern MercID* MercID_ptr;

struct ID_FILE_HEADER
{
	char Version_0[4];
	uint8_t GroupNo_4;
	uint8_t UnitNum_5;
	uint8_t dummy_6[2];
};
assert_size(ID_FILE_HEADER, 8);

struct ID_DATA_V1
{
	uint32_t be_flag_0;
	uint8_t markNo_4;
	uint8_t unitNo_5;
	uint8_t levelNo_6;
	uint8_t parentNo_7;
	uint8_t rowNo_8;
	uint8_t type_9;
	uint8_t Id_A;
	uint8_t texId_B;
	uint8_t pos_flag_C;
	uint8_t loop_flag_D;
	uint8_t size_flag_E;
	uint8_t rot_flag_F;
	uint8_t rev_flag_10;
	Vec pos0_14;
	Vec ver_20[4];
	float size0_W_50;
	float size0_H_54;
	uint8_t col0_R_58;
	uint8_t col0_G_59;
	uint8_t col0_B_5A;
	uint8_t col0_A_5B;
	Vec rot0_5C;
	uint8_t blend_type_68;
	uint8_t trans_type_69;
	uint8_t maskId_6A;
	uint8_t tex_flag_6B;
	uint8_t pow_6C;
	FUNC_PATH_PTR* pPath_70;
	B_SPLINE_3_PTR* pBSpline_74;
	HERMITE_1_PTR* pScurve_78;
	HERMITE_1_PTR* pSizeCurve_7C;
	HERMITE_1_PTR* pAlphaCurve_80;
	HERMITE_1_PTR* pRotCurve_84;
};
assert_size(ID_DATA_V1, 0x88);

struct ID_DATA_V2
{
	uint32_t be_flag_0;
	uint8_t markNo_4;
	uint8_t unitNo_5;
	uint8_t levelNo_6;
	uint8_t parentNo_7;
	uint8_t rowNo_8;
	uint8_t type_9;
	uint8_t Id_A;
	uint8_t texId_B;
	uint8_t pos_flag_C;
	uint8_t loop_flag_D;
	uint8_t size_flag_E;
	uint8_t rot_flag_F;
	uint8_t rev_flag_10;
	Vec pos0_14;
	Vec ver_20[4];
	float size0_W_50;
	float size0_H_54;
	uint8_t col0_R_58;
	uint8_t col0_G_59;
	uint8_t col0_B_5A;
	uint8_t col0_A_5B;
	uint8_t col1_R_5C;
	uint8_t col1_G_5D;
	uint8_t col1_B_5E;
	uint8_t col1_A_5F;
	Vec rot0_60;
	uint8_t blend_type_6C;
	uint8_t trans_type_6D;
	uint8_t maskId_6E;
	uint8_t tex_flag_6F;
	uint8_t pow_70;
	FUNC_PATH_PTR* pPath_74;
	B_SPLINE_3_PTR* pBSpline_78;
	HERMITE_1_PTR* pScurve_7C;
	HERMITE_1_PTR* pSizeCurve_80;
	HERMITE_1_PTR* pAlphaCurve_84;
	HERMITE_1_PTR* pRotCurve_88;
};
assert_size(ID_DATA_V2, 0x8C);