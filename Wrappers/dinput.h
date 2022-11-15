#pragma once
#include "wrappers.h"
#include "shared.h"

#if !X64
struct dinput_dll
{
    HMODULE dll;
    FARPROC DirectInputCreateA;
    FARPROC DirectInputCreateEx;
    FARPROC DirectInputCreateW;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        DirectInputCreateA = GetProcAddress(dll, "DirectInputCreateA");
        DirectInputCreateEx = GetProcAddress(dll, "DirectInputCreateEx");
        DirectInputCreateW = GetProcAddress(dll, "DirectInputCreateW");
    }
} dinput;

__declspec(naked) void _DirectInputCreateA() { _asm { jmp[dinput.DirectInputCreateA] } }
__declspec(naked) void _DirectInputCreateEx() { _asm { jmp[dinput.DirectInputCreateEx] } }
__declspec(naked) void _DirectInputCreateW() { _asm { jmp[dinput.DirectInputCreateW] } }
#endif