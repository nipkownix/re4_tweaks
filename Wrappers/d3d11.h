#pragma once
#include "wrappers.h"
#include "shared.h"

struct d3d11_dll
{
    HMODULE dll;
    FARPROC CreateDirect3D11DeviceFromDXGIDevice;
    FARPROC CreateDirect3D11SurfaceFromDXGISurface;
    FARPROC D3D11CoreCreateDevice;
    FARPROC D3D11CoreCreateLayeredDevice;
    FARPROC D3D11CoreGetLayeredDeviceSize;
    FARPROC D3D11CoreRegisterLayers;
    FARPROC D3D11CreateDevice;
    FARPROC D3D11CreateDeviceAndSwapChain;
    FARPROC D3D11CreateDeviceForD3D12;
    FARPROC D3D11On12CreateDevice;
    FARPROC D3DKMTCloseAdapter;
    FARPROC D3DKMTCreateAllocation;
    FARPROC D3DKMTCreateContext;
    FARPROC D3DKMTCreateDevice;
    FARPROC D3DKMTCreateSynchronizationObject;
    FARPROC D3DKMTDestroyAllocation;
    FARPROC D3DKMTDestroyContext;
    FARPROC D3DKMTDestroyDevice;
    FARPROC D3DKMTDestroySynchronizationObject;
    FARPROC D3DKMTEscape;
    FARPROC D3DKMTGetContextSchedulingPriority;
    FARPROC D3DKMTGetDeviceState;
    FARPROC D3DKMTGetDisplayModeList;
    FARPROC D3DKMTGetMultisampleMethodList;
    FARPROC D3DKMTGetRuntimeData;
    FARPROC D3DKMTGetSharedPrimaryHandle;
    FARPROC D3DKMTLock;
    FARPROC D3DKMTOpenAdapterFromHdc;
    FARPROC D3DKMTOpenResource;
    FARPROC D3DKMTPresent;
    FARPROC D3DKMTQueryAdapterInfo;
    FARPROC D3DKMTQueryAllocationResidency;
    FARPROC D3DKMTQueryResourceInfo;
    FARPROC D3DKMTRender;
    FARPROC D3DKMTSetAllocationPriority;
    FARPROC D3DKMTSetContextSchedulingPriority;
    FARPROC D3DKMTSetDisplayMode;
    FARPROC D3DKMTSetDisplayPrivateDriverFormat;
    FARPROC D3DKMTSetGammaRamp;
    FARPROC D3DKMTSetVidPnSourceOwner;
    FARPROC D3DKMTSignalSynchronizationObject;
    FARPROC D3DKMTUnlock;
    FARPROC D3DKMTWaitForSynchronizationObject;
    FARPROC D3DKMTWaitForVerticalBlankEvent;
    FARPROC D3DPerformance_BeginEvent;
    FARPROC D3DPerformance_EndEvent;
    FARPROC D3DPerformance_GetStatus;
    FARPROC D3DPerformance_SetMarker;
    FARPROC EnableFeatureLevelUpgrade;
    FARPROC OpenAdapter10;
    FARPROC OpenAdapter10_2;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        CreateDirect3D11DeviceFromDXGIDevice = GetProcAddress(dll, "CreateDirect3D11DeviceFromDXGIDevice");
        CreateDirect3D11SurfaceFromDXGISurface = GetProcAddress(dll, "CreateDirect3D11SurfaceFromDXGISurface");
        D3D11CoreCreateDevice = GetProcAddress(dll, "D3D11CoreCreateDevice");
        D3D11CoreCreateLayeredDevice = GetProcAddress(dll, "D3D11CoreCreateLayeredDevice");
        D3D11CoreGetLayeredDeviceSize = GetProcAddress(dll, "D3D11CoreGetLayeredDeviceSize");
        D3D11CoreRegisterLayers = GetProcAddress(dll, "D3D11CoreRegisterLayers");
        D3D11CreateDevice = GetProcAddress(dll, "D3D11CreateDevice");
        D3D11CreateDeviceAndSwapChain = GetProcAddress(dll, "D3D11CreateDeviceAndSwapChain");
        D3D11CreateDeviceForD3D12 = GetProcAddress(dll, "D3D11CreateDeviceForD3D12");
        D3D11On12CreateDevice = GetProcAddress(dll, "D3D11On12CreateDevice");
        D3DKMTCloseAdapter = GetProcAddress(dll, "D3DKMTCloseAdapter");
        D3DKMTCreateAllocation = GetProcAddress(dll, "D3DKMTCreateAllocation");
        D3DKMTCreateContext = GetProcAddress(dll, "D3DKMTCreateContext");
        D3DKMTCreateDevice = GetProcAddress(dll, "D3DKMTCreateDevice");
        D3DKMTCreateSynchronizationObject = GetProcAddress(dll, "D3DKMTCreateSynchronizationObject");
        D3DKMTDestroyAllocation = GetProcAddress(dll, "D3DKMTDestroyAllocation");
        D3DKMTDestroyContext = GetProcAddress(dll, "D3DKMTDestroyContext");
        D3DKMTDestroyDevice = GetProcAddress(dll, "D3DKMTDestroyDevice");
        D3DKMTDestroySynchronizationObject = GetProcAddress(dll, "D3DKMTDestroySynchronizationObject");
        D3DKMTEscape = GetProcAddress(dll, "D3DKMTEscape");
        D3DKMTGetContextSchedulingPriority = GetProcAddress(dll, "D3DKMTGetContextSchedulingPriority");
        D3DKMTGetDeviceState = GetProcAddress(dll, "D3DKMTGetDeviceState");
        D3DKMTGetDisplayModeList = GetProcAddress(dll, "D3DKMTGetDisplayModeList");
        D3DKMTGetMultisampleMethodList = GetProcAddress(dll, "D3DKMTGetMultisampleMethodList");
        D3DKMTGetRuntimeData = GetProcAddress(dll, "D3DKMTGetRuntimeData");
        D3DKMTGetSharedPrimaryHandle = GetProcAddress(dll, "D3DKMTGetSharedPrimaryHandle");
        D3DKMTLock = GetProcAddress(dll, "D3DKMTLock");
        D3DKMTOpenAdapterFromHdc = GetProcAddress(dll, "D3DKMTOpenAdapterFromHdc");
        D3DKMTOpenResource = GetProcAddress(dll, "D3DKMTOpenResource");
        D3DKMTPresent = GetProcAddress(dll, "D3DKMTPresent");
        D3DKMTQueryAdapterInfo = GetProcAddress(dll, "D3DKMTQueryAdapterInfo");
        D3DKMTQueryAllocationResidency = GetProcAddress(dll, "D3DKMTQueryAllocationResidency");
        D3DKMTQueryResourceInfo = GetProcAddress(dll, "D3DKMTQueryResourceInfo");
        D3DKMTRender = GetProcAddress(dll, "D3DKMTRender");
        D3DKMTSetAllocationPriority = GetProcAddress(dll, "D3DKMTSetAllocationPriority");
        D3DKMTSetContextSchedulingPriority = GetProcAddress(dll, "D3DKMTSetContextSchedulingPriority");
        D3DKMTSetDisplayMode = GetProcAddress(dll, "D3DKMTSetDisplayMode");
        D3DKMTSetDisplayPrivateDriverFormat = GetProcAddress(dll, "D3DKMTSetDisplayPrivateDriverFormat");
        D3DKMTSetGammaRamp = GetProcAddress(dll, "D3DKMTSetGammaRamp");
        D3DKMTSetVidPnSourceOwner = GetProcAddress(dll, "D3DKMTSetVidPnSourceOwner");
        D3DKMTSignalSynchronizationObject = GetProcAddress(dll, "D3DKMTSignalSynchronizationObject");
        D3DKMTUnlock = GetProcAddress(dll, "D3DKMTUnlock");
        D3DKMTWaitForSynchronizationObject = GetProcAddress(dll, "D3DKMTWaitForSynchronizationObject");
        D3DKMTWaitForVerticalBlankEvent = GetProcAddress(dll, "D3DKMTWaitForVerticalBlankEvent");
        D3DPerformance_BeginEvent = GetProcAddress(dll, "D3DPerformance_BeginEvent");
        D3DPerformance_EndEvent = GetProcAddress(dll, "D3DPerformance_EndEvent");
        D3DPerformance_GetStatus = GetProcAddress(dll, "D3DPerformance_GetStatus");
        D3DPerformance_SetMarker = GetProcAddress(dll, "D3DPerformance_SetMarker");
        EnableFeatureLevelUpgrade = GetProcAddress(dll, "EnableFeatureLevelUpgrade");
        OpenAdapter10 = GetProcAddress(dll, "OpenAdapter10");
        OpenAdapter10_2 = GetProcAddress(dll, "OpenAdapter10_2");
    }
} d3d11;

#if !X64
__declspec(naked) void _CreateDirect3D11DeviceFromDXGIDevice() { _asm { jmp[d3d11.CreateDirect3D11DeviceFromDXGIDevice] } }
__declspec(naked) void _CreateDirect3D11SurfaceFromDXGISurface() { _asm { jmp[d3d11.CreateDirect3D11SurfaceFromDXGISurface] } }
__declspec(naked) void _D3D11CoreCreateDevice() { _asm { jmp[d3d11.D3D11CoreCreateDevice] } }
__declspec(naked) void _D3D11CoreCreateLayeredDevice() { _asm { jmp[d3d11.D3D11CoreCreateLayeredDevice] } }
__declspec(naked) void _D3D11CoreGetLayeredDeviceSize() { _asm { jmp[d3d11.D3D11CoreGetLayeredDeviceSize] } }
__declspec(naked) void _D3D11CoreRegisterLayers() { _asm { jmp[d3d11.D3D11CoreRegisterLayers] } }
__declspec(naked) void _D3D11CreateDevice() { _asm { jmp[d3d11.D3D11CreateDevice] } }
__declspec(naked) void _D3D11CreateDeviceAndSwapChain() { _asm { jmp[d3d11.D3D11CreateDeviceAndSwapChain] } }
__declspec(naked) void _D3D11CreateDeviceForD3D12() { _asm { jmp[d3d11.D3D11CreateDeviceForD3D12] } }
__declspec(naked) void _D3D11On12CreateDevice() { _asm { jmp[d3d11.D3D11On12CreateDevice] } }
__declspec(naked) void _D3DKMTCloseAdapter() { _asm { jmp[d3d11.D3DKMTCloseAdapter] } }
__declspec(naked) void _D3DKMTCreateAllocation() { _asm { jmp[d3d11.D3DKMTCreateAllocation] } }
__declspec(naked) void _D3DKMTCreateContext() { _asm { jmp[d3d11.D3DKMTCreateContext] } }
__declspec(naked) void _D3DKMTCreateDevice() { _asm { jmp[d3d11.D3DKMTCreateDevice] } }
__declspec(naked) void _D3DKMTCreateSynchronizationObject() { _asm { jmp[d3d11.D3DKMTCreateSynchronizationObject] } }
__declspec(naked) void _D3DKMTDestroyAllocation() { _asm { jmp[d3d11.D3DKMTDestroyAllocation] } }
__declspec(naked) void _D3DKMTDestroyContext() { _asm { jmp[d3d11.D3DKMTDestroyContext] } }
__declspec(naked) void _D3DKMTDestroyDevice() { _asm { jmp[d3d11.D3DKMTDestroyDevice] } }
__declspec(naked) void _D3DKMTDestroySynchronizationObject() { _asm { jmp[d3d11.D3DKMTDestroySynchronizationObject] } }
__declspec(naked) void _D3DKMTEscape() { _asm { jmp[d3d11.D3DKMTEscape] } }
__declspec(naked) void _D3DKMTGetContextSchedulingPriority() { _asm { jmp[d3d11.D3DKMTGetContextSchedulingPriority] } }
__declspec(naked) void _D3DKMTGetDeviceState() { _asm { jmp[d3d11.D3DKMTGetDeviceState] } }
__declspec(naked) void _D3DKMTGetDisplayModeList() { _asm { jmp[d3d11.D3DKMTGetDisplayModeList] } }
__declspec(naked) void _D3DKMTGetMultisampleMethodList() { _asm { jmp[d3d11.D3DKMTGetMultisampleMethodList] } }
__declspec(naked) void _D3DKMTGetRuntimeData() { _asm { jmp[d3d11.D3DKMTGetRuntimeData] } }
__declspec(naked) void _D3DKMTGetSharedPrimaryHandle() { _asm { jmp[d3d11.D3DKMTGetSharedPrimaryHandle] } }
__declspec(naked) void _D3DKMTLock() { _asm { jmp[d3d11.D3DKMTLock] } }
__declspec(naked) void _D3DKMTOpenAdapterFromHdc() { _asm { jmp[d3d11.D3DKMTOpenAdapterFromHdc] } }
__declspec(naked) void _D3DKMTOpenResource() { _asm { jmp[d3d11.D3DKMTOpenResource] } }
__declspec(naked) void _D3DKMTPresent() { _asm { jmp[d3d11.D3DKMTPresent] } }
__declspec(naked) void _D3DKMTQueryAdapterInfo() { _asm { jmp[d3d11.D3DKMTQueryAdapterInfo] } }
__declspec(naked) void _D3DKMTQueryAllocationResidency() { _asm { jmp[d3d11.D3DKMTQueryAllocationResidency] } }
__declspec(naked) void _D3DKMTQueryResourceInfo() { _asm { jmp[d3d11.D3DKMTQueryResourceInfo] } }
__declspec(naked) void _D3DKMTRender() { _asm { jmp[d3d11.D3DKMTRender] } }
__declspec(naked) void _D3DKMTSetAllocationPriority() { _asm { jmp[d3d11.D3DKMTSetAllocationPriority] } }
__declspec(naked) void _D3DKMTSetContextSchedulingPriority() { _asm { jmp[d3d11.D3DKMTSetContextSchedulingPriority] } }
__declspec(naked) void _D3DKMTSetDisplayMode() { _asm { jmp[d3d11.D3DKMTSetDisplayMode] } }
__declspec(naked) void _D3DKMTSetDisplayPrivateDriverFormat() { _asm { jmp[d3d11.D3DKMTSetDisplayPrivateDriverFormat] } }
__declspec(naked) void _D3DKMTSetGammaRamp() { _asm { jmp[d3d11.D3DKMTSetGammaRamp] } }
__declspec(naked) void _D3DKMTSetVidPnSourceOwner() { _asm { jmp[d3d11.D3DKMTSetVidPnSourceOwner] } }
__declspec(naked) void _D3DKMTSignalSynchronizationObject() { _asm { jmp[d3d11.D3DKMTSignalSynchronizationObject] } }
__declspec(naked) void _D3DKMTUnlock() { _asm { jmp[d3d11.D3DKMTUnlock] } }
__declspec(naked) void _D3DKMTWaitForSynchronizationObject() { _asm { jmp[d3d11.D3DKMTWaitForSynchronizationObject] } }
__declspec(naked) void _D3DKMTWaitForVerticalBlankEvent() { _asm { jmp[d3d11.D3DKMTWaitForVerticalBlankEvent] } }
__declspec(naked) void _D3DPerformance_BeginEvent() { _asm { jmp[d3d11.D3DPerformance_BeginEvent] } }
__declspec(naked) void _D3DPerformance_EndEvent() { _asm { jmp[d3d11.D3DPerformance_EndEvent] } }
__declspec(naked) void _D3DPerformance_GetStatus() { _asm { jmp[d3d11.D3DPerformance_GetStatus] } }
__declspec(naked) void _D3DPerformance_SetMarker() { _asm { jmp[d3d11.D3DPerformance_SetMarker] } }
__declspec(naked) void _EnableFeatureLevelUpgrade() { _asm { jmp[d3d11.EnableFeatureLevelUpgrade] } }
__declspec(naked) void _OpenAdapter10() { _asm { jmp[d3d11.OpenAdapter10] } }
__declspec(naked) void _OpenAdapter10_2() { _asm { jmp[d3d11.OpenAdapter10_2] } }
#endif

#if X64
void _CreateDirect3D11DeviceFromDXGIDevice() { d3d11.CreateDirect3D11DeviceFromDXGIDevice(); }
void _CreateDirect3D11SurfaceFromDXGISurface() { d3d11.CreateDirect3D11SurfaceFromDXGISurface(); }
void _D3D11CoreCreateDevice() { d3d11.D3D11CoreCreateDevice(); }
void _D3D11CoreCreateLayeredDevice() { d3d11.D3D11CoreCreateLayeredDevice(); }
void _D3D11CoreGetLayeredDeviceSize() { d3d11.D3D11CoreGetLayeredDeviceSize(); }
void _D3D11CoreRegisterLayers() { d3d11.D3D11CoreRegisterLayers(); }
void _D3D11CreateDevice() { d3d11.D3D11CreateDevice(); }
void _D3D11CreateDeviceAndSwapChain() { d3d11.D3D11CreateDeviceAndSwapChain(); }
void _D3D11CreateDeviceForD3D12() { d3d11.D3D11CreateDeviceForD3D12(); }
void _D3D11On12CreateDevice() { d3d11.D3D11On12CreateDevice(); }
void _D3DKMTCloseAdapter() { d3d11.D3DKMTCloseAdapter(); }
void _D3DKMTCreateAllocation() { d3d11.D3DKMTCreateAllocation(); }
void _D3DKMTCreateContext() { d3d11.D3DKMTCreateContext(); }
void _D3DKMTCreateDevice() { d3d11.D3DKMTCreateDevice(); }
void _D3DKMTCreateSynchronizationObject() { d3d11.D3DKMTCreateSynchronizationObject(); }
void _D3DKMTDestroyAllocation() { d3d11.D3DKMTDestroyAllocation(); }
void _D3DKMTDestroyContext() { d3d11.D3DKMTDestroyContext(); }
void _D3DKMTDestroyDevice() { d3d11.D3DKMTDestroyDevice(); }
void _D3DKMTDestroySynchronizationObject() { d3d11.D3DKMTDestroySynchronizationObject(); }
void _D3DKMTEscape() { d3d11.D3DKMTEscape(); }
void _D3DKMTGetContextSchedulingPriority() { d3d11.D3DKMTGetContextSchedulingPriority(); }
void _D3DKMTGetDeviceState() { d3d11.D3DKMTGetDeviceState(); }
void _D3DKMTGetDisplayModeList() { d3d11.D3DKMTGetDisplayModeList(); }
void _D3DKMTGetMultisampleMethodList() { d3d11.D3DKMTGetMultisampleMethodList(); }
void _D3DKMTGetRuntimeData() { d3d11.D3DKMTGetRuntimeData(); }
void _D3DKMTGetSharedPrimaryHandle() { d3d11.D3DKMTGetSharedPrimaryHandle(); }
void _D3DKMTLock() { d3d11.D3DKMTLock(); }
void _D3DKMTOpenAdapterFromHdc() { d3d11.D3DKMTOpenAdapterFromHdc(); }
void _D3DKMTOpenResource() { d3d11.D3DKMTOpenResource(); }
void _D3DKMTPresent() { d3d11.D3DKMTPresent(); }
void _D3DKMTQueryAdapterInfo() { d3d11.D3DKMTQueryAdapterInfo(); }
void _D3DKMTQueryAllocationResidency() { d3d11.D3DKMTQueryAllocationResidency(); }
void _D3DKMTQueryResourceInfo() { d3d11.D3DKMTQueryResourceInfo(); }
void _D3DKMTRender() { d3d11.D3DKMTRender(); }
void _D3DKMTSetAllocationPriority() { d3d11.D3DKMTSetAllocationPriority(); }
void _D3DKMTSetContextSchedulingPriority() { d3d11.D3DKMTSetContextSchedulingPriority(); }
void _D3DKMTSetDisplayMode() { d3d11.D3DKMTSetDisplayMode(); }
void _D3DKMTSetDisplayPrivateDriverFormat() { d3d11.D3DKMTSetDisplayPrivateDriverFormat(); }
void _D3DKMTSetGammaRamp() { d3d11.D3DKMTSetGammaRamp(); }
void _D3DKMTSetVidPnSourceOwner() { d3d11.D3DKMTSetVidPnSourceOwner(); }
void _D3DKMTSignalSynchronizationObject() { d3d11.D3DKMTSignalSynchronizationObject(); }
void _D3DKMTUnlock() { d3d11.D3DKMTUnlock(); }
void _D3DKMTWaitForSynchronizationObject() { d3d11.D3DKMTWaitForSynchronizationObject(); }
void _D3DKMTWaitForVerticalBlankEvent() { d3d11.D3DKMTWaitForVerticalBlankEvent(); }
void _D3DPerformance_BeginEvent() { d3d11.D3DPerformance_BeginEvent(); }
void _D3DPerformance_EndEvent() { d3d11.D3DPerformance_EndEvent(); }
void _D3DPerformance_GetStatus() { d3d11.D3DPerformance_GetStatus(); }
void _D3DPerformance_SetMarker() { d3d11.D3DPerformance_SetMarker(); }
void _EnableFeatureLevelUpgrade() { d3d11.EnableFeatureLevelUpgrade(); }
void _OpenAdapter10() { d3d11.OpenAdapter10(); }
void _OpenAdapter10_2() { d3d11.OpenAdapter10_2(); }
#endif