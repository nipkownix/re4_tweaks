#pragma once
#include "wrappers.h"
#include "shared.h"

#if !X64
struct ddraw_dll
{
    HMODULE dll;
    FARPROC AcquireDDThreadLock;
    FARPROC CompleteCreateSysmemSurface;
    FARPROC D3DParseUnknownCommand;
    FARPROC DDGetAttachedSurfaceLcl;
    FARPROC DDInternalLock;
    FARPROC DDInternalUnlock;
    FARPROC DSoundHelp;
    FARPROC DirectDrawCreate;
    FARPROC DirectDrawCreateClipper;
    FARPROC DirectDrawCreateEx;
    FARPROC DirectDrawEnumerateA;
    FARPROC DirectDrawEnumerateExA;
    FARPROC DirectDrawEnumerateExW;
    FARPROC DirectDrawEnumerateW;
    FARPROC GetDDSurfaceLocal;
    FARPROC GetOLEThunkData;
    FARPROC GetSurfaceFromDC;
    FARPROC RegisterSpecialCase;
    FARPROC ReleaseDDThreadLock;
    FARPROC SetAppCompatData;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        AcquireDDThreadLock = GetProcAddress(dll, "AcquireDDThreadLock");
        CompleteCreateSysmemSurface = GetProcAddress(dll, "CompleteCreateSysmemSurface");
        D3DParseUnknownCommand = GetProcAddress(dll, "D3DParseUnknownCommand");
        DDGetAttachedSurfaceLcl = GetProcAddress(dll, "DDGetAttachedSurfaceLcl");
        DDInternalLock = GetProcAddress(dll, "DDInternalLock");
        DDInternalUnlock = GetProcAddress(dll, "DDInternalUnlock");
        DSoundHelp = GetProcAddress(dll, "DSoundHelp");
        DirectDrawCreate = GetProcAddress(dll, "DirectDrawCreate");
        DirectDrawCreateClipper = GetProcAddress(dll, "DirectDrawCreateClipper");
        DirectDrawCreateEx = GetProcAddress(dll, "DirectDrawCreateEx");
        DirectDrawEnumerateA = GetProcAddress(dll, "DirectDrawEnumerateA");
        DirectDrawEnumerateExA = GetProcAddress(dll, "DirectDrawEnumerateExA");
        DirectDrawEnumerateExW = GetProcAddress(dll, "DirectDrawEnumerateExW");
        DirectDrawEnumerateW = GetProcAddress(dll, "DirectDrawEnumerateW");
        GetDDSurfaceLocal = GetProcAddress(dll, "GetDDSurfaceLocal");
        GetOLEThunkData = GetProcAddress(dll, "GetOLEThunkData");
        GetSurfaceFromDC = GetProcAddress(dll, "GetSurfaceFromDC");
        RegisterSpecialCase = GetProcAddress(dll, "RegisterSpecialCase");
        ReleaseDDThreadLock = GetProcAddress(dll, "ReleaseDDThreadLock");
        SetAppCompatData = GetProcAddress(dll, "SetAppCompatData");
    }
} ddraw;

__declspec(naked) void _AcquireDDThreadLock() { _asm { jmp[ddraw.AcquireDDThreadLock] } }
__declspec(naked) void _CompleteCreateSysmemSurface() { _asm { jmp[ddraw.CompleteCreateSysmemSurface] } }
__declspec(naked) void _D3DParseUnknownCommand() { _asm { jmp[ddraw.D3DParseUnknownCommand] } }
__declspec(naked) void _DDGetAttachedSurfaceLcl() { _asm { jmp[ddraw.DDGetAttachedSurfaceLcl] } }
__declspec(naked) void _DDInternalLock() { _asm { jmp[ddraw.DDInternalLock] } }
__declspec(naked) void _DDInternalUnlock() { _asm { jmp[ddraw.DDInternalUnlock] } }
__declspec(naked) void _DSoundHelp() { _asm { jmp[ddraw.DSoundHelp] } }
__declspec(naked) void _DirectDrawCreate() { _asm { jmp[ddraw.DirectDrawCreate] } }
__declspec(naked) void _DirectDrawCreateClipper() { _asm { jmp[ddraw.DirectDrawCreateClipper] } }
__declspec(naked) void _DirectDrawCreateEx() { _asm { jmp[ddraw.DirectDrawCreateEx] } }
__declspec(naked) void _DirectDrawEnumerateA() { _asm { jmp[ddraw.DirectDrawEnumerateA] } }
__declspec(naked) void _DirectDrawEnumerateExA() { _asm { jmp[ddraw.DirectDrawEnumerateExA] } }
__declspec(naked) void _DirectDrawEnumerateExW() { _asm { jmp[ddraw.DirectDrawEnumerateExW] } }
__declspec(naked) void _DirectDrawEnumerateW() { _asm { jmp[ddraw.DirectDrawEnumerateW] } }
//__declspec(naked) void _DllCanUnloadNow() { _asm { jmp[ddraw.DllCanUnloadNow] } }
//__declspec(naked) void _DllGetClassObject() { _asm { jmp[ddraw.DllGetClassObject] } }
__declspec(naked) void _GetDDSurfaceLocal() { _asm { jmp[ddraw.GetDDSurfaceLocal] } }
__declspec(naked) void _GetOLEThunkData() { _asm { jmp[ddraw.GetOLEThunkData] } }
__declspec(naked) void _GetSurfaceFromDC() { _asm { jmp[ddraw.GetSurfaceFromDC] } }
__declspec(naked) void _RegisterSpecialCase() { _asm { jmp[ddraw.RegisterSpecialCase] } }
__declspec(naked) void _ReleaseDDThreadLock() { _asm { jmp[ddraw.ReleaseDDThreadLock] } }
__declspec(naked) void _SetAppCompatData() { _asm { jmp[ddraw.SetAppCompatData] } }
#endif