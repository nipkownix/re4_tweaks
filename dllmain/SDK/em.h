#pragma once
#include "basic_types.h"
#include "model.h"
#include "cManager.h"

struct YARARE_INFO
{
	Vec offset_0;
	Vec cross_C;
	float radius_18;
	float height_1C;
	float extent_20;
	uint16_t flag_24;
	int16_t parts_no_26;
	float len_28;
	float c_dis_2C;
	YARARE_INFO* nextInfo_30;
};
assert_size(YARARE_INFO, 0x34);

class cDmgInfo
{
	// old version of cDmgInfo only had a single float member
	// unsure if this float is actually m_Dist or not, it might be m_Timer, since a lot of timer related stuff got changed to float during 60FPS change
	float m_Dist_mb_0;
	uint8_t m_Flag_4;
	uint8_t m_Timer_mb_5;
	uint8_t m_Wep_6;
	uint8_t m_Padding03_7;
	Vec m_PosFrom_8;
	float m_Dist_mb_14;
	YARARE_INFO* m_pDamageYarare_18;
};
assert_size(cDmgInfo, 0x1C);

class cEm : public cModel
{
public:
	enum class Routine0 : uint8_t
	{
		// r0 values that are used by pretty much all cEms
		// (other r* values are usually defined per-cEm though)
		Init,
		Move,
		Damage,
		Die
	};

	int16_t hp_324;
	int16_t hp_max_326;
	cDmgInfo m_DmgInfo_328;
	YARARE_INFO yarare_344;
	float l_pl_378;
	float l_sub_37C;
	void* pFile_380;
	void* pFile2_384;
	Vec target_offset0_388;
	char target_parts0_394;
	uint8_t set_395;
	uint8_t emunk_396[2];
	int(__cdecl* sce_func_398)(cEm*);
	uint8_t emunk_39C[4];
	uint8_t emListIndex_3A0;
	uint8_t emunk_field_3A1;
	uint8_t emunk_3A2[2];
	Vec motionMove2Vec_3A4;
	Vec Catch_at_adj_3B0;
	float Catch_dir_3BC;
	cEm* pEmCatch_3C0;
	uint8_t RckStat_3C4;
	int8_t RckMy_3C5;
	int8_t RckTo_3C6;
	int8_t RckNear_3C7;
	int8_t RckRno_3C8;
	int8_t RckPad0_3C9;
	uint8_t emunk_3CA[2];
	uint32_t m_StatusFlag_3CC;
	uint32_t flag_3D0;
	float Guard_r_3D4;
	uint8_t characterNum_3D8;
	uint8_t Item_eff_3D9;
	uint8_t emunk_3DA[4];
	uint16_t Item_id_3DE;
	uint16_t Item_num_3E0;
	uint16_t Item_flg_3E2;
	uint16_t Auto_item_flg_3E4;
	uint8_t emunk_3E6[2];
	uint32_t flags_3E8;
	Vec emunk_field_3EC;
	Vec target_offset1_3F8;
	int32_t target_parts1_404;

	virtual void setItem(uint16_t item_id, uint16_t num, uint16_t item_flg, uint16_t auto_item_flg, int8_t item_eff) = 0;
	virtual void setNoItem() = 0;
	virtual bool checkThrow() = 0;

	inline bool IsValid()
	{
		return (be_flag_4 & 0x601) != 0;
	}

	inline bool IsSpawnedByESL()
	{
		return IsValid() && emListIndex_3A0 != 255;
	}
};
assert_size(cEm, 0x408);

// NOTE: valid cEms can be anywhere from itemPtr[0] to itemPtr[maxItemCount], invalid/unused ones can also occur anywhere inside there
// use cEm::IsValid() to check for validity and skip over invalid cEms if needed
class cEmMgr : public cManager<cEm>
{
public:
	int count_valid() { int i = 0; for (auto& em : *this) if (em.IsValid()) i++; return i; }

	static std::string EmIdToName(int id);
};
assert_size(cEmMgr, 0x20);
