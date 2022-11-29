#pragma once
#include "wrappers.h"
#include "shared.h"

struct d3d10_dll
{
    HMODULE dll;
    FARPROC D3D10CompileEffectFromMemory;
    FARPROC D3D10CompileShader;
    FARPROC D3D10CreateBlob;
    FARPROC D3D10CreateDevice;
    FARPROC D3D10CreateDeviceAndSwapChain;
    FARPROC D3D10CreateEffectFromMemory;
    FARPROC D3D10CreateEffectPoolFromMemory;
    FARPROC D3D10CreateStateBlock;
    FARPROC D3D10DisassembleEffect;
    FARPROC D3D10DisassembleShader;
    FARPROC D3D10GetGeometryShaderProfile;
    FARPROC D3D10GetInputAndOutputSignatureBlob;
    FARPROC D3D10GetInputSignatureBlob;
    FARPROC D3D10GetOutputSignatureBlob;
    FARPROC D3D10GetPixelShaderProfile;
    FARPROC D3D10GetShaderDebugInfo;
    FARPROC D3D10GetVersion;
    FARPROC D3D10GetVertexShaderProfile;
    FARPROC D3D10PreprocessShader;
    FARPROC D3D10ReflectShader;
    FARPROC D3D10RegisterLayers;
    FARPROC D3D10StateBlockMaskDifference;
    FARPROC D3D10StateBlockMaskDisableAll;
    FARPROC D3D10StateBlockMaskDisableCapture;
    FARPROC D3D10StateBlockMaskEnableAll;
    FARPROC D3D10StateBlockMaskEnableCapture;
    FARPROC D3D10StateBlockMaskGetSetting;
    FARPROC D3D10StateBlockMaskIntersect;
    FARPROC D3D10StateBlockMaskUnion;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        D3D10CompileEffectFromMemory = GetProcAddress(dll, "D3D10CompileEffectFromMemory");
        D3D10CompileShader = GetProcAddress(dll, "D3D10CompileShader");
        D3D10CreateBlob = GetProcAddress(dll, "D3D10CreateBlob");
        D3D10CreateDevice = GetProcAddress(dll, "D3D10CreateDevice");
        D3D10CreateDeviceAndSwapChain = GetProcAddress(dll, "D3D10CreateDeviceAndSwapChain");
        D3D10CreateEffectFromMemory = GetProcAddress(dll, "D3D10CreateEffectFromMemory");
        D3D10CreateEffectPoolFromMemory = GetProcAddress(dll, "D3D10CreateEffectPoolFromMemory");
        D3D10CreateStateBlock = GetProcAddress(dll, "D3D10CreateStateBlock");
        D3D10DisassembleEffect = GetProcAddress(dll, "D3D10DisassembleEffect");
        D3D10DisassembleShader = GetProcAddress(dll, "D3D10DisassembleShader");
        D3D10GetGeometryShaderProfile = GetProcAddress(dll, "D3D10GetGeometryShaderProfile");
        D3D10GetInputAndOutputSignatureBlob = GetProcAddress(dll, "D3D10GetInputAndOutputSignatureBlob");
        D3D10GetInputSignatureBlob = GetProcAddress(dll, "D3D10GetInputSignatureBlob");
        D3D10GetOutputSignatureBlob = GetProcAddress(dll, "D3D10GetOutputSignatureBlob");
        D3D10GetPixelShaderProfile = GetProcAddress(dll, "D3D10GetPixelShaderProfile");
        D3D10GetShaderDebugInfo = GetProcAddress(dll, "D3D10GetShaderDebugInfo");
        D3D10GetVersion = GetProcAddress(dll, "D3D10GetVersion");
        D3D10GetVertexShaderProfile = GetProcAddress(dll, "D3D10GetVertexShaderProfile");
        D3D10PreprocessShader = GetProcAddress(dll, "D3D10PreprocessShader");
        D3D10ReflectShader = GetProcAddress(dll, "D3D10ReflectShader");
        D3D10RegisterLayers = GetProcAddress(dll, "D3D10RegisterLayers");
        D3D10StateBlockMaskDifference = GetProcAddress(dll, "D3D10StateBlockMaskDifference");
        D3D10StateBlockMaskDisableAll = GetProcAddress(dll, "D3D10StateBlockMaskDisableAll");
        D3D10StateBlockMaskDisableCapture = GetProcAddress(dll, "D3D10StateBlockMaskDisableCapture");
        D3D10StateBlockMaskEnableAll = GetProcAddress(dll, "D3D10StateBlockMaskEnableAll");
        D3D10StateBlockMaskEnableCapture = GetProcAddress(dll, "D3D10StateBlockMaskEnableCapture");
        D3D10StateBlockMaskGetSetting = GetProcAddress(dll, "D3D10StateBlockMaskGetSetting");
        D3D10StateBlockMaskIntersect = GetProcAddress(dll, "D3D10StateBlockMaskIntersect");
        D3D10StateBlockMaskUnion = GetProcAddress(dll, "D3D10StateBlockMaskUnion");
    }
} d3d10;

#if !X64
__declspec(naked) void _D3D10CompileEffectFromMemory() { _asm { jmp[d3d10.D3D10CompileEffectFromMemory] } }
__declspec(naked) void _D3D10CompileShader() { _asm { jmp[d3d10.D3D10CompileShader] } }
__declspec(naked) void _D3D10CreateBlob() { _asm { jmp[d3d10.D3D10CreateBlob] } }
__declspec(naked) void _D3D10CreateDevice() { _asm { jmp[d3d10.D3D10CreateDevice] } }
__declspec(naked) void _D3D10CreateDeviceAndSwapChain() { _asm { jmp[d3d10.D3D10CreateDeviceAndSwapChain] } }
__declspec(naked) void _D3D10CreateEffectFromMemory() { _asm { jmp[d3d10.D3D10CreateEffectFromMemory] } }
__declspec(naked) void _D3D10CreateEffectPoolFromMemory() { _asm { jmp[d3d10.D3D10CreateEffectPoolFromMemory] } }
__declspec(naked) void _D3D10CreateStateBlock() { _asm { jmp[d3d10.D3D10CreateStateBlock] } }
__declspec(naked) void _D3D10DisassembleEffect() { _asm { jmp[d3d10.D3D10DisassembleEffect] } }
__declspec(naked) void _D3D10DisassembleShader() { _asm { jmp[d3d10.D3D10DisassembleShader] } }
__declspec(naked) void _D3D10GetGeometryShaderProfile() { _asm { jmp[d3d10.D3D10GetGeometryShaderProfile] } }
__declspec(naked) void _D3D10GetInputAndOutputSignatureBlob() { _asm { jmp[d3d10.D3D10GetInputAndOutputSignatureBlob] } }
__declspec(naked) void _D3D10GetInputSignatureBlob() { _asm { jmp[d3d10.D3D10GetInputSignatureBlob] } }
__declspec(naked) void _D3D10GetOutputSignatureBlob() { _asm { jmp[d3d10.D3D10GetOutputSignatureBlob] } }
__declspec(naked) void _D3D10GetPixelShaderProfile() { _asm { jmp[d3d10.D3D10GetPixelShaderProfile] } }
__declspec(naked) void _D3D10GetShaderDebugInfo() { _asm { jmp[d3d10.D3D10GetShaderDebugInfo] } }
__declspec(naked) void _D3D10GetVersion() { _asm { jmp[d3d10.D3D10GetVersion] } }
__declspec(naked) void _D3D10GetVertexShaderProfile() { _asm { jmp[d3d10.D3D10GetVertexShaderProfile] } }
__declspec(naked) void _D3D10PreprocessShader() { _asm { jmp[d3d10.D3D10PreprocessShader] } }
__declspec(naked) void _D3D10ReflectShader() { _asm { jmp[d3d10.D3D10ReflectShader] } }
__declspec(naked) void _D3D10RegisterLayers() { _asm { jmp[d3d10.D3D10RegisterLayers] } }
__declspec(naked) void _D3D10StateBlockMaskDifference() { _asm { jmp[d3d10.D3D10StateBlockMaskDifference] } }
__declspec(naked) void _D3D10StateBlockMaskDisableAll() { _asm { jmp[d3d10.D3D10StateBlockMaskDisableAll] } }
__declspec(naked) void _D3D10StateBlockMaskDisableCapture() { _asm { jmp[d3d10.D3D10StateBlockMaskDisableCapture] } }
__declspec(naked) void _D3D10StateBlockMaskEnableAll() { _asm { jmp[d3d10.D3D10StateBlockMaskEnableAll] } }
__declspec(naked) void _D3D10StateBlockMaskEnableCapture() { _asm { jmp[d3d10.D3D10StateBlockMaskEnableCapture] } }
__declspec(naked) void _D3D10StateBlockMaskGetSetting() { _asm { jmp[d3d10.D3D10StateBlockMaskGetSetting] } }
__declspec(naked) void _D3D10StateBlockMaskIntersect() { _asm { jmp[d3d10.D3D10StateBlockMaskIntersect] } }
__declspec(naked) void _D3D10StateBlockMaskUnion() { _asm { jmp[d3d10.D3D10StateBlockMaskUnion] } }
#endif

#if X64
void _D3D10CompileEffectFromMemory() { d3d10.D3D10CompileEffectFromMemory(); }
void _D3D10CompileShader() { d3d10.D3D10CompileShader(); }
void _D3D10CreateBlob() { d3d10.D3D10CreateBlob(); }
void _D3D10CreateDevice() { d3d10.D3D10CreateDevice(); }
void _D3D10CreateDeviceAndSwapChain() { d3d10.D3D10CreateDeviceAndSwapChain(); }
void _D3D10CreateEffectFromMemory() { d3d10.D3D10CreateEffectFromMemory(); }
void _D3D10CreateEffectPoolFromMemory() { d3d10.D3D10CreateEffectPoolFromMemory(); }
void _D3D10CreateStateBlock() { d3d10.D3D10CreateStateBlock(); }
void _D3D10DisassembleEffect() { d3d10.D3D10DisassembleEffect(); }
void _D3D10DisassembleShader() { d3d10.D3D10DisassembleShader(); }
void _D3D10GetGeometryShaderProfile() { d3d10.D3D10GetGeometryShaderProfile(); }
void _D3D10GetInputAndOutputSignatureBlob() { d3d10.D3D10GetInputAndOutputSignatureBlob(); }
void _D3D10GetInputSignatureBlob() { d3d10.D3D10GetInputSignatureBlob(); }
void _D3D10GetOutputSignatureBlob() { d3d10.D3D10GetOutputSignatureBlob(); }
void _D3D10GetPixelShaderProfile() { d3d10.D3D10GetPixelShaderProfile(); }
void _D3D10GetShaderDebugInfo() { d3d10.D3D10GetShaderDebugInfo(); }
void _D3D10GetVersion() { d3d10.D3D10GetVersion(); }
void _D3D10GetVertexShaderProfile() { d3d10.D3D10GetVertexShaderProfile(); }
void _D3D10PreprocessShader() { d3d10.D3D10PreprocessShader(); }
void _D3D10ReflectShader() { d3d10.D3D10ReflectShader(); }
void _D3D10RegisterLayers() { d3d10.D3D10RegisterLayers(); }
void _D3D10StateBlockMaskDifference() { d3d10.D3D10StateBlockMaskDifference(); }
void _D3D10StateBlockMaskDisableAll() { d3d10.D3D10StateBlockMaskDisableAll(); }
void _D3D10StateBlockMaskDisableCapture() { d3d10.D3D10StateBlockMaskDisableCapture(); }
void _D3D10StateBlockMaskEnableAll() { d3d10.D3D10StateBlockMaskEnableAll(); }
void _D3D10StateBlockMaskEnableCapture() { d3d10.D3D10StateBlockMaskEnableCapture(); }
void _D3D10StateBlockMaskGetSetting() { d3d10.D3D10StateBlockMaskGetSetting(); }
void _D3D10StateBlockMaskIntersect() { d3d10.D3D10StateBlockMaskIntersect(); }
void _D3D10StateBlockMaskUnion() { d3d10.D3D10StateBlockMaskUnion(); }
#endif