#pragma once
#include "wrappers.h"
#include "shared.h"

#if X64
struct bink2w64_dll
{
    HMODULE dll;
    FARPROC BinkAllocateFrameBuffers;
    FARPROC BinkClose;
    FARPROC BinkCloseTrack;
    FARPROC BinkControlBackgroundIO;
    FARPROC BinkCopyToBuffer;
    FARPROC BinkCopyToBufferRect;
    FARPROC BinkDoFrame;
    FARPROC BinkDoFrameAsync;
    FARPROC BinkDoFrameAsyncMulti;
    FARPROC BinkDoFrameAsyncWait;
    FARPROC BinkDoFramePlane;
    FARPROC BinkFindXAudio2WinDevice;
    FARPROC BinkFreeGlobals;
    FARPROC BinkGetError;
    FARPROC BinkGetFrameBuffersInfo;
    FARPROC BinkGetGPUDataBuffersInfo;
    FARPROC BinkGetKeyFrame;
    FARPROC BinkGetPlatformInfo;
    FARPROC BinkGetRealtime;
    FARPROC BinkGetRects;
    FARPROC BinkGetSummary;
    FARPROC BinkGetTrackData;
    FARPROC BinkGetTrackID;
    FARPROC BinkGetTrackMaxSize;
    FARPROC BinkGetTrackType;
    FARPROC BinkGoto;
    FARPROC BinkLogoAddress;
    FARPROC BinkNextFrame;
    FARPROC BinkOpen;
    FARPROC BinkOpenDirectSound;
    FARPROC BinkOpenMiles;
    FARPROC BinkOpenTrack;
    FARPROC BinkOpenWaveOut;
    FARPROC BinkOpenWithOptions;
    FARPROC BinkOpenXAudio2;
    FARPROC BinkOpenXAudio27;
    FARPROC BinkOpenXAudio28;
    FARPROC BinkOpenXAudio29;
    FARPROC BinkPause;
    FARPROC BinkRegisterFrameBuffers;
    FARPROC BinkRegisterGPUDataBuffers;
    FARPROC BinkRequestStopAsyncThread;
    FARPROC BinkRequestStopAsyncThreadsMulti;
    FARPROC BinkService;
    FARPROC BinkSetError;
    FARPROC BinkSetFileOffset;
    FARPROC BinkSetFrameRate;
    FARPROC BinkSetIO;
    FARPROC BinkSetIOSize;
    FARPROC BinkSetMemory;
    FARPROC BinkSetOSFileCallbacks;
    FARPROC BinkSetPan;
    FARPROC BinkSetSimulate;
    FARPROC BinkSetSoundOnOff;
    FARPROC BinkSetSoundSystem;
    FARPROC BinkSetSoundSystem2;
    FARPROC BinkSetSoundTrack;
    FARPROC BinkSetSpeakerVolumes;
    FARPROC BinkSetVideoOnOff;
    FARPROC BinkSetVolume;
    FARPROC BinkSetWillLoop;
    FARPROC BinkShouldSkip;
    FARPROC BinkStartAsyncThread;
    FARPROC BinkUtilCPUs;
    FARPROC BinkUtilFree;
    FARPROC BinkUtilMalloc;
    FARPROC BinkUtilMutexCreate;
    FARPROC BinkUtilMutexDestroy;
    FARPROC BinkUtilMutexLock;
    FARPROC BinkUtilMutexLockTimeOut;
    FARPROC BinkUtilMutexUnlock;
    FARPROC BinkUtilSoundGlobalLock;
    FARPROC BinkUtilSoundGlobalUnlock;
    FARPROC BinkWait;
    FARPROC BinkWaitStopAsyncThread;
    FARPROC BinkWaitStopAsyncThreadsMulti;
    FARPROC RADTimerRead;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        BinkAllocateFrameBuffers = GetProcAddress(dll, "BinkAllocateFrameBuffers");
        BinkClose = GetProcAddress(dll, "BinkClose");
        BinkCloseTrack = GetProcAddress(dll, "BinkCloseTrack");
        BinkControlBackgroundIO = GetProcAddress(dll, "BinkControlBackgroundIO");
        BinkCopyToBuffer = GetProcAddress(dll, "BinkCopyToBuffer");
        BinkCopyToBufferRect = GetProcAddress(dll, "BinkCopyToBufferRect");
        BinkDoFrame = GetProcAddress(dll, "BinkDoFrame");
        BinkDoFrameAsync = GetProcAddress(dll, "BinkDoFrameAsync");
        BinkDoFrameAsyncMulti = GetProcAddress(dll, "BinkDoFrameAsyncMulti");
        BinkDoFrameAsyncWait = GetProcAddress(dll, "BinkDoFrameAsyncWait");
        BinkDoFramePlane = GetProcAddress(dll, "BinkDoFramePlane");
        BinkFindXAudio2WinDevice = GetProcAddress(dll, "BinkFindXAudio2WinDevice");
        BinkFreeGlobals = GetProcAddress(dll, "BinkFreeGlobals");
        BinkGetError = GetProcAddress(dll, "BinkGetError");
        BinkGetFrameBuffersInfo = GetProcAddress(dll, "BinkGetFrameBuffersInfo");
        BinkGetGPUDataBuffersInfo = GetProcAddress(dll, "BinkGetGPUDataBuffersInfo");
        BinkGetKeyFrame = GetProcAddress(dll, "BinkGetKeyFrame");
        BinkGetPlatformInfo = GetProcAddress(dll, "BinkGetPlatformInfo");
        BinkGetRealtime = GetProcAddress(dll, "BinkGetRealtime");
        BinkGetRects = GetProcAddress(dll, "BinkGetRects");
        BinkGetSummary = GetProcAddress(dll, "BinkGetSummary");
        BinkGetTrackData = GetProcAddress(dll, "BinkGetTrackData");
        BinkGetTrackID = GetProcAddress(dll, "BinkGetTrackID");
        BinkGetTrackMaxSize = GetProcAddress(dll, "BinkGetTrackMaxSize");
        BinkGetTrackType = GetProcAddress(dll, "BinkGetTrackType");
        BinkGoto = GetProcAddress(dll, "BinkGoto");
        BinkLogoAddress = GetProcAddress(dll, "BinkLogoAddress");
        BinkNextFrame = GetProcAddress(dll, "BinkNextFrame");
        BinkOpen = GetProcAddress(dll, "BinkOpen");
        BinkOpenDirectSound = GetProcAddress(dll, "BinkOpenDirectSound");
        BinkOpenMiles = GetProcAddress(dll, "BinkOpenMiles");
        BinkOpenTrack = GetProcAddress(dll, "BinkOpenTrack");
        BinkOpenWaveOut = GetProcAddress(dll, "BinkOpenWaveOut");
        BinkOpenWithOptions = GetProcAddress(dll, "BinkOpenWithOptions");
        BinkOpenXAudio2 = GetProcAddress(dll, "BinkOpenXAudio2");
        BinkOpenXAudio27 = GetProcAddress(dll, "BinkOpenXAudio27");
        BinkOpenXAudio28 = GetProcAddress(dll, "BinkOpenXAudio28");
        BinkOpenXAudio29 = GetProcAddress(dll, "BinkOpenXAudio29");
        BinkPause = GetProcAddress(dll, "BinkPause");
        BinkRegisterFrameBuffers = GetProcAddress(dll, "BinkRegisterFrameBuffers");
        BinkRegisterGPUDataBuffers = GetProcAddress(dll, "BinkRegisterGPUDataBuffers");
        BinkRequestStopAsyncThread = GetProcAddress(dll, "BinkRequestStopAsyncThread");
        BinkRequestStopAsyncThreadsMulti = GetProcAddress(dll, "BinkRequestStopAsyncThreadsMulti");
        BinkService = GetProcAddress(dll, "BinkService");
        BinkSetError = GetProcAddress(dll, "BinkSetError");
        BinkSetFileOffset = GetProcAddress(dll, "BinkSetFileOffset");
        BinkSetFrameRate = GetProcAddress(dll, "BinkSetFrameRate");
        BinkSetIO = GetProcAddress(dll, "BinkSetIO");
        BinkSetIOSize = GetProcAddress(dll, "BinkSetIOSize");
        BinkSetMemory = GetProcAddress(dll, "BinkSetMemory");
        BinkSetOSFileCallbacks = GetProcAddress(dll, "BinkSetOSFileCallbacks");
        BinkSetPan = GetProcAddress(dll, "BinkSetPan");
        BinkSetSimulate = GetProcAddress(dll, "BinkSetSimulate");
        BinkSetSoundOnOff = GetProcAddress(dll, "BinkSetSoundOnOff");
        BinkSetSoundSystem = GetProcAddress(dll, "BinkSetSoundSystem");
        BinkSetSoundSystem2 = GetProcAddress(dll, "BinkSetSoundSystem2");
        BinkSetSoundTrack = GetProcAddress(dll, "BinkSetSoundTrack");
        BinkSetSpeakerVolumes = GetProcAddress(dll, "BinkSetSpeakerVolumes");
        BinkSetVideoOnOff = GetProcAddress(dll, "BinkSetVideoOnOff");
        BinkSetVolume = GetProcAddress(dll, "BinkSetVolume");
        BinkSetWillLoop = GetProcAddress(dll, "BinkSetWillLoop");
        BinkShouldSkip = GetProcAddress(dll, "BinkShouldSkip");
        BinkStartAsyncThread = GetProcAddress(dll, "BinkStartAsyncThread");
        BinkUtilCPUs = GetProcAddress(dll, "BinkUtilCPUs");
        BinkUtilFree = GetProcAddress(dll, "BinkUtilFree");
        BinkUtilMalloc = GetProcAddress(dll, "BinkUtilMalloc");
        BinkUtilMutexCreate = GetProcAddress(dll, "BinkUtilMutexCreate");
        BinkUtilMutexDestroy = GetProcAddress(dll, "BinkUtilMutexDestroy");
        BinkUtilMutexLock = GetProcAddress(dll, "BinkUtilMutexLock");
        BinkUtilMutexLockTimeOut = GetProcAddress(dll, "BinkUtilMutexLockTimeOut");
        BinkUtilMutexUnlock = GetProcAddress(dll, "BinkUtilMutexUnlock");
        BinkUtilSoundGlobalLock = GetProcAddress(dll, "BinkUtilSoundGlobalLock");
        BinkUtilSoundGlobalUnlock = GetProcAddress(dll, "BinkUtilSoundGlobalUnlock");
        BinkWait = GetProcAddress(dll, "BinkWait");
        BinkWaitStopAsyncThread = GetProcAddress(dll, "BinkWaitStopAsyncThread");
        BinkWaitStopAsyncThreadsMulti = GetProcAddress(dll, "BinkWaitStopAsyncThreadsMulti");
        RADTimerRead = GetProcAddress(dll, "RADTimerRead");
    }
} bink2w64;

void _BinkAllocateFrameBuffers() { bink2w64.BinkAllocateFrameBuffers(); }
void _BinkClose() { bink2w64.BinkClose(); }
void _BinkCloseTrack() { bink2w64.BinkCloseTrack(); }
void _BinkControlBackgroundIO() { bink2w64.BinkControlBackgroundIO(); }
void _BinkCopyToBuffer() { bink2w64.BinkCopyToBuffer(); }
void _BinkCopyToBufferRect() { bink2w64.BinkCopyToBufferRect(); }
void _BinkDoFrame() { bink2w64.BinkDoFrame(); }
void _BinkDoFrameAsync() { bink2w64.BinkDoFrameAsync(); }
void _BinkDoFrameAsyncMulti() { bink2w64.BinkDoFrameAsyncMulti(); }
void _BinkDoFrameAsyncWait() { bink2w64.BinkDoFrameAsyncWait(); }
void _BinkDoFramePlane() { bink2w64.BinkDoFramePlane(); }
void _BinkFindXAudio2WinDevice() { bink2w64.BinkFindXAudio2WinDevice(); }
void _BinkFreeGlobals() { bink2w64.BinkFreeGlobals(); }
void _BinkGetError() { bink2w64.BinkGetError(); }
void _BinkGetFrameBuffersInfo() { bink2w64.BinkGetFrameBuffersInfo(); }
void _BinkGetGPUDataBuffersInfo() { bink2w64.BinkGetGPUDataBuffersInfo(); }
void _BinkGetKeyFrame() { bink2w64.BinkGetKeyFrame(); }
void _BinkGetPlatformInfo() { bink2w64.BinkGetPlatformInfo(); }
void _BinkGetRealtime() { bink2w64.BinkGetRealtime(); }
void _BinkGetRects() { bink2w64.BinkGetRects(); }
void _BinkGetSummary() { bink2w64.BinkGetSummary(); }
void _BinkGetTrackData() { bink2w64.BinkGetTrackData(); }
void _BinkGetTrackID() { bink2w64.BinkGetTrackID(); }
void _BinkGetTrackMaxSize() { bink2w64.BinkGetTrackMaxSize(); }
void _BinkGetTrackType() { bink2w64.BinkGetTrackType(); }
void _BinkGoto() { bink2w64.BinkGoto(); }
void _BinkLogoAddress() { bink2w64.BinkLogoAddress(); }
void _BinkNextFrame() { bink2w64.BinkNextFrame(); }
void _BinkOpen() { bink2w64.BinkOpen(); }
void _BinkOpenDirectSound() { bink2w64.BinkOpenDirectSound(); }
void _BinkOpenMiles() { bink2w64.BinkOpenMiles(); }
void _BinkOpenTrack() { bink2w64.BinkOpenTrack(); }
void _BinkOpenWaveOut() { bink2w64.BinkOpenWaveOut(); }
void _BinkOpenWithOptions() { bink2w64.BinkOpenWithOptions(); }
void _BinkOpenXAudio2() { bink2w64.BinkOpenXAudio2(); }
void _BinkOpenXAudio27() { bink2w64.BinkOpenXAudio27(); }
void _BinkOpenXAudio28() { bink2w64.BinkOpenXAudio28(); }
void _BinkOpenXAudio29() { bink2w64.BinkOpenXAudio29(); }
void _BinkPause() { bink2w64.BinkPause(); }
void _BinkRegisterFrameBuffers() { bink2w64.BinkRegisterFrameBuffers(); }
void _BinkRegisterGPUDataBuffers() { bink2w64.BinkRegisterGPUDataBuffers(); }
void _BinkRequestStopAsyncThread() { bink2w64.BinkRequestStopAsyncThread(); }
void _BinkRequestStopAsyncThreadsMulti() { bink2w64.BinkRequestStopAsyncThreadsMulti(); }
void _BinkService() { bink2w64.BinkService(); }
void _BinkSetError() { bink2w64.BinkSetError(); }
void _BinkSetFileOffset() { bink2w64.BinkSetFileOffset(); }
void _BinkSetFrameRate() { bink2w64.BinkSetFrameRate(); }
void _BinkSetIO() { bink2w64.BinkSetIO(); }
void _BinkSetIOSize() { bink2w64.BinkSetIOSize(); }
void _BinkSetMemory() { bink2w64.BinkSetMemory(); }
void _BinkSetOSFileCallbacks() { bink2w64.BinkSetOSFileCallbacks(); }
void _BinkSetPan() { bink2w64.BinkSetPan(); }
void _BinkSetSimulate() { bink2w64.BinkSetSimulate(); }
void _BinkSetSoundOnOff() { bink2w64.BinkSetSoundOnOff(); }
void _BinkSetSoundSystem() { bink2w64.BinkSetSoundSystem(); }
void _BinkSetSoundSystem2() { bink2w64.BinkSetSoundSystem2(); }
void _BinkSetSoundTrack() { bink2w64.BinkSetSoundTrack(); }
void _BinkSetSpeakerVolumes() { bink2w64.BinkSetSpeakerVolumes(); }
void _BinkSetVideoOnOff() { bink2w64.BinkSetVideoOnOff(); }
void _BinkSetVolume() { bink2w64.BinkSetVolume(); }
void _BinkSetWillLoop() { bink2w64.BinkSetWillLoop(); }
void _BinkShouldSkip() { bink2w64.BinkShouldSkip(); }
void _BinkStartAsyncThread() { bink2w64.BinkStartAsyncThread(); }
void _BinkUtilCPUs() { bink2w64.BinkUtilCPUs(); }
void _BinkUtilFree() { bink2w64.BinkUtilFree(); }
void _BinkUtilMalloc() { bink2w64.BinkUtilMalloc(); }
void _BinkUtilMutexCreate() { bink2w64.BinkUtilMutexCreate(); }
void _BinkUtilMutexDestroy() { bink2w64.BinkUtilMutexDestroy(); }
void _BinkUtilMutexLock() { bink2w64.BinkUtilMutexLock(); }
void _BinkUtilMutexLockTimeOut() { bink2w64.BinkUtilMutexLockTimeOut(); }
void _BinkUtilMutexUnlock() { bink2w64.BinkUtilMutexUnlock(); }
void _BinkUtilSoundGlobalLock() { bink2w64.BinkUtilSoundGlobalLock(); }
void _BinkUtilSoundGlobalUnlock() { bink2w64.BinkUtilSoundGlobalUnlock(); }
void _BinkWait() { bink2w64.BinkWait(); }
void _BinkWaitStopAsyncThread() { bink2w64.BinkWaitStopAsyncThread(); }
void _BinkWaitStopAsyncThreadsMulti() { bink2w64.BinkWaitStopAsyncThreadsMulti(); }
void _RADTimerRead() { bink2w64.RADTimerRead(); }
#endif