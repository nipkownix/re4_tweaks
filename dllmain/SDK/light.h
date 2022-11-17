#pragma once
#include "basic_types.h"
#include "cManager.h"
#include "model.h"

struct cPenWind
{
	int8_t direction_0;
	uint8_t power_1;
	uint8_t frequency_2;
	uint8_t dummy03_3;
};
assert_size(cPenWind, 4);

struct cLightTune
{
	uint8_t m_Flag_0;
	uint8_t m_Dummy01_1;
	uint8_t m_Dummy02_2;
	uint8_t m_Dummy03_3;
	GXColor m_LitCol_4;
	GXColor m_AmbCol_8;
	GXColor m_EspCol_C;
};
assert_size(cLightTune, 0x10);

struct FOG
{
	GXFogType Type_0;
	float Start_4;
	float End_8;
	GXColor Color_C;
};
assert_size(FOG, 0x10);

// cLightMgr::GetEnvPtr returns ptr to this, mostly affects Filter01 "LeLit" params
struct cLightEnv
{
	GXColor AmbientScr_0;
	uint32_t nLight_4;
	FOG Fog_8;
	FOG MirrorFog_18;
	int32_t FocusZ_28;
	uint8_t FocusFlag_2C;
	uint8_t FocusLevel_2D;
	uint8_t FocusMode_2E;
	uint8_t blur_rate_2F;
	cLightTune Tune_30;
	uint8_t model_tev_scale_40;
	uint8_t player_tev_scale_41;
	uint8_t unk_tev_scale_42[2];
	float fog_play_ratio_44;
	uint8_t Hokan;
	uint8_t dummy01;
	uint8_t dummy02;
	uint8_t dummy03;
	uint32_t dummy00[40];
	cPenWind wind_EC;
	int8_t blur_type_F0;
	uint8_t blur_power_F1;
	uint8_t min_lod_F2;
	uint8_t max_lod_F3;
	GXAnisotropy aniso_F4;
	uint8_t contrast_level_F5;
	uint8_t contrast_pow_F6;
	uint8_t contrast_bias_F7;
	float lod_bias_F8;
	GXColor AmbientEm_FC;
	GXColor AmbientEsp_100;
};
assert_size(cLightEnv, 0x104);

class cLight : public cUnit
{
public:
	uint8_t enable_C;
	uint8_t Type_D;
	uint8_t Id_E;
	uint8_t EnableMask_F;
	Vec Pos_10;
	float Radius_1C;
	GXColor Col_20;
	float Intensity_24;
	uint8_t ParentType_28;
	uint8_t Kind_29;
	uint8_t Attribute_2A;
	uint8_t Priority_2B;
	uint32_t ParentNo_2C;
	uint16_t HitRadius_30;
	uint16_t Dummy82_32;
	uint32_t Dummy9_34;
	Vec TypeFree_38;
	uint8_t unk_44[52];
	uint8_t MoveFree_78[128];
	uint8_t unk_F8[64];
	uint8_t Rno0_138;
	uint8_t Rno1_139;
	uint8_t Rno2_13A;
	uint8_t Rno3_13B;
	GXColor DispCol_13C;
	uint16_t LitIndex_140;
	uint16_t Dummy102_142;
	Vec World_144;
	cModel* pParent_150;
};
assert_size(cLight, 0x154);

class cLightMgr;
typedef BOOL(__fastcall* cLightMgr__setEnv_Fn)(cLightMgr* thisptr, void* unused, cLightEnv* pLe, int8_t hokan);
extern cLightMgr__setEnv_Fn cLightMgr__setEnv;

extern cLightMgr* LightMgr; // Game.cpp

class cLightMgr : public cManager<cLight>
{
public:
	struct cLit* pLitHeader_1C;
	cLightEnv LightEnv_20;
	uint32_t LightEnableFlag_128[8];
	uint32_t LightEnableBak_148[8];
	struct cLightPathHeader* pLitPath_164;
	void* m_pLitCore_168;
	void* m_pLitRoom_16C;
	void* m_pLitRoom2_170;
	int m_oldCutNo_174;
	uint8_t m_Hokan_178;
	uint8_t m_logMode_179;
	uint8_t m_Dummy02_17A;
	uint8_t m_Dummy03_17B;
	float ElecPower_17C;
	cLightTune m_Tune_180;
	float m_ColBrendRate_190;
	uint32_t dbFlag_194;
	struct cLit* dbMem_198;
	cLight* LightArrayStack_19C[10];
	uint32_t LightNumStack_1C4[10];

	inline BOOL setEnv(cLightEnv* pLe, int8_t hokan)
	{
		return cLightMgr__setEnv(this, nullptr, pLe, hokan);
	}
};
assert_size(cLightMgr, 0x1EC);
