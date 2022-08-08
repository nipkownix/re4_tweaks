#pragma once
#include "basic_types.h"
#include "objWep.h"

class cPlWep
{
public:
	uint8_t m_EmRank[32];
	uint8_t m_EmRankPtr_20;
	uint8_t m_ShotCancelCtr_21;
	uint8_t m_dummy12_22;
	uint8_t m_ShotTimer_23;
	uint8_t m_WepUd_24;
	uint8_t m_WepUdRno_25;
	uint8_t m_Flag_26;
	uint8_t m_Dummy13_27;
	float m_CenterX_28;
	float m_CenterY_2C;
	float m_CamAdjY_30;
	cObjWep* m_pWep_34;
	cObjWep* m_pWepHand_38;
	uint32_t m_OldWeaponAddr_3C;
	uint8_t m_LockTime_40;
	uint8_t unk_41[1];
	char field_42;
	uint8_t field_43;
	Vec field_44;
};
assert_size(cPlWep, 0x50);
