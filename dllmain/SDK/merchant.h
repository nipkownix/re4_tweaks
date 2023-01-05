#pragma once
#include "basic_types.h"

struct __declspec(align(2)) LEVEL_INFO
{
    uint16_t id_0;
    uint8_t level_2[4];
    uint8_t flag[2];
};
assert_size(LEVEL_INFO, 0x8);

struct __declspec(align(2)) STOCK_INFO
{
    uint16_t id_0;
    int16_t num_2;
    int8_t flag[4];
};
assert_size(STOCK_INFO, 0x8);

struct MERCHANT_DATA
{
    STOCK_INFO stock_0[64];
    LEVEL_INFO level_200[32];
    int8_t friendship_300;
    int8_t study_num_301;
    int8_t reduction_ratio_302;
    int8_t bonus_flag_303;
};
assert_size(MERCHANT_DATA, 0x304);

struct PRICE_INFO
{
    int16_t item_id_0;
    int16_t price_2;
    int16_t valid_4;
};
assert_size(PRICE_INFO, 0x6);

struct LEVEL_PRICE
{
    uint16_t id_0;
    int16_t power_2[7];
    int16_t speed_10[3];
    int16_t reload_16[3];
    int16_t bullet_1C[7];
};
assert_size(PRICE_INFO, 0x6);

namespace bio4 {
    extern void(__cdecl* levelDataAdd)(MERCHANT_DATA* p_data, LEVEL_INFO* p_level, uint32_t add_flag);
}