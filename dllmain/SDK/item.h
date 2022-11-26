#pragma once
#include "basic_types.h"

// enum names from RE4VR, unverified...
// internal names are used for EItemId enum, see GameFlags.cpp EItemId_Names for friendly in-game names
enum class EItemId
{
	Bullet_45in_H = 0,
	Grenade = 1,
	Flame_Grenade = 2,
	VP70 = 3,
	Bullet_9mm_H = 4,
	Spray = 5,
	Herb_G = 6,
	Bullet_223in = 7,
	Hen_Egg = 8,
	Iodine_Egg = 9,
	Golden_Egg = 10,
	Luis_Drug = 11,
	Parasite_Sample = 12,
	Krauser_Knife = 13,
	Light_Grenade = 14,
	Salazar_Crest = 15,
	Bow_Gun = 16,
	Bullet_Bow_Gun = 17,
	Herb_G_G = 18,
	Herb_G_G_G = 19,
	Herb_G_R = 20,
	Herb_G_R_Y = 21,
	Herb_G_Y = 22,
	Ada_RPG = 23,
	Bullet_12gg = 24,
	Herb_R = 25,
	Bullet_5in = 26,
	Key_Sand_Clock = 27,
	Herb_Y = 28,
	Piece_Of_Slate = 29,
	Golden_Gem = 30,
	Silver_Gem = 31,
	Bullet_9mm_M = 32,
	FN57 = 33,
	Scope_PRL_412 = 34, // "Scope_New_Weapon" in re4vr
	Ruger = 35,
	Ruger_SA = 36,
	Mauser = 37,
	Mauser_ST = 38,
	XD9 = 39,
	New_Weapon_SC = 40, // Missing piece_info data, can't be spawned...
	Civilian = 41,
	Gov = 42,
	Ada_New_Weapon = 43, // Missing piece_info data, can't be spawned...
	Shotgun = 44,
	Striker = 45,
	S_Field = 46,
	HK_Sniper = 47,
	Styer = 48,
	R513_Key_0a = 49,
	Styer_St = 50,
	R513_Key_0b = 51,
	Thompson = 52,
	RPG7 = 53,
	Mine = 54,
	SW500 = 55,
	Knife = 56,
	Cupric_Gem = 57,
	Moon_Spall_1 = 58,
	Cult_Key = 59,
	Cult_Crest = 60,
	False_Eye = 61,
	Krauser_Machine_Gun = 62,
	Silencer_9mm = 63,
	Punisher = 64, // "Item_40" in re4vr
	PRL_412 = 65, // "New_Weapon" in re4vr
	Stock_Mauser = 66,
	Stock_Styer = 67,
	Scope_Sniper = 68,
	Scope_HK_Sniper = 69,
	Bullet_Mine_A = 70,
	Ada_Shot_Gun = 71,
	File_13 = 72,
	File_14 = 73,
	File_15 = 74,
	File_16 = 75,
	File_17 = 76,
	File_18 = 77,
	File_19 = 78,
	File_20 = 79,
	File_21 = 80,
	HK_Sniper_Thermo = 81,
	Krauser_Bow = 82,
	Ada_Machine_Gun = 83,
	Treasure_Map_2 = 84,
	Treasure_Map_3 = 85,
	Ore_White = 86,
	Ore_Black = 87,
	Pearl_Pendant = 88,
	Brass_Fob_Watch = 89,
	Silver_Whistle = 90,
	Gold_Dish = 91,
	Platinum_Cup = 92,
	Crystal_Mask = 93,
	Beer_Stein = 94,
	Agate_Green = 95,
	Garnett_Red = 96,
	Amber_Yellow = 97,
	Beer_Stein_G = 98,
	Beer_Stein_R = 99,
	Beer_Stein_Y = 100,
	Beer_Stein_G_R = 101,
	Beer_Stein_G_Y = 102,
	Beer_Stein_R_Y = 103,
	Beer_Stein_G_R_Y = 104,
	Moon_Spall_2 = 105,
	Bullet_45in_M = 106,
	S_Field_Sc = 107,
	HK_Sniper_Sc = 108,
	Omake_RPG = 109,
	Pagan_Grail_1 = 110,
	Pagan_Grail_2 = 111,
	Pagan_Rod = 112,
	Ingot_Bar = 113,
	Bullet_Arrow = 114,
	Time_Bonus = 115,
	R327_Card_Key = 116,
	Point_Bonus = 117,
	Key_Green_Gem = 118,
	Ruby = 119,
	Gold_Box_S_Std = 120,
	Gold_Box_L_Std = 121,
	Moon_Crest = 122,
	Seal_Key = 123,
	Attache_Case_S = 124,
	Attache_Case_M = 125,
	Attache_Case_L = 126,
	Attache_Case_O = 127,
	Golden_Sword = 128,
	Key_To_R50b = 129,
	Dragon_Dream = 130,
	Key_To_R30c = 131,
	Key_To_R308 = 132,
	Crest_A = 133,
	Crest_B = 134,
	Crest_C = 135,
	Key_To_Jet_Ski = 136,
	Smelly_Pendant = 137,
	Smelly_Fob_Watch = 138,
	Key_To_Shrine = 139,
	Key_To_Barrier = 140,
	Dynamite = 141,
	Key_To_Control_Room = 142,
	Gold_Bracelet = 143,
	Perfume_Bottle = 144,
	Pearl_Ruby_Mirror = 145,
	Key_To_R30b = 146,
	Plush_Chessboard = 147,
	Riot_Gun = 148,
	Black_Bass = 149,
	Sand_Clock = 150,
	Black_Bass_L = 151,
	Evil_Gem = 152,
	S_Field_Thermo = 153,
	Pot_Holed_Crown = 154,
	Kingdom_Heart = 155,
	Royal_Crest = 156,
	Crown_Heart = 157,
	Crown_Crest = 158,
	Crown_Heart_Crest = 159,
	Item_A0 = 160,
	Spinel_02 = 161,
	Pedestal = 162,
	Key_To_Salon = 163,
	Crest_Right = 164,
	Crest_Left = 165,
	Crest_Full = 166,
	Key_To_201 = 167,
	Herb_R_Y = 168,
	Treasure_Map = 169,
	Scope_Mine = 170,
	Mine_SC = 171,
	File_01 = 172,
	File_02 = 173,
	File_03 = 174,
	File_04 = 175,
	File_05 = 176,
	File_06 = 177,
	File_07 = 178,
	File_08 = 179,
	File_09 = 180,
	File_10 = 181,
	File_11 = 182,
	File_12 = 183,
	Lantern = 184,
	Lantern_Stone_G = 185,
	Lantern_Stone_R = 186,
	Lantern_Stone_Y = 187,
	Lantern_G = 188,
	Lantern_R = 189,
	Lantern_Y = 190,
	Lantern_G_R = 191,
	Lantern_G_Y = 192,
	Lantern_R_Y = 193,
	Lantern_G_R_Y = 194,
	Dungeon_Key = 195,
	Silver_Sword = 196,
	Scope_Thermo = 197,
	Mask = 198,
	Mask_Stone_G = 199,
	Mask_Stone_R = 200,
	Mask_Stone_Y = 201,
	Mask_G = 202,
	Mask_R = 203,
	Mask_Y = 204,
	Mask_G_R = 205,
	Mask_G_Y = 206,
	Mask_R_Y = 207,
	Mask_G_R_Y = 208,
	Cat_Statue = 209,
	Cat_Stone_G = 210,
	Cat_Stone_R = 211,
	Cat_Stone_Y = 212,
	Cat_Statue_G = 213,
	Cat_Statue_R = 214,
	Cat_Statue_Y = 215,
	Cat_Statue_G_R = 216,
	Cat_Statue_G_Y = 217,
	Cat_Statue_R_Y = 218,
	Cat_Statue_G_R_Y = 219,
	Bottle_Cap_01 = 220,
	Bottle_Cap_02 = 221,
	Bottle_Cap_03 = 222,
	Bottle_Cap_04 = 223,
	Bottle_Cap_05 = 224,
	Bottle_Cap_06 = 225,
	Bottle_Cap_07 = 226,
	Bottle_Cap_08 = 227,
	Bottle_Cap_09 = 228,
	Bottle_Cap_10 = 229,
	Bottle_Cap_11 = 230,
	Bottle_Cap_12 = 231,
	Bottle_Cap_13 = 232,
	Bottle_Cap_14 = 233,
	Bottle_Cap_15 = 234,
	Bottle_Cap_16 = 235,
	Bottle_Cap_17 = 236,
	Bottle_Cap_18 = 237,
	Bottle_Cap_19 = 238,
	Bottle_Cap_20 = 239,
	Bottle_Cap_21 = 240,
	Bottle_Cap_22 = 241,
	Bottle_Cap_23 = 242,
	Bottle_Cap_24 = 243,
	File_22 = 244,
	File_23 = 245,
	File_24 = 246,
	File_25 = 247,
	File_26 = 248,
	File_27 = 249,
	File_28 = 250,
	File_29 = 251,
	File_30 = 252,
	File_31 = 253,
	Assault_Jacket = 254,
	Any = 255,
};
extern const char* EItemId_Names[]; // GameFlags.cpp

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
assert_size(cItem, 0xE);

struct ITEM_ORDER
{
	cItem* p_item_0;
	uint16_t num_4;
};
assert_size(ITEM_ORDER, 8);

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
assert_size(cItemMgr, 0x30);

extern cItemMgr* ItemMgr;
