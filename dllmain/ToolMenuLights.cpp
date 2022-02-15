#include <iostream>
#include "..\includes\stdafx.h"
#include "GameFlags.h"

// tool_menu.cpp externs
extern uint32_t* PadButtonStates;
void MenuTask_Hook();
void eprintf_Hook(int a1, int a2, int a3, int a4, char* Format, ...);

// LIGHT TOOL recreation

typedef int(__cdecl* TaskSleep_Fn)(int ms);
TaskSleep_Fn TaskSleep = nullptr;

typedef void(__cdecl* TaskChain_Fn)(void* a1, int a2);
TaskChain_Fn TaskChain = nullptr;

class cLightMgr;
typedef void(__fastcall* cLightMgr__setEnv_Fn)(cLightMgr* thisptr, void* unused, struct cLightMgr_EnvInfo* a2, int8_t a3);
cLightMgr__setEnv_Fn cLightMgr__setEnv = nullptr;

void ToolMenu_Return()
{
    TaskChain(&MenuTask_Hook, 0);
}

// TODO: make sure all versions have these in same order!
struct cFilter00Params
{
    uint8_t blur_rate;
    uint8_t eff_blur_rate;
    float eff_spread_num;
};

struct cPenWind
{
    uint8_t direction;
    uint8_t power;
    uint8_t frequency;
    uint8_t unk3;
};
static_assert(sizeof(cPenWind) == 4);

enum class GXAnisotropy : uint8_t
{
    GX_ANISO_1,
    GX_ANISO_2,
    GX_ANISO_4
};

struct cLightMgr_EnvInfo // cLightMgr::GetEnvPtr returns ptr to this, mostly affects Filter01 "LeLit" params
{
    uint32_t amb_color_scroll;
    uint8_t unk4[4];
    uint32_t fog_type;
    float fog_start;
    float fog_end;
    float fog_color;
    uint32_t mirror_fog_type;
    float mirror_fog_start;
    float mirror_fog_end;
    float mirror_fog_color;
    int32_t dof_distance;
    uint8_t unk2C;
    int8_t dof_alpha_level;
    int8_t dof_mode;
    uint8_t blur_rate;
    uint32_t tune[4];
    uint8_t model_tev_scale;
    uint8_t player_tev_scale;
    float fog_far_play; // named FAR_PLAY by GC debug, wtf does FAR_PLAY mean? (color related?)
    uint8_t unk48[148];
    uint8_t unkDC[16];
    cPenWind wind;
    int8_t blur_type;
    uint8_t blur_power;
    uint8_t min_lod;
    uint8_t max_lod;
    GXAnisotropy aniso;
    uint8_t g_cont_level;
    uint8_t g_cont_pow;
    uint8_t g_cont_bias;
    float lod_bias;
    uint32_t amb_color_emobj;
    uint32_t amb_color_effect;
};
static_assert(sizeof(cLightMgr_EnvInfo) == 0x104); // size based on bzero call

class cLightMgr
{
public:
    void* vftable;
    uint32_t unk4;
    uint32_t unk8;
    uint32_t unkC;
    uint8_t unk10;
    struct cLight* lights;
    uint32_t unk18;
    void* room_lit_pointer;
    cLightMgr_EnvInfo env_info;
    uint32_t unkFlagsBuffer124[8];
    uint8_t unk144[32];
    uint32_t path;
    void* core_pointer;  // pointer to int16?
    void* room_pointer1; // pointer to int16?
    void* room_pointer2; // pointer to int16?
    uint32_t unk174;
    int8_t unk178;
    float unk17C;
    uint32_t unk180;
    uint32_t unk184;
    uint32_t unk188;
    uint32_t unk18C;
    float unk190;
    uint8_t unk194[64];

    void setEnv(cLightMgr_EnvInfo* env, int8_t a3)
    {
        cLightMgr__setEnv(this, nullptr, env, a3);
    }
};
// size based on ctor code (same size as GC, but offsets changed some reason?)
static_assert(sizeof(cLightMgr) == 0x1D4); 

#pragma pack(push, 1)
enum class GXTevScale : uint32_t
{
    GX_CS_SCALE_1,
    GX_CS_SCALE_2,
    GX_CS_SCALE_4,
    GX_CS_DIVIDE_2,
    GX_MAX_TEVSCALE
};

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
    GXTevScale scale;
};
#pragma pack(pop)

uint32_t* ptrpG;

cLightMgr* LightMgr;
cFilter00Params* Filter00Params;
cFilter00Params2* Filter00Params2;

const char* ToolMenu_LightToolMenuName = "DOF/BLUR MENU";
void ToolMenu_LightToolMenu()
{
    const int NumMenuItems = 11;
    const int LineHeight = 14;

    int sizetest = sizeof(cLightMgr);
    int sizetest2 = sizeof(cLightMgr_EnvInfo);

    // Make sure game knows we're in a tool menu
    uint8_t* Global = *(uint8_t**)ptrpG;
    uint32_t* pFlags_DEBUG = (uint32_t*)(Global + 0x60);
    uint32_t* pFlags_STOP = (uint32_t*)(Global + 0x170);
    *pFlags_DEBUG |= GetFlagValue(uint32_t(Flags_DEBUG::DBG_TEST_MODE));

    uint32_t Flags_STOP_Orig = *pFlags_STOP;
    *pFlags_STOP |= ~GetFlagValue(uint32_t(Flags_STOP::SPF_CINESCO)); // stops all except cinesco?

    int SelectedIdx = 0;
    int PrevButtons = 0;
    while (true)
    {
        int buttonStates = PadButtonStates[0];
        if (PrevButtons == buttonStates)
            buttonStates = 0;
        else
            PrevButtons = buttonStates;

        // handle gamepad
        if (buttonStates & (uint32_t(GamePadButton::DPad_Down) | uint32_t(GamePadButton::LS_Down)))
            SelectedIdx++;
        if (buttonStates & (uint32_t(GamePadButton::DPad_Up) | uint32_t(GamePadButton::LS_Up)))
            SelectedIdx--;
        if (SelectedIdx < 0)
            SelectedIdx = NumMenuItems;
        if (SelectedIdx > NumMenuItems)
            SelectedIdx = 0;

        if (buttonStates & (uint32_t(GamePadButton::A) | uint32_t(GamePadButton::B)))
            if (SelectedIdx == NumMenuItems || (buttonStates & uint32_t(GamePadButton::B)))
            {
                *pFlags_STOP = Flags_STOP_Orig;
                ToolMenu_Return();
                break;
            }

        bool changed = false;
        if (buttonStates & uint32_t(GamePadButton::Y))
        {
            changed = true;
            switch (SelectedIdx)
            {
            case 0:
                LightMgr->env_info.dof_distance = 0;
                break;
            case 1:
                LightMgr->env_info.dof_alpha_level = 0;
                break;
            case 2:
                LightMgr->env_info.dof_mode = 0;
                break;
            case 3:
                LightMgr->env_info.blur_type = 0;
                break;
            case 4:
                LightMgr->env_info.blur_rate = 0;
                break;
            case 5:
                LightMgr->env_info.blur_power = 0;
                break;
            case 6:
                LightMgr->env_info.g_cont_level = 0;
                break;
            case 7:
                LightMgr->env_info.g_cont_pow = 0;
                break;
            case 8:
                LightMgr->env_info.g_cont_bias = 0;
                break;
            case 9:
                Filter00Params->eff_blur_rate = 0;
                break;
            case 10:
                Filter00Params->eff_spread_num = 0;
                break;
            default:
                changed = false;
                break;
            }
        }
        if (buttonStates & (uint32_t(GamePadButton::DPad_Right) | uint32_t(GamePadButton::LS_Right)))
        {
            changed = true;
            switch (SelectedIdx)
            {
            case 0:
                LightMgr->env_info.dof_distance++;
                break;
            case 1:
                LightMgr->env_info.dof_alpha_level++;
                break;
            case 2:
                LightMgr->env_info.dof_mode++;
                break;
            case 3:
                LightMgr->env_info.blur_type++;
                break;
            case 4:
                LightMgr->env_info.blur_rate++;
                break;
            case 5:
                LightMgr->env_info.blur_power++;
                break;
            case 6:
                LightMgr->env_info.g_cont_level++;
                break;
            case 7:
                LightMgr->env_info.g_cont_pow++;
                break;
            case 8:
                LightMgr->env_info.g_cont_bias++;
                break;
            case 9:
                Filter00Params->eff_blur_rate++;
                break;
            case 10:
                Filter00Params->eff_spread_num++;
                break;
            default:
                changed = false;
                break;
            }
        }

        if (buttonStates & (uint32_t(GamePadButton::DPad_Left) | uint32_t(GamePadButton::LS_Left)))
        {
            changed = true;
            switch (SelectedIdx)
            {
            case 0:
                LightMgr->env_info.dof_distance--;
                break;
            case 1:
                LightMgr->env_info.dof_alpha_level--;
                break;
            case 2:
                LightMgr->env_info.dof_mode--;
                break;
            case 3:
                LightMgr->env_info.blur_type--;
                break;
            case 4:
                LightMgr->env_info.blur_rate--;
                break;
            case 5:
                LightMgr->env_info.blur_power--;
                break;
            case 6:
                LightMgr->env_info.g_cont_level--;
                break;
            case 7:
                LightMgr->env_info.g_cont_pow--;
                break;
            case 8:
                LightMgr->env_info.g_cont_bias--;
                break;
            case 9:
                Filter00Params->eff_blur_rate--;
                break;
            case 10:
                Filter00Params->eff_spread_num--;
                break;
            default:
                changed = false;
                break;
            }
        }

        if (changed)
        {
            if (LightMgr->env_info.dof_distance > 65535)
                LightMgr->env_info.dof_distance = 0;
            if (LightMgr->env_info.dof_distance < 0)
                LightMgr->env_info.dof_distance = 65535;

            if (LightMgr->env_info.dof_alpha_level > 10)
                LightMgr->env_info.dof_alpha_level = 0;
            if (LightMgr->env_info.dof_alpha_level < 0)
                LightMgr->env_info.dof_alpha_level = 10;

            if (LightMgr->env_info.dof_mode > 3)
                LightMgr->env_info.dof_mode = 0;
            if (LightMgr->env_info.dof_mode < 0)
                LightMgr->env_info.dof_mode = 3;

            if (LightMgr->env_info.blur_type > 3)
                LightMgr->env_info.blur_type = 0;
            if (LightMgr->env_info.blur_type < 0)
                LightMgr->env_info.blur_type = 3;

            // Get lightMgr to update itself
            LightMgr->setEnv(&LightMgr->env_info, LightMgr->unk178);
        }

        const char* DoFModeNames[] = { "NEAR", "FAR", "FollowPL NEAR", "FollowPL FAR" };
        const char* BlurTypeNames[] = { "NORMAL", "SPREAD", "ADD", "SUBTRACT" };
        const char* FogTypeNames[] = { "NONE", "????", "LINEAR", "????", "EXP", "EXP2", "REVEXP", "REVEXP2" };

        int curLine = 2;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "LIGHT TOOL");
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "(tap A/ENTER to change values faster)");
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "(press Y to reset value to 0)");

        curLine++;
        eprintf_Hook(32, LineHeight* curLine++, 0, 0, "DoF / Filter01");
        int startLineHeight = LineHeight * curLine;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " Distance    %7d", LightMgr->env_info.dof_distance);
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " AlphaLvl    %7d", int32_t(LightMgr->env_info.dof_alpha_level));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " BlurMode    %7d (%s)", int32_t(LightMgr->env_info.dof_mode), DoFModeNames[LightMgr->env_info.dof_mode % 4]);
        
        curLine++;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "Blur / Filter00");
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " blur_type       %3d (%d) (%s)", int32_t(LightMgr->env_info.blur_type), int32_t(Filter00Params2->blur_type), BlurTypeNames[Filter00Params2->blur_type % 4]);
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " blur_rate       %3d (%d)", int32_t(LightMgr->env_info.blur_rate), int32_t(Filter00Params->blur_rate));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " blur_power      %3d (%d)", int32_t(LightMgr->env_info.blur_power), int32_t(Filter00Params2->blur_power));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " contrast_level  %3d (%d)", int32_t(LightMgr->env_info.g_cont_level), int32_t(Filter00Params2->g_cont_level));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " contrast_pow    %3d (%d)", int32_t(LightMgr->env_info.g_cont_pow), int32_t(Filter00Params2->g_cont_pow));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " contrast_bias   %3d (%d)", int32_t(LightMgr->env_info.g_cont_bias), int32_t(Filter00Params2->g_cont_bias));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " eff_blur_rate   %3d", int32_t(Filter00Params->eff_blur_rate));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " eff_spread_num  %f", Filter00Params->eff_spread_num);

        curLine++;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "Exit");

        // selection cursor
        int lineSkip = 0;
        if (SelectedIdx >= 3)
            lineSkip += LineHeight * 2;
        if (SelectedIdx >= NumMenuItems)
            lineSkip += LineHeight;

        eprintf_Hook(16, startLineHeight + (0xE * SelectedIdx) + lineSkip, 0, 0, ">");

        TaskSleep(1);
    }
}

void LightTool_GetPointers()
{
    // pG (Globals) pointer
    auto pattern = hook::pattern("A1 ? ? ? ? B9 FF FF FF 7F 21 48 ? A1");
    ptrpG = *pattern.count(1).get(0).get<uint32_t*>(1);

    pattern = hook::pattern("6A 00 53 6A 00 57 B9 ? ? ? ?");
    auto varPtr = pattern.count(1).get(0).get<uint32_t>(7);
    LightMgr = (cLightMgr*)*varPtr;

    pattern = hook::pattern("D9 EE 33 C0 D9 15 ? ? ? ? A2 ? ? ? ?");
    varPtr = pattern.count(3).get(0).get<uint32_t>(11);
    Filter00Params = (cFilter00Params*)*varPtr;

    varPtr = pattern.count(3).get(0).get<uint32_t>(22);
    Filter00Params2 = (cFilter00Params2*)*varPtr;

    pattern = hook::pattern("55 8B EC 8B 45 08 85 C0 0F 84 ? ? ? ?");
    TaskSleep = (TaskSleep_Fn)pattern.count(1).get(0).get<uint8_t>(0);

    pattern = hook::pattern("55 8B EC 8B 45 08 8B 4D 0C 56 8B 35 ? ? ? ?");
    TaskChain = (TaskChain_Fn)pattern.count(1).get(0).get<uint8_t>(0);

    pattern = hook::pattern("55 8B EC 51 53 56 8B 75 ? 8B 46 ? A3 ? ? ? ? 8B D9 8B 4E ?");
    cLightMgr__setEnv = (cLightMgr__setEnv_Fn)pattern.count(1).get(0).get<uint8_t>(0);
}
