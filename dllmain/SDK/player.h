#pragma once
#include "basic_types.h"
#include "em.h"
#include "obj.h"
#include "cMotBase.h"
#include "pl_wep.h"

enum LASER_TYPE
{
	LASER_LOW = 0x0,
	LASER_HIGH = 0x1,
};

class cPlayer;
typedef BOOL(__fastcall* cPlayer__subScrCheck_Fn)(cPlayer* thisptr, void* unused);
extern cPlayer__subScrCheck_Fn cPlayer__subScrCheck;

class cPlayer : public cEm
{
public:
	float m_Work0_408;
	float m_Work1_40C;
	float plunk_deltaTimeRelated_410;
	float plunk_field_414;
	float m_Work4_418;
	float plunk_field_41C;
	float plunk_field_420;
	float plunk_field_424;
	int32_t m_Work0_428;
	int32_t m_Work1_42C;
	int32_t m_Work2_430;
	int32_t m_Work3_434;
	int32_t plunk_438;
	float m_Work5_43C;
	int32_t m_Work6_440;
	int32_t m_Work7_444;
	float m_Fwork0_448;
	Vec m_VecWork0_44C;
	Vec m_VecWork1_458;
	uint32_t m_Flag_464;
	uint32_t stat_468;
	uint32_t* m_MotTbl_46C;
	uint32_t* m_MotTbl2_470;
	MOTION_INFO m_SubMot_474;
	float m_Frame_544;
	uint8_t plunk_field_548;
	uint8_t m_Hokan_549;
	uint8_t m_BbtnCnt_54A;
	float m_CmdTimer_54C;
	float m_Blend_550;
	int32_t m_SeId_554;
	cEm* m_pEm_558;
	cEm* m_pBoat_55C;
	cObj* m_pSpear_560;
	float m_BoatPlDir_564;
	uint32_t m_GachaCtr_568;
	uint8_t m_SplashCtr_56C;
	uint8_t m_OCMode_56D;
	uint8_t m_EyeMode_56E;
	uint8_t m_BinoRno_56F;
	uint8_t m_ConDmFlag_570;
	uint8_t m_ConDm_Dummy_571;
	uint16_t m_ConDmTimer_572;
	Vec m_PosOldWater_574;
	YARARE_INFO m_Yarare_580[10];
	uint32_t m_pSatMask_788;
	void* pFuncAux_78C;
	struct PlEffRoom* m_pEffRoom_790;
	cEm* m_pBoss_794;
	uint32_t m_pBossRmf_798;
	Vec m_FallVec_79C;
	Vec m_JumpVec_7A8;
	float m_JumpAdjY_7B4;
	Vec m_ActCross_7B8;
	Vec m_ActNorm_7C4;
	uint32_t m_ActAttr_7D0;
	class cPlAlert* Alert_7D4;
	cPlWep* Wep_7D8;
	class cPlNeck* Neck_7DC;
	class cPlWaist* Waist_7E0;
	class cPlBody* Body_7E4;
	class cMental* Mental_7E8;
	class cPlForm* Form_7EC;
	class cPlPush* Push_7F0;
	cMotBase* MotBase_7F4;
	Vec pos_bak_7F8;
	LASER_TYPE laser_type_804;
	uint8_t plunk_field_808;
	float m_invisi_rate_80C;
	uint32_t pc_func_810;

	BOOL subScrCheck()
	{
		return cPlayer__subScrCheck(this, nullptr);
	}
};
assert_size(cPlayer, 0x814);

enum LVADD
{
	LVADD_UPDATE = 0,
	LVADD_DIE = 1,
	LVADD_PL_DAMAGE = 2,
	LVADD_PL_BIG_DAMAGE = 3,
	LVADD_MISS_HANDGUN = 4,
	LVADD_MISS_SHOTGUN = 5,
	LVADD_MISS_GRENADE = 6,
	LVADD_MISS_SNIPER = 7,
	LVADD_MISS_MACHINEGUN = 8,
	LVADD_CRITICALHIT = 9,
	LVADD_RECOVERY = 0xA,
	LVADD_ESCAPEATTACK = 0xB,
	LVADD_EM_DAMAGE = 0xC,
	LVADD_EM_DIE = 0xD,
	LVADD_TIMECOUNT = 0xE,
	LVADD_NUM = 0xF
};
