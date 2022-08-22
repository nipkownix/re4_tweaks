#pragma once
#include "basic_types.h"

class cItem
{
public:
	ITEM_ID id_0;
	uint16_t num_2;
	uint8_t be_flag_4;
	uint8_t chr_5;
	uint16_t weapon_6[2];
	int8_t pos_x_A;
	int8_t pos_y_B;
	int8_t orientation_C;
	uint8_t onboard_D;
};

struct ITEM_ORDER
{
	cItem* p_item_0;
	uint16_t num_4;
};

class cItemMgr;

typedef cItem* (__fastcall* cItemMgr__search_Fn)(cItemMgr* thisptr, void* unused, ITEM_ID id);
typedef bool(__fastcall* cItemMgr__arm_Fn)(cItemMgr* thisptr, void* unused, cItem* pItem);
extern cItemMgr__search_Fn cItemMgr__search;
extern cItemMgr__arm_Fn cItemMgr__arm;

class cItemMgr
{
public:
	uint32_t* m_pAvailable_0;
	int m_flag_num_4;
	ITEM_ID used_id_8;
	uint8_t unk_A[2];
	cItem* m_pWep_C;
	ITEM_ID m_wep_id_10;
	uint8_t m_to_whom_12;
	uint8_t m_char_13;
	cItem* m_pItem_14;
	cItem* m_pNew_18;
	int m_array_num_1C;
	ITEM_ORDER* m_p_order_tbl_20;
	int m_order_tbl_num_24;
	int m_bonus_time_28;
	int m_bonus_point_2C;

	inline cItem* search(ITEM_ID id)
	{
		return cItemMgr__search(this, nullptr, id);
	}

	inline bool arm(cItem* pItem)
	{
		return cItemMgr__arm(this, nullptr, pItem);
	}
};

extern cItemMgr* ItemMgr;
