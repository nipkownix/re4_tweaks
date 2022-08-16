#pragma once
#include "basic_types.h"

struct cFilter00Params;
struct cFilter00Params2;

extern cFilter00Params* Filter00Params;
extern cFilter00Params2* Filter00Params2;

// TODO: make sure all versions have these in same order!
// 0xC061E8 in 1.1.0
struct cFilter00Params
{
    uint8_t blur_rate;
    uint8_t eff_blur_rate;
    float eff_spread_num;

    static cFilter00Params* get()
    {
        return Filter00Params;
    }
};
assert_size(cFilter00Params, 8);

// Located much further away from cFilter00Params for some reason
struct cFilter00Params2
{
    uint8_t blur_type;
    uint8_t blur_power;
    uint8_t eff_spread_pri;
    uint8_t eff_blur_r;
    uint32_t is_eff_spread_on;
    uint8_t eff_blur_g;
    uint8_t eff_blur_b;
    uint8_t eff_blur_type;
    uint8_t g_cont_level;
    uint32_t eff_spread_center_y;
    uint32_t eff_spread_center_x;
    uint32_t eff_spread_pow;
    void* filter00_buff;
    uint8_t g_cont_pow;
    uint8_t g_cont_bias;

    // are the following always after the params above?
    // they are used by filter00 at least...
    uint16_t pad;
    float unk;
    uint64_t pad2;
    GXTevScale blur_scale;

    static cFilter00Params2* get()
    {
        return Filter00Params2;
    }
};
#pragma pack(pop)
