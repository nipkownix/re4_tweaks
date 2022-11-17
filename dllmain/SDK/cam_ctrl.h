#pragma once
#include "camera.h"

class CameraControl;
extern CameraControl* CamCtrl; // Game.cpp

class cModel;

struct CameraInterpolation
{
  CAMERA_POINT m_effect_0;
  float m_counter_20;
};
assert_size(CameraInterpolation, 0x24);

struct QFPS_OFFSET
{
  Vec m_campos_0[2];
  Vec m_target_18;
  float m_roll_24;
  float m_fovy_28;
};
assert_size(QFPS_OFFSET, 0x2C);

typedef QFPS_OFFSET QFPS_OFS_ARRAY[2][3];
assert_size(QFPS_OFS_ARRAY, 0x108);

class CameraQuasiFPS : public CAMERA
{
public:
  QFPS_OFS_ARRAY* m_p_ready_tbl_F8[15];
  QFPS_OFS_ARRAY* m_p_trans_134[6];
  QFPS_OFS_ARRAY* m_p_ready_array_14C;
  QFPS_OFS_ARRAY* m_p_trans_array_150;
  QFPS_OFFSET* m_p_offset_154;
  QFPS_OFFSET* m_p_blend_158;
  void* m_LR_info_15C;
  Vec m_pl_ofs_160;
  Vec m_pl_dir_16C;
  Mtx m_pl_mat_178;
  Vec* m_p_floor_norm_1A8;
  float m_zoom_ratio_1AC;
  float m_walk_ratio_1B0;
  uint8_t m_trans_type_1B4;
  uint8_t m_ready_type_1B5;
  uint8_t m_init_flag_1B6;
  uint8_t unk_1B7[1];
  Mtx m_pl_mat_bak_1B8;
  float m_blend_ratio_1E8;
  int m_blend_frame_1EC;
  int m_blend_count_1F0;
  Vec m_Aim_1F4;
  uint8_t m_site_200;
  float m_depression_ratio_204;
  float m_direction_ratio_208;
  float m_floor_ratio_20C;
  int16_t m_search_frame_210;
  int16_t m_search_cnt_212;
  uint32_t m_state_214;
};
assert_size(CameraQuasiFPS, 0x218);

struct AREA_DATA
{
  uint8_t Be_flag_0;
  int8_t No_1;
  int8_t Suffix_2;
  uint8_t Attr_3;
  float Dir_4;
  uint8_t Type_char_8;
  uint8_t Type_addr_9;
  uint8_t dummy_A[22];
  struct
  {
    float Height_20;
    float Y_24;
    int nVer_28;
    Vec* pVer_2C;
  };
};
assert_size(AREA_DATA, 0x30);

struct CAMERA_DATA
{
  uint8_t Be_flag_0;
  int8_t No_1;
  int8_t Id_2;
  uint8_t Attr_3;
  Vec offset_4;
  int16_t* pFrame_10;
  union
  {
    struct
    {
      unsigned char dummy_14[12];
    };
    Vec target_offset_14;
    float floor_ratio_14;
  };
  int nPoint_20;
  Vec* pCampos_24;
  Vec* pTarget_28;
  float* pRoll_2C;
  float* pFovy_30;
};
assert_size(CAMERA_DATA, 0x34);

struct CUT_INFO
{
  uint8_t Attr_0;
  uint8_t dummy_1[7];
  AREA_DATA* pAdat_8;
  CAMERA_DATA* pCdat_C;
};
assert_size(CUT_INFO, 0x10);

class CameraControl
{
public:
  uint8_t m_attached_cam_flag_old_0;
  uint8_t m_attach_cam_flag_1;
  uint8_t m_attach_num_2;
  uint8_t unk_3[1];
  ATTACH_CAMERA* m_p_attach_4[3];
  cModel* m_p_model_10[3];
  cModel* m_p_attach_model_old_1C;
  float m_scope_zoom_20;
  float m_scope_ang_x_24;
  uint32_t be_flag_28;
  uint32_t m_system_flag_2C;
  uint32_t m_state_flag_30;
  uint8_t r0_34;
  uint8_t r1_35;
  uint8_t r2_36;
  uint8_t r0_old_37;
  CAMERA_POINT m_camera_38;
  uint32_t m_timer_58;
  uint8_t* pCamData_5C;
  CAMERA Camera_60;
  CAMERA Camera_old_158;
  CAMERA* m_pExtraCamera_250;
  CameraInterpolation m_Inter_254;
  CameraQuasiFPS m_QuasiFPS_278;
  uint8_t m_Free[640];
  cCamera* m_pProc_710;
  char areaNo_714;
  char areaSuffix_715;
  char cameraNo_716;
  uint8_t m_cut_attr_717;
  CUT_INFO* m_pCut_718;
  int Battle_delay_71C;
  Vec Aim_720;
  Vec upcut_pos_72C;
  Vec upcut_ang_738;
  Vec upcut_scale_744;
  float m_behind_fovy_750;
  float m_side_play_754;
  float m_back_play_758;
  float m_ang_h_limit_75C;
  float m_ang_v_limit_760;
  int m_quick_cnt_764;
  float m_key_speed_768;
  float m_behind_A_ratio_76C;
  Vec campos_ofs_770;
  Vec target_ofs_77C;
};
assert_size(CameraControl, 0x788);
