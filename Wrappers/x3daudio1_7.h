#pragma once
#include "wrappers.h"
#include "shared.h"

struct X3DAudio1_7
{
    HMODULE dll;
    FARPROC X3DAudioInitialize;
    FARPROC X3DAudioCalculate;
    FARPROC CreateFX;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        X3DAudioInitialize = GetProcAddress(dll, "X3DAudioInitialize");
        X3DAudioCalculate = GetProcAddress(dll, "X3DAudioCalculate");
        CreateFX = GetProcAddress(dll, "CreateFX");
    }
} X3DAudio1_7;

#if !X64
__declspec(naked) void _X3DAudioInitialize() { _asm { jmp[X3DAudio1_7.X3DAudioInitialize] } }
__declspec(naked) void _X3DAudioCalculate() { _asm { jmp[X3DAudio1_7.X3DAudioCalculate] } }
__declspec(naked) void _CreateFX() { _asm { jmp[X3DAudio1_7.CreateFX] } }
#endif

#if X64
void _X3DAudioInitialize() { X3DAudio1_7.X3DAudioInitialize(); }
void _X3DAudioCalculate() { X3DAudio1_7.X3DAudioCalculate(); }
void _CreateFX() { X3DAudio1_7.CreateFX(); }
#endif