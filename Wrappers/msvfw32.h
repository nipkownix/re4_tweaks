#pragma once
#include "shared.h"

#if !X64
struct msvfw32_dll
{
    HMODULE dll;
    FARPROC DrawDibBegin;
    FARPROC DrawDibChangePalette;
    FARPROC DrawDibClose;
    FARPROC DrawDibDraw;
    FARPROC DrawDibEnd;
    FARPROC DrawDibGetBuffer;
    FARPROC DrawDibGetPalette;
    FARPROC DrawDibOpen;
    FARPROC DrawDibProfileDisplay;
    FARPROC DrawDibRealize;
    FARPROC DrawDibSetPalette;
    FARPROC DrawDibStart;
    FARPROC DrawDibStop;
    FARPROC DrawDibTime;
    FARPROC GetOpenFileNamePreview;
    FARPROC GetOpenFileNamePreviewA;
    FARPROC GetOpenFileNamePreviewW;
    FARPROC GetSaveFileNamePreviewA;
    FARPROC GetSaveFileNamePreviewW;
    FARPROC ICClose;
    FARPROC ICCompress;
    FARPROC ICCompressorChoose;
    FARPROC ICCompressorFree;
    FARPROC ICDecompress;
    FARPROC ICDraw;
    FARPROC ICDrawBegin;
    FARPROC ICGetDisplayFormat;
    FARPROC ICGetInfo;
    FARPROC ICImageCompress;
    FARPROC ICImageDecompress;
    FARPROC ICInfo;
    FARPROC ICInstall;
    FARPROC ICLocate;
    FARPROC ICMThunk32;
    FARPROC ICOpen;
    FARPROC ICOpenFunction;
    FARPROC ICRemove;
    FARPROC ICSendMessage;
    FARPROC ICSeqCompressFrame;
    FARPROC ICSeqCompressFrameEnd;
    FARPROC ICSeqCompressFrameStart;
    FARPROC MCIWndCreate;
    FARPROC MCIWndCreateA;
    FARPROC MCIWndCreateW;
    FARPROC MCIWndRegisterClass;
    FARPROC StretchDIB;
    FARPROC VideoForWindowsVersion;

    void LoadOriginalLibrary(HMODULE module)
    {
        dll = module;
        shared.LoadOriginalLibrary(dll);
        DrawDibBegin = GetProcAddress(dll, "DrawDibBegin");
        DrawDibChangePalette = GetProcAddress(dll, "DrawDibChangePalette");
        DrawDibClose = GetProcAddress(dll, "DrawDibClose");
        DrawDibDraw = GetProcAddress(dll, "DrawDibDraw");
        DrawDibEnd = GetProcAddress(dll, "DrawDibEnd");
        DrawDibGetBuffer = GetProcAddress(dll, "DrawDibGetBuffer");
        DrawDibGetPalette = GetProcAddress(dll, "DrawDibGetPalette");
        DrawDibOpen = GetProcAddress(dll, "DrawDibOpen");
        DrawDibProfileDisplay = GetProcAddress(dll, "DrawDibProfileDisplay");
        DrawDibRealize = GetProcAddress(dll, "DrawDibRealize");
        DrawDibSetPalette = GetProcAddress(dll, "DrawDibSetPalette");
        DrawDibStart = GetProcAddress(dll, "DrawDibStart");
        DrawDibStop = GetProcAddress(dll, "DrawDibStop");
        DrawDibTime = GetProcAddress(dll, "DrawDibTime");
        GetOpenFileNamePreview = GetProcAddress(dll, "GetOpenFileNamePreview");
        GetOpenFileNamePreviewA = GetProcAddress(dll, "GetOpenFileNamePreviewA");
        GetOpenFileNamePreviewW = GetProcAddress(dll, "GetOpenFileNamePreviewW");
        GetSaveFileNamePreviewA = GetProcAddress(dll, "GetSaveFileNamePreviewA");
        GetSaveFileNamePreviewW = GetProcAddress(dll, "GetSaveFileNamePreviewW");
        ICClose = GetProcAddress(dll, "ICClose");
        ICCompress = GetProcAddress(dll, "ICCompress");
        ICCompressorChoose = GetProcAddress(dll, "ICCompressorChoose");
        ICCompressorFree = GetProcAddress(dll, "ICCompressorFree");
        ICDecompress = GetProcAddress(dll, "ICDecompress");
        ICDraw = GetProcAddress(dll, "ICDraw");
        ICDrawBegin = GetProcAddress(dll, "ICDrawBegin");
        ICGetDisplayFormat = GetProcAddress(dll, "ICGetDisplayFormat");
        ICGetInfo = GetProcAddress(dll, "ICGetInfo");
        ICImageCompress = GetProcAddress(dll, "ICImageCompress");
        ICImageDecompress = GetProcAddress(dll, "ICImageDecompress");
        ICInfo = GetProcAddress(dll, "ICInfo");
        ICInstall = GetProcAddress(dll, "ICInstall");
        ICLocate = GetProcAddress(dll, "ICLocate");
        ICMThunk32 = GetProcAddress(dll, "ICMThunk32");
        ICOpen = GetProcAddress(dll, "ICOpen");
        ICOpenFunction = GetProcAddress(dll, "ICOpenFunction");
        ICRemove = GetProcAddress(dll, "ICRemove");
        ICSendMessage = GetProcAddress(dll, "ICSendMessage");
        ICSeqCompressFrame = GetProcAddress(dll, "ICSeqCompressFrame");
        ICSeqCompressFrameEnd = GetProcAddress(dll, "ICSeqCompressFrameEnd");
        ICSeqCompressFrameStart = GetProcAddress(dll, "ICSeqCompressFrameStart");
        MCIWndCreate = GetProcAddress(dll, "MCIWndCreate");
        MCIWndCreateA = GetProcAddress(dll, "MCIWndCreateA");
        MCIWndCreateW = GetProcAddress(dll, "MCIWndCreateW");
        MCIWndRegisterClass = GetProcAddress(dll, "MCIWndRegisterClass");
        StretchDIB = GetProcAddress(dll, "StretchDIB");
        VideoForWindowsVersion = GetProcAddress(dll, "VideoForWindowsVersion");
    }
} msvfw32;

__declspec(naked) void _DrawDibBegin() { _asm { jmp[msvfw32.DrawDibBegin] } }
__declspec(naked) void _DrawDibChangePalette() { _asm { jmp[msvfw32.DrawDibChangePalette] } }
__declspec(naked) void _DrawDibClose() { _asm { jmp[msvfw32.DrawDibClose] } }
__declspec(naked) void _DrawDibDraw() { _asm { jmp[msvfw32.DrawDibDraw] } }
__declspec(naked) void _DrawDibEnd() { _asm { jmp[msvfw32.DrawDibEnd] } }
__declspec(naked) void _DrawDibGetBuffer() { _asm { jmp[msvfw32.DrawDibGetBuffer] } }
__declspec(naked) void _DrawDibGetPalette() { _asm { jmp[msvfw32.DrawDibGetPalette] } }
__declspec(naked) void _DrawDibOpen() { _asm { jmp[msvfw32.DrawDibOpen] } }
__declspec(naked) void _DrawDibProfileDisplay() { _asm { jmp[msvfw32.DrawDibProfileDisplay] } }
__declspec(naked) void _DrawDibRealize() { _asm { jmp[msvfw32.DrawDibRealize] } }
__declspec(naked) void _DrawDibSetPalette() { _asm { jmp[msvfw32.DrawDibSetPalette] } }
__declspec(naked) void _DrawDibStart() { _asm { jmp[msvfw32.DrawDibStart] } }
__declspec(naked) void _DrawDibStop() { _asm { jmp[msvfw32.DrawDibStop] } }
__declspec(naked) void _DrawDibTime() { _asm { jmp[msvfw32.DrawDibTime] } }
__declspec(naked) void _GetOpenFileNamePreview() { _asm { jmp[msvfw32.GetOpenFileNamePreview] } }
__declspec(naked) void _GetOpenFileNamePreviewA() { _asm { jmp[msvfw32.GetOpenFileNamePreviewA] } }
__declspec(naked) void _GetOpenFileNamePreviewW() { _asm { jmp[msvfw32.GetOpenFileNamePreviewW] } }
__declspec(naked) void _GetSaveFileNamePreviewA() { _asm { jmp[msvfw32.GetSaveFileNamePreviewA] } }
__declspec(naked) void _GetSaveFileNamePreviewW() { _asm { jmp[msvfw32.GetSaveFileNamePreviewW] } }
__declspec(naked) void _ICClose() { _asm { jmp[msvfw32.ICClose] } }
__declspec(naked) void _ICCompress() { _asm { jmp[msvfw32.ICCompress] } }
__declspec(naked) void _ICCompressorChoose() { _asm { jmp[msvfw32.ICCompressorChoose] } }
__declspec(naked) void _ICCompressorFree() { _asm { jmp[msvfw32.ICCompressorFree] } }
__declspec(naked) void _ICDecompress() { _asm { jmp[msvfw32.ICDecompress] } }
__declspec(naked) void _ICDraw() { _asm { jmp[msvfw32.ICDraw] } }
__declspec(naked) void _ICDrawBegin() { _asm { jmp[msvfw32.ICDrawBegin] } }
__declspec(naked) void _ICGetDisplayFormat() { _asm { jmp[msvfw32.ICGetDisplayFormat] } }
__declspec(naked) void _ICGetInfo() { _asm { jmp[msvfw32.ICGetInfo] } }
__declspec(naked) void _ICImageCompress() { _asm { jmp[msvfw32.ICImageCompress] } }
__declspec(naked) void _ICImageDecompress() { _asm { jmp[msvfw32.ICImageDecompress] } }
__declspec(naked) void _ICInfo() { _asm { jmp[msvfw32.ICInfo] } }
__declspec(naked) void _ICInstall() { _asm { jmp[msvfw32.ICInstall] } }
__declspec(naked) void _ICLocate() { _asm { jmp[msvfw32.ICLocate] } }
__declspec(naked) void _ICMThunk32() { _asm { jmp[msvfw32.ICMThunk32] } }
__declspec(naked) void _ICOpen() { _asm { jmp[msvfw32.ICOpen] } }
__declspec(naked) void _ICOpenFunction() { _asm { jmp[msvfw32.ICOpenFunction] } }
__declspec(naked) void _ICRemove() { _asm { jmp[msvfw32.ICRemove] } }
__declspec(naked) void _ICSendMessage() { _asm { jmp[msvfw32.ICSendMessage] } }
__declspec(naked) void _ICSeqCompressFrame() { _asm { jmp[msvfw32.ICSeqCompressFrame] } }
__declspec(naked) void _ICSeqCompressFrameEnd() { _asm { jmp[msvfw32.ICSeqCompressFrameEnd] } }
__declspec(naked) void _ICSeqCompressFrameStart() { _asm { jmp[msvfw32.ICSeqCompressFrameStart] } }
__declspec(naked) void _MCIWndCreate() { _asm { jmp[msvfw32.MCIWndCreate] } }
__declspec(naked) void _MCIWndCreateA() { _asm { jmp[msvfw32.MCIWndCreateA] } }
__declspec(naked) void _MCIWndCreateW() { _asm { jmp[msvfw32.MCIWndCreateW] } }
__declspec(naked) void _MCIWndRegisterClass() { _asm { jmp[msvfw32.MCIWndRegisterClass] } }
__declspec(naked) void _StretchDIB() { _asm { jmp[msvfw32.StretchDIB] } }
__declspec(naked) void _VideoForWindowsVersion() { _asm { jmp[msvfw32.VideoForWindowsVersion] } }
#endif