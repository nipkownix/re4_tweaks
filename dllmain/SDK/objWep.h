#pragma once
#include "basic_types.h"
#include "obj.h"
#include "em.h"

class cObjWep : public cObj
{
	enum class FLAG : uint8_t
	{
		F_ON_LASER_SIGHT_VAL = 0x1,
		F_ON_LASER_SIGHT_D_VAL = 0x2,
		F_DISP_0_VAL = 0x4,
		F_DISP_1_VAL = 0x8,
		F_DISP_2_VAL = 0x10,
	};

public:
	uint16_t* motionDataBackup_32C;
	uint16_t* motionData_330;
	float bureX_334;
	float bureY_338;
	float bureSpeedX_33C;
	float bureSpeedY_340;
	uint8_t shotFrame_344[4];
	uint32_t m_EraseTime_348;
	cModel* m_pParent_34C;
	ITEM_ID itemId_350;
	uint8_t r_no_0_352;
	uint8_t r_no_1_353;
	cObjWep::FLAG flag_354;
	uint8_t etcflag_355;
	uint8_t m_EtcTimer_356;
	uint8_t cObjWep_unk_357[1];
	uint32_t m_StopSeId_358;
	Vec m_ShotPos_35C;
	cEm* m_SightEm_368;
	int32_t SndIdStr_36C;
	cObjWep* bowPtr_370;
	Vec cObjWep_field_374;
	Vec cObjWep_field_380;
	uint8_t cObjWep_unk_38C[992];

	virtual void moveAll() = 0;
	virtual void moveStay() = 0;
	virtual void moveReady() = 0;
	virtual void moveSet() = 0;
	virtual void moveFire() = 0;
	virtual void moveDown() = 0;
	virtual void moveReload() = 0;
	virtual void moveDrop() = 0;
	virtual void init(cModel* cmdl) = 0;
	virtual void setMotion(cEm* cpl) = 0;
	virtual void setHand() = 0;
	virtual void cObjWep__interrupt() = 0;
	virtual void endReload(int val) = 0;
	virtual bool reloadable() = 0;
	virtual void keyKamae() = 0;
	virtual void fire() = 0;
	virtual void beginReload() = 0;
};
assert_size(cObjWep, 0x76C);

namespace bio4
{
	extern void(__cdecl* WeaponChange)(); // TODO: maybe should be in a different header...
};
