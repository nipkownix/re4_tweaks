#pragma once
#include "shared.h"

#if !X64
struct d3d8_dll
{
    HMODULE dll;
    FARPROC DebugSetMute;
    FARPROC Direct3D8EnableMaximizedWindowedModeShim;
    FARPROC Direct3DCreate8;
    FARPROC ValidatePixelShader;
    FARPROC ValidateVertexShader;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        DebugSetMute = GetProcAddress(dll, "DebugSetMute");
        Direct3D8EnableMaximizedWindowedModeShim = GetProcAddress(dll, "Direct3D8EnableMaximizedWindowedModeShim");
        Direct3DCreate8 = GetProcAddress(dll, "Direct3DCreate8");
        ValidatePixelShader = GetProcAddress(dll, "ValidatePixelShader");
        ValidateVertexShader = GetProcAddress(dll, "ValidateVertexShader");
    }
} d3d8;

//__declspec(naked) void _DebugSetMute() { _asm { jmp[d3d8.DebugSetMute] } }
__declspec(naked) void _Direct3D8EnableMaximizedWindowedModeShim() { _asm { jmp[d3d8.Direct3D8EnableMaximizedWindowedModeShim] } }
__declspec(naked) void _Direct3DCreate8() { _asm { jmp[d3d8.Direct3DCreate8] } }
__declspec(naked) void _ValidatePixelShader() { _asm { jmp[d3d8.ValidatePixelShader] } }
__declspec(naked) void _ValidateVertexShader() { _asm { jmp[d3d8.ValidateVertexShader] } }
#endif