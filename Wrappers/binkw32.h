#pragma once
#include "wrappers.h"
#include "shared.h"

#if !X64
struct binkw32_dll
{
    HMODULE dll;
    FARPROC BinkBufferBlit;
    FARPROC BinkBufferCheckWinPos;
    FARPROC BinkBufferClear;
    FARPROC BinkBufferClose;
    FARPROC BinkBufferGetDescription;
    FARPROC BinkBufferGetError;
    FARPROC BinkBufferLock;
    FARPROC BinkBufferOpen;
    FARPROC BinkBufferSetDirectDraw;
    FARPROC BinkBufferSetHWND;
    FARPROC BinkBufferSetOffset;
    FARPROC BinkBufferSetResolution;
    FARPROC BinkBufferSetScale;
    FARPROC BinkBufferUnlock;
    FARPROC BinkCheckCursor;
    FARPROC BinkClose;
    FARPROC BinkCloseTrack;
    FARPROC BinkControlBackgroundIO;
    FARPROC BinkControlPlatformFeatures;
    FARPROC BinkCopyToBuffer;
    FARPROC BinkCopyToBufferRect;
    FARPROC BinkDDSurfaceType;
    FARPROC BinkDX8SurfaceType;
    FARPROC BinkDX9SurfaceType;
    FARPROC BinkDoFrame;
    FARPROC BinkDoFrameAsync;
    FARPROC BinkDoFrameAsyncWait;
    FARPROC BinkDoFramePlane;
    FARPROC BinkFreeGlobals;
    FARPROC BinkGetError;
    FARPROC BinkGetFrameBuffersInfo;
    FARPROC BinkGetKeyFrame;
    FARPROC BinkGetPalette;
    FARPROC BinkGetPlatformInfo;
    FARPROC BinkGetRealtime;
    FARPROC BinkGetRects;
    FARPROC BinkGetSummary;
    FARPROC BinkGetTrackData;
    FARPROC BinkGetTrackID;
    FARPROC BinkGetTrackMaxSize;
    FARPROC BinkGetTrackType;
    FARPROC BinkGoto;
    FARPROC BinkIsSoftwareCursor;
    FARPROC BinkLogoAddress;
    FARPROC BinkNextFrame;
    FARPROC BinkOpen;
    FARPROC BinkOpenDirectSound;
    FARPROC BinkOpenMiles;
    FARPROC BinkOpenTrack;
    FARPROC BinkOpenWaveOut;
    FARPROC BinkOpenWithOptions;
    FARPROC BinkOpenXAudio2;
    FARPROC BinkPause;
    FARPROC BinkRegisterFrameBuffers;
    FARPROC BinkRequestStopAsyncThread;
    FARPROC BinkRestoreCursor;
    FARPROC BinkService;
    FARPROC BinkServiceSound;
    FARPROC BinkSetError;
    FARPROC BinkSetFileOffset;
    FARPROC BinkSetFrameRate;
    FARPROC BinkSetIO;
    FARPROC BinkSetIOSize;
    FARPROC BinkSetMemory;
    FARPROC BinkSetMixBinVolumes;
    FARPROC BinkSetMixBins;
    FARPROC BinkSetPan;
    FARPROC BinkSetSimulate;
    FARPROC BinkSetSoundOnOff;
    FARPROC BinkSetSoundSystem;
    FARPROC BinkSetSoundTrack;
    FARPROC BinkSetSpeakerVolumes;
    FARPROC BinkSetVideoOnOff;
    FARPROC BinkSetVolume;
    FARPROC BinkSetWillLoop;
    FARPROC BinkShouldSkip;
    FARPROC BinkStartAsyncThread;
    FARPROC BinkUseTelemetry;
    FARPROC BinkUseTmLite;
    FARPROC BinkWait;
    FARPROC BinkWaitStopAsyncThread;
    FARPROC RADSetMemory;
    FARPROC RADTimerRead;
    FARPROC YUV_blit_16a1bpp40;
    FARPROC YUV_blit_16a1bpp52;
    FARPROC YUV_blit_16a1bpp_mask48;
    FARPROC YUV_blit_16a1bpp_mask60;
    FARPROC YUV_blit_16a4bpp40;
    FARPROC YUV_blit_16a4bpp52;
    FARPROC YUV_blit_16a4bpp_mask48;
    FARPROC YUV_blit_16a4bpp_mask60;
    FARPROC YUV_blit_16bpp40;
    FARPROC YUV_blit_16bpp48;
    FARPROC YUV_blit_16bpp52;
    FARPROC YUV_blit_16bpp_mask48;
    FARPROC YUV_blit_16bpp_mask56;
    FARPROC YUV_blit_16bpp_mask60;
    FARPROC YUV_blit_24bpp40;
    FARPROC YUV_blit_24bpp48;
    FARPROC YUV_blit_24bpp52;
    FARPROC YUV_blit_24bpp_mask48;
    FARPROC YUV_blit_24bpp_mask56;
    FARPROC YUV_blit_24bpp_mask60;
    FARPROC YUV_blit_24rbpp40;
    FARPROC YUV_blit_24rbpp48;
    FARPROC YUV_blit_24rbpp52;
    FARPROC YUV_blit_24rbpp_mask48;
    FARPROC YUV_blit_24rbpp_mask56;
    FARPROC YUV_blit_24rbpp_mask60;
    FARPROC YUV_blit_32abpp40;
    FARPROC YUV_blit_32abpp52;
    FARPROC YUV_blit_32abpp_mask48;
    FARPROC YUV_blit_32abpp_mask60;
    FARPROC YUV_blit_32bpp40;
    FARPROC YUV_blit_32bpp48;
    FARPROC YUV_blit_32bpp52;
    FARPROC YUV_blit_32bpp_mask48;
    FARPROC YUV_blit_32bpp_mask56;
    FARPROC YUV_blit_32bpp_mask60;
    FARPROC YUV_blit_32rabpp40;
    FARPROC YUV_blit_32rabpp52;
    FARPROC YUV_blit_32rabpp_mask48;
    FARPROC YUV_blit_32rabpp_mask60;
    FARPROC YUV_blit_32rbpp40;
    FARPROC YUV_blit_32rbpp48;
    FARPROC YUV_blit_32rbpp52;
    FARPROC YUV_blit_32rbpp_mask48;
    FARPROC YUV_blit_32rbpp_mask56;
    FARPROC YUV_blit_32rbpp_mask60;
    FARPROC YUV_blit_UYVY40;
    FARPROC YUV_blit_UYVY48;
    FARPROC YUV_blit_UYVY52;
    FARPROC YUV_blit_UYVY_mask48;
    FARPROC YUV_blit_UYVY_mask56;
    FARPROC YUV_blit_UYVY_mask60;
    FARPROC YUV_blit_YUY240;
    FARPROC YUV_blit_YUY248;
    FARPROC YUV_blit_YUY252;
    FARPROC YUV_blit_YUY2_mask48;
    FARPROC YUV_blit_YUY2_mask56;
    FARPROC YUV_blit_YUY2_mask60;
    FARPROC YUV_blit_YV1244;
    FARPROC YUV_blit_YV1252;
    FARPROC YUV_blit_YV1256;
    FARPROC YUV_init4;
    FARPROC radfree;
    FARPROC radmalloc;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;

        BinkBufferBlit = GetProcAddress(dll, "_BinkBufferBlit@12");
        BinkBufferCheckWinPos = GetProcAddress(dll, "_BinkBufferCheckWinPos@12");
        BinkBufferClear = GetProcAddress(dll, "_BinkBufferClear@8");
        BinkBufferClose = GetProcAddress(dll, "_BinkBufferClose@4");
        BinkBufferGetDescription = GetProcAddress(dll, "_BinkBufferGetDescription@4");
        BinkBufferGetError = GetProcAddress(dll, "_BinkBufferGetError@0");
        BinkBufferLock = GetProcAddress(dll, "_BinkBufferLock@4");
        BinkBufferOpen = GetProcAddress(dll, "_BinkBufferOpen@16");
        BinkBufferSetDirectDraw = GetProcAddress(dll, "_BinkBufferSetDirectDraw@8");
        BinkBufferSetHWND = GetProcAddress(dll, "_BinkBufferSetHWND@8");
        BinkBufferSetOffset = GetProcAddress(dll, "_BinkBufferSetOffset@12");
        BinkBufferSetResolution = GetProcAddress(dll, "_BinkBufferSetResolution@12");
        BinkBufferSetScale = GetProcAddress(dll, "_BinkBufferSetScale@12");
        BinkBufferUnlock = GetProcAddress(dll, "_BinkBufferUnlock@4");
        BinkCheckCursor = GetProcAddress(dll, "_BinkCheckCursor@20");
        BinkClose = GetProcAddress(dll, "_BinkClose@4");
        BinkCloseTrack = GetProcAddress(dll, "_BinkCloseTrack@4");
        BinkControlBackgroundIO = GetProcAddress(dll, "_BinkControlBackgroundIO@8");
        BinkControlPlatformFeatures = GetProcAddress(dll, "_BinkControlPlatformFeatures@8");
        BinkCopyToBuffer = GetProcAddress(dll, "_BinkCopyToBuffer@28");
        BinkCopyToBufferRect = GetProcAddress(dll, "_BinkCopyToBufferRect@44");
        BinkDDSurfaceType = GetProcAddress(dll, "_BinkDDSurfaceType@4");
        BinkDX8SurfaceType = GetProcAddress(dll, "_BinkDX8SurfaceType@4");
        BinkDX9SurfaceType = GetProcAddress(dll, "_BinkDX9SurfaceType@4");
        BinkDoFrame = GetProcAddress(dll, "_BinkDoFrame@4");
        BinkDoFrameAsync = GetProcAddress(dll, "_BinkDoFrameAsync@12");
        BinkDoFrameAsyncWait = GetProcAddress(dll, "_BinkDoFrameAsyncWait@8");
        BinkDoFramePlane = GetProcAddress(dll, "_BinkDoFramePlane@8");
        BinkFreeGlobals = GetProcAddress(dll, "_BinkFreeGlobals@0");
        BinkGetError = GetProcAddress(dll, "_BinkGetError@0");
        BinkGetFrameBuffersInfo = GetProcAddress(dll, "_BinkGetFrameBuffersInfo@8");
        BinkGetKeyFrame = GetProcAddress(dll, "_BinkGetKeyFrame@12");
        BinkGetPalette = GetProcAddress(dll, "_BinkGetPalette@4");
        BinkGetPlatformInfo = GetProcAddress(dll, "_BinkGetPlatformInfo@8");
        BinkGetRealtime = GetProcAddress(dll, "_BinkGetRealtime@12");
        BinkGetRects = GetProcAddress(dll, "_BinkGetRects@8");
        BinkGetSummary = GetProcAddress(dll, "_BinkGetSummary@8");
        BinkGetTrackData = GetProcAddress(dll, "_BinkGetTrackData@8");
        BinkGetTrackID = GetProcAddress(dll, "_BinkGetTrackID@8");
        BinkGetTrackMaxSize = GetProcAddress(dll, "_BinkGetTrackMaxSize@8");
        BinkGetTrackType = GetProcAddress(dll, "_BinkGetTrackType@8");
        BinkGoto = GetProcAddress(dll, "_BinkGoto@12");
        BinkIsSoftwareCursor = GetProcAddress(dll, "_BinkIsSoftwareCursor@8");
        BinkLogoAddress = GetProcAddress(dll, "_BinkLogoAddress@0");
        BinkNextFrame = GetProcAddress(dll, "_BinkNextFrame@4");
        BinkOpen = GetProcAddress(dll, "_BinkOpen@8");
        BinkOpenDirectSound = GetProcAddress(dll, "_BinkOpenDirectSound@4");
        BinkOpenMiles = GetProcAddress(dll, "_BinkOpenMiles@4");
        BinkOpenTrack = GetProcAddress(dll, "_BinkOpenTrack@8");
        BinkOpenWaveOut = GetProcAddress(dll, "_BinkOpenWaveOut@4");
        BinkOpenWithOptions = GetProcAddress(dll, "_BinkOpenWithOptions@12");
        BinkOpenXAudio2 = GetProcAddress(dll, "_BinkOpenXAudio2@4");
        BinkPause = GetProcAddress(dll, "_BinkPause@8");
        BinkRegisterFrameBuffers = GetProcAddress(dll, "_BinkRegisterFrameBuffers@8");
        BinkRequestStopAsyncThread = GetProcAddress(dll, "_BinkRequestStopAsyncThread@4");
        BinkRestoreCursor = GetProcAddress(dll, "_BinkRestoreCursor@4");
        BinkService = GetProcAddress(dll, "_BinkService@4");
        BinkServiceSound = GetProcAddress(dll, "_BinkServiceSound@0");
        BinkSetError = GetProcAddress(dll, "_BinkSetError@4");
        BinkSetFileOffset = GetProcAddress(dll, "_BinkSetFileOffset@8");
        BinkSetFrameRate = GetProcAddress(dll, "_BinkSetFrameRate@8");
        BinkSetIO = GetProcAddress(dll, "_BinkSetIO@4");
        BinkSetIOSize = GetProcAddress(dll, "_BinkSetIOSize@4");
        BinkSetMemory = GetProcAddress(dll, "_BinkSetMemory@8");
        BinkSetMixBinVolumes = GetProcAddress(dll, "_BinkSetMixBinVolumes@20");
        BinkSetMixBins = GetProcAddress(dll, "_BinkSetMixBins@16");
        BinkSetPan = GetProcAddress(dll, "_BinkSetPan@12");
        BinkSetSimulate = GetProcAddress(dll, "_BinkSetSimulate@4");
        BinkSetSoundOnOff = GetProcAddress(dll, "_BinkSetSoundOnOff@8");
        BinkSetSoundSystem = GetProcAddress(dll, "_BinkSetSoundSystem@8");
        BinkSetSoundTrack = GetProcAddress(dll, "_BinkSetSoundTrack@8");
        BinkSetSpeakerVolumes = GetProcAddress(dll, "_BinkSetSpeakerVolumes@20");
        BinkSetVideoOnOff = GetProcAddress(dll, "_BinkSetVideoOnOff@8");
        BinkSetVolume = GetProcAddress(dll, "_BinkSetVolume@12");
        BinkSetWillLoop = GetProcAddress(dll, "_BinkSetWillLoop@8");
        BinkShouldSkip = GetProcAddress(dll, "_BinkShouldSkip@4");
        BinkStartAsyncThread = GetProcAddress(dll, "_BinkStartAsyncThread@8");
        BinkUseTelemetry = GetProcAddress(dll, "_BinkUseTelemetry@4");
        BinkUseTmLite = GetProcAddress(dll, "_BinkUseTmLite@4");
        BinkWait = GetProcAddress(dll, "_BinkWait@4");
        BinkWaitStopAsyncThread = GetProcAddress(dll, "_BinkWaitStopAsyncThread@4");
        RADSetMemory = GetProcAddress(dll, "_RADSetMemory@8");
        RADTimerRead = GetProcAddress(dll, "_RADTimerRead@0");
        YUV_blit_16a1bpp40 = GetProcAddress(dll, "_YUV_blit_16a1bpp@40");
        YUV_blit_16a1bpp52 = GetProcAddress(dll, "_YUV_blit_16a1bpp@52");
        YUV_blit_16a1bpp_mask48 = GetProcAddress(dll, "_YUV_blit_16a1bpp_mask@48");
        YUV_blit_16a1bpp_mask60 = GetProcAddress(dll, "_YUV_blit_16a1bpp_mask@60");
        YUV_blit_16a4bpp40 = GetProcAddress(dll, "_YUV_blit_16a4bpp@40");
        YUV_blit_16a4bpp52 = GetProcAddress(dll, "_YUV_blit_16a4bpp@52");
        YUV_blit_16a4bpp_mask48 = GetProcAddress(dll, "_YUV_blit_16a4bpp_mask@48");
        YUV_blit_16a4bpp_mask60 = GetProcAddress(dll, "_YUV_blit_16a4bpp_mask@60");
        YUV_blit_16bpp40 = GetProcAddress(dll, "_YUV_blit_16bpp@40");
        YUV_blit_16bpp48 = GetProcAddress(dll, "_YUV_blit_16bpp@48");
        YUV_blit_16bpp52 = GetProcAddress(dll, "_YUV_blit_16bpp@52");
        YUV_blit_16bpp_mask48 = GetProcAddress(dll, "_YUV_blit_16bpp_mask@48");
        YUV_blit_16bpp_mask56 = GetProcAddress(dll, "_YUV_blit_16bpp_mask@56");
        YUV_blit_16bpp_mask60 = GetProcAddress(dll, "_YUV_blit_16bpp_mask@60");
        YUV_blit_24bpp40 = GetProcAddress(dll, "_YUV_blit_24bpp@40");
        YUV_blit_24bpp48 = GetProcAddress(dll, "_YUV_blit_24bpp@48");
        YUV_blit_24bpp52 = GetProcAddress(dll, "_YUV_blit_24bpp@52");
        YUV_blit_24bpp_mask48 = GetProcAddress(dll, "_YUV_blit_24bpp_mask@48");
        YUV_blit_24bpp_mask56 = GetProcAddress(dll, "_YUV_blit_24bpp_mask@56");
        YUV_blit_24bpp_mask60 = GetProcAddress(dll, "_YUV_blit_24bpp_mask@60");
        YUV_blit_24rbpp40 = GetProcAddress(dll, "_YUV_blit_24rbpp@40");
        YUV_blit_24rbpp48 = GetProcAddress(dll, "_YUV_blit_24rbpp@48");
        YUV_blit_24rbpp52 = GetProcAddress(dll, "_YUV_blit_24rbpp@52");
        YUV_blit_24rbpp_mask48 = GetProcAddress(dll, "_YUV_blit_24rbpp_mask@48");
        YUV_blit_24rbpp_mask56 = GetProcAddress(dll, "_YUV_blit_24rbpp_mask@56");
        YUV_blit_24rbpp_mask60 = GetProcAddress(dll, "_YUV_blit_24rbpp_mask@60");
        YUV_blit_32abpp40 = GetProcAddress(dll, "_YUV_blit_32abpp@40");
        YUV_blit_32abpp52 = GetProcAddress(dll, "_YUV_blit_32abpp@52");
        YUV_blit_32abpp_mask48 = GetProcAddress(dll, "_YUV_blit_32abpp_mask@48");
        YUV_blit_32abpp_mask60 = GetProcAddress(dll, "_YUV_blit_32abpp_mask@60");
        YUV_blit_32bpp40 = GetProcAddress(dll, "_YUV_blit_32bpp@40");
        YUV_blit_32bpp48 = GetProcAddress(dll, "_YUV_blit_32bpp@48");
        YUV_blit_32bpp52 = GetProcAddress(dll, "_YUV_blit_32bpp@52");
        YUV_blit_32bpp_mask48 = GetProcAddress(dll, "_YUV_blit_32bpp_mask@48");
        YUV_blit_32bpp_mask56 = GetProcAddress(dll, "_YUV_blit_32bpp_mask@56");
        YUV_blit_32bpp_mask60 = GetProcAddress(dll, "_YUV_blit_32bpp_mask@60");
        YUV_blit_32rabpp40 = GetProcAddress(dll, "_YUV_blit_32rabpp@40");
        YUV_blit_32rabpp52 = GetProcAddress(dll, "_YUV_blit_32rabpp@52");
        YUV_blit_32rabpp_mask48 = GetProcAddress(dll, "_YUV_blit_32rabpp_mask@48");
        YUV_blit_32rabpp_mask60 = GetProcAddress(dll, "_YUV_blit_32rabpp_mask@60");
        YUV_blit_32rbpp40 = GetProcAddress(dll, "_YUV_blit_32rbpp@40");
        YUV_blit_32rbpp48 = GetProcAddress(dll, "_YUV_blit_32rbpp@48");
        YUV_blit_32rbpp52 = GetProcAddress(dll, "_YUV_blit_32rbpp@52");
        YUV_blit_32rbpp_mask48 = GetProcAddress(dll, "_YUV_blit_32rbpp_mask@48");
        YUV_blit_32rbpp_mask56 = GetProcAddress(dll, "_YUV_blit_32rbpp_mask@56");
        YUV_blit_32rbpp_mask60 = GetProcAddress(dll, "_YUV_blit_32rbpp_mask@60");
        YUV_blit_UYVY40 = GetProcAddress(dll, "_YUV_blit_UYVY@40");
        YUV_blit_UYVY48 = GetProcAddress(dll, "_YUV_blit_UYVY@48");
        YUV_blit_UYVY52 = GetProcAddress(dll, "_YUV_blit_UYVY@52");
        YUV_blit_UYVY_mask48 = GetProcAddress(dll, "_YUV_blit_UYVY_mask@48");
        YUV_blit_UYVY_mask56 = GetProcAddress(dll, "_YUV_blit_UYVY_mask@56");
        YUV_blit_UYVY_mask60 = GetProcAddress(dll, "_YUV_blit_UYVY_mask@60");
        YUV_blit_YUY240 = GetProcAddress(dll, "_YUV_blit_YUY2@40");
        YUV_blit_YUY248 = GetProcAddress(dll, "_YUV_blit_YUY2@48");
        YUV_blit_YUY252 = GetProcAddress(dll, "_YUV_blit_YUY2@52");
        YUV_blit_YUY2_mask48 = GetProcAddress(dll, "_YUV_blit_YUY2_mask@48");
        YUV_blit_YUY2_mask56 = GetProcAddress(dll, "_YUV_blit_YUY2_mask@56");
        YUV_blit_YUY2_mask60 = GetProcAddress(dll, "_YUV_blit_YUY2_mask@60");
        YUV_blit_YV1244 = GetProcAddress(dll, "_YUV_blit_YV12@44");
        YUV_blit_YV1252 = GetProcAddress(dll, "_YUV_blit_YV12@52");
        YUV_blit_YV1256 = GetProcAddress(dll, "_YUV_blit_YV12@56");
        YUV_init4 = GetProcAddress(dll, "_YUV_init@4");
        radfree = GetProcAddress(dll, "_radfree@4");
        radmalloc = GetProcAddress(dll, "_radmalloc@4");
    }
} binkw32;

extern "C" __declspec(naked) void __stdcall _BinkBufferBlit(int, int, int) { _asm { jmp[binkw32.BinkBufferBlit] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferCheckWinPos(int, int, int) { _asm { jmp[binkw32.BinkBufferCheckWinPos] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferClear(int, int) { _asm { jmp[binkw32.BinkBufferClear] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferClose(int) { _asm { jmp[binkw32.BinkBufferClose] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferGetDescription(int) { _asm { jmp[binkw32.BinkBufferGetDescription] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferGetError() { _asm { jmp[binkw32.BinkBufferGetError] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferLock(int) { _asm { jmp[binkw32.BinkBufferLock] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferOpen(int, int, int, int) { _asm { jmp[binkw32.BinkBufferOpen] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferSetDirectDraw(int, int) { _asm { jmp[binkw32.BinkBufferSetDirectDraw] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferSetHWND(int, int) { _asm { jmp[binkw32.BinkBufferSetHWND] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferSetOffset(int, int, int) { _asm { jmp[binkw32.BinkBufferSetOffset] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferSetResolution(int, int, int) { _asm { jmp[binkw32.BinkBufferSetResolution] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferSetScale(int, int, int) { _asm { jmp[binkw32.BinkBufferSetScale] } }
extern "C" __declspec(naked) void __stdcall _BinkBufferUnlock(int) { _asm { jmp[binkw32.BinkBufferUnlock] } }
extern "C" __declspec(naked) void __stdcall _BinkCheckCursor(int, int, int, int, int) { _asm { jmp[binkw32.BinkCheckCursor] } }
extern "C" __declspec(naked) void __stdcall _BinkClose(int) { _asm { jmp[binkw32.BinkClose] } }
extern "C" __declspec(naked) void __stdcall _BinkCloseTrack(int) { _asm { jmp[binkw32.BinkCloseTrack] } }
extern "C" __declspec(naked) void __stdcall _BinkControlBackgroundIO(int, int) { _asm { jmp[binkw32.BinkControlBackgroundIO] } }
extern "C" __declspec(naked) void __stdcall _BinkControlPlatformFeatures(int, int) { _asm { jmp[binkw32.BinkControlPlatformFeatures] } }
extern "C" __declspec(naked) void __stdcall _BinkCopyToBuffer(int, int, int, int, int, int, int) { _asm { jmp[binkw32.BinkCopyToBuffer] } }
extern "C" __declspec(naked) void __stdcall _BinkCopyToBufferRect(int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.BinkCopyToBufferRect] } }
extern "C" __declspec(naked) void __stdcall _BinkDDSurfaceType(int) { _asm { jmp[binkw32.BinkDDSurfaceType] } }
extern "C" __declspec(naked) void __stdcall _BinkDX8SurfaceType(int) { _asm { jmp[binkw32.BinkDX8SurfaceType] } }
extern "C" __declspec(naked) void __stdcall _BinkDX9SurfaceType(int) { _asm { jmp[binkw32.BinkDX9SurfaceType] } }
extern "C" __declspec(naked) void __stdcall _BinkDoFrame(int) { _asm { jmp[binkw32.BinkDoFrame] } }
extern "C" __declspec(naked) void __stdcall _BinkDoFrameAsync(int, int, int) { _asm { jmp[binkw32.BinkDoFrameAsync] } }
extern "C" __declspec(naked) void __stdcall _BinkDoFrameAsyncWait(int, int) { _asm { jmp[binkw32.BinkDoFrameAsyncWait] } }
extern "C" __declspec(naked) void __stdcall _BinkDoFramePlane(int, int) { _asm { jmp[binkw32.BinkDoFramePlane] } }
extern "C" __declspec(naked) void __stdcall _BinkFreeGlobals() { _asm { jmp[binkw32.BinkFreeGlobals] } }
extern "C" __declspec(naked) void __stdcall _BinkGetError() { _asm { jmp[binkw32.BinkGetError] } }
extern "C" __declspec(naked) void __stdcall _BinkGetFrameBuffersInfo(int, int) { _asm { jmp[binkw32.BinkGetFrameBuffersInfo] } }
extern "C" __declspec(naked) void __stdcall _BinkGetKeyFrame(int, int, int) { _asm { jmp[binkw32.BinkGetKeyFrame] } }
extern "C" __declspec(naked) void __stdcall _BinkGetPalette(int) { _asm { jmp[binkw32.BinkGetPalette] } }
extern "C" __declspec(naked) void __stdcall _BinkGetPlatformInfo(int, int) { _asm { jmp[binkw32.BinkGetPlatformInfo] } }
extern "C" __declspec(naked) void __stdcall _BinkGetRealtime(int, int, int) { _asm { jmp[binkw32.BinkGetRealtime] } }
extern "C" __declspec(naked) void __stdcall _BinkGetRects(int, int) { _asm { jmp[binkw32.BinkGetRects] } }
extern "C" __declspec(naked) void __stdcall _BinkGetSummary(int, int) { _asm { jmp[binkw32.BinkGetSummary] } }
extern "C" __declspec(naked) void __stdcall _BinkGetTrackData(int, int) { _asm { jmp[binkw32.BinkGetTrackData] } }
extern "C" __declspec(naked) void __stdcall _BinkGetTrackID(int, int) { _asm { jmp[binkw32.BinkGetTrackID] } }
extern "C" __declspec(naked) void __stdcall _BinkGetTrackMaxSize(int, int) { _asm { jmp[binkw32.BinkGetTrackMaxSize] } }
extern "C" __declspec(naked) void __stdcall _BinkGetTrackType(int, int) { _asm { jmp[binkw32.BinkGetTrackType] } }
extern "C" __declspec(naked) void __stdcall _BinkGoto(int, int, int) { _asm { jmp[binkw32.BinkGoto] } }
extern "C" __declspec(naked) void __stdcall _BinkIsSoftwareCursor(int, int) { _asm { jmp[binkw32.BinkIsSoftwareCursor] } }
extern "C" __declspec(naked) void __stdcall _BinkLogoAddress() { _asm { jmp[binkw32.BinkLogoAddress] } }
extern "C" __declspec(naked) void __stdcall _BinkNextFrame(int) { _asm { jmp[binkw32.BinkNextFrame] } }
extern "C" __declspec(naked) void __stdcall _BinkOpen(int, int) { _asm { jmp[binkw32.BinkOpen] } }
extern "C" __declspec(naked) void __stdcall _BinkOpenDirectSound(int) { _asm { jmp[binkw32.BinkOpenDirectSound] } }
extern "C" __declspec(naked) void __stdcall _BinkOpenMiles(int) { _asm { jmp[binkw32.BinkOpenMiles] } }
extern "C" __declspec(naked) void __stdcall _BinkOpenTrack(int, int) { _asm { jmp[binkw32.BinkOpenTrack] } }
extern "C" __declspec(naked) void __stdcall _BinkOpenWaveOut(int) { _asm { jmp[binkw32.BinkOpenWaveOut] } }
extern "C" __declspec(naked) void __stdcall _BinkOpenWithOptions(int, int, int) { _asm { jmp[binkw32.BinkOpenWithOptions] } }
extern "C" __declspec(naked) void __stdcall _BinkOpenXAudio2(int) { _asm { jmp[binkw32.BinkOpenXAudio2] } }
extern "C" __declspec(naked) void __stdcall _BinkPause(int, int) { _asm { jmp[binkw32.BinkPause] } }
extern "C" __declspec(naked) void __stdcall _BinkRegisterFrameBuffers(int, int) { _asm { jmp[binkw32.BinkRegisterFrameBuffers] } }
extern "C" __declspec(naked) void __stdcall _BinkRequestStopAsyncThread(int) { _asm { jmp[binkw32.BinkRequestStopAsyncThread] } }
extern "C" __declspec(naked) void __stdcall _BinkRestoreCursor(int) { _asm { jmp[binkw32.BinkRestoreCursor] } }
extern "C" __declspec(naked) void __stdcall _BinkService(int) { _asm { jmp[binkw32.BinkService] } }
extern "C" __declspec(naked) void __stdcall _BinkServiceSound() { _asm { jmp[binkw32.BinkServiceSound] } }
extern "C" __declspec(naked) void __stdcall _BinkSetError(int) { _asm { jmp[binkw32.BinkSetError] } }
extern "C" __declspec(naked) void __stdcall _BinkSetFileOffset(int, int) { _asm { jmp[binkw32.BinkSetFileOffset] } }
extern "C" __declspec(naked) void __stdcall _BinkSetFrameRate(int, int) { _asm { jmp[binkw32.BinkSetFrameRate] } }
extern "C" __declspec(naked) void __stdcall _BinkSetIO(int) { _asm { jmp[binkw32.BinkSetIO] } }
extern "C" __declspec(naked) void __stdcall _BinkSetIOSize(int) { _asm { jmp[binkw32.BinkSetIOSize] } }
extern "C" __declspec(naked) void __stdcall _BinkSetMemory(int, int) { _asm { jmp[binkw32.BinkSetMemory] } }
extern "C" __declspec(naked) void __stdcall _BinkSetMixBinVolumes(int, int, int, int, int) { _asm { jmp[binkw32.BinkSetMixBinVolumes] } }
extern "C" __declspec(naked) void __stdcall _BinkSetMixBins(int, int, int, int) { _asm { jmp[binkw32.BinkSetMixBins] } }
extern "C" __declspec(naked) void __stdcall _BinkSetPan(int, int, int) { _asm { jmp[binkw32.BinkSetPan] } }
extern "C" __declspec(naked) void __stdcall _BinkSetSimulate(int) { _asm { jmp[binkw32.BinkSetSimulate] } }
extern "C" __declspec(naked) void __stdcall _BinkSetSoundOnOff(int, int) { _asm { jmp[binkw32.BinkSetSoundOnOff] } }
extern "C" __declspec(naked) void __stdcall _BinkSetSoundSystem(int, int) { _asm { jmp[binkw32.BinkSetSoundSystem] } }
extern "C" __declspec(naked) void __stdcall _BinkSetSoundTrack(int, int) { _asm { jmp[binkw32.BinkSetSoundTrack] } }
extern "C" __declspec(naked) void __stdcall _BinkSetSpeakerVolumes(int, int, int, int, int) { _asm { jmp[binkw32.BinkSetSpeakerVolumes] } }
extern "C" __declspec(naked) void __stdcall _BinkSetVideoOnOff(int, int) { _asm { jmp[binkw32.BinkSetVideoOnOff] } }
extern "C" __declspec(naked) void __stdcall _BinkSetVolume(int, int, int) { _asm { jmp[binkw32.BinkSetVolume] } }
extern "C" __declspec(naked) void __stdcall _BinkSetWillLoop(int, int) { _asm { jmp[binkw32.BinkSetWillLoop] } }
extern "C" __declspec(naked) void __stdcall _BinkShouldSkip(int) { _asm { jmp[binkw32.BinkShouldSkip] } }
extern "C" __declspec(naked) void __stdcall _BinkStartAsyncThread(int, int) { _asm { jmp[binkw32.BinkStartAsyncThread] } }
extern "C" __declspec(naked) void __stdcall _BinkUseTelemetry(int) { _asm { jmp[binkw32.BinkUseTelemetry] } }
extern "C" __declspec(naked) void __stdcall _BinkUseTmLite(int) { _asm { jmp[binkw32.BinkUseTmLite] } }
extern "C" __declspec(naked) void __stdcall _BinkWait(int) { _asm { jmp[binkw32.BinkWait] } }
extern "C" __declspec(naked) void __stdcall _BinkWaitStopAsyncThread(int) { _asm { jmp[binkw32.BinkWaitStopAsyncThread] } }
extern "C" __declspec(naked) void __stdcall _RADSetMemory(int, int) { _asm { jmp[binkw32.RADSetMemory] } }
extern "C" __declspec(naked) void __stdcall _RADTimerRead() { _asm { jmp[binkw32.RADTimerRead] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a1bpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a1bpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a1bpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a1bpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a1bpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a1bpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a1bpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a1bpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a4bpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a4bpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a4bpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a4bpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a4bpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a4bpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16a4bpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16a4bpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16bpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16bpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16bpp48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16bpp48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16bpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16bpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16bpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16bpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16bpp_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16bpp_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_16bpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_16bpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24bpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24bpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24bpp48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24bpp48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24bpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24bpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24bpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24bpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24bpp_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24bpp_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24bpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24bpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24rbpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24rbpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24rbpp48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24rbpp48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24rbpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24rbpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24rbpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24rbpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24rbpp_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24rbpp_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_24rbpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_24rbpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32abpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32abpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32abpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32abpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32abpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32abpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32abpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32abpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32bpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32bpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32bpp48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32bpp48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32bpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32bpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32bpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32bpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32bpp_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32bpp_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32bpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32bpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rabpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rabpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rabpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rabpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rabpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rabpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rabpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rabpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rbpp40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rbpp40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rbpp48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rbpp48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rbpp52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rbpp52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rbpp_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rbpp_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rbpp_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rbpp_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_32rbpp_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_32rbpp_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_UYVY40(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_UYVY40] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_UYVY48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_UYVY48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_UYVY52(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_UYVY52] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_UYVY_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_UYVY_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_UYVY_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_UYVY_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_UYVY_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_UYVY_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YUY240(int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YUY240] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YUY248(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YUY248] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YUY252(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YUY252] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YUY2_mask48(int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YUY2_mask48] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YUY2_mask56(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YUY2_mask56] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YUY2_mask60(int, int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YUY2_mask60] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YV1244(int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YV1244] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YV1252(int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YV1252] } }
extern "C" __declspec(naked) void __stdcall _YUV_blit_YV1256(int, int, int, int, int, int, int, int, int, int, int, int, int, int) { _asm { jmp[binkw32.YUV_blit_YV1256] } }
extern "C" __declspec(naked) void __stdcall _YUV_init4(int) { _asm { jmp[binkw32.YUV_init4] } }
extern "C" __declspec(naked) void __stdcall _radfree(int) { _asm { jmp[binkw32.radfree] } }
extern "C" __declspec(naked) void __stdcall _radmalloc(int) { _asm { jmp[binkw32.radmalloc] } }
#endif