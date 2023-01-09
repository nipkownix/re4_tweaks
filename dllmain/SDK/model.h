#pragma once
#include "cCoord.h"
#include "atariInfo.h" // cAtariInfo
#include "camera.h" // ATTACH_CAMERA

#pragma pack(push, 1)
struct PEN_INFO
{
	Vec Offset_0;
	Vec Normal_C;
	float Length_l_18;
	float Length_r_1C;
	float Length_lu_20;
	float Length_ru_24;
	Vec Speed_28;
	Vec Pos_34;
	Vec Pos_old_40;
	Vec Fix_pos_4C;
	float Length_58;
	float Gravity_5C;
	float Fix_rate_60;
	float Max_rad_64;
	float Spd_rate_68;
	float Wind_rate_6C;
	uint8_t At_ck_70;
	uint8_t Flag_71;
};
assert_size(PEN_INFO, 0x72);
#pragma pack(pop)

struct PBOMB_INFO
{
	Vec Pos_0[5];
	Vec Speed_3C[5];
	uint16_t Flag_78;
	int16_t Delay_7A;
};
assert_size(PBOMB_INFO, 0x7C);

class cParts : public cCoord
{
public:
	Mtx hokan_mat_C4;
	cParts* nextParts_F4;
	Mtx lt_inv_mat_F8;
	union
	{
		struct
		{
			float length_128;
			Mtx ik_inv_mat_12C;
			Vec up_vector_15C;
			Vec ik_dir_168;
			Vec pos_bak_174;
			Vec ang_bak_180;
			Vec scale_bak_18C;
			float ang_x_bak_198;
			uint16_t key_hist_19C[3][3];
			uint16_t key_histB_1AE[3][3];
		};
		struct
		{
			Vec world_ang_128;
			uint8_t world_ang_padd_134[132];
		};
		struct
		{
			PEN_INFO Pen;
		};
		struct
		{
			uint32_t Kaboom_flg_128;
			Vec Kaboom_spd_12C;
			Vec Kaboom_ang_spd_138;
		};
		struct
		{
			PBOMB_INFO Bomb_128;
		};
	};
	uint32_t flag_1C0;
	uint8_t unk_1C4[20];
};
assert_size(cParts, 0x1D8); // TODO: check size

struct cModelData
{
	uint32_t offset_offset_0;
	uint16_t mdl_flag_num_4;
	uint16_t mdl_flag_6[3];
	uint32_t color_palette_offset_C;
	uint32_t uv_palette_offset_10;
	void* weight_palette_offset_14;
	uint8_t weight_palette_num_18;
	uint8_t nParts_19;
	uint16_t displaylist_num_1A;
	uint32_t displaylist_offset_1C;
	uint32_t flag_20;
	uint32_t nTex_24;
	uint8_t vertex_frac_28;
	uint8_t pad_29[1];
	uint16_t weight_palette_num_extended_2A;
	uint32_t shape_offset_2C; // offset to SHAPE_MOD_TBL minus 4
	float* vertex_palette_offset2_30;
	Vec* normal_palette_offset2_34;
	uint16_t vertex_num_38;
	uint16_t normal_num_3A;
	uint32_t version_3C;
	uint32_t* dbl_jnt_offset_40;
	uint32_t symmetry_offset_44;
	uint16_t* CalcSk1_x_offset_48;
	uint16_t* CalcSk1_x2_offset_4C;
};
assert_size(cModelData, 0x50);

struct cBoundingBox
{
	// TODO: figure out what first 0xC bytes are, they dont exist in PS2 symbols...
	uint8_t unk_0[0xC];
	Vec offset_C;
	float w_18;
	float h_1C;
	float d_20;
};
assert_size(cBoundingBox, 0x24);

struct SHAPE_WK
{
	float frame_0;
	uint32_t spd_addr_4;
};
assert_size(SHAPE_WK, 0x8);

struct TEXANM_INFO
{
	uint16_t flg_0;
	uint16_t blend_ratio_2;
	uint8_t anm_no_4;
	uint8_t blend_type_5;
	uint8_t pad1_6;
	uint8_t pad2_7;
	struct TEXBLEND_TBL* pBlendTbl_8;
	float scr_u_C;
	float scr_v_10;
	float scr_spd_u_14;
	float scr_spd_v_18;
	struct TEXANM_TBL* pAnmTbl_1C;
};
assert_size(TEXANM_INFO, 0x20);

class cModelInfo : public cUnit
{
public:
	cModelData* modelData_addr_C;
	uint8_t* tpl_addr_10;
	cModelInfo* pList_14;
	uint8_t cModelInfo_unk_18[32];
	cBoundingBox boundingBox_38;
	Mtx mat_5C;
	GXColor color_8C;
	uint8_t cModelInfo_field_90;
	uint8_t cModelInfo_unk_91[2];
	uint8_t cModelInfo_field_93;
	float* CalcSk1_x_data_94[3];
	float* CalcSk1_x2_data_A0[3];
	uint16_t* shape_addr_AC;
	SHAPE_WK shape_list_B0[5];
	uint32_t shape_status_D8_flags;
	float shape_frame_DC;
	uint8_t blendSetNum_mb_E0;
	uint8_t cModelInfo_unk_E1[3];
	float cModelInfo_field_E4;
	TEXANM_INFO texanm_info_E8;
	TEXANM_INFO bump_texanm_info_108;
	uint32_t add_tpl_num_128;
	uint8_t* add_tpl_addr_12C;
};
assert_size(cModelInfo, 0x130);

class cLightInfo
{
public:
	Mtx mat_0;
	class cLight* pLight_30[8];
	uint8_t EnableMask_50;
	uint8_t Flag_51;
	int8_t PartsNo_52;
	uint8_t dummy03_53;
	uint32_t SelectMask_54;
	Vec Offset_58;
	Vec Size_64;
	float Radius_70;
};
assert_size(cLightInfo, 0x74);

struct MOTION_INFO
{
  int32_t fcvDataPtr_0;
  uint32_t *pHermite_data_4;
  uint16_t Key_hist[4][3];
  float Mot_frame_max_20;
  float Mot_frame_24;
  float Mot_frame_sav_28;
  float Mot_frame_old_2C;
  uint8_t Joint_num_30;
  uint8_t *pJoint_no_34;
  struct FCV_DESC *pJoint_kind_38;
  uint16_t Null_pos_3C;
  uint16_t Null_rot_3E;
  uint16_t Mot_attr_40;
  uint16_t Mot_state_42;
  uint32_t Mot_flag_44;
  Vec Pos_move_old_48;
  Vec Ang_54;
  Vec Ang_old_60;
  Vec Ang_dist_6C;
  Vec Pos_78;
  Vec Pos_old_84;
  Vec Pos_dist_90;
  Vec Pos_world_9C;
  struct SEQUENCE_DATA *pSeq_top_A8;
  int32_t Seq_AC;
  int32_t Seq_old_B0;
  int32_t Seq_old2_B4;
  float Seq_frame_B8;
  uint16_t Seq_frame_num_BC;
  float Seq_speed_C0;
  uint8_t Hokan_frame_C4;
  uint8_t Hokan_cnt_C5;
  uint8_t unk_C6[2];
  float Brate_C8;
  ATTACH_CAMERA *attachCamera_CC;
};
assert_size(MOTION_INFO, 0xD0);

struct FOOTSHADOW_DATA
{
	uint8_t joint_0;
	uint8_t div_1;
	uint8_t flag_2;
	uint8_t color_3;
	float size_4;
};
assert_size(FOOTSHADOW_DATA, 0x8);

struct FOOTSHADOW_TBL
{
	uint32_t nTbl_0;
	FOOTSHADOW_DATA* tbl_4;
};
assert_size(FOOTSHADOW_TBL, 0x8);

struct cModelState
{
	uint32_t m_Flag_0;
	uint32_t m_LightFlag_4;
	uint32_t m_LightNo_8;
	float m_LightPow_C;
};
assert_size(cModelState, 0x10);

class cModel : public cCoord
{
public:
	Mtx mtx_C4;
	cParts* childParts_F4;
	uint32_t guid_F8;
	uint8_t r_no_0_FC;
	uint8_t r_no_1_FD;
	uint8_t r_no_2_FE;
	uint8_t r_no_3_FF;
	uint8_t id_100;
	uint8_t type_101;
	uint8_t nParts_102;
	uint8_t alpha_omit_103;
	Vec speed_104;
	Vec pos_old_110;
	Vec Wall_norm_11C;
	Vec* pFloor_norm_128;
	uint8_t z_mode_12C;
	uint8_t TevScaleGroup_12D;
	uint8_t kindid_12E;
	uint8_t ot_type_12F;
	cModel* pChildShadowModel_130;
	uint8_t Shd_color_134;
	uint8_t CullMode_135;
	uint8_t Shader_type_136;
	uint8_t Refract_pow_137;
	uint8_t Refract_ratio_138;
	uint8_t AddAmb_r_139;
	uint8_t AddAmb_g_13A;
	uint8_t AddAmb_b_13B;
	uint32_t Fix_parts_13C;
	Vec Fix_pos_140;
	uint8_t invisible_trg_14C;
	uint8_t invisible_old_14D;
	uint8_t invisible_mode_14E;
	uint8_t invisible_busy_14F;
	int32_t invisible_timer_150;
	float invisible_factor_154;
	float invisible_factor2_158;
	cModelInfo* pModelInfo_15C;
	cModelInfo* pShadowModelInfo_160;
	cLightInfo LightInfo_164;
	MOTION_INFO Motion_1D8;
	MOTION_INFO* pMotionB_2A8;
	int16_t* pXFlip_2AC;
	uint32_t* pDblJnt_2B0;
	cAtariInfo atari_2B4;
	Vec* inscreen_pos_300;
	uint32_t pPath_304;
	FOOTSHADOW_TBL* pFsdTbl_308;
	cModelState State_30C;
	int32_t field_31C;
	uint8_t field_320;
	uint8_t field_321;
	uint8_t field_322;
	uint8_t field_323;

	virtual void move() = 0;
	virtual void setNoSuspend(BOOL onoff) = 0;

	int PartCount()
	{
		int i = 0;
		cParts* part = childParts_F4;
		while (part != nullptr)
		{
			i++;
			part = part->nextParts_F4;
		}
		return i;
	}
};
assert_size(cModel, 0x324);
