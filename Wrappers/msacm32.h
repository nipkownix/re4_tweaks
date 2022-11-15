#pragma once
#include "wrappers.h"
#include "shared.h"

#if !X64
struct msacm32_dll
{
    HMODULE dll;
    FARPROC acmDriverAddA;
    FARPROC acmDriverAddW;
    FARPROC acmDriverClose;
    FARPROC acmDriverDetailsA;
    FARPROC acmDriverDetailsW;
    FARPROC acmDriverEnum;
    FARPROC acmDriverID;
    FARPROC acmDriverMessage;
    FARPROC acmDriverOpen;
    FARPROC acmDriverPriority;
    FARPROC acmDriverRemove;
    FARPROC acmFilterChooseA;
    FARPROC acmFilterChooseW;
    FARPROC acmFilterDetailsA;
    FARPROC acmFilterDetailsW;
    FARPROC acmFilterEnumA;
    FARPROC acmFilterEnumW;
    FARPROC acmFilterTagDetailsA;
    FARPROC acmFilterTagDetailsW;
    FARPROC acmFilterTagEnumA;
    FARPROC acmFilterTagEnumW;
    FARPROC acmFormatChooseA;
    FARPROC acmFormatChooseW;
    FARPROC acmFormatDetailsA;
    FARPROC acmFormatDetailsW;
    FARPROC acmFormatEnumA;
    FARPROC acmFormatEnumW;
    FARPROC acmFormatSuggest;
    FARPROC acmFormatTagDetailsA;
    FARPROC acmFormatTagDetailsW;
    FARPROC acmFormatTagEnumA;
    FARPROC acmFormatTagEnumW;
    FARPROC acmGetVersion;
    FARPROC acmMetrics;
    FARPROC acmStreamClose;
    FARPROC acmStreamConvert;
    FARPROC acmStreamMessage;
    FARPROC acmStreamOpen;
    FARPROC acmStreamPrepareHeader;
    FARPROC acmStreamReset;
    FARPROC acmStreamSize;
    FARPROC acmStreamUnprepareHeader;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        acmDriverAddA = GetProcAddress(dll, "acmDriverAddA");
        acmDriverAddW = GetProcAddress(dll, "acmDriverAddW");
        acmDriverClose = GetProcAddress(dll, "acmDriverClose");
        acmDriverDetailsA = GetProcAddress(dll, "acmDriverDetailsA");
        acmDriverDetailsW = GetProcAddress(dll, "acmDriverDetailsW");
        acmDriverEnum = GetProcAddress(dll, "acmDriverEnum");
        acmDriverID = GetProcAddress(dll, "acmDriverID");
        acmDriverMessage = GetProcAddress(dll, "acmDriverMessage");
        acmDriverOpen = GetProcAddress(dll, "acmDriverOpen");
        acmDriverPriority = GetProcAddress(dll, "acmDriverPriority");
        acmDriverRemove = GetProcAddress(dll, "acmDriverRemove");
        acmFilterChooseA = GetProcAddress(dll, "acmFilterChooseA");
        acmFilterChooseW = GetProcAddress(dll, "acmFilterChooseW");
        acmFilterDetailsA = GetProcAddress(dll, "acmFilterDetailsA");
        acmFilterDetailsW = GetProcAddress(dll, "acmFilterDetailsW");
        acmFilterEnumA = GetProcAddress(dll, "acmFilterEnumA");
        acmFilterEnumW = GetProcAddress(dll, "acmFilterEnumW");
        acmFilterTagDetailsA = GetProcAddress(dll, "acmFilterTagDetailsA");
        acmFilterTagDetailsW = GetProcAddress(dll, "acmFilterTagDetailsW");
        acmFilterTagEnumA = GetProcAddress(dll, "acmFilterTagEnumA");
        acmFilterTagEnumW = GetProcAddress(dll, "acmFilterTagEnumW");
        acmFormatChooseA = GetProcAddress(dll, "acmFormatChooseA");
        acmFormatChooseW = GetProcAddress(dll, "acmFormatChooseW");
        acmFormatDetailsA = GetProcAddress(dll, "acmFormatDetailsA");
        acmFormatDetailsW = GetProcAddress(dll, "acmFormatDetailsW");
        acmFormatEnumA = GetProcAddress(dll, "acmFormatEnumA");
        acmFormatEnumW = GetProcAddress(dll, "acmFormatEnumW");
        acmFormatSuggest = GetProcAddress(dll, "acmFormatSuggest");
        acmFormatTagDetailsA = GetProcAddress(dll, "acmFormatTagDetailsA");
        acmFormatTagDetailsW = GetProcAddress(dll, "acmFormatTagDetailsW");
        acmFormatTagEnumA = GetProcAddress(dll, "acmFormatTagEnumA");
        acmFormatTagEnumW = GetProcAddress(dll, "acmFormatTagEnumW");
        acmGetVersion = GetProcAddress(dll, "acmGetVersion");
        acmMetrics = GetProcAddress(dll, "acmMetrics");
        acmStreamClose = GetProcAddress(dll, "acmStreamClose");
        acmStreamConvert = GetProcAddress(dll, "acmStreamConvert");
        acmStreamMessage = GetProcAddress(dll, "acmStreamMessage");
        acmStreamOpen = GetProcAddress(dll, "acmStreamOpen");
        acmStreamPrepareHeader = GetProcAddress(dll, "acmStreamPrepareHeader");
        acmStreamReset = GetProcAddress(dll, "acmStreamReset");
        acmStreamSize = GetProcAddress(dll, "acmStreamSize");
        acmStreamUnprepareHeader = GetProcAddress(dll, "acmStreamUnprepareHeader");
    }
} msacm32;

__declspec(naked) void _acmDriverAddA() { _asm { jmp[msacm32.acmDriverAddA] } }
__declspec(naked) void _acmDriverAddW() { _asm { jmp[msacm32.acmDriverAddW] } }
__declspec(naked) void _acmDriverClose() { _asm { jmp[msacm32.acmDriverClose] } }
__declspec(naked) void _acmDriverDetailsA() { _asm { jmp[msacm32.acmDriverDetailsA] } }
__declspec(naked) void _acmDriverDetailsW() { _asm { jmp[msacm32.acmDriverDetailsW] } }
__declspec(naked) void _acmDriverEnum() { _asm { jmp[msacm32.acmDriverEnum] } }
__declspec(naked) void _acmDriverID() { _asm { jmp[msacm32.acmDriverID] } }
__declspec(naked) void _acmDriverMessage() { _asm { jmp[msacm32.acmDriverMessage] } }
__declspec(naked) void _acmDriverOpen() { _asm { jmp[msacm32.acmDriverOpen] } }
__declspec(naked) void _acmDriverPriority() { _asm { jmp[msacm32.acmDriverPriority] } }
__declspec(naked) void _acmDriverRemove() { _asm { jmp[msacm32.acmDriverRemove] } }
__declspec(naked) void _acmFilterChooseA() { _asm { jmp[msacm32.acmFilterChooseA] } }
__declspec(naked) void _acmFilterChooseW() { _asm { jmp[msacm32.acmFilterChooseW] } }
__declspec(naked) void _acmFilterDetailsA() { _asm { jmp[msacm32.acmFilterDetailsA] } }
__declspec(naked) void _acmFilterDetailsW() { _asm { jmp[msacm32.acmFilterDetailsW] } }
__declspec(naked) void _acmFilterEnumA() { _asm { jmp[msacm32.acmFilterEnumA] } }
__declspec(naked) void _acmFilterEnumW() { _asm { jmp[msacm32.acmFilterEnumW] } }
__declspec(naked) void _acmFilterTagDetailsA() { _asm { jmp[msacm32.acmFilterTagDetailsA] } }
__declspec(naked) void _acmFilterTagDetailsW() { _asm { jmp[msacm32.acmFilterTagDetailsW] } }
__declspec(naked) void _acmFilterTagEnumA() { _asm { jmp[msacm32.acmFilterTagEnumA] } }
__declspec(naked) void _acmFilterTagEnumW() { _asm { jmp[msacm32.acmFilterTagEnumW] } }
__declspec(naked) void _acmFormatChooseA() { _asm { jmp[msacm32.acmFormatChooseA] } }
__declspec(naked) void _acmFormatChooseW() { _asm { jmp[msacm32.acmFormatChooseW] } }
__declspec(naked) void _acmFormatDetailsA() { _asm { jmp[msacm32.acmFormatDetailsA] } }
__declspec(naked) void _acmFormatDetailsW() { _asm { jmp[msacm32.acmFormatDetailsW] } }
__declspec(naked) void _acmFormatEnumA() { _asm { jmp[msacm32.acmFormatEnumA] } }
__declspec(naked) void _acmFormatEnumW() { _asm { jmp[msacm32.acmFormatEnumW] } }
__declspec(naked) void _acmFormatSuggest() { _asm { jmp[msacm32.acmFormatSuggest] } }
__declspec(naked) void _acmFormatTagDetailsA() { _asm { jmp[msacm32.acmFormatTagDetailsA] } }
__declspec(naked) void _acmFormatTagDetailsW() { _asm { jmp[msacm32.acmFormatTagDetailsW] } }
__declspec(naked) void _acmFormatTagEnumA() { _asm { jmp[msacm32.acmFormatTagEnumA] } }
__declspec(naked) void _acmFormatTagEnumW() { _asm { jmp[msacm32.acmFormatTagEnumW] } }
__declspec(naked) void _acmGetVersion() { _asm { jmp[msacm32.acmGetVersion] } }
__declspec(naked) void _acmMetrics() { _asm { jmp[msacm32.acmMetrics] } }
__declspec(naked) void _acmStreamClose() { _asm { jmp[msacm32.acmStreamClose] } }
__declspec(naked) void _acmStreamConvert() { _asm { jmp[msacm32.acmStreamConvert] } }
__declspec(naked) void _acmStreamMessage() { _asm { jmp[msacm32.acmStreamMessage] } }
__declspec(naked) void _acmStreamOpen() { _asm { jmp[msacm32.acmStreamOpen] } }
__declspec(naked) void _acmStreamPrepareHeader() { _asm { jmp[msacm32.acmStreamPrepareHeader] } }
__declspec(naked) void _acmStreamReset() { _asm { jmp[msacm32.acmStreamReset] } }
__declspec(naked) void _acmStreamSize() { _asm { jmp[msacm32.acmStreamSize] } }
__declspec(naked) void _acmStreamUnprepareHeader() { _asm { jmp[msacm32.acmStreamUnprepareHeader] } }
#endif