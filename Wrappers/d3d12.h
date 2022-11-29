#pragma once
#include "wrappers.h"
#include "shared.h"

struct d3d12_dll
{
    HMODULE dll;
    FARPROC D3D12CoreCreateLayeredDevice;
    FARPROC D3D12CoreGetLayeredDeviceSize;
    FARPROC D3D12CoreRegisterLayers;
    FARPROC D3D12CreateDevice;
    FARPROC D3D12CreateRootSignatureDeserializer;
    FARPROC D3D12CreateVersionedRootSignatureDeserializer;
    FARPROC D3D12DeviceRemovedExtendedData;
    FARPROC D3D12EnableExperimentalFeatures;
    FARPROC D3D12GetDebugInterface;
    FARPROC D3D12GetInterface;
    FARPROC D3D12PIXEventsReplaceBlock;
    FARPROC D3D12PIXGetThreadInfo;
    FARPROC D3D12PIXNotifyWakeFromFenceSignal;
    FARPROC D3D12PIXReportCounter;
    FARPROC D3D12SerializeRootSignature;
    FARPROC D3D12SerializeVersionedRootSignature;
    FARPROC GetBehaviorValue;
    FARPROC SetAppCompatStringPointer;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        D3D12CoreCreateLayeredDevice = GetProcAddress(dll, "D3D12CoreCreateLayeredDevice");
        D3D12CoreGetLayeredDeviceSize = GetProcAddress(dll, "D3D12CoreGetLayeredDeviceSize");
        D3D12CoreRegisterLayers = GetProcAddress(dll, "D3D12CoreRegisterLayers");
        D3D12CreateDevice = GetProcAddress(dll, "D3D12CreateDevice");
        D3D12CreateRootSignatureDeserializer = GetProcAddress(dll, "D3D12CreateRootSignatureDeserializer");
        D3D12CreateVersionedRootSignatureDeserializer = GetProcAddress(dll, "D3D12CreateVersionedRootSignatureDeserializer");
        D3D12DeviceRemovedExtendedData = GetProcAddress(dll, "D3D12DeviceRemovedExtendedData");
        D3D12EnableExperimentalFeatures = GetProcAddress(dll, "D3D12EnableExperimentalFeatures");
        D3D12GetDebugInterface = GetProcAddress(dll, "D3D12GetDebugInterface");
        D3D12GetInterface = GetProcAddress(dll, "D3D12GetInterface");
        D3D12PIXEventsReplaceBlock = GetProcAddress(dll, "D3D12PIXEventsReplaceBlock");
        D3D12PIXGetThreadInfo = GetProcAddress(dll, "D3D12PIXGetThreadInfo");
        D3D12PIXNotifyWakeFromFenceSignal = GetProcAddress(dll, "D3D12PIXNotifyWakeFromFenceSignal");
        D3D12PIXReportCounter = GetProcAddress(dll, "D3D12PIXReportCounter");
        D3D12SerializeRootSignature = GetProcAddress(dll, "D3D12SerializeRootSignature");
        D3D12SerializeVersionedRootSignature = GetProcAddress(dll, "D3D12SerializeVersionedRootSignature");
        GetBehaviorValue = GetProcAddress(dll, "GetBehaviorValue");
        SetAppCompatStringPointer = GetProcAddress(dll, "SetAppCompatStringPointer");
    }
} d3d12;

#if !X64
__declspec(naked) void _D3D12CoreCreateLayeredDevice() { _asm { jmp[d3d12.D3D12CoreCreateLayeredDevice] } }
__declspec(naked) void _D3D12CoreGetLayeredDeviceSize() { _asm { jmp[d3d12.D3D12CoreGetLayeredDeviceSize] } }
__declspec(naked) void _D3D12CoreRegisterLayers() { _asm { jmp[d3d12.D3D12CoreRegisterLayers] } }
__declspec(naked) void _D3D12CreateDevice() { _asm { jmp[d3d12.D3D12CreateDevice] } }
__declspec(naked) void _D3D12CreateRootSignatureDeserializer() { _asm { jmp[d3d12.D3D12CreateRootSignatureDeserializer] } }
__declspec(naked) void _D3D12CreateVersionedRootSignatureDeserializer() { _asm { jmp[d3d12.D3D12CreateVersionedRootSignatureDeserializer] } }
__declspec(naked) void _D3D12DeviceRemovedExtendedData() { _asm { jmp[d3d12.D3D12DeviceRemovedExtendedData] } }
__declspec(naked) void _D3D12EnableExperimentalFeatures() { _asm { jmp[d3d12.D3D12EnableExperimentalFeatures] } }
__declspec(naked) void _D3D12GetDebugInterface() { _asm { jmp[d3d12.D3D12GetDebugInterface] } }
__declspec(naked) void _D3D12GetInterface() { _asm { jmp[d3d12.D3D12GetInterface] } }
__declspec(naked) void _D3D12PIXEventsReplaceBlock() { _asm { jmp[d3d12.D3D12PIXEventsReplaceBlock] } }
__declspec(naked) void _D3D12PIXGetThreadInfo() { _asm { jmp[d3d12.D3D12PIXGetThreadInfo] } }
__declspec(naked) void _D3D12PIXNotifyWakeFromFenceSignal() { _asm { jmp[d3d12.D3D12PIXNotifyWakeFromFenceSignal] } }
__declspec(naked) void _D3D12PIXReportCounter() { _asm { jmp[d3d12.D3D12PIXReportCounter] } }
__declspec(naked) void _D3D12SerializeRootSignature() { _asm { jmp[d3d12.D3D12SerializeRootSignature] } }
__declspec(naked) void _D3D12SerializeVersionedRootSignature() { _asm { jmp[d3d12.D3D12SerializeVersionedRootSignature] } }
__declspec(naked) void _GetBehaviorValue() { _asm { jmp[d3d12.GetBehaviorValue] } }
__declspec(naked) void _SetAppCompatStringPointer() { _asm { jmp[d3d12.SetAppCompatStringPointer] } }
#endif

#if X64
void _D3D12CoreCreateLayeredDevice() { d3d12.D3D12CoreCreateLayeredDevice(); }
void _D3D12CoreGetLayeredDeviceSize() { d3d12.D3D12CoreGetLayeredDeviceSize(); }
void _D3D12CoreRegisterLayers() { d3d12.D3D12CoreRegisterLayers(); }
void _D3D12CreateDevice() { d3d12.D3D12CreateDevice(); }
void _D3D12CreateRootSignatureDeserializer() { d3d12.D3D12CreateRootSignatureDeserializer(); }
void _D3D12CreateVersionedRootSignatureDeserializer() { d3d12.D3D12CreateVersionedRootSignatureDeserializer(); }
void _D3D12DeviceRemovedExtendedData() { d3d12.D3D12DeviceRemovedExtendedData(); }
void _D3D12EnableExperimentalFeatures() { d3d12.D3D12EnableExperimentalFeatures(); }
void _D3D12GetDebugInterface() { d3d12.D3D12GetDebugInterface(); }
void _D3D12GetInterface() { d3d12.D3D12GetInterface(); }
void _D3D12PIXEventsReplaceBlock() { d3d12.D3D12PIXEventsReplaceBlock(); }
void _D3D12PIXGetThreadInfo() { d3d12.D3D12PIXGetThreadInfo(); }
void _D3D12PIXNotifyWakeFromFenceSignal() { d3d12.D3D12PIXNotifyWakeFromFenceSignal(); }
void _D3D12PIXReportCounter() { d3d12.D3D12PIXReportCounter(); }
void _D3D12SerializeRootSignature() { d3d12.D3D12SerializeRootSignature(); }
void _D3D12SerializeVersionedRootSignature() { d3d12.D3D12SerializeVersionedRootSignature(); }
void _GetBehaviorValue() { d3d12.GetBehaviorValue(); }
void _SetAppCompatStringPointer() { d3d12.SetAppCompatStringPointer(); }
#endif