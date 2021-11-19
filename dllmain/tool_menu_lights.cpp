#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\MinHook\MinHook.h"
#include "game_flags.h"

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

#pragma pack(pop)

cLightMgr* LightMgr;

const char* ToolMenu_LightToolMenuName = "DOF/BLUR MENU";
void ToolMenu_LightToolMenu()
{
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
        if (SelectedIdx > 3)
            SelectedIdx = 0;

        if (buttonStates & (uint32_t(GamePadButton::A) | uint32_t(GamePadButton::B)))
            if (SelectedIdx == 3 || (buttonStates & uint32_t(GamePadButton::B)))
            {
                ToolMenu_Return();
                break;
            }

        bool changed = false;
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

        eprintf_Hook(32, 28, 0, 0, "LIGHT TOOL");
        eprintf_Hook(32, 56, 0, 0, "Distance %7d", LightMgr->EnvInfo.BlurDistance);
        eprintf_Hook(32, 70, 0, 0, "AlphaLvl %7d", int32_t(LightMgr->EnvInfo.BlurAlphaLevel));
        eprintf_Hook(32, 84, 0, 0, "BlurMode %7d (%s)", int32_t(LightMgr->EnvInfo.BlurMode), BlurModeNames[LightMgr->EnvInfo.BlurMode % 4]);
        eprintf_Hook(32, 98, 0, 0, "Exit");

        // selection cursor
        eprintf_Hook(16, 0x38 + (0xE * SelectedIdx), 0, 0, ">");

        TaskSleep(1);
    }
}

void LightTool_GetPointers()
{
    auto pattern = hook::pattern("6A 00 53 6A 00 57 B9 ? ? ? ?");
    auto varPtr = pattern.count(1).get(0).get<uint32_t>(7);
    LightMgr = (cLightMgr*)*varPtr;

    pattern = hook::pattern("55 8B EC 8B 45 08 85 C0 0F 84 ? ? ? ?");
    TaskSleep = (TaskSleep_Fn)pattern.count(1).get(0).get<uint8_t>(0);

    pattern = hook::pattern("55 8B EC 8B 45 08 8B 4D 0C 56 8B 35 ? ? ? ?");
    TaskChain = (TaskChain_Fn)pattern.count(1).get(0).get<uint8_t>(0);
}
