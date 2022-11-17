#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "SDK/filter00.h"
#include "SDK/light.h"

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

const char* ToolMenu_LightToolMenuName = "DOF/BLUR MENU";
void ToolMenu_LightToolMenu()
{
    const int NumMenuItems = 11;
    const int LineHeight = 14;

    // Make sure game knows we're in a tool menu
    GLOBAL_WK* Global = GlobalPtr();
    Global->flags_DEBUG_0_60[0] |= GetFlagValue(uint32_t(Flags_DEBUG::DBG_TEST_MODE));

    uint32_t Flags_STOP_Orig = Global->flags_STOP_0_170[0];
    Global->flags_STOP_0_170[0] |= ~GetFlagValue(uint32_t(Flags_STOP::SPF_CINESCO)); // stops all except cinesco?

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
                Global->flags_STOP_0_170[0] = Flags_STOP_Orig;
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
                LightMgr->LightEnv_20.FocusZ_28 = 0;
                break;
            case 1:
                LightMgr->LightEnv_20.FocusLevel_2D = 0;
                break;
            case 2:
                LightMgr->LightEnv_20.FocusMode_2E = 0;
                break;
            case 3:
                LightMgr->LightEnv_20.blur_type_F0 = 0;
                break;
            case 4:
                LightMgr->LightEnv_20.blur_rate_2F = 0;
                break;
            case 5:
                LightMgr->LightEnv_20.blur_power_F1 = 0;
                break;
            case 6:
                LightMgr->LightEnv_20.contrast_level_F5 = 0;
                break;
            case 7:
                LightMgr->LightEnv_20.contrast_pow_F6 = 0;
                break;
            case 8:
                LightMgr->LightEnv_20.contrast_bias_F7 = 0;
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
                LightMgr->LightEnv_20.FocusZ_28++;
                break;
            case 1:
                LightMgr->LightEnv_20.FocusLevel_2D++;
                break;
            case 2:
                LightMgr->LightEnv_20.FocusMode_2E++;
                break;
            case 3:
                LightMgr->LightEnv_20.blur_type_F0++;
                break;
            case 4:
                LightMgr->LightEnv_20.blur_rate_2F++;
                break;
            case 5:
                LightMgr->LightEnv_20.blur_power_F1++;
                break;
            case 6:
                LightMgr->LightEnv_20.contrast_level_F5++;
                break;
            case 7:
                LightMgr->LightEnv_20.contrast_pow_F6++;
                break;
            case 8:
                LightMgr->LightEnv_20.contrast_bias_F7++;
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
                LightMgr->LightEnv_20.FocusZ_28--;
                break;
            case 1:
                LightMgr->LightEnv_20.FocusLevel_2D--;
                break;
            case 2:
                LightMgr->LightEnv_20.FocusMode_2E--;
                break;
            case 3:
                LightMgr->LightEnv_20.blur_type_F0--;
                break;
            case 4:
                LightMgr->LightEnv_20.blur_rate_2F--;
                break;
            case 5:
                LightMgr->LightEnv_20.blur_power_F1--;
                break;
            case 6:
                LightMgr->LightEnv_20.contrast_level_F5--;
                break;
            case 7:
                LightMgr->LightEnv_20.contrast_pow_F6--;
                break;
            case 8:
                LightMgr->LightEnv_20.contrast_bias_F7--;
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
            if (LightMgr->LightEnv_20.FocusZ_28 > 65535)
                LightMgr->LightEnv_20.FocusZ_28 = 0;
            if (LightMgr->LightEnv_20.FocusZ_28 < 0)
                LightMgr->LightEnv_20.FocusZ_28 = 65535;

            if (LightMgr->LightEnv_20.FocusLevel_2D > 10)
                LightMgr->LightEnv_20.FocusLevel_2D = 0;
            if (LightMgr->LightEnv_20.FocusLevel_2D < 0)
                LightMgr->LightEnv_20.FocusLevel_2D = 10;

            if (LightMgr->LightEnv_20.FocusMode_2E > 3)
                LightMgr->LightEnv_20.FocusMode_2E = 0;
            if (LightMgr->LightEnv_20.FocusMode_2E < 0)
                LightMgr->LightEnv_20.FocusMode_2E = 3;

            if (LightMgr->LightEnv_20.blur_type_F0 > 3)
                LightMgr->LightEnv_20.blur_type_F0 = 0;
            if (LightMgr->LightEnv_20.blur_type_F0 < 0)
                LightMgr->LightEnv_20.blur_type_F0 = 3;

            // Get lightMgr to update itself
            LightMgr->setEnv(&LightMgr->LightEnv_20, LightMgr->m_Hokan_178);
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
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " Distance    %7d", LightMgr->LightEnv_20.FocusZ_28);
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " AlphaLvl    %7d", int32_t(LightMgr->LightEnv_20.FocusLevel_2D));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " BlurMode    %7d (%s)", int32_t(LightMgr->LightEnv_20.FocusMode_2E), DoFModeNames[LightMgr->LightEnv_20.FocusMode_2E % 4]);
        
        curLine++;
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, "Blur / Filter00");
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " blur_type       %3d (%d) (%s)", int32_t(LightMgr->LightEnv_20.blur_type_F0), int32_t(Filter00Params2->blur_type), BlurTypeNames[Filter00Params2->blur_type % 4]);
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " blur_rate       %3d (%d)", int32_t(LightMgr->LightEnv_20.blur_rate_2F), int32_t(Filter00Params->blur_rate));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " blur_power      %3d (%d)", int32_t(LightMgr->LightEnv_20.blur_power_F1), int32_t(Filter00Params2->blur_power));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " contrast_level  %3d (%d)", int32_t(LightMgr->LightEnv_20.contrast_level_F5), int32_t(Filter00Params2->g_cont_level));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " contrast_pow    %3d (%d)", int32_t(LightMgr->LightEnv_20.contrast_pow_F6), int32_t(Filter00Params2->g_cont_pow));
        eprintf_Hook(32, LineHeight * curLine++, 0, 0, " contrast_bias   %3d (%d)", int32_t(LightMgr->LightEnv_20.contrast_bias_F7), int32_t(Filter00Params2->g_cont_bias));
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
    auto pattern = hook::pattern("55 8B EC 8B 45 08 85 C0 0F 84 ? ? ? ?");
    TaskSleep = (TaskSleep_Fn)pattern.count(1).get(0).get<uint8_t>(0);

    pattern = hook::pattern("55 8B EC 8B 45 08 8B 4D 0C 56 8B 35 ? ? ? ?");
    TaskChain = (TaskChain_Fn)pattern.count(1).get(0).get<uint8_t>(0);
}
