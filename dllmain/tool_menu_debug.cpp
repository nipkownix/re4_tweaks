#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\MinHook\MinHook.h"
#include "tool_menu.h"
#include "game_flags.h"

// Fixes for debug-builds tool menu
// (mainly just hooks for funcs that handle input for those menus, so we can slow down inputs to them)

typedef void(__cdecl* Fn_0args_cdecl)();
typedef void(__cdecl* move_Fn)(void* a1);

extern uint32_t* PadButtonStates; // tool_menu.cpp
uint32_t Keyboard2Gamepad(); // tool_menu.cpp

uint32_t* ToolOptionClass_ptr = nullptr;

extern uint32_t PrevButtons;
move_Fn move_Orig = nullptr;
void __cdecl move_Hook(void* a1)
{
    uint32_t curButtons = PadButtonStates[3] | Keyboard2Gamepad();
    PadButtonStates[1] = 0;
    PadButtonStates[3] = 0;

    if (PrevButtons != curButtons)
    {
        PadButtonStates[1] = curButtons;
        PadButtonStates[3] = curButtons;
    }

    PrevButtons = curButtons;

    move_Orig(a1);

    PadButtonStates[1] = curButtons;
    PadButtonStates[3] = curButtons;
}

move_Fn roomJumpMove_Orig = nullptr;
void __cdecl roomJumpMove_Hook(void* a1)
{
    uint32_t curButtons = PadButtonStates[3] | Keyboard2Gamepad();
    PadButtonStates[1] = 0;
    PadButtonStates[3] = 0;
    PadButtonStates[4] = 0;

    if (PrevButtons != curButtons)
    {
        PadButtonStates[1] = curButtons;
        PadButtonStates[3] = curButtons;
        PadButtonStates[4] = curButtons;
    }

    PrevButtons = curButtons;

    roomJumpMove_Orig(a1);

    PadButtonStates[1] = curButtons;
    PadButtonStates[3] = curButtons;
    PadButtonStates[4] = curButtons;
}

uint32_t RebindButtons(uint32_t buttons)
{
    // DEBUG OPTION menu uses a bunch of different funcs, all with their own code to check the buttons
    // It's easier to change the value it checks than to patch all those
    // We'll change A to X, and B to A
    if (buttons & uint32_t(GamePadButton::A))
    {
        buttons &= ~uint32_t(GamePadButton::A);
        buttons |= uint32_t(GamePadButton::X);
    }
    if (buttons & uint32_t(GamePadButton::B))
    {
        buttons &= ~uint32_t(GamePadButton::B);
        buttons |= uint32_t(GamePadButton::A);
    }
    return buttons;
}

Fn_0args_cdecl tp_menu_Orig;
void tp_menu_Hook()
{
    uint32_t* ToolOption_Buttons = (uint32_t*)(*ToolOptionClass_ptr + 0x20);

    uint32_t curButtons = *ToolOption_Buttons | Keyboard2Gamepad();
    *ToolOption_Buttons = 0;

    if (PrevButtons != curButtons)
        *ToolOption_Buttons = RebindButtons(curButtons);

    PrevButtons = curButtons;

    tp_menu_Orig();

    *ToolOption_Buttons = curButtons;
}

Fn_0args_cdecl tp_pl_Orig;
void tp_pl_Hook()
{
    uint32_t* ToolOption_Buttons = (uint32_t*)(*ToolOptionClass_ptr + 0x20);

    uint32_t curButtons = *ToolOption_Buttons | Keyboard2Gamepad();
    *ToolOption_Buttons = 0;

    if (PrevButtons != curButtons)
        *ToolOption_Buttons = RebindButtons(curButtons);

    PrevButtons = curButtons;

    tp_pl_Orig();

    *ToolOption_Buttons = curButtons;
}

Fn_0args_cdecl tp_scr_Orig;
void tp_scr_Hook()
{
    uint32_t* ToolOption_Buttons = (uint32_t*)(*ToolOptionClass_ptr + 0x20);

    uint32_t curButtons = *ToolOption_Buttons | Keyboard2Gamepad();
    *ToolOption_Buttons = 0;

    if (PrevButtons != curButtons)
        *ToolOption_Buttons = RebindButtons(curButtons);

    PrevButtons = curButtons;

    tp_scr_Orig();

    *ToolOption_Buttons = curButtons;
}

void GetToolMenuDebugPointers()
{
    auto pattern = hook::pattern("A1 ? ? ? ? C6 80 EA 04 00 00 00 C3");
    ToolOptionClass_ptr = *pattern.count(1).get(0).get<uint32_t*>(1);
}

void Init_ToolMenuDebug()
{
    GetToolMenuDebugPointers();

    auto pattern = hook::pattern("55 8B EC 83 EC ? A1 ? ? ? ? 33 C5 89 45 ? 0F BE 05 ? ? ? ? 99 83 E2 0F 03 C2 C1 F8 04");
    MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), move_Hook, (LPVOID*)&move_Orig);

    pattern = hook::pattern("55 8B EC F7 05 ? ? ? ? 04 00 04 00 53 56 8B 75 ? 57 74 ? FE 46 01");
    MH_CreateHook(pattern.count(1).get(0).get<uint8_t>(0), roomJumpMove_Hook, (LPVOID*)&roomJumpMove_Orig);

    pattern = hook::pattern("8D B8 70 02 00 00 0F B6 80 E0 04 00 00 B9 9C 00 00 00 BE ? ? ? ? F3 A5 8B 0C 85 ? ? ? ?");
    uint32_t* tp_funcs = *pattern.count(1).get(0).get<uint32_t*>(28);

    MH_CreateHook((LPVOID)tp_funcs[0], tp_menu_Hook, (LPVOID*)&tp_menu_Orig);
    MH_CreateHook((LPVOID)tp_funcs[1], tp_pl_Hook, (LPVOID*)&tp_pl_Orig);
    MH_CreateHook((LPVOID)tp_funcs[2], tp_scr_Hook, (LPVOID*)&tp_scr_Orig);
}
