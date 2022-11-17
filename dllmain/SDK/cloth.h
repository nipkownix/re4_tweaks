#pragma once
#include "em.h"

struct CLOTH_AT_SET
{
	uint16_t Type_0;
	uint8_t P1_2;
	uint8_t P2_3;
	float Weight_4;
	float R_8;
	Vec Ofs1_C;
	Vec Ofs2_18;
};
assert_size(CLOTH_AT_SET, 0x24);

struct CLOTH_INFO
{
	uint32_t Num_0;
	uint8_t* pCloth_4;
	uint8_t* pLeft_8;
	uint8_t* pRight_C;
	uint8_t* pUpLeft_10;
	uint8_t* pUpRight_14;
	uint8_t* pParent_18;
	uint8_t* pChild_1C;
	float* pGravity_20;
	float* pRate_24;
	float* pMax_28;
	float* pWindSin_2C;
	float* pWindRate_30;
	CLOTH_AT_SET* pAtset_34;
	uint32_t At_num_38;
	float Gravity_3C;
	float Rate_40;
	uint32_t Bundle_num_44;
	float WindSin_48;
	float Stretchy_4C;
	float Move_rate_50;
	cParts** pPtbl_54;
	cEm* pEm_at_58;
	uint32_t Flag_5C;
};
assert_size(CLOTH_INFO, 0x60);
