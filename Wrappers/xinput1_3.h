#pragma once
#include "wrappers.h"
#include "shared.h"

struct xinput1_3_dll
{
    HMODULE dll;
    FARPROC DllMain;
    FARPROC XInputEnable;
    FARPROC XInputGetBatteryInformation;
    FARPROC XInputGetCapabilities;
    FARPROC XInputGetDSoundAudioDeviceGuids;
    FARPROC XInputGetKeystroke;
    FARPROC XInputGetState;
    FARPROC XInputSetState;
    FARPROC XInputGetStateEx;
    FARPROC XInputWaitForGuideButton;
    FARPROC XInputCancelGuideButtonWait;
    FARPROC XInputPowerOffController;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        DllMain = GetProcAddress(dll, "DllMain");
        XInputEnable = GetProcAddress(dll, "XInputEnable");
        XInputGetBatteryInformation = GetProcAddress(dll, "XInputGetBatteryInformation");
        XInputGetCapabilities = GetProcAddress(dll, "XInputGetCapabilities");
        XInputGetDSoundAudioDeviceGuids = GetProcAddress(dll, "XInputGetDSoundAudioDeviceGuids");
        XInputGetKeystroke = GetProcAddress(dll, "XInputGetKeystroke");
        XInputGetState = GetProcAddress(dll, "XInputGetState");
        XInputSetState = GetProcAddress(dll, "XInputSetState");
        XInputGetStateEx = GetProcAddress(dll, (LPCSTR)100);
        XInputWaitForGuideButton = GetProcAddress(dll, (LPCSTR)101);
        XInputCancelGuideButtonWait = GetProcAddress(dll, (LPCSTR)102);
        XInputPowerOffController = GetProcAddress(dll, (LPCSTR)103);
    }
} xinput1_3;

#if !X64
__declspec(naked) void _DllMain() { _asm { jmp[xinput1_3.DllMain] } }
__declspec(naked) void _XInputEnable() { _asm { jmp[xinput1_3.XInputEnable] } }
__declspec(naked) void _XInputGetBatteryInformation() { _asm { jmp[xinput1_3.XInputGetBatteryInformation] } }
__declspec(naked) void _XInputGetCapabilities() { _asm { jmp[xinput1_3.XInputGetCapabilities] } }
__declspec(naked) void _XInputGetDSoundAudioDeviceGuids() { _asm { jmp[xinput1_3.XInputGetDSoundAudioDeviceGuids] } }
__declspec(naked) void _XInputGetKeystroke() { _asm { jmp[xinput1_3.XInputGetKeystroke] } }
__declspec(naked) void _XInputGetState() { _asm { jmp[xinput1_3.XInputGetState] } }
__declspec(naked) void _XInputSetState() { _asm { jmp[xinput1_3.XInputSetState] } }
__declspec(naked) void _XInputGetStateEx() { _asm { jmp[xinput1_3.XInputGetStateEx] } }
__declspec(naked) void _XInputWaitForGuideButton() { _asm { jmp[xinput1_3.XInputWaitForGuideButton] } }
__declspec(naked) void _XInputCancelGuideButtonWait() { _asm { jmp[xinput1_3.XInputCancelGuideButtonWait] } }
__declspec(naked) void _XInputPowerOffController() { _asm { jmp[xinput1_3.XInputPowerOffController] } }
#endif

#if X64
void _DllMain() { xinput1_3.DllMain(); }
void _XInputEnable() { xinput1_3.XInputEnable(); }
void _XInputGetBatteryInformation() { xinput1_3.XInputGetBatteryInformation(); }
void _XInputGetCapabilities() { xinput1_3.XInputGetCapabilities(); }
void _XInputGetDSoundAudioDeviceGuids() { xinput1_3.XInputGetDSoundAudioDeviceGuids(); }
void _XInputGetKeystroke() { xinput1_3.XInputGetKeystroke(); }
void _XInputGetState() { xinput1_3.XInputGetState(); }
void _XInputSetState() { xinput1_3.XInputSetState(); }
void _XInputGetStateEx() { xinput1_3.XInputGetStateEx(); }
void _XInputWaitForGuideButton() { xinput1_3.XInputWaitForGuideButton(); }
void _XInputCancelGuideButtonWait() { xinput1_3.XInputCancelGuideButtonWait(); }
void _XInputPowerOffController() { xinput1_3.XInputPowerOffController(); }
#endif