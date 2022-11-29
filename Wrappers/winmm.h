#pragma once
#include "wrappers.h"
#include "shared.h"

#undef PlaySound
struct winmm_dll
{
    HMODULE dll;
    FARPROC CloseDriver;
    FARPROC DefDriverProc;
    FARPROC DriverCallback;
    FARPROC DrvGetModuleHandle;
    FARPROC GetDriverModuleHandle;
    FARPROC NotifyCallbackData;
    FARPROC OpenDriver;
    FARPROC PlaySound;
    FARPROC PlaySoundA;
    FARPROC PlaySoundW;
    FARPROC SendDriverMessage;
    FARPROC WOW32DriverCallback;
    FARPROC WOW32ResolveMultiMediaHandle;
    FARPROC WOWAppExit;
    FARPROC aux32Message;
    FARPROC auxGetDevCapsA;
    FARPROC auxGetDevCapsW;
    FARPROC auxGetNumDevs;
    FARPROC auxGetVolume;
    FARPROC auxOutMessage;
    FARPROC auxSetVolume;
    FARPROC joy32Message;
    FARPROC joyConfigChanged;
    FARPROC joyGetDevCapsA;
    FARPROC joyGetDevCapsW;
    FARPROC joyGetNumDevs;
    FARPROC joyGetPos;
    FARPROC joyGetPosEx;
    FARPROC joyGetThreshold;
    FARPROC joyReleaseCapture;
    FARPROC joySetCapture;
    FARPROC joySetThreshold;
    FARPROC mci32Message;
    FARPROC mciDriverNotify;
    FARPROC mciDriverYield;
    FARPROC mciExecute;
    FARPROC mciFreeCommandResource;
    FARPROC mciGetCreatorTask;
    FARPROC mciGetDeviceIDA;
    FARPROC mciGetDeviceIDFromElementIDA;
    FARPROC mciGetDeviceIDFromElementIDW;
    FARPROC mciGetDeviceIDW;
    FARPROC mciGetDriverData;
    FARPROC mciGetErrorStringA;
    FARPROC mciGetErrorStringW;
    FARPROC mciGetYieldProc;
    FARPROC mciLoadCommandResource;
    FARPROC mciSendCommandA;
    FARPROC mciSendCommandW;
    FARPROC mciSendStringA;
    FARPROC mciSendStringW;
    FARPROC mciSetDriverData;
    FARPROC mciSetYieldProc;
    FARPROC mid32Message;
    FARPROC midiConnect;
    FARPROC midiDisconnect;
    FARPROC midiInAddBuffer;
    FARPROC midiInClose;
    FARPROC midiInGetDevCapsA;
    FARPROC midiInGetDevCapsW;
    FARPROC midiInGetErrorTextA;
    FARPROC midiInGetErrorTextW;
    FARPROC midiInGetID;
    FARPROC midiInGetNumDevs;
    FARPROC midiInMessage;
    FARPROC midiInOpen;
    FARPROC midiInPrepareHeader;
    FARPROC midiInReset;
    FARPROC midiInStart;
    FARPROC midiInStop;
    FARPROC midiInUnprepareHeader;
    FARPROC midiOutCacheDrumPatches;
    FARPROC midiOutCachePatches;
    FARPROC midiOutClose;
    FARPROC midiOutGetDevCapsA;
    FARPROC midiOutGetDevCapsW;
    FARPROC midiOutGetErrorTextA;
    FARPROC midiOutGetErrorTextW;
    FARPROC midiOutGetID;
    FARPROC midiOutGetNumDevs;
    FARPROC midiOutGetVolume;
    FARPROC midiOutLongMsg;
    FARPROC midiOutMessage;
    FARPROC midiOutOpen;
    FARPROC midiOutPrepareHeader;
    FARPROC midiOutReset;
    FARPROC midiOutSetVolume;
    FARPROC midiOutShortMsg;
    FARPROC midiOutUnprepareHeader;
    FARPROC midiStreamClose;
    FARPROC midiStreamOpen;
    FARPROC midiStreamOut;
    FARPROC midiStreamPause;
    FARPROC midiStreamPosition;
    FARPROC midiStreamProperty;
    FARPROC midiStreamRestart;
    FARPROC midiStreamStop;
    FARPROC mixerClose;
    FARPROC mixerGetControlDetailsA;
    FARPROC mixerGetControlDetailsW;
    FARPROC mixerGetDevCapsA;
    FARPROC mixerGetDevCapsW;
    FARPROC mixerGetID;
    FARPROC mixerGetLineControlsA;
    FARPROC mixerGetLineControlsW;
    FARPROC mixerGetLineInfoA;
    FARPROC mixerGetLineInfoW;
    FARPROC mixerGetNumDevs;
    FARPROC mixerMessage;
    FARPROC mixerOpen;
    FARPROC mixerSetControlDetails;
    FARPROC mmDrvInstall;
    FARPROC mmGetCurrentTask;
    FARPROC mmTaskBlock;
    FARPROC mmTaskCreate;
    FARPROC mmTaskSignal;
    FARPROC mmTaskYield;
    FARPROC mmioAdvance;
    FARPROC mmioAscend;
    FARPROC mmioClose;
    FARPROC mmioCreateChunk;
    FARPROC mmioDescend;
    FARPROC mmioFlush;
    FARPROC mmioGetInfo;
    FARPROC mmioInstallIOProcA;
    FARPROC mmioInstallIOProcW;
    FARPROC mmioOpenA;
    FARPROC mmioOpenW;
    FARPROC mmioRead;
    FARPROC mmioRenameA;
    FARPROC mmioRenameW;
    FARPROC mmioSeek;
    FARPROC mmioSendMessage;
    FARPROC mmioSetBuffer;
    FARPROC mmioSetInfo;
    FARPROC mmioStringToFOURCCA;
    FARPROC mmioStringToFOURCCW;
    FARPROC mmioWrite;
    FARPROC mmsystemGetVersion;
    FARPROC mod32Message;
    FARPROC mxd32Message;
    FARPROC sndPlaySoundA;
    FARPROC sndPlaySoundW;
    FARPROC tid32Message;
    FARPROC timeBeginPeriod;
    FARPROC timeEndPeriod;
    FARPROC timeGetDevCaps;
    FARPROC timeGetSystemTime;
    FARPROC timeGetTime;
    FARPROC timeKillEvent;
    FARPROC timeSetEvent;
    FARPROC waveInAddBuffer;
    FARPROC waveInClose;
    FARPROC waveInGetDevCapsA;
    FARPROC waveInGetDevCapsW;
    FARPROC waveInGetErrorTextA;
    FARPROC waveInGetErrorTextW;
    FARPROC waveInGetID;
    FARPROC waveInGetNumDevs;
    FARPROC waveInGetPosition;
    FARPROC waveInMessage;
    FARPROC waveInOpen;
    FARPROC waveInPrepareHeader;
    FARPROC waveInReset;
    FARPROC waveInStart;
    FARPROC waveInStop;
    FARPROC waveInUnprepareHeader;
    FARPROC waveOutBreakLoop;
    FARPROC waveOutClose;
    FARPROC waveOutGetDevCapsA;
    FARPROC waveOutGetDevCapsW;
    FARPROC waveOutGetErrorTextA;
    FARPROC waveOutGetErrorTextW;
    FARPROC waveOutGetID;
    FARPROC waveOutGetNumDevs;
    FARPROC waveOutGetPitch;
    FARPROC waveOutGetPlaybackRate;
    FARPROC waveOutGetPosition;
    FARPROC waveOutGetVolume;
    FARPROC waveOutMessage;
    FARPROC waveOutOpen;
    FARPROC waveOutPause;
    FARPROC waveOutPrepareHeader;
    FARPROC waveOutReset;
    FARPROC waveOutRestart;
    FARPROC waveOutSetPitch;
    FARPROC waveOutSetPlaybackRate;
    FARPROC waveOutSetVolume;
    FARPROC waveOutUnprepareHeader;
    FARPROC waveOutWrite;
    FARPROC wid32Message;
    FARPROC wod32Message;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        CloseDriver = GetProcAddress(dll, "CloseDriver");
        DefDriverProc = GetProcAddress(dll, "DefDriverProc");
        DriverCallback = GetProcAddress(dll, "DriverCallback");
        DrvGetModuleHandle = GetProcAddress(dll, "DrvGetModuleHandle");
        GetDriverModuleHandle = GetProcAddress(dll, "GetDriverModuleHandle");
        NotifyCallbackData = GetProcAddress(dll, "NotifyCallbackData");
        OpenDriver = GetProcAddress(dll, "OpenDriver");
        PlaySound = GetProcAddress(dll, "PlaySound");
        PlaySoundA = GetProcAddress(dll, "PlaySoundA");
        PlaySoundW = GetProcAddress(dll, "PlaySoundW");
        SendDriverMessage = GetProcAddress(dll, "SendDriverMessage");
        WOW32DriverCallback = GetProcAddress(dll, "WOW32DriverCallback");
        WOW32ResolveMultiMediaHandle = GetProcAddress(dll, "WOW32ResolveMultiMediaHandle");
        WOWAppExit = GetProcAddress(dll, "WOWAppExit");
        aux32Message = GetProcAddress(dll, "aux32Message");
        auxGetDevCapsA = GetProcAddress(dll, "auxGetDevCapsA");
        auxGetDevCapsW = GetProcAddress(dll, "auxGetDevCapsW");
        auxGetNumDevs = GetProcAddress(dll, "auxGetNumDevs");
        auxGetVolume = GetProcAddress(dll, "auxGetVolume");
        auxOutMessage = GetProcAddress(dll, "auxOutMessage");
        auxSetVolume = GetProcAddress(dll, "auxSetVolume");
        joy32Message = GetProcAddress(dll, "joy32Message");
        joyConfigChanged = GetProcAddress(dll, "joyConfigChanged");
        joyGetDevCapsA = GetProcAddress(dll, "joyGetDevCapsA");
        joyGetDevCapsW = GetProcAddress(dll, "joyGetDevCapsW");
        joyGetNumDevs = GetProcAddress(dll, "joyGetNumDevs");
        joyGetPos = GetProcAddress(dll, "joyGetPos");
        joyGetPosEx = GetProcAddress(dll, "joyGetPosEx");
        joyGetThreshold = GetProcAddress(dll, "joyGetThreshold");
        joyReleaseCapture = GetProcAddress(dll, "joyReleaseCapture");
        joySetCapture = GetProcAddress(dll, "joySetCapture");
        joySetThreshold = GetProcAddress(dll, "joySetThreshold");
        mci32Message = GetProcAddress(dll, "mci32Message");
        mciDriverNotify = GetProcAddress(dll, "mciDriverNotify");
        mciDriverYield = GetProcAddress(dll, "mciDriverYield");
        mciExecute = GetProcAddress(dll, "mciExecute");
        mciFreeCommandResource = GetProcAddress(dll, "mciFreeCommandResource");
        mciGetCreatorTask = GetProcAddress(dll, "mciGetCreatorTask");
        mciGetDeviceIDA = GetProcAddress(dll, "mciGetDeviceIDA");
        mciGetDeviceIDFromElementIDA = GetProcAddress(dll, "mciGetDeviceIDFromElementIDA");
        mciGetDeviceIDFromElementIDW = GetProcAddress(dll, "mciGetDeviceIDFromElementIDW");
        mciGetDeviceIDW = GetProcAddress(dll, "mciGetDeviceIDW");
        mciGetDriverData = GetProcAddress(dll, "mciGetDriverData");
        mciGetErrorStringA = GetProcAddress(dll, "mciGetErrorStringA");
        mciGetErrorStringW = GetProcAddress(dll, "mciGetErrorStringW");
        mciGetYieldProc = GetProcAddress(dll, "mciGetYieldProc");
        mciLoadCommandResource = GetProcAddress(dll, "mciLoadCommandResource");
        mciSendCommandA = GetProcAddress(dll, "mciSendCommandA");
        mciSendCommandW = GetProcAddress(dll, "mciSendCommandW");
        mciSendStringA = GetProcAddress(dll, "mciSendStringA");
        mciSendStringW = GetProcAddress(dll, "mciSendStringW");
        mciSetDriverData = GetProcAddress(dll, "mciSetDriverData");
        mciSetYieldProc = GetProcAddress(dll, "mciSetYieldProc");
        mid32Message = GetProcAddress(dll, "mid32Message");
        midiConnect = GetProcAddress(dll, "midiConnect");
        midiDisconnect = GetProcAddress(dll, "midiDisconnect");
        midiInAddBuffer = GetProcAddress(dll, "midiInAddBuffer");
        midiInClose = GetProcAddress(dll, "midiInClose");
        midiInGetDevCapsA = GetProcAddress(dll, "midiInGetDevCapsA");
        midiInGetDevCapsW = GetProcAddress(dll, "midiInGetDevCapsW");
        midiInGetErrorTextA = GetProcAddress(dll, "midiInGetErrorTextA");
        midiInGetErrorTextW = GetProcAddress(dll, "midiInGetErrorTextW");
        midiInGetID = GetProcAddress(dll, "midiInGetID");
        midiInGetNumDevs = GetProcAddress(dll, "midiInGetNumDevs");
        midiInMessage = GetProcAddress(dll, "midiInMessage");
        midiInOpen = GetProcAddress(dll, "midiInOpen");
        midiInPrepareHeader = GetProcAddress(dll, "midiInPrepareHeader");
        midiInReset = GetProcAddress(dll, "midiInReset");
        midiInStart = GetProcAddress(dll, "midiInStart");
        midiInStop = GetProcAddress(dll, "midiInStop");
        midiInUnprepareHeader = GetProcAddress(dll, "midiInUnprepareHeader");
        midiOutCacheDrumPatches = GetProcAddress(dll, "midiOutCacheDrumPatches");
        midiOutCachePatches = GetProcAddress(dll, "midiOutCachePatches");
        midiOutClose = GetProcAddress(dll, "midiOutClose");
        midiOutGetDevCapsA = GetProcAddress(dll, "midiOutGetDevCapsA");
        midiOutGetDevCapsW = GetProcAddress(dll, "midiOutGetDevCapsW");
        midiOutGetErrorTextA = GetProcAddress(dll, "midiOutGetErrorTextA");
        midiOutGetErrorTextW = GetProcAddress(dll, "midiOutGetErrorTextW");
        midiOutGetID = GetProcAddress(dll, "midiOutGetID");
        midiOutGetNumDevs = GetProcAddress(dll, "midiOutGetNumDevs");
        midiOutGetVolume = GetProcAddress(dll, "midiOutGetVolume");
        midiOutLongMsg = GetProcAddress(dll, "midiOutLongMsg");
        midiOutMessage = GetProcAddress(dll, "midiOutMessage");
        midiOutOpen = GetProcAddress(dll, "midiOutOpen");
        midiOutPrepareHeader = GetProcAddress(dll, "midiOutPrepareHeader");
        midiOutReset = GetProcAddress(dll, "midiOutReset");
        midiOutSetVolume = GetProcAddress(dll, "midiOutSetVolume");
        midiOutShortMsg = GetProcAddress(dll, "midiOutShortMsg");
        midiOutUnprepareHeader = GetProcAddress(dll, "midiOutUnprepareHeader");
        midiStreamClose = GetProcAddress(dll, "midiStreamClose");
        midiStreamOpen = GetProcAddress(dll, "midiStreamOpen");
        midiStreamOut = GetProcAddress(dll, "midiStreamOut");
        midiStreamPause = GetProcAddress(dll, "midiStreamPause");
        midiStreamPosition = GetProcAddress(dll, "midiStreamPosition");
        midiStreamProperty = GetProcAddress(dll, "midiStreamProperty");
        midiStreamRestart = GetProcAddress(dll, "midiStreamRestart");
        midiStreamStop = GetProcAddress(dll, "midiStreamStop");
        mixerClose = GetProcAddress(dll, "mixerClose");
        mixerGetControlDetailsA = GetProcAddress(dll, "mixerGetControlDetailsA");
        mixerGetControlDetailsW = GetProcAddress(dll, "mixerGetControlDetailsW");
        mixerGetDevCapsA = GetProcAddress(dll, "mixerGetDevCapsA");
        mixerGetDevCapsW = GetProcAddress(dll, "mixerGetDevCapsW");
        mixerGetID = GetProcAddress(dll, "mixerGetID");
        mixerGetLineControlsA = GetProcAddress(dll, "mixerGetLineControlsA");
        mixerGetLineControlsW = GetProcAddress(dll, "mixerGetLineControlsW");
        mixerGetLineInfoA = GetProcAddress(dll, "mixerGetLineInfoA");
        mixerGetLineInfoW = GetProcAddress(dll, "mixerGetLineInfoW");
        mixerGetNumDevs = GetProcAddress(dll, "mixerGetNumDevs");
        mixerMessage = GetProcAddress(dll, "mixerMessage");
        mixerOpen = GetProcAddress(dll, "mixerOpen");
        mixerSetControlDetails = GetProcAddress(dll, "mixerSetControlDetails");
        mmDrvInstall = GetProcAddress(dll, "mmDrvInstall");
        mmGetCurrentTask = GetProcAddress(dll, "mmGetCurrentTask");
        mmTaskBlock = GetProcAddress(dll, "mmTaskBlock");
        mmTaskCreate = GetProcAddress(dll, "mmTaskCreate");
        mmTaskSignal = GetProcAddress(dll, "mmTaskSignal");
        mmTaskYield = GetProcAddress(dll, "mmTaskYield");
        mmioAdvance = GetProcAddress(dll, "mmioAdvance");
        mmioAscend = GetProcAddress(dll, "mmioAscend");
        mmioClose = GetProcAddress(dll, "mmioClose");
        mmioCreateChunk = GetProcAddress(dll, "mmioCreateChunk");
        mmioDescend = GetProcAddress(dll, "mmioDescend");
        mmioFlush = GetProcAddress(dll, "mmioFlush");
        mmioGetInfo = GetProcAddress(dll, "mmioGetInfo");
        mmioInstallIOProcA = GetProcAddress(dll, "mmioInstallIOProcA");
        mmioInstallIOProcW = GetProcAddress(dll, "mmioInstallIOProcW");
        mmioOpenA = GetProcAddress(dll, "mmioOpenA");
        mmioOpenW = GetProcAddress(dll, "mmioOpenW");
        mmioRead = GetProcAddress(dll, "mmioRead");
        mmioRenameA = GetProcAddress(dll, "mmioRenameA");
        mmioRenameW = GetProcAddress(dll, "mmioRenameW");
        mmioSeek = GetProcAddress(dll, "mmioSeek");
        mmioSendMessage = GetProcAddress(dll, "mmioSendMessage");
        mmioSetBuffer = GetProcAddress(dll, "mmioSetBuffer");
        mmioSetInfo = GetProcAddress(dll, "mmioSetInfo");
        mmioStringToFOURCCA = GetProcAddress(dll, "mmioStringToFOURCCA");
        mmioStringToFOURCCW = GetProcAddress(dll, "mmioStringToFOURCCW");
        mmioWrite = GetProcAddress(dll, "mmioWrite");
        mmsystemGetVersion = GetProcAddress(dll, "mmsystemGetVersion");
        mod32Message = GetProcAddress(dll, "mod32Message");
        mxd32Message = GetProcAddress(dll, "mxd32Message");
        sndPlaySoundA = GetProcAddress(dll, "sndPlaySoundA");
        sndPlaySoundW = GetProcAddress(dll, "sndPlaySoundW");
        tid32Message = GetProcAddress(dll, "tid32Message");
        timeBeginPeriod = GetProcAddress(dll, "timeBeginPeriod");
        timeEndPeriod = GetProcAddress(dll, "timeEndPeriod");
        timeGetDevCaps = GetProcAddress(dll, "timeGetDevCaps");
        timeGetSystemTime = GetProcAddress(dll, "timeGetSystemTime");
        timeGetTime = GetProcAddress(dll, "timeGetTime");
        timeKillEvent = GetProcAddress(dll, "timeKillEvent");
        timeSetEvent = GetProcAddress(dll, "timeSetEvent");
        waveInAddBuffer = GetProcAddress(dll, "waveInAddBuffer");
        waveInClose = GetProcAddress(dll, "waveInClose");
        waveInGetDevCapsA = GetProcAddress(dll, "waveInGetDevCapsA");
        waveInGetDevCapsW = GetProcAddress(dll, "waveInGetDevCapsW");
        waveInGetErrorTextA = GetProcAddress(dll, "waveInGetErrorTextA");
        waveInGetErrorTextW = GetProcAddress(dll, "waveInGetErrorTextW");
        waveInGetID = GetProcAddress(dll, "waveInGetID");
        waveInGetNumDevs = GetProcAddress(dll, "waveInGetNumDevs");
        waveInGetPosition = GetProcAddress(dll, "waveInGetPosition");
        waveInMessage = GetProcAddress(dll, "waveInMessage");
        waveInOpen = GetProcAddress(dll, "waveInOpen");
        waveInPrepareHeader = GetProcAddress(dll, "waveInPrepareHeader");
        waveInReset = GetProcAddress(dll, "waveInReset");
        waveInStart = GetProcAddress(dll, "waveInStart");
        waveInStop = GetProcAddress(dll, "waveInStop");
        waveInUnprepareHeader = GetProcAddress(dll, "waveInUnprepareHeader");
        waveOutBreakLoop = GetProcAddress(dll, "waveOutBreakLoop");
        waveOutClose = GetProcAddress(dll, "waveOutClose");
        waveOutGetDevCapsA = GetProcAddress(dll, "waveOutGetDevCapsA");
        waveOutGetDevCapsW = GetProcAddress(dll, "waveOutGetDevCapsW");
        waveOutGetErrorTextA = GetProcAddress(dll, "waveOutGetErrorTextA");
        waveOutGetErrorTextW = GetProcAddress(dll, "waveOutGetErrorTextW");
        waveOutGetID = GetProcAddress(dll, "waveOutGetID");
        waveOutGetNumDevs = GetProcAddress(dll, "waveOutGetNumDevs");
        waveOutGetPitch = GetProcAddress(dll, "waveOutGetPitch");
        waveOutGetPlaybackRate = GetProcAddress(dll, "waveOutGetPlaybackRate");
        waveOutGetPosition = GetProcAddress(dll, "waveOutGetPosition");
        waveOutGetVolume = GetProcAddress(dll, "waveOutGetVolume");
        waveOutMessage = GetProcAddress(dll, "waveOutMessage");
        waveOutOpen = GetProcAddress(dll, "waveOutOpen");
        waveOutPause = GetProcAddress(dll, "waveOutPause");
        waveOutPrepareHeader = GetProcAddress(dll, "waveOutPrepareHeader");
        waveOutReset = GetProcAddress(dll, "waveOutReset");
        waveOutRestart = GetProcAddress(dll, "waveOutRestart");
        waveOutSetPitch = GetProcAddress(dll, "waveOutSetPitch");
        waveOutSetPlaybackRate = GetProcAddress(dll, "waveOutSetPlaybackRate");
        waveOutSetVolume = GetProcAddress(dll, "waveOutSetVolume");
        waveOutUnprepareHeader = GetProcAddress(dll, "waveOutUnprepareHeader");
        waveOutWrite = GetProcAddress(dll, "waveOutWrite");
        wid32Message = GetProcAddress(dll, "wid32Message");
        wod32Message = GetProcAddress(dll, "wod32Message");
    }
} winmm;

#if !X64
__declspec(naked) void _CloseDriver() { _asm { jmp[winmm.CloseDriver] } }
__declspec(naked) void _DefDriverProc() { _asm { jmp[winmm.DefDriverProc] } }
__declspec(naked) void _DriverCallback() { _asm { jmp[winmm.DriverCallback] } }
__declspec(naked) void _DrvGetModuleHandle() { _asm { jmp[winmm.DrvGetModuleHandle] } }
__declspec(naked) void _GetDriverModuleHandle() { _asm { jmp[winmm.GetDriverModuleHandle] } }
__declspec(naked) void _NotifyCallbackData() { _asm { jmp[winmm.NotifyCallbackData] } }
__declspec(naked) void _OpenDriver() { _asm { jmp[winmm.OpenDriver] } }
__declspec(naked) void _PlaySound() { _asm { jmp[winmm.PlaySound] } }
__declspec(naked) void _PlaySoundA() { _asm { jmp[winmm.PlaySoundA] } }
__declspec(naked) void _PlaySoundW() { _asm { jmp[winmm.PlaySoundW] } }
__declspec(naked) void _SendDriverMessage() { _asm { jmp[winmm.SendDriverMessage] } }
__declspec(naked) void _WOW32DriverCallback() { _asm { jmp[winmm.WOW32DriverCallback] } }
__declspec(naked) void _WOW32ResolveMultiMediaHandle() { _asm { jmp[winmm.WOW32ResolveMultiMediaHandle] } }
__declspec(naked) void _WOWAppExit() { _asm { jmp[winmm.WOWAppExit] } }
__declspec(naked) void _aux32Message() { _asm { jmp[winmm.aux32Message] } }
__declspec(naked) void _auxGetDevCapsA() { _asm { jmp[winmm.auxGetDevCapsA] } }
__declspec(naked) void _auxGetDevCapsW() { _asm { jmp[winmm.auxGetDevCapsW] } }
__declspec(naked) void _auxGetNumDevs() { _asm { jmp[winmm.auxGetNumDevs] } }
__declspec(naked) void _auxGetVolume() { _asm { jmp[winmm.auxGetVolume] } }
__declspec(naked) void _auxOutMessage() { _asm { jmp[winmm.auxOutMessage] } }
__declspec(naked) void _auxSetVolume() { _asm { jmp[winmm.auxSetVolume] } }
__declspec(naked) void _joy32Message() { _asm { jmp[winmm.joy32Message] } }
__declspec(naked) void _joyConfigChanged() { _asm { jmp[winmm.joyConfigChanged] } }
__declspec(naked) void _joyGetDevCapsA() { _asm { jmp[winmm.joyGetDevCapsA] } }
__declspec(naked) void _joyGetDevCapsW() { _asm { jmp[winmm.joyGetDevCapsW] } }
__declspec(naked) void _joyGetNumDevs() { _asm { jmp[winmm.joyGetNumDevs] } }
__declspec(naked) void _joyGetPos() { _asm { jmp[winmm.joyGetPos] } }
__declspec(naked) void _joyGetPosEx() { _asm { jmp[winmm.joyGetPosEx] } }
__declspec(naked) void _joyGetThreshold() { _asm { jmp[winmm.joyGetThreshold] } }
__declspec(naked) void _joyReleaseCapture() { _asm { jmp[winmm.joyReleaseCapture] } }
__declspec(naked) void _joySetCapture() { _asm { jmp[winmm.joySetCapture] } }
__declspec(naked) void _joySetThreshold() { _asm { jmp[winmm.joySetThreshold] } }
__declspec(naked) void _mci32Message() { _asm { jmp[winmm.mci32Message] } }
__declspec(naked) void _mciDriverNotify() { _asm { jmp[winmm.mciDriverNotify] } }
__declspec(naked) void _mciDriverYield() { _asm { jmp[winmm.mciDriverYield] } }
__declspec(naked) void _mciExecute() { _asm { jmp[winmm.mciExecute] } }
__declspec(naked) void _mciFreeCommandResource() { _asm { jmp[winmm.mciFreeCommandResource] } }
__declspec(naked) void _mciGetCreatorTask() { _asm { jmp[winmm.mciGetCreatorTask] } }
__declspec(naked) void _mciGetDeviceIDA() { _asm { jmp[winmm.mciGetDeviceIDA] } }
__declspec(naked) void _mciGetDeviceIDFromElementIDA() { _asm { jmp[winmm.mciGetDeviceIDFromElementIDA] } }
__declspec(naked) void _mciGetDeviceIDFromElementIDW() { _asm { jmp[winmm.mciGetDeviceIDFromElementIDW] } }
__declspec(naked) void _mciGetDeviceIDW() { _asm { jmp[winmm.mciGetDeviceIDW] } }
__declspec(naked) void _mciGetDriverData() { _asm { jmp[winmm.mciGetDriverData] } }
__declspec(naked) void _mciGetErrorStringA() { _asm { jmp[winmm.mciGetErrorStringA] } }
__declspec(naked) void _mciGetErrorStringW() { _asm { jmp[winmm.mciGetErrorStringW] } }
__declspec(naked) void _mciGetYieldProc() { _asm { jmp[winmm.mciGetYieldProc] } }
__declspec(naked) void _mciLoadCommandResource() { _asm { jmp[winmm.mciLoadCommandResource] } }
__declspec(naked) void _mciSendCommandA() { _asm { jmp[winmm.mciSendCommandA] } }
__declspec(naked) void _mciSendCommandW() { _asm { jmp[winmm.mciSendCommandW] } }
__declspec(naked) void _mciSendStringA() { _asm { jmp[winmm.mciSendStringA] } }
__declspec(naked) void _mciSendStringW() { _asm { jmp[winmm.mciSendStringW] } }
__declspec(naked) void _mciSetDriverData() { _asm { jmp[winmm.mciSetDriverData] } }
__declspec(naked) void _mciSetYieldProc() { _asm { jmp[winmm.mciSetYieldProc] } }
__declspec(naked) void _mid32Message() { _asm { jmp[winmm.mid32Message] } }
__declspec(naked) void _midiConnect() { _asm { jmp[winmm.midiConnect] } }
__declspec(naked) void _midiDisconnect() { _asm { jmp[winmm.midiDisconnect] } }
__declspec(naked) void _midiInAddBuffer() { _asm { jmp[winmm.midiInAddBuffer] } }
__declspec(naked) void _midiInClose() { _asm { jmp[winmm.midiInClose] } }
__declspec(naked) void _midiInGetDevCapsA() { _asm { jmp[winmm.midiInGetDevCapsA] } }
__declspec(naked) void _midiInGetDevCapsW() { _asm { jmp[winmm.midiInGetDevCapsW] } }
__declspec(naked) void _midiInGetErrorTextA() { _asm { jmp[winmm.midiInGetErrorTextA] } }
__declspec(naked) void _midiInGetErrorTextW() { _asm { jmp[winmm.midiInGetErrorTextW] } }
__declspec(naked) void _midiInGetID() { _asm { jmp[winmm.midiInGetID] } }
__declspec(naked) void _midiInGetNumDevs() { _asm { jmp[winmm.midiInGetNumDevs] } }
__declspec(naked) void _midiInMessage() { _asm { jmp[winmm.midiInMessage] } }
__declspec(naked) void _midiInOpen() { _asm { jmp[winmm.midiInOpen] } }
__declspec(naked) void _midiInPrepareHeader() { _asm { jmp[winmm.midiInPrepareHeader] } }
__declspec(naked) void _midiInReset() { _asm { jmp[winmm.midiInReset] } }
__declspec(naked) void _midiInStart() { _asm { jmp[winmm.midiInStart] } }
__declspec(naked) void _midiInStop() { _asm { jmp[winmm.midiInStop] } }
__declspec(naked) void _midiInUnprepareHeader() { _asm { jmp[winmm.midiInUnprepareHeader] } }
__declspec(naked) void _midiOutCacheDrumPatches() { _asm { jmp[winmm.midiOutCacheDrumPatches] } }
__declspec(naked) void _midiOutCachePatches() { _asm { jmp[winmm.midiOutCachePatches] } }
__declspec(naked) void _midiOutClose() { _asm { jmp[winmm.midiOutClose] } }
__declspec(naked) void _midiOutGetDevCapsA() { _asm { jmp[winmm.midiOutGetDevCapsA] } }
__declspec(naked) void _midiOutGetDevCapsW() { _asm { jmp[winmm.midiOutGetDevCapsW] } }
__declspec(naked) void _midiOutGetErrorTextA() { _asm { jmp[winmm.midiOutGetErrorTextA] } }
__declspec(naked) void _midiOutGetErrorTextW() { _asm { jmp[winmm.midiOutGetErrorTextW] } }
__declspec(naked) void _midiOutGetID() { _asm { jmp[winmm.midiOutGetID] } }
__declspec(naked) void _midiOutGetNumDevs() { _asm { jmp[winmm.midiOutGetNumDevs] } }
__declspec(naked) void _midiOutGetVolume() { _asm { jmp[winmm.midiOutGetVolume] } }
__declspec(naked) void _midiOutLongMsg() { _asm { jmp[winmm.midiOutLongMsg] } }
__declspec(naked) void _midiOutMessage() { _asm { jmp[winmm.midiOutMessage] } }
__declspec(naked) void _midiOutOpen() { _asm { jmp[winmm.midiOutOpen] } }
__declspec(naked) void _midiOutPrepareHeader() { _asm { jmp[winmm.midiOutPrepareHeader] } }
__declspec(naked) void _midiOutReset() { _asm { jmp[winmm.midiOutReset] } }
__declspec(naked) void _midiOutSetVolume() { _asm { jmp[winmm.midiOutSetVolume] } }
__declspec(naked) void _midiOutShortMsg() { _asm { jmp[winmm.midiOutShortMsg] } }
__declspec(naked) void _midiOutUnprepareHeader() { _asm { jmp[winmm.midiOutUnprepareHeader] } }
__declspec(naked) void _midiStreamClose() { _asm { jmp[winmm.midiStreamClose] } }
__declspec(naked) void _midiStreamOpen() { _asm { jmp[winmm.midiStreamOpen] } }
__declspec(naked) void _midiStreamOut() { _asm { jmp[winmm.midiStreamOut] } }
__declspec(naked) void _midiStreamPause() { _asm { jmp[winmm.midiStreamPause] } }
__declspec(naked) void _midiStreamPosition() { _asm { jmp[winmm.midiStreamPosition] } }
__declspec(naked) void _midiStreamProperty() { _asm { jmp[winmm.midiStreamProperty] } }
__declspec(naked) void _midiStreamRestart() { _asm { jmp[winmm.midiStreamRestart] } }
__declspec(naked) void _midiStreamStop() { _asm { jmp[winmm.midiStreamStop] } }
__declspec(naked) void _mixerClose() { _asm { jmp[winmm.mixerClose] } }
__declspec(naked) void _mixerGetControlDetailsA() { _asm { jmp[winmm.mixerGetControlDetailsA] } }
__declspec(naked) void _mixerGetControlDetailsW() { _asm { jmp[winmm.mixerGetControlDetailsW] } }
__declspec(naked) void _mixerGetDevCapsA() { _asm { jmp[winmm.mixerGetDevCapsA] } }
__declspec(naked) void _mixerGetDevCapsW() { _asm { jmp[winmm.mixerGetDevCapsW] } }
__declspec(naked) void _mixerGetID() { _asm { jmp[winmm.mixerGetID] } }
__declspec(naked) void _mixerGetLineControlsA() { _asm { jmp[winmm.mixerGetLineControlsA] } }
__declspec(naked) void _mixerGetLineControlsW() { _asm { jmp[winmm.mixerGetLineControlsW] } }
__declspec(naked) void _mixerGetLineInfoA() { _asm { jmp[winmm.mixerGetLineInfoA] } }
__declspec(naked) void _mixerGetLineInfoW() { _asm { jmp[winmm.mixerGetLineInfoW] } }
__declspec(naked) void _mixerGetNumDevs() { _asm { jmp[winmm.mixerGetNumDevs] } }
__declspec(naked) void _mixerMessage() { _asm { jmp[winmm.mixerMessage] } }
__declspec(naked) void _mixerOpen() { _asm { jmp[winmm.mixerOpen] } }
__declspec(naked) void _mixerSetControlDetails() { _asm { jmp[winmm.mixerSetControlDetails] } }
__declspec(naked) void _mmDrvInstall() { _asm { jmp[winmm.mmDrvInstall] } }
__declspec(naked) void _mmGetCurrentTask() { _asm { jmp[winmm.mmGetCurrentTask] } }
__declspec(naked) void _mmTaskBlock() { _asm { jmp[winmm.mmTaskBlock] } }
__declspec(naked) void _mmTaskCreate() { _asm { jmp[winmm.mmTaskCreate] } }
__declspec(naked) void _mmTaskSignal() { _asm { jmp[winmm.mmTaskSignal] } }
__declspec(naked) void _mmTaskYield() { _asm { jmp[winmm.mmTaskYield] } }
__declspec(naked) void _mmioAdvance() { _asm { jmp[winmm.mmioAdvance] } }
__declspec(naked) void _mmioAscend() { _asm { jmp[winmm.mmioAscend] } }
__declspec(naked) void _mmioClose() { _asm { jmp[winmm.mmioClose] } }
__declspec(naked) void _mmioCreateChunk() { _asm { jmp[winmm.mmioCreateChunk] } }
__declspec(naked) void _mmioDescend() { _asm { jmp[winmm.mmioDescend] } }
__declspec(naked) void _mmioFlush() { _asm { jmp[winmm.mmioFlush] } }
__declspec(naked) void _mmioGetInfo() { _asm { jmp[winmm.mmioGetInfo] } }
__declspec(naked) void _mmioInstallIOProcA() { _asm { jmp[winmm.mmioInstallIOProcA] } }
__declspec(naked) void _mmioInstallIOProcW() { _asm { jmp[winmm.mmioInstallIOProcW] } }
__declspec(naked) void _mmioOpenA() { _asm { jmp[winmm.mmioOpenA] } }
__declspec(naked) void _mmioOpenW() { _asm { jmp[winmm.mmioOpenW] } }
__declspec(naked) void _mmioRead() { _asm { jmp[winmm.mmioRead] } }
__declspec(naked) void _mmioRenameA() { _asm { jmp[winmm.mmioRenameA] } }
__declspec(naked) void _mmioRenameW() { _asm { jmp[winmm.mmioRenameW] } }
__declspec(naked) void _mmioSeek() { _asm { jmp[winmm.mmioSeek] } }
__declspec(naked) void _mmioSendMessage() { _asm { jmp[winmm.mmioSendMessage] } }
__declspec(naked) void _mmioSetBuffer() { _asm { jmp[winmm.mmioSetBuffer] } }
__declspec(naked) void _mmioSetInfo() { _asm { jmp[winmm.mmioSetInfo] } }
__declspec(naked) void _mmioStringToFOURCCA() { _asm { jmp[winmm.mmioStringToFOURCCA] } }
__declspec(naked) void _mmioStringToFOURCCW() { _asm { jmp[winmm.mmioStringToFOURCCW] } }
__declspec(naked) void _mmioWrite() { _asm { jmp[winmm.mmioWrite] } }
__declspec(naked) void _mmsystemGetVersion() { _asm { jmp[winmm.mmsystemGetVersion] } }
__declspec(naked) void _mod32Message() { _asm { jmp[winmm.mod32Message] } }
__declspec(naked) void _mxd32Message() { _asm { jmp[winmm.mxd32Message] } }
__declspec(naked) void _sndPlaySoundA() { _asm { jmp[winmm.sndPlaySoundA] } }
__declspec(naked) void _sndPlaySoundW() { _asm { jmp[winmm.sndPlaySoundW] } }
__declspec(naked) void _tid32Message() { _asm { jmp[winmm.tid32Message] } }
__declspec(naked) void _timeBeginPeriod() { _asm { jmp[winmm.timeBeginPeriod] } }
__declspec(naked) void _timeEndPeriod() { _asm { jmp[winmm.timeEndPeriod] } }
__declspec(naked) void _timeGetDevCaps() { _asm { jmp[winmm.timeGetDevCaps] } }
__declspec(naked) void _timeGetSystemTime() { _asm { jmp[winmm.timeGetSystemTime] } }
__declspec(naked) void _timeGetTime() { _asm { jmp[winmm.timeGetTime] } }
__declspec(naked) void _timeKillEvent() { _asm { jmp[winmm.timeKillEvent] } }
__declspec(naked) void _timeSetEvent() { _asm { jmp[winmm.timeSetEvent] } }
__declspec(naked) void _waveInAddBuffer() { _asm { jmp[winmm.waveInAddBuffer] } }
__declspec(naked) void _waveInClose() { _asm { jmp[winmm.waveInClose] } }
__declspec(naked) void _waveInGetDevCapsA() { _asm { jmp[winmm.waveInGetDevCapsA] } }
__declspec(naked) void _waveInGetDevCapsW() { _asm { jmp[winmm.waveInGetDevCapsW] } }
__declspec(naked) void _waveInGetErrorTextA() { _asm { jmp[winmm.waveInGetErrorTextA] } }
__declspec(naked) void _waveInGetErrorTextW() { _asm { jmp[winmm.waveInGetErrorTextW] } }
__declspec(naked) void _waveInGetID() { _asm { jmp[winmm.waveInGetID] } }
__declspec(naked) void _waveInGetNumDevs() { _asm { jmp[winmm.waveInGetNumDevs] } }
__declspec(naked) void _waveInGetPosition() { _asm { jmp[winmm.waveInGetPosition] } }
__declspec(naked) void _waveInMessage() { _asm { jmp[winmm.waveInMessage] } }
__declspec(naked) void _waveInOpen() { _asm { jmp[winmm.waveInOpen] } }
__declspec(naked) void _waveInPrepareHeader() { _asm { jmp[winmm.waveInPrepareHeader] } }
__declspec(naked) void _waveInReset() { _asm { jmp[winmm.waveInReset] } }
__declspec(naked) void _waveInStart() { _asm { jmp[winmm.waveInStart] } }
__declspec(naked) void _waveInStop() { _asm { jmp[winmm.waveInStop] } }
__declspec(naked) void _waveInUnprepareHeader() { _asm { jmp[winmm.waveInUnprepareHeader] } }
__declspec(naked) void _waveOutBreakLoop() { _asm { jmp[winmm.waveOutBreakLoop] } }
__declspec(naked) void _waveOutClose() { _asm { jmp[winmm.waveOutClose] } }
__declspec(naked) void _waveOutGetDevCapsA() { _asm { jmp[winmm.waveOutGetDevCapsA] } }
__declspec(naked) void _waveOutGetDevCapsW() { _asm { jmp[winmm.waveOutGetDevCapsW] } }
__declspec(naked) void _waveOutGetErrorTextA() { _asm { jmp[winmm.waveOutGetErrorTextA] } }
__declspec(naked) void _waveOutGetErrorTextW() { _asm { jmp[winmm.waveOutGetErrorTextW] } }
__declspec(naked) void _waveOutGetID() { _asm { jmp[winmm.waveOutGetID] } }
__declspec(naked) void _waveOutGetNumDevs() { _asm { jmp[winmm.waveOutGetNumDevs] } }
__declspec(naked) void _waveOutGetPitch() { _asm { jmp[winmm.waveOutGetPitch] } }
__declspec(naked) void _waveOutGetPlaybackRate() { _asm { jmp[winmm.waveOutGetPlaybackRate] } }
__declspec(naked) void _waveOutGetPosition() { _asm { jmp[winmm.waveOutGetPosition] } }
__declspec(naked) void _waveOutGetVolume() { _asm { jmp[winmm.waveOutGetVolume] } }
__declspec(naked) void _waveOutMessage() { _asm { jmp[winmm.waveOutMessage] } }
__declspec(naked) void _waveOutOpen() { _asm { jmp[winmm.waveOutOpen] } }
__declspec(naked) void _waveOutPause() { _asm { jmp[winmm.waveOutPause] } }
__declspec(naked) void _waveOutPrepareHeader() { _asm { jmp[winmm.waveOutPrepareHeader] } }
__declspec(naked) void _waveOutReset() { _asm { jmp[winmm.waveOutReset] } }
__declspec(naked) void _waveOutRestart() { _asm { jmp[winmm.waveOutRestart] } }
__declspec(naked) void _waveOutSetPitch() { _asm { jmp[winmm.waveOutSetPitch] } }
__declspec(naked) void _waveOutSetPlaybackRate() { _asm { jmp[winmm.waveOutSetPlaybackRate] } }
__declspec(naked) void _waveOutSetVolume() { _asm { jmp[winmm.waveOutSetVolume] } }
__declspec(naked) void _waveOutUnprepareHeader() { _asm { jmp[winmm.waveOutUnprepareHeader] } }
__declspec(naked) void _waveOutWrite() { _asm { jmp[winmm.waveOutWrite] } }
__declspec(naked) void _wid32Message() { _asm { jmp[winmm.wid32Message] } }
__declspec(naked) void _wod32Message() { _asm { jmp[winmm.wod32Message] } }
#endif

#if X64
void _CloseDriver() { winmm.CloseDriver(); }
void _DefDriverProc() { winmm.DefDriverProc(); }
void _DriverCallback() { winmm.DriverCallback(); }
void _DrvGetModuleHandle() { winmm.DrvGetModuleHandle(); }
void _GetDriverModuleHandle() { winmm.GetDriverModuleHandle(); }
void _NotifyCallbackData() { winmm.NotifyCallbackData(); }
void _OpenDriver() { winmm.OpenDriver(); }
void _PlaySound() { winmm.PlaySound(); }
void _PlaySoundA() { winmm.PlaySoundA(); }
void _PlaySoundW() { winmm.PlaySoundW(); }
void _SendDriverMessage() { winmm.SendDriverMessage(); }
void _WOW32DriverCallback() { winmm.WOW32DriverCallback(); }
void _WOW32ResolveMultiMediaHandle() { winmm.WOW32ResolveMultiMediaHandle(); }
void _WOWAppExit() { winmm.WOWAppExit(); }
void _aux32Message() { winmm.aux32Message(); }
void _auxGetDevCapsA() { winmm.auxGetDevCapsA(); }
void _auxGetDevCapsW() { winmm.auxGetDevCapsW(); }
void _auxGetNumDevs() { winmm.auxGetNumDevs(); }
void _auxGetVolume() { winmm.auxGetVolume(); }
void _auxOutMessage() { winmm.auxOutMessage(); }
void _auxSetVolume() { winmm.auxSetVolume(); }
void _joy32Message() { winmm.joy32Message(); }
void _joyConfigChanged() { winmm.joyConfigChanged(); }
void _joyGetDevCapsA() { winmm.joyGetDevCapsA(); }
void _joyGetDevCapsW() { winmm.joyGetDevCapsW(); }
void _joyGetNumDevs() { winmm.joyGetNumDevs(); }
void _joyGetPos() { winmm.joyGetPos(); }
void _joyGetPosEx() { winmm.joyGetPosEx(); }
void _joyGetThreshold() { winmm.joyGetThreshold(); }
void _joyReleaseCapture() { winmm.joyReleaseCapture(); }
void _joySetCapture() { winmm.joySetCapture(); }
void _joySetThreshold() { winmm.joySetThreshold(); }
void _mci32Message() { winmm.mci32Message(); }
void _mciDriverNotify() { winmm.mciDriverNotify(); }
void _mciDriverYield() { winmm.mciDriverYield(); }
void _mciExecute() { winmm.mciExecute(); }
void _mciFreeCommandResource() { winmm.mciFreeCommandResource(); }
void _mciGetCreatorTask() { winmm.mciGetCreatorTask(); }
void _mciGetDeviceIDA() { winmm.mciGetDeviceIDA(); }
void _mciGetDeviceIDFromElementIDA() { winmm.mciGetDeviceIDFromElementIDA(); }
void _mciGetDeviceIDFromElementIDW() { winmm.mciGetDeviceIDFromElementIDW(); }
void _mciGetDeviceIDW() { winmm.mciGetDeviceIDW(); }
void _mciGetDriverData() { winmm.mciGetDriverData(); }
void _mciGetErrorStringA() { winmm.mciGetErrorStringA(); }
void _mciGetErrorStringW() { winmm.mciGetErrorStringW(); }
void _mciGetYieldProc() { winmm.mciGetYieldProc(); }
void _mciLoadCommandResource() { winmm.mciLoadCommandResource(); }
void _mciSendCommandA() { winmm.mciSendCommandA(); }
void _mciSendCommandW() { winmm.mciSendCommandW(); }
void _mciSendStringA() { winmm.mciSendStringA(); }
void _mciSendStringW() { winmm.mciSendStringW(); }
void _mciSetDriverData() { winmm.mciSetDriverData(); }
void _mciSetYieldProc() { winmm.mciSetYieldProc(); }
void _mid32Message() { winmm.mid32Message(); }
void _midiConnect() { winmm.midiConnect(); }
void _midiDisconnect() { winmm.midiDisconnect(); }
void _midiInAddBuffer() { winmm.midiInAddBuffer(); }
void _midiInClose() { winmm.midiInClose(); }
void _midiInGetDevCapsA() { winmm.midiInGetDevCapsA(); }
void _midiInGetDevCapsW() { winmm.midiInGetDevCapsW(); }
void _midiInGetErrorTextA() { winmm.midiInGetErrorTextA(); }
void _midiInGetErrorTextW() { winmm.midiInGetErrorTextW(); }
void _midiInGetID() { winmm.midiInGetID(); }
void _midiInGetNumDevs() { winmm.midiInGetNumDevs(); }
void _midiInMessage() { winmm.midiInMessage(); }
void _midiInOpen() { winmm.midiInOpen(); }
void _midiInPrepareHeader() { winmm.midiInPrepareHeader(); }
void _midiInReset() { winmm.midiInReset(); }
void _midiInStart() { winmm.midiInStart(); }
void _midiInStop() { winmm.midiInStop(); }
void _midiInUnprepareHeader() { winmm.midiInUnprepareHeader(); }
void _midiOutCacheDrumPatches() { winmm.midiOutCacheDrumPatches(); }
void _midiOutCachePatches() { winmm.midiOutCachePatches(); }
void _midiOutClose() { winmm.midiOutClose(); }
void _midiOutGetDevCapsA() { winmm.midiOutGetDevCapsA(); }
void _midiOutGetDevCapsW() { winmm.midiOutGetDevCapsW(); }
void _midiOutGetErrorTextA() { winmm.midiOutGetErrorTextA(); }
void _midiOutGetErrorTextW() { winmm.midiOutGetErrorTextW(); }
void _midiOutGetID() { winmm.midiOutGetID(); }
void _midiOutGetNumDevs() { winmm.midiOutGetNumDevs(); }
void _midiOutGetVolume() { winmm.midiOutGetVolume(); }
void _midiOutLongMsg() { winmm.midiOutLongMsg(); }
void _midiOutMessage() { winmm.midiOutMessage(); }
void _midiOutOpen() { winmm.midiOutOpen(); }
void _midiOutPrepareHeader() { winmm.midiOutPrepareHeader(); }
void _midiOutReset() { winmm.midiOutReset(); }
void _midiOutSetVolume() { winmm.midiOutSetVolume(); }
void _midiOutShortMsg() { winmm.midiOutShortMsg(); }
void _midiOutUnprepareHeader() { winmm.midiOutUnprepareHeader(); }
void _midiStreamClose() { winmm.midiStreamClose(); }
void _midiStreamOpen() { winmm.midiStreamOpen(); }
void _midiStreamOut() { winmm.midiStreamOut(); }
void _midiStreamPause() { winmm.midiStreamPause(); }
void _midiStreamPosition() { winmm.midiStreamPosition(); }
void _midiStreamProperty() { winmm.midiStreamProperty(); }
void _midiStreamRestart() { winmm.midiStreamRestart(); }
void _midiStreamStop() { winmm.midiStreamStop(); }
void _mixerClose() { winmm.mixerClose(); }
void _mixerGetControlDetailsA() { winmm.mixerGetControlDetailsA(); }
void _mixerGetControlDetailsW() { winmm.mixerGetControlDetailsW(); }
void _mixerGetDevCapsA() { winmm.mixerGetDevCapsA(); }
void _mixerGetDevCapsW() { winmm.mixerGetDevCapsW(); }
void _mixerGetID() { winmm.mixerGetID(); }
void _mixerGetLineControlsA() { winmm.mixerGetLineControlsA(); }
void _mixerGetLineControlsW() { winmm.mixerGetLineControlsW(); }
void _mixerGetLineInfoA() { winmm.mixerGetLineInfoA(); }
void _mixerGetLineInfoW() { winmm.mixerGetLineInfoW(); }
void _mixerGetNumDevs() { winmm.mixerGetNumDevs(); }
void _mixerMessage() { winmm.mixerMessage(); }
void _mixerOpen() { winmm.mixerOpen(); }
void _mixerSetControlDetails() { winmm.mixerSetControlDetails(); }
void _mmDrvInstall() { winmm.mmDrvInstall(); }
void _mmGetCurrentTask() { winmm.mmGetCurrentTask(); }
void _mmTaskBlock() { winmm.mmTaskBlock(); }
void _mmTaskCreate() { winmm.mmTaskCreate(); }
void _mmTaskSignal() { winmm.mmTaskSignal(); }
void _mmTaskYield() { winmm.mmTaskYield(); }
void _mmioAdvance() { winmm.mmioAdvance(); }
void _mmioAscend() { winmm.mmioAscend(); }
void _mmioClose() { winmm.mmioClose(); }
void _mmioCreateChunk() { winmm.mmioCreateChunk(); }
void _mmioDescend() { winmm.mmioDescend(); }
void _mmioFlush() { winmm.mmioFlush(); }
void _mmioGetInfo() { winmm.mmioGetInfo(); }
void _mmioInstallIOProcA() { winmm.mmioInstallIOProcA(); }
void _mmioInstallIOProcW() { winmm.mmioInstallIOProcW(); }
void _mmioOpenA() { winmm.mmioOpenA(); }
void _mmioOpenW() { winmm.mmioOpenW(); }
void _mmioRead() { winmm.mmioRead(); }
void _mmioRenameA() { winmm.mmioRenameA(); }
void _mmioRenameW() { winmm.mmioRenameW(); }
void _mmioSeek() { winmm.mmioSeek(); }
void _mmioSendMessage() { winmm.mmioSendMessage(); }
void _mmioSetBuffer() { winmm.mmioSetBuffer(); }
void _mmioSetInfo() { winmm.mmioSetInfo(); }
void _mmioStringToFOURCCA() { winmm.mmioStringToFOURCCA(); }
void _mmioStringToFOURCCW() { winmm.mmioStringToFOURCCW(); }
void _mmioWrite() { winmm.mmioWrite(); }
void _mmsystemGetVersion() { winmm.mmsystemGetVersion(); }
void _mod32Message() { winmm.mod32Message(); }
void _mxd32Message() { winmm.mxd32Message(); }
void _sndPlaySoundA() { winmm.sndPlaySoundA(); }
void _sndPlaySoundW() { winmm.sndPlaySoundW(); }
void _tid32Message() { winmm.tid32Message(); }
void _timeBeginPeriod() { winmm.timeBeginPeriod(); }
void _timeEndPeriod() { winmm.timeEndPeriod(); }
void _timeGetDevCaps() { winmm.timeGetDevCaps(); }
void _timeGetSystemTime() { winmm.timeGetSystemTime(); }
void _timeGetTime() { winmm.timeGetTime(); }
void _timeKillEvent() { winmm.timeKillEvent(); }
void _timeSetEvent() { winmm.timeSetEvent(); }
void _waveInAddBuffer() { winmm.waveInAddBuffer(); }
void _waveInClose() { winmm.waveInClose(); }
void _waveInGetDevCapsA() { winmm.waveInGetDevCapsA(); }
void _waveInGetDevCapsW() { winmm.waveInGetDevCapsW(); }
void _waveInGetErrorTextA() { winmm.waveInGetErrorTextA(); }
void _waveInGetErrorTextW() { winmm.waveInGetErrorTextW(); }
void _waveInGetID() { winmm.waveInGetID(); }
void _waveInGetNumDevs() { winmm.waveInGetNumDevs(); }
void _waveInGetPosition() { winmm.waveInGetPosition(); }
void _waveInMessage() { winmm.waveInMessage(); }
void _waveInOpen() { winmm.waveInOpen(); }
void _waveInPrepareHeader() { winmm.waveInPrepareHeader(); }
void _waveInReset() { winmm.waveInReset(); }
void _waveInStart() { winmm.waveInStart(); }
void _waveInStop() { winmm.waveInStop(); }
void _waveInUnprepareHeader() { winmm.waveInUnprepareHeader(); }
void _waveOutBreakLoop() { winmm.waveOutBreakLoop(); }
void _waveOutClose() { winmm.waveOutClose(); }
void _waveOutGetDevCapsA() { winmm.waveOutGetDevCapsA(); }
void _waveOutGetDevCapsW() { winmm.waveOutGetDevCapsW(); }
void _waveOutGetErrorTextA() { winmm.waveOutGetErrorTextA(); }
void _waveOutGetErrorTextW() { winmm.waveOutGetErrorTextW(); }
void _waveOutGetID() { winmm.waveOutGetID(); }
void _waveOutGetNumDevs() { winmm.waveOutGetNumDevs(); }
void _waveOutGetPitch() { winmm.waveOutGetPitch(); }
void _waveOutGetPlaybackRate() { winmm.waveOutGetPlaybackRate(); }
void _waveOutGetPosition() { winmm.waveOutGetPosition(); }
void _waveOutGetVolume() { winmm.waveOutGetVolume(); }
void _waveOutMessage() { winmm.waveOutMessage(); }
void _waveOutOpen() { winmm.waveOutOpen(); }
void _waveOutPause() { winmm.waveOutPause(); }
void _waveOutPrepareHeader() { winmm.waveOutPrepareHeader(); }
void _waveOutReset() { winmm.waveOutReset(); }
void _waveOutRestart() { winmm.waveOutRestart(); }
void _waveOutSetPitch() { winmm.waveOutSetPitch(); }
void _waveOutSetPlaybackRate() { winmm.waveOutSetPlaybackRate(); }
void _waveOutSetVolume() { winmm.waveOutSetVolume(); }
void _waveOutUnprepareHeader() { winmm.waveOutUnprepareHeader(); }
void _waveOutWrite() { winmm.waveOutWrite(); }
void _wid32Message() { winmm.wid32Message(); }
void _wod32Message() { winmm.wod32Message(); }
#endif