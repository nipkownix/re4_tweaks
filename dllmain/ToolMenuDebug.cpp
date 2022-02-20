#include <iostream>
#include "..\includes\stdafx.h"
#include "ToolMenu.h"
#include "Game.h"
#include "ConsoleWnd.h"

// Fixes for debug-builds tool menu
// (mainly just hooks for funcs that handle input for those menus, so we can slow down inputs to them)

extern uint32_t* PadButtonStates; // tool_menu.cpp
uint32_t Keyboard2Gamepad(); // tool_menu.cpp

uint32_t* ToolOptionClass_ptr = nullptr;

extern uint32_t PrevButtons;

void(__cdecl* move_Orig)(void* a1);
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

void(__cdecl* roomJumpMove_Orig)(void* a1);
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

void(*tp_menu_Orig)();
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

void(*tp_pl_Orig)();
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

void(*tp_scr_Orig)();
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
void Init_ToolMenuDebug()
{
    // Move
    auto pattern = hook::pattern("E8 ? ? ? ? 6A 01 E8 ? ? ? ? 83 C4 08 EB EA");
    auto func = pattern.count(1).get(0).get<uint32_t>(0);
    ReadCall(func, move_Orig);
    InjectHook(func, move_Hook);

    // roomJump
    pattern = hook::pattern("8B 0C 85 ? ? ? ? 83 C4 ? 68 ? ? ? ? FF D1 6A");
    uint32_t* roomJump_funcs = *pattern.count(1).get(0).get<uint32_t*>(3);

    roomJumpMove_Orig = (decltype(roomJumpMove_Orig))roomJump_funcs[1];
    injector::WriteMemory(&roomJump_funcs[1], &roomJumpMove_Hook, true);

    // tp funcs
    if (GameVersionIsDebug())
    {
        pattern = hook::pattern("A1 ? ? ? ? C6 80 EA 04 00 00 00 C3");
        ToolOptionClass_ptr = *pattern.count(1).get(0).get<uint32_t*>(1);

        pattern = hook::pattern("8B 0C 85 ? ? ? ? FF D1 A1 ? ? ? ? FE 80 ? ? ? ? 39 1D");
        uint32_t* tp_funcs = *pattern.count(1).get(0).get<uint32_t*>(3);

        tp_menu_Orig = (decltype(tp_menu_Orig))tp_funcs[0];
        injector::WriteMemory(&tp_funcs[0], &tp_menu_Hook, true);

        tp_pl_Orig = (decltype(tp_pl_Orig))tp_funcs[1];
        injector::WriteMemory(&tp_funcs[1], &tp_pl_Hook, true);

        tp_scr_Orig = (decltype(tp_scr_Orig))tp_funcs[2];
        injector::WriteMemory(&tp_funcs[2], &tp_scr_Hook, true);
    }
}
