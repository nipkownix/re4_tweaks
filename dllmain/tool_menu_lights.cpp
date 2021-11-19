#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\MinHook\MinHook.h"
#include "game_flags.h"

// dllmain.cpp externs
extern uint32_t* ptrpG;

// tool_menu.cpp externs
extern uint32_t* PadButtonStates;
void MenuTask_Hook();
void eprintf_Hook(int a1, int a2, int a3, int a4, char* Format, ...);

// LIGHT TOOL recreation

typedef int(__cdecl* TaskSleep_Fn)(int ms);
TaskSleep_Fn TaskSleep = nullptr;

typedef void(__cdecl* TaskChain_Fn)(void* a1, int a2);
TaskChain_Fn TaskChain = nullptr;

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

#pragma pack(push, 1)

struct cLightMgr_EnvInfo // cLightMgr::GetEnvPtr returns ptr to this, mostly affects Filter01 "LeLit" params
{
    uint8_t unk[0x28];
    int32_t BlurDistance;
    uint8_t unk2C;
    int8_t BlurAlphaLevel;
    int8_t BlurMode;
};

struct cLightMgr
{
    void* vftable;
    uint32_t unk4;
    uint32_t unk8;
    uint32_t unkC;
    uint8_t unk10;
    uint8_t pad11[3];
    uint32_t unk14;
    uint32_t unk18;
    uint32_t unk1C;
    cLightMgr_EnvInfo EnvInfo;
};

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

cLightMgr* LightMgr;
cFilter00Params* Filter00Params;
cFilter00Params2* Filter00Params2;

const char* ToolMenu_LightToolMenuName = "DOF/BLUR MENU";
void ToolMenu_LightToolMenu()
{
    const int NumMenuItems = 8;
    const int LineHeight = 14;

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
            SelectedIdx = 3;
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
                LightMgr->EnvInfo.BlurDistance = 0;
                break;
            case 1:
                LightMgr->EnvInfo.BlurAlphaLevel = 0;
                break;
            case 2:
                LightMgr->EnvInfo.BlurMode = 0;
                break;
            case 3:
                Filter00Params->blur_rate = 0;
                break;
            case 4:
                Filter00Params2->blur_type = 0;
                break;
            case 5:
                Filter00Params2->blur_power = 0;
                break;
            case 6:
                Filter00Params->eff_blur_rate = 0;
                break;
            case 7:
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
                LightMgr->EnvInfo.BlurDistance++;
                break;
            case 1:
                LightMgr->EnvInfo.BlurAlphaLevel++;
                break;
            case 2:
                LightMgr->EnvInfo.BlurMode++;
                break;
            case 3:
                Filter00Params->blur_rate++;
                break;
            case 4:
                Filter00Params2->blur_type++;
                break;
            case 5:
                Filter00Params2->blur_power++;
                break;
            case 6:
                Filter00Params->eff_blur_rate++;
                break;
            case 7:
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
                LightMgr->EnvInfo.BlurDistance--;
                break;
            case 1:
                LightMgr->EnvInfo.BlurAlphaLevel--;
                break;
            case 2:
                LightMgr->EnvInfo.BlurMode--;
                break;
            case 3:
                Filter00Params->blur_rate--;
                break;
            case 4:
                Filter00Params2->blur_type--;
                break;
            case 5:
                Filter00Params2->blur_power--;
                break;
            case 6:
                Filter00Params->eff_blur_rate--;
                break;
            case 7:
                Filter00Params->eff_spread_num--;
                break;
            default:
                changed = false;
                break;
            }
        }

        if (changed)
        {
            if (LightMgr->EnvInfo.BlurDistance > 65535)
                LightMgr->EnvInfo.BlurDistance = 0;
            if (LightMgr->EnvInfo.BlurDistance < 0)
                LightMgr->EnvInfo.BlurDistance = 65535;

            if (LightMgr->EnvInfo.BlurAlphaLevel > 10)
                LightMgr->EnvInfo.BlurAlphaLevel = 0;
            if (LightMgr->EnvInfo.BlurAlphaLevel < 0)
                LightMgr->EnvInfo.BlurAlphaLevel = 10;

            if (LightMgr->EnvInfo.BlurMode > 3)
                LightMgr->EnvInfo.BlurMode = 0;
            if (LightMgr->EnvInfo.BlurMode < 0)
                LightMgr->EnvInfo.BlurMode = 3;
        }

        const char* BlurModeNames[] = { "NEAR", "FAR", "FollowPL NEAR", "FollowPL FAR" };

        int curLine = 2;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "LIGHT TOOL");
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "(tap A/ENTER to change values faster)");
        curLine++;
        int startLineHeight = LineHeight * curLine;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "Distance %7d", LightMgr->EnvInfo.BlurDistance);
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "AlphaLvl %7d", int32_t(LightMgr->EnvInfo.BlurAlphaLevel));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "BlurMode %7d (%s)", int32_t(LightMgr->EnvInfo.BlurMode), BlurModeNames[LightMgr->EnvInfo.BlurMode % 4]);
        curLine++;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "Filter00");
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "blur_rate %6d", int32_t(Filter00Params->blur_rate));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "blur_type %6d", int32_t(Filter00Params2->blur_type));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "blur_power %6d", int32_t(Filter00Params2->blur_power));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "eff_blur_rate %2d", int32_t(Filter00Params->eff_blur_rate));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "eff_spread_num %f", Filter00Params->eff_spread_num);
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
    auto pattern = hook::pattern("6A 00 53 6A 00 57 B9 ? ? ? ?");
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
}
