#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"
#include "..\external\MinHook\MinHook.h"

std::string RealDllPath;
std::string WrapperMode;
std::string WrapperName;
std::string game_version;
std::string QTE_key_1;
std::string QTE_key_2;
std::string flip_item_up;
std::string flip_item_down;
std::string flip_item_left;
std::string flip_item_right;

HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;

//#define VERBOSE

float fFOVAdditional = 0.0f;
float fQTESpeedMult = 1.5f;
float newMovPosX;
float newMovNegX;
float newMovPosY;
float newMovNegY;

double fDefaultEngineWidthScale = 1280.0;
double fDefaultEngineAspectRatio = 1.777777791;
double fDefaultAspectRatio = 1.333333373;
double fNewInvItemNamePos;
double fNewFilesTitlePos;
double fNewFilesItemsPos;
double fNewMapIconsPos;
double fNewMerchItemListPos;
double fNewMerchItemDescPos;
double fNewMerchGreetingPos;
double fNewTItemNamesPos;
double fNewRadioNamesPos;

bool bShouldFlipX;
bool bShouldFlipY;
bool bIsItemUp;
bool bFixSniperZoom;
bool bFixVsyncToggle;
bool bFixUltraWideAspectRatio;
bool bFixQTE;
bool bSkipIntroLogos;
bool bFixRetryLoadMouseSelector;
bool bRestorePickupTransparency;
bool bFixBlurryImage;
bool bAllowHighResolutionSFD;
bool bDisableFilmGrain;
bool bRaiseVertexAlloc;
bool bRaiseInventoryAlloc;
bool bUseMemcpy;
bool bIgnoreFPSWarning;
bool bWindowBorderless;
bool bEnableGCBlur;

uintptr_t* ptrResMovAddr;
uintptr_t* ptrSFDMovAddr;
uintptr_t* ptrInvMovAddr;
uintptr_t ptrRetryLoadDLGstate;

static uint32_t ptrGameState;
static uint32_t* ptrGameFrameRate;
static uint32_t* ptrEngineWidthScale;
static uint32_t* ptrAspectRatio;

static uint32_t* ptrcSofdec__startApp;
static uint32_t* ptrmwPlyCalcWorkCprmSfd;
static uint32_t* ptrcSofdec__finishMovie;
static uint32_t* ptrMemPoolMovie;

static uint32_t* ptrpG;
static uint32_t* ptrpzzl_size;
static uint32_t* ptrstageInit;
static uint32_t* ptrSubScreenAramRead;
static uint32_t* ptrp_MemPool_SubScreen1;
static uint32_t* ptrp_MemPool_SubScreen2;
static uint32_t* ptrp_MemPool_SubScreen3;
static uint32_t* ptrp_MemPool_SubScreen4;
static uint32_t* ptrp_MemPool_SubScreen5;
static uint32_t* ptrp_MemPool_SubScreen6;
static uint32_t* ptrp_MemPool_SubScreen7;
static uint32_t* ptrp_MemPool_SubScreen8;
static uint32_t* ptrp_MemPool_SubScreen9;
static uint32_t* ptrp_MemPool_SubScreen10;
static uint32_t* ptrp_MemPool_SubScreen11;
static uint32_t* ptrp_MemPool_SubScreen12;
static uint32_t* ptrp_MemPool_SubScreen13;
static uint32_t* ptrp_MemPool_SubScreen14;
static uint32_t* ptrp_MemPool_SubScreen15;
static uint32_t* ptrp_MemPool_SubScreen16;
static uint32_t* ptrp_MemPool_SubScreen17;
static uint32_t* ptrg_MemPool_SubScreen1;
static uint32_t* ptrg_MemPool_SubScreen2;
static uint32_t* ptrg_MemPool_SubScreen3;
static uint32_t* ptrg_MemPool_SubScreen4;
static uint32_t* ptrg_MemPool_SubScreen5;
static uint32_t* ptrg_MemPool_SubScreen6;

int iWindowPositionX;
int iWindowPositionY;
int intQTE_key_1;
int intQTE_key_2;
int intGameState;

// SFD functions
typedef int(__cdecl* mwPlyCalcWorkCprmSfd_Fn)(/* MwsfdCrePrm * */ void* cprm);
mwPlyCalcWorkCprmSfd_Fn mwPlyCalcWorkCprmSfd_Orig;
int __cdecl mwPlyCalcWorkCprmSfd_Hook(/* MwsfdCrePrm * */ void* cprm)
{
	int workarea_size = mwPlyCalcWorkCprmSfd_Orig(cprm);

	// alloc a workarea based on what mwPlyCalcWorkCprmSfd_Orig returned - allows high-res videos to play without crashing :)
	void* workarea = malloc(workarea_size);

	void** g_MemPoolMovie = (void**)(ptrMemPoolMovie);
	*g_MemPoolMovie = workarea;

	return workarea_size;
}

typedef void(__fastcall* cSofdec__startApp_Fn)(void* thisptr);
cSofdec__startApp_Fn cSofdec__startApp_Orig;

void __fastcall cSofdec__startApp_Hook(void* thisptr)
{
	// Backup original g_MemPoolMovie value
	// we need to change this so that cSofdec::startApp uses our own memory block instead of the hardcoded (& incorrectly-sized) memory pool
	// a few other things rely on g_MemPoolMovie value though, so it needs to be changed back after this func has ran
	// unfortunately we dont know the size of g_MemPoolMovie until mwPlyCalcWorkCprmSfd is called though (during cSofdec::startApp)
	// so a mwPlyCalcWorkCprmSfd hook is used to handle allocating & updating g_MemPoolMovie, while cSofdec::startApp hook is just used to backup/restore original addr

	void** g_MemPoolMovie = (void**)(ptrMemPoolMovie);
	void* g_MemPoolMovie_Original = *g_MemPoolMovie;

	cSofdec__startApp_Orig(thisptr);

	*g_MemPoolMovie = g_MemPoolMovie_Original;
}

typedef void(__fastcall* cSofdec__finishMovie_Fn)(uint8_t* thisptr);
cSofdec__finishMovie_Fn cSofdec__finishMovie_Orig;
void __fastcall cSofdec__finishMovie_Hook(uint8_t* thisptr)
{
	cSofdec__finishMovie_Orig(thisptr);

	// cleanup the workarea we allocated
	// ps2 version seems to memset(0) the workarea here, PC version doesn't do this though
	// should mean it's safe to assume the workarea isn't being used anymore, so we can free it here
	void* workarea = *(void**)(thisptr + 0x130);
	free(workarea);
}

// Inventory screen mem functions
uint8_t g_MemPool_SubScreen[0x4000000];
uint8_t g_MemPool_SubScreen2[0x4000000];

uint8_t* p_MemPool_SubScreen = (uint8_t*)&g_MemPool_SubScreen;

typedef bool(__fastcall* MessageControl__stageInit_Fn)(void* thisptr);
MessageControl__stageInit_Fn MessageControl__stageInit_Orig;

bool __fastcall MessageControl__stageInit_Hook(void* thisptr)
{
	bool ret = MessageControl__stageInit_Orig(thisptr);

	// Set pG+0x3C to point to an extended buffer we control
	// The game uses this for some MemorySwap operation later on - not really sure why that swap is needed though, but oh well
	uint8_t* pG = *(uint8_t**)(ptrpG);

	*(void**)(pG + 0x3C) = &g_MemPool_SubScreen2;

	return ret;
}

typedef int(*SubScreenAramRead_Fn)();
SubScreenAramRead_Fn SubScreenAramRead_Orig;
int SubScreenAramRead_Hook()
{
	int pzzl_size = SubScreenAramRead_Orig();

	// Patch SubScreenExec heap size, seems to be max size to read from ss_pzzl/omk_pzzl file
	injector::WriteMemory<int>(ptrpzzl_size, pzzl_size, true);

	return pzzl_size;
}

HWND __stdcall CreateWindowExA_Hook(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	int windowX = iWindowPositionX < 0 ? CW_USEDEFAULT : iWindowPositionX;
	int windowY = iWindowPositionY < 0 ? CW_USEDEFAULT : iWindowPositionY;
	return CreateWindowExA(dwExStyle, lpClassName, lpWindowName, bWindowBorderless ? WS_POPUP : dwStyle, windowX, windowY, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

BOOL __stdcall SetWindowPos_Hook(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
	int windowX = iWindowPositionX < 0 ? 0 : iWindowPositionX;
	int windowY = iWindowPositionY < 0 ? 0 : iWindowPositionY;
	return SetWindowPos(hWnd, hWndInsertAfter, windowX, windowY, cx, cy, uFlags);
}

struct Filter01Params
{
	void* OtHandle; // OT = render pass or something
	int Distance;
	float AlphaLevel;
	uint8_t FocusMode;
	uint8_t UnkD; // if set then it uses code-path with constant 4-pass loop, else uses pass with loop based on AlphaLevel
};

float* ptr_InternalWidth;
float* ptr_InternalHeight;
float* ptr_InternalHeightScale;
int* ptr_RenderHeight;
uint32_t* ptr_filter01_buff;

typedef void(__cdecl* GXBegin_Fn)(int a1, int a2, short a3);
typedef void(__cdecl* GXPosition3f32_Fn)(float a1, float a2, float a3);
typedef void(__cdecl* GXColor4u8_Fn)(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
typedef void(__cdecl* GXTexCoord2f32_Fn)(float a1, float a2);
typedef void(__cdecl* GXShaderCall_Maybe_Fn)(int a1);

typedef void(__cdecl* GXSetTexCopySrc_Fn)(short a1, short a2, short a3, short a4);
typedef void(__cdecl* GXSetTexCopyDst_Fn)(short a1, short a2, int a3, char a4);
typedef void(__cdecl* GXCopyTex_Fn)(uint32_t a1, char a2, int a3);

GXBegin_Fn GXBegin = nullptr;
GXPosition3f32_Fn GXPosition3f32 = nullptr;
GXColor4u8_Fn GXColor4u8 = nullptr;
GXTexCoord2f32_Fn GXTexCoord2f32 = nullptr;
GXShaderCall_Maybe_Fn GXShaderCall_Maybe = nullptr;
GXSetTexCopySrc_Fn GXSetTexCopySrc = nullptr;
GXSetTexCopyDst_Fn GXSetTexCopyDst = nullptr;
GXCopyTex_Fn GXCopyTex = nullptr;

void __cdecl Filter01Render_Hook1(Filter01Params* params)
{
	float blurPositionOffset = params->AlphaLevel * 0.33f;

	float GameWidth = *ptr_InternalWidth;
	float GameHeight = *ptr_InternalHeight;

	// Code that was in original Filter01Render
	// Seems to usually be 1/8th of 448, might change depending on ptr_InternalHeightScale / ptr_RenderHeight though
	// Offset is used to fix Y coords of the blur effect, otherwise the blur ends up offset from the image for some reason
	float heightScaled = *ptr_InternalHeightScale * 448.0f;
	float offsetY = 448.0f * (heightScaled - *ptr_RenderHeight) / (heightScaled + heightScaled);

	float deltaX = 0;
	float deltaY = 0;
	float posZ = params->Distance;
	for (int loopIdx = 0; loopIdx < 4; loopIdx++)
	{
		// Main reason for this hook is to add the deltaX/deltaY vars below, needed for the DoF effect to work properly
		// GC debug had this, but was removed in X360 port onwards for some reason
		// (yet even though this was removed, the X360 devs did add the code for offsetY above to fix the blur...)

		if (loopIdx == 0) {
			deltaX = 0.0 - blurPositionOffset;
			deltaY = 0.0 - blurPositionOffset;
		}
		else if (loopIdx == 1) {
			deltaX = 0.0 + blurPositionOffset;
			deltaY = 0.0 + blurPositionOffset;
		}
		else if (loopIdx == 2) {
			deltaX = 0.0 + blurPositionOffset;
			deltaY = 0.0 - blurPositionOffset;
		}
		else if (loopIdx == 3) {
			deltaX = 0.0 - blurPositionOffset;
			deltaY = 0.0 + blurPositionOffset;
		}

		posZ = posZ + 100.f;
		if (posZ > 65536)
			posZ = 65536;

		GXBegin(0x80, 0, 4);

		GXPosition3f32(0.0 + deltaX, 0.0 + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(0.0, 0.0);

		GXPosition3f32(0.0 + deltaX + GameWidth, 0.0 + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(1.0, 0.0);

		GXPosition3f32(0.0 + deltaX + GameWidth, 0.0 + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(1.0, 1.0);

		GXPosition3f32(0.0 + deltaX, 0.0 + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(0.0, 1.0);

		GXShaderCall_Maybe(0xE);
	}

	if (params->AlphaLevel > 1)
	{
		float blurAlpha = (params->AlphaLevel - 1) * 25;
		if (blurAlpha > 255)
			blurAlpha = 255;

		if (blurAlpha > 0)
		{
			GXSetTexCopySrc(0, 0, GameWidth, GameHeight);
			GXSetTexCopyDst(((int)GameWidth) >> 1, ((int)GameHeight) >> 1, 6, 1);
			GXCopyTex(*ptr_filter01_buff, 0, 0);

			GXBegin(0x80, 0, 4);

			GXPosition3f32(0.25 + deltaX, 0.25 + deltaY + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (int)blurAlpha);
			GXTexCoord2f32(0.0, 0.0);

			GXPosition3f32(0.25 + deltaX + GameWidth, 0.25 + deltaY + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (int)blurAlpha);
			GXTexCoord2f32(1.0, 0.0);

			GXPosition3f32(0.25 + deltaX + GameWidth, 0.25 + deltaY + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (int)blurAlpha);
			GXTexCoord2f32(1.0, 1.0);

			GXPosition3f32(0.25 + deltaX, 0.25 + deltaY + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (int)blurAlpha);
			GXTexCoord2f32(0.0, 1.0);

			GXShaderCall_Maybe(0xE);
		}
	}
}

void __cdecl Filter01Render_Hook2(Filter01Params* params)
{
	float GameWidth = *ptr_InternalWidth;
	float GameHeight = *ptr_InternalHeight;

	float AlphaLvlTable[] = { // level_tbl1 in GC debug
		0,
		0.60, // 0.60000002
		1.5,
		2.60, // 2.5999999
		1.5,
		1.6,
		1.6,
		2.5,
		2.60, // 2.5999999
		4.5,
		6.60 // 6.5999999
	};

	int AlphaLevel_floor = (int)floorf(params->AlphaLevel); // TODO: should this be ceil instead?
	float blurPositionOffset = AlphaLvlTable[AlphaLevel_floor];

	int numBlurPasses = 4;
	switch (AlphaLevel_floor)
	{
	case 0:
		numBlurPasses = 0;
		break;
	case 1:
	case 2:
	case 3:
		numBlurPasses = 1;
		break;
	case 4:
		numBlurPasses = 2;
		break;
	case 5:
		numBlurPasses = 3;
		break;
	}

	// Code that was in original Filter01Render
	// Seems to usually be 1/8th of 448, might change depending on ptr_InternalHeightScale / ptr_RenderHeight though
	// Offset is used to fix Y coords of the blur effect, otherwise the blur ends up offset from the image for some reason
	float heightScaled = *ptr_InternalHeightScale * 448.0f;
	float offsetY = 448.0f * (heightScaled - *ptr_RenderHeight) / (heightScaled + heightScaled);

	float deltaX = 0;
	float deltaY = 0;
	float posZ = params->Distance;
	for (int loopIdx = 0; loopIdx < numBlurPasses; loopIdx++)
	{
		// Main reason for this hook is to add the deltaX/deltaY vars below, needed for the DoF effect to work properly
		// GC debug had this, but was removed in X360 port onwards for some reason
		// (yet even though this was removed, the X360 devs did add the code for offsetY above to fix the blur...)

		if (loopIdx == 0) {
			deltaX = 0.0 - blurPositionOffset;
			deltaY = 0.0 - blurPositionOffset;
		}
		else if (loopIdx == 1) {
			deltaX = 0.0 + blurPositionOffset;
			deltaY = 0.0 + blurPositionOffset;
		}
		else if (loopIdx == 2) {
			deltaX = 0.0 + blurPositionOffset;
			deltaY = 0.0 - blurPositionOffset;
		}
		else if (loopIdx == 3) {
			deltaX = 0.0 - blurPositionOffset;
			deltaY = 0.0 + blurPositionOffset;
		}

		posZ = posZ + 100.f;
		if (posZ > 65536)
			posZ = 65536;

		GXBegin(0xA0, 0, 4); // TODO: 0xA0 (GX_TRIANGLEFAN) in PC, 0x80 (GX_QUADS) in GC - should this be changed?

		GXPosition3f32(0.0 + deltaX, 0.0 + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(0.0, 0.0);

		GXPosition3f32(0.0 + deltaX + GameWidth, 0.0 + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(1.0, 0.0);

		GXPosition3f32(0.0 + deltaX + GameWidth, 0.0 + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(1.0, 1.0);

		GXPosition3f32(0.0 + deltaX, 0.0 + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, 0x80);
		GXTexCoord2f32(0.0, 1.0);

		GXShaderCall_Maybe(0xE);
	}
}

void ReadSettings()
{
	CIniReader iniReader("");
	fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", 0.0f);
	bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", true);
	bFixVsyncToggle = iniReader.ReadBoolean("DISPLAY", "FixVsyncToggle", true);
	bRestorePickupTransparency = iniReader.ReadBoolean("DISPLAY", "RestorePickupTransparency", true);
	bFixBlurryImage = iniReader.ReadBoolean("DISPLAY", "FixBlurryImage", true);
	bDisableFilmGrain = iniReader.ReadBoolean("DISPLAY", "DisableFilmGrain", true);
	bEnableGCBlur = iniReader.ReadBoolean("DISPLAY", "EnableGCBlur", true);
	bWindowBorderless = iniReader.ReadBoolean("DISPLAY", "WindowBorderless", false);
	iWindowPositionX = iniReader.ReadInteger("DISPLAY", "WindowPositionX", -1);
	iWindowPositionY = iniReader.ReadInteger("DISPLAY", "WindowPositionY", -1);
	bFixQTE = iniReader.ReadBoolean("MISC", "FixQTE", true);
	bSkipIntroLogos = iniReader.ReadBoolean("MISC", "SkipIntroLogos", false);
	bFixSniperZoom = iniReader.ReadBoolean("MOUSE", "FixSniperZoom", true);
	bFixRetryLoadMouseSelector = iniReader.ReadBoolean("MOUSE", "FixRetryLoadMouseSelector", true);
	flip_item_up = iniReader.ReadString("KEYBOARD", "flip_item_up", "HOME");
	flip_item_down = iniReader.ReadString("KEYBOARD", "flip_item_down", "END");
	flip_item_left = iniReader.ReadString("KEYBOARD", "flip_item_left", "INSERT");
	flip_item_right = iniReader.ReadString("KEYBOARD", "flip_item_right", "PAGEUP");
	QTE_key_1 = iniReader.ReadString("KEYBOARD", "QTE_key_1", "D");
	QTE_key_2 = iniReader.ReadString("KEYBOARD", "QTE_key_2", "A");
	bAllowHighResolutionSFD = iniReader.ReadBoolean("MOVIE", "AllowHighResolutionSFD", true);
	bRaiseVertexAlloc = iniReader.ReadBoolean("MEMORY", "RaiseVertexAlloc", true);
	bRaiseInventoryAlloc = iniReader.ReadBoolean("MEMORY", "RaiseInventoryAlloc", true);
	bUseMemcpy = iniReader.ReadBoolean("MEMORY", "UseMemcpy", true);
	bIgnoreFPSWarning = iniReader.ReadBoolean("FRAME RATE", "IgnoreFPSWarning", false);
}

void HandleAppID()
{
	//Create missing steam_appid file
	const char *filename = "steam_appid.txt";
	if (std::filesystem::exists(filename) == false) {
		std::ofstream appid(filename);
		appid << "254700";
		appid.close();
	}
}

void GetPointers()
{
	static uint32_t* StateBase;

	// pG (globals?) pointer
	auto pattern = hook::pattern("A1 ? ? ? ? B9 FF FF FF 7F 21 48 ? A1");
	ptrpG = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Current game state
	ptrGameState = injector::ReadMemory<int>(ptrpG, true) + 0x20;

	// StateBase pointer
	pattern = hook::pattern("A1 ? ? ? ? F7 40 ? ? ? ? ? 75 ? E8 ? ? ? ? 85 C0 74 ? 80 3D ? ? ? ? ? 75 ? 32 DB EB ? B3 ? E8 ? ? ? ? 85 C0 74 ? 32 DB");
	StateBase = *pattern.count(1).get(0).get<uint32_t*>(1);

	// SFD
	pattern = hook::pattern("56 57 8B F9 6A ? 8D 77 ? 6A ? 56 E8 ? ? ? ? C7 46 ? ? ? ? ? C7 06");
	ptrcSofdec__startApp = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B 74 24 ? 85 F6 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 33 C0 5E C3 8B 06");
	ptrmwPlyCalcWorkCprmSfd = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B F1 57 8D 86 ? ? ? ? 50 33 FF 68 ? ? ? ? 89 3D ? ? ? ? E8 ? ? ? ? 8B 4E");
	ptrcSofdec__finishMovie = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 15 ? ? ? ? 81 C2 ? ? ? ? 6A ? 51 89 56 ? E8 ? ? ? ? 8B 4E");
	ptrMemPoolMovie = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(2), true);

	// Inventory screen mem
	pattern = hook::pattern("05 ? ? ? ? F6 C1 ? 8B 0D ? ? ? ? A3 ? ? ? ? 8D 91 ? ? ? ? 75 ? 8D 91 ? ? ? ? 52 03 C1");
	ptrpzzl_size = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? A1 ? ? ? ? 8A 80 ? ? ? ? 56 8B F1 84 C0 0F 84 ? ? ? ? 0F B6 C8 51");
	ptrstageInit = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("55 8B EC 83 EC ? A1 ? ? ? ? 56 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 05");
	ptrSubScreenAramRead = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 0D ? ? ? ? 03 F8 81 C1 ? ? ? ? 3B F9 73 ? 89 46 ? EB ? 81 FF ? ? ? ? 73 ? 8B 15");
	ptrp_MemPool_SubScreen1 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("8B 15 ? ? ? ? 81 C2 ? ? ? ? 89 56 ? 83 0E ? 8B 06 A8 ? 74 ? 8B 4E ? 8B 56 ? 8B 45");
	ptrp_MemPool_SubScreen2 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("A1 ? ? ? ? 05 ? ? ? ? 39 46 ? 72 ? 6A ? E8 ? ? ? ? 83 C4 ? 8B 4E ? 51");
	ptrp_MemPool_SubScreen3 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("08 A1 ? ? ? ? 56 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 05 ? ? ? ? 50 6A ? 68");
	ptrp_MemPool_SubScreen4 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("8B 0D ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 8D 94 ? ? ? ? ? 52 6A");
	if (game_version == "1.0.6") // only in 1.0.6 non-debug build
		ptrp_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(2);
	else
		ptrp_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(-4);

	pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 8D 8C");
	if (game_version == "1.0.6") // only in 1.0.6 non-debug build
		ptrp_MemPool_SubScreen6 = pattern.count(1).get(0).get<uint32_t>(2);
	else
		ptrp_MemPool_SubScreen6 = pattern.count(1).get(0).get<uint32_t>(7);

	pattern = hook::pattern("8B 0D ? ? ? ? A1 ? ? ? ? 8B 40 ? 68 ? ? ? ? 81 C1 ? ? ? ? 51 50 A3");
	ptrp_MemPool_SubScreen7 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("15 ? ? ? ? 8B 86 ? ? ? ? 68 ? ? ? ? 81 C2 ? ? ? ? 52 50 E8 ? ? ? ? 6A");
	ptrp_MemPool_SubScreen8 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("8B 0D ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 53 53 8D 94 08 ? ? ? ? 52 53");
	ptrp_MemPool_SubScreen9 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("05 ? ? ? ? 39 46 ? 72 ? 6A ? E8 ? ? ? ? 83 C4 ? 8B 4E ? 51");
	ptrp_MemPool_SubScreen11 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("81 C1 ? ? ? ? 51 50 A3 ? ? ? ? 88 1D ? ? ? ? E8 ? ? ? ? 0F BE 05");
	ptrp_MemPool_SubScreen12 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("05 ? ? ? ? 50 6A ? 68 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? E8");
	ptrp_MemPool_SubScreen13 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("6A 09 6A 00 6A 00 ? ? ? ? ? ? ? ? 6A 00 68");
	ptrp_MemPool_SubScreen14 = pattern.count(2).get(0).get<uint32_t>(9);
	ptrp_MemPool_SubScreen15 = pattern.count(2).get(1).get<uint32_t>(9);

	pattern = hook::pattern("81 C2 ? ? ? ? 52 50 E8 ? ? ? ? 6A ? E8 ? ? ? ? 6A ? E8");
	ptrp_MemPool_SubScreen16 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("8D 94 08 ? ? ? ? 52 53 68 ? ? ? ? E8 ? ? ? ? 83");
	ptrp_MemPool_SubScreen17 = pattern.count(1).get(0).get<uint32_t>(3);

	pattern = hook::pattern("68 ? ? ? ? 81 C1 ? ? ? ? 51 50 A3 ? ? ? ? 88 1D ? ? ? ? E8");
	ptrg_MemPool_SubScreen1 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("68 ? ? ? ? 81 C2 ? ? ? ? 52 50 E8 ? ? ? ? 6A ? E8 ? ? ? ? 6A ? E8 ");
	ptrg_MemPool_SubScreen2 = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("8D 91 ? ? ? ? 75 ? 8D 91 ? ? ? ? 52 03 C1 50 6A ? E8 ? ? ? ? 83");
	ptrg_MemPool_SubScreen3 = pattern.count(1).get(0).get<uint32_t>(2);
	ptrg_MemPool_SubScreen4 = pattern.count(1).get(0).get<uint32_t>(10);

	pattern = hook::pattern("81 C2 ? ? ? ? 89 15 ? ? ? ? 80 B8 ? ? ? ? ? 74 ? 0F B7 0D ? ? ? ? 51");
	ptrg_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(2);

	pattern = hook::pattern("81 FF ? ? ? ? 73 ? 8B 15 ? ? ? ? 81 C2 ? ? ? ? 89 56 ? 83 0E");
	ptrg_MemPool_SubScreen6 = pattern.count(1).get(0).get<uint32_t>(2);
	ptrp_MemPool_SubScreen10 = pattern.count(1).get(0).get<uint32_t>(16);

	// GC blur fix
	pattern = hook::pattern("6A 04 53 68 A0 00 00 00 E8 ? ? ? ?");
	uint8_t* fnCallPtr = pattern.count(1).get(0).get<uint8_t>(9);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXBegin = (GXBegin_Fn)fnCallPtr;

	pattern = hook::pattern("D9 ? ? ? ? ? D9 ? ? ? D9 ? ? ? ? ? D9 ? ? ? D9 EE D9 ? ? E8 ? ? ? ?");
	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(26);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXPosition3f32 = (GXPosition3f32_Fn)fnCallPtr;

	pattern = hook::pattern("68 80 00 00 00 68 FF 00 00 00 68 FF 00 00 00 68 FF 00 00 00 E8 ? ? ? ?");
	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(21);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXColor4u8 = (GXColor4u8_Fn)fnCallPtr;

	pattern = hook::pattern("E8 ? ? ? ? D9 EE 83 ? ? D9 ? ? ? D9 ? ? E8 ? ? ? ?");
	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(18);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXTexCoord2f32 = (GXTexCoord2f32_Fn)fnCallPtr;

	pattern = hook::pattern("D9 EE D9 ? ? E8 ? ? ? ? 6A 0E E8 ? ? ? ?");
	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(13);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXShaderCall_Maybe = (GXShaderCall_Maybe_Fn)fnCallPtr;

	pattern = hook::pattern("E8 ? ? ? ? 6A 01 6A 06 D9");
	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(1);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXSetTexCopySrc = (GXSetTexCopySrc_Fn)fnCallPtr;

	uint32_t* varPtr = pattern.count(1).get(0).get<uint32_t>(11);
	ptr_InternalHeight = (float*)*varPtr;

	pattern = hook::pattern("D1 E8 50 D9 AD ? ? ? ? D9 05 ? ? ? ?");
	varPtr = pattern.count(1).get(0).get<uint32_t>(11);
	ptr_InternalWidth = (float*)*varPtr;

	pattern = hook::pattern("D1 E9 51 D9 AD ? ? ? ? E8 ? ? ? ? 8B 15");
	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(10);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXSetTexCopyDst = (GXSetTexCopyDst_Fn)fnCallPtr;

	varPtr = pattern.count(1).get(0).get<uint32_t>(16);
	ptr_filter01_buff = (uint32_t*)*varPtr;

	fnCallPtr = pattern.count(1).get(0).get<uint8_t>(27);
	fnCallPtr += sizeof(int32_t) + *(int32_t*)fnCallPtr;
	GXCopyTex = (GXCopyTex_Fn)fnCallPtr;

	pattern = hook::pattern("83 C4 ? D9 ? ? ? ? ? D9 05 ? ? ? ? 8B ? ? ? ? ? DD");
	varPtr = pattern.count(1).get(0).get<uint32_t>(11);
	ptr_InternalHeightScale = (float*)*varPtr;

	varPtr = pattern.count(1).get(0).get<uint32_t>(17);
	ptr_RenderHeight = (int*)*varPtr;
}

void HandleLimits()
{
	// vertex buffers
	if (bRaiseVertexAlloc)
	{
		auto pattern = hook::pattern("68 80 1A 06 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? 46");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68000C3500, true);  // 400000 -> 800000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68000C3500, true);

		pattern = hook::pattern("68 00 09 3D 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? A1 ? ? ? ? 8B 08");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68007A1200, true);  // 4000000 -> 8000000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68007A1200, true);

		pattern = hook::pattern("68 80 84 1E 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? A1");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68003D0900, true);  // 2000000 -> 4000000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68003D0900, true);

		pattern = hook::pattern("68 80 8D 5B 00 50 8B 41 ? FF D0 85 C0 0F 85");
		injector::WriteMemory<int>(pattern.count(4).get(0).get<uint32_t>(1), 0x6800B71B00, true);  // 6000000 -> 12000000
		injector::WriteMemory<int>(pattern.count(4).get(1).get<uint32_t>(1), 0x6800B71B00, true);
		injector::WriteMemory<int>(pattern.count(4).get(2).get<uint32_t>(1), 0x6800B71B00, true);
		injector::WriteMemory<int>(pattern.count(4).get(3).get<uint32_t>(1), 0x6800B71B00, true);
	}

	// Inventory screen mem
	if (bRaiseInventoryAlloc)
	{
		MH_CreateHook(ptrstageInit, MessageControl__stageInit_Hook, (LPVOID*)&MessageControl__stageInit_Orig);
		MH_CreateHook(ptrSubScreenAramRead, SubScreenAramRead_Hook, (LPVOID*)&SubScreenAramRead_Orig);

		p_MemPool_SubScreen = (uint8_t*)&g_MemPool_SubScreen;

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen1, (uintptr_t)&p_MemPool_SubScreen, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen2, (uintptr_t)&p_MemPool_SubScreen, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen3, (uintptr_t)&p_MemPool_SubScreen, true);

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen4, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenAramRead, reads rel/Sscrn.rel
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen5, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenAramRead, reads  ss_cmmn.dat read
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen6, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenAramRead, reads  omk_pzzl.dat / ss_pzzl.dat

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen7, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenExec MemorySwap call
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen8, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenExitCore MemorySwap call
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen9, (uintptr_t)&p_MemPool_SubScreen, true); // SubScreenExit

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen10, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen11, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen12, 0x000000, true);

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen13, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen14, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen15, 0x000000, true);

		injector::WriteMemory<int>(ptrp_MemPool_SubScreen16, 0x000000, true);
		injector::WriteMemory<int>(ptrp_MemPool_SubScreen17, 0x000000, true);

		injector::WriteMemory<int>(ptrg_MemPool_SubScreen1, (uint32_t)sizeof(g_MemPool_SubScreen), true); // SubScreenExec MemorySwap size
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen2, (uint32_t)sizeof(g_MemPool_SubScreen), true); // SubScreenExitCore MemorySwap size

		injector::WriteMemory<int>(ptrg_MemPool_SubScreen3, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x1000000), true); // SubScreenExec, some heap size
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen4, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true); // SubScreenExec, some heap size
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen5, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true);
		injector::WriteMemory<int>(ptrg_MemPool_SubScreen6, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true);
	}
}

// New WndProc func
WNDPROC wndProcOld = NULL;
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYDOWN:
		if (wParam == getMapKey(flip_item_left, "vk") || wParam == getMapKey(flip_item_right, "vk"))
		{
			bShouldFlipX = true;
		}
		else if (wParam == getMapKey(flip_item_up, "vk") || wParam == getMapKey(flip_item_down, "vk"))
		{
			bShouldFlipY = true;
		}
		#ifdef VERBOSE
		else if (wParam == VK_NUMPAD3)
		{
			std::cout << "Sono me... dare no me?" << std::endl;
		}
		#endif

		break;
	case WM_KEYUP:
		if (wParam == getMapKey(flip_item_left, "vk") || wParam == getMapKey(flip_item_right, "vk"))
		{
			bShouldFlipX = false;
		}
		else if (wParam == getMapKey(flip_item_up, "vk") || wParam == getMapKey(flip_item_down, "vk"))
		{
			bShouldFlipY = false;
		}
		break;
	case WM_CLOSE:
		ExitProcess(0);
		break;
	}

	return CallWindowProc(wndProcOld, hwnd, uMsg, wParam, lParam);
}

// WndProc hook
DWORD WINAPI WindowCheck()
{
	auto pattern = hook::pattern("A1 ? ? ? ? 6A ? 50 E8 ? ? ? ? 8B 36");
	auto hWnd = injector::ReadMemory<HWND>(*pattern.get_first<HWND*>(1), true);

	while (hWnd == 0) {
		hWnd = injector::ReadMemory<HWND>(*pattern.get_first<HWND*>(1), true);
		Sleep(1000);
	}

	#ifdef VERBOSE
	std::cout << "hWnd = " << hWnd << std::endl;
	#endif

	wndProcOld = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
	SetWindowLong(hWnd, GWL_WNDPROC, (LONG)WndProc);
	return 0;
}

bool Init()
{
	std::cout << "Big ironic thanks to QLOC S.A." << std::endl;

	// Detect game version
	auto pattern = hook::pattern("31 2E ? ? ? 00 00 00 6D 6F 76 69 65 2F 64 65 6D 6F 30 65 6E 67 2E 73 66 64");
	int ver = injector::ReadMemory<int>(pattern.get_first(2));

	if (ver == 0x362E30) {
		game_version = "1.0.6";
	} else if (ver == 0x302E31) {
		game_version = "1.1.0";
	} else {
		MessageBoxA(NULL, "This version of RE4 is not supported.\nre4_tweaks will be disabled.", "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		return false;
	}

	// Check for part of gameDebug function, if exists this must be a debug-enabled build
	pattern = hook::pattern("6A 00 6A 00 6A 08 68 AE 01 00 00 6A 10 6A 0A");
	if (pattern.size() > 0)
		game_version += "d";

	#ifdef VERBOSE
	std::cout << "Game version = " << game_version << std::endl;
	#endif

	MH_Initialize();

	ReadSettings();

	HandleAppID();

	GetPointers();

	HandleLimits();

	// FOV
	if (fFOVAdditional != 0.0f)
	{
		// Hook function that loads the FOV
		auto pattern = hook::pattern("D9 45 ? 89 4E ? D9 5E ? 8B 8D ? ? ? ? 89 46 ? 8B 85 ? ? ? ? 8D 7E");
		struct ScaleFOV
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaFov = *(float*)(regs.ebp - 0x34);
				_asm
				{
					fld   vanillaFov
					fadd  fFOVAdditional
				}
				*(int32_t*)(regs.esi + 0x4) = regs.ecx;
			}
		}; injector::MakeInline<ScaleFOV>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
	
	// Fix vsync option
	if (bFixVsyncToggle)
	{
		auto pattern = hook::pattern("50 E8 ? ? ? ? C7 07 01 00 00 00 E9");
		injector::MakeNOP(pattern.get_first(6), 6);
	}

	// Apply window changes
	if (bWindowBorderless || iWindowPositionX > -1 || iWindowPositionY > -1)
	{
		auto pattern = hook::pattern("68 00 00 00 80 56 68 ? ? ? ? 68 ? ? ? ? 6A 00");
		injector::MakeNOP(pattern.get_first(0x12), 6);
		injector::MakeCALL(pattern.get_first(0x12), CreateWindowExA_Hook, true);

		// hook SetWindowPos, can't nop as it's used for resizing window on resolution changes
		pattern = hook::pattern("BE 00 01 04 00 BF 00 00 C8 00");
		injector::MakeNOP(pattern.get_first(0xA), 6);
		injector::MakeCALL(pattern.get_first(0xA), SetWindowPos_Hook, true);

		// nop MoveWindow
		pattern = hook::pattern("53 51 52 53 53 50");
		injector::MakeNOP(pattern.get_first(6), 6);

		if (bWindowBorderless)
		{
			// if borderless, update the style set by SetWindowLongA
			pattern = hook::pattern("25 00 00 38 7F 05 00 00 C8 00");
			injector::WriteMemory(pattern.get_first(5), uint8_t(0xb8), true); // mov eax, XXXXXXXX
			injector::WriteMemory(pattern.get_first(6), uint32_t(WS_POPUP), true);
		}
	}

	// Prevents the game from overriding your selection in the "Retry/Load" screen when moving the mouse before confirming an action.
	if (bFixRetryLoadMouseSelector)
	{
		// Get pointer for the state. Only reliable way I found to achieve this.
		pattern = hook::pattern("C7 06 ? ? ? ? A1 ? ? ? ? F7 80 ? ? ? ? ? ? ? ? 74"); //0x48C1C0
		struct RLDLGState
		{
			void operator()(injector::reg_pack& regs)
			{
				*(int*)(regs.esi) = 0;
				ptrRetryLoadDLGstate = regs.esi + 0x3;
			}
		}; injector::MakeInline<RLDLGState>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Check if the "yes/no" prompt is open before sending any index updates
		pattern = hook::pattern("89 8F ? ? ? ? FF 85 ? ? ? ? 83 C6 ? 3B 77");
		struct MouseMenuSelector
		{
			void operator()(injector::reg_pack& regs)
			{
				if (*(int32_t*)ptrRetryLoadDLGstate != 1)
				{
					*(int32_t*)(regs.edi + 0x160) = regs.ecx;
				}
			}
		}; injector::MakeInline<MouseMenuSelector>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// SFD size
	if (bAllowHighResolutionSFD)
	{
		// Create mem hooks
		MH_CreateHook(ptrcSofdec__startApp, cSofdec__startApp_Hook, (LPVOID*)&cSofdec__startApp_Orig);
		MH_CreateHook(ptrmwPlyCalcWorkCprmSfd, mwPlyCalcWorkCprmSfd_Hook, (LPVOID*)&mwPlyCalcWorkCprmSfd_Orig);
		MH_CreateHook(ptrcSofdec__finishMovie, cSofdec__finishMovie_Hook, (LPVOID*)&cSofdec__finishMovie_Orig);

		// Get resolution and calculate new position
		pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? DB 45 ? 0F B7 0D");
		ptrSFDMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
		struct SFDRes
		{
			void operator()(injector::reg_pack& regs)
			{
				int MovResX = regs.ecx;
				int MovResY = regs.eax;

				#ifdef VERBOSE
				std::cout << "MovResX = " << MovResX << std::endl;
				std::cout << "MovResY = " << MovResY << std::endl;
				#endif

				// Calculate new position
				newMovPosX = MovResX / 2;
				newMovNegX = -newMovPosX;

				newMovPosY = MovResY / 1.54;
				newMovNegY = -newMovPosY;

				#ifdef VERBOSE
				std::cout << "newMovPosX = " << newMovPosX << std::endl;
				std::cout << "newMovNegX = " << newMovNegX << std::endl;

				std::cout << "newMovPosY = " << newMovPosY << std::endl;
				std::cout << "newMovNegY = " << newMovNegY << std::endl;
				#endif

				*(int32_t*)(ptrSFDMovAddr) = regs.eax;
			}
		}; injector::MakeInline<SFDRes>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// Write new pointers
		auto pattern = hook::pattern("D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 1C ? 50 E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 56 E8 ? ? ? ? 8B 4D ? 33 CD");
		injector::WriteMemory(pattern.get_first(2), &newMovPosX, true);
		injector::WriteMemory(pattern.get_first(12), &newMovNegX, true);
		injector::WriteMemory(pattern.get_first(32), &newMovPosY, true);
		injector::WriteMemory(pattern.get_first(22), &newMovNegY, true);
	}

	// Check if the game is running at a valid frame rate
	if (!bIgnoreFPSWarning)
	{
		// Hook function to read the FPS value from config.ini
		auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
		ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);
		struct ReadFPS
		{
			void operator()(injector::reg_pack& regs)
			{
				int intIniFPS = regs.ecx;
				int intNewFPS;

				#ifdef VERBOSE
				std::cout << "Config.ini frame rate = " << intIniFPS << std::endl;
				#endif

				if (intIniFPS != 30 && intIniFPS != 60) {

					// Calculate new fps
					if (intIniFPS > 45)
						intNewFPS = 60;
					else
						intNewFPS = 30;

					// Show warning
					std::string Msg = "ERROR: Invalid frame rate detected\n\nYour game appears to be configured to run at " + std::to_string(intIniFPS) +
						" FPS, which isn't supported and will break multiple aspects of the game.\nThis could mean you edited the game's \"config.ini\" file" +
						" and changed the \"variableframerate\" value to something other than 30 or 60.\n\nTo prevent the problems mentioned, re4_tweaks automatically" +
						" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' \"winmm.ini\" file.";

					MessageBoxA(NULL, Msg.c_str(), "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);

					// Use new FPS value
					*(int32_t*)(ptrGameFrameRate) = intNewFPS;
				} else {
					// Use .ini FPS value.
					*(int32_t*)(ptrGameFrameRate) = intIniFPS;
				}
			}
		}; injector::MakeInline<ReadFPS>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Replace MemorySwap with memcpy
	if (bUseMemcpy)
	{
		auto pattern = hook::pattern("8b 49 14 33 c0 a3 ? ? ? ? 85 c9 74 ? 8b ff f6 81 ? ? ? ? 20 8b 49 ? 74 ? b8 01 00 00 00 a3 ? ? ? ? 85 c9 75 ? c3");
		injector::MakeJMP(pattern.get_first(0), memcpy, true);
	}

	// Fix QTE mashing speed issues
	if (bFixQTE)
	{
		// Each one of these uses a different way to calculate how fast you're pressing the QTE key.

		// Running from boulders
		auto pattern = hook::pattern("D9 87 ? ? ? ? D8 87 ? ? ? ? D9 9F ? ? ? ? D9 EE D9 9F ? ? ? ? D9 05 ? ? ? ? D8 97 ? ? ? ? DF E0 F6 C4 ? 7A 20");
		struct BouldersQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaSpeed = *(float*)(regs.edi + 0x418);
				_asm
				{
					fld  vanillaSpeed
					fmul fQTESpeedMult
				}
			}
		}; injector::MakeInline<BouldersQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Climbing up after the minecart ride
		pattern = hook::pattern("D9 86 ? ? ? ? 8B 0D ? ? ? ? D8 61 ? D9 9E ? ? ? ? 6A ? 56 E8 ? ? ? ? D9 EE");
		struct MinecartQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				float vanillaSpeed = *(float*)(regs.esi + 0x40C);
				_asm
				{
					fld  vanillaSpeed
					fdiv fQTESpeedMult
				}
			}
		}; injector::MakeInline<MinecartQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Running from Salazar's statue
		pattern = hook::pattern("8B 7D 14 D9 07 E8 ? ? ? ? 0F AF 5D 24 D9 EE 01 06 8D 43 FF D9 1F 39 06");
		struct StatueRunQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.edi = *(int32_t*)(regs.ebp + 0x14);
				float vanillaSpeed = *(float*)(regs.edi);
				_asm
				{
					fld  vanillaSpeed
					fmul fQTESpeedMult
				}
			}
		}; injector::MakeInline<StatueRunQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// Climbing up after running from Salazar's statue
		pattern = hook::pattern("FF 80 ? ? ? ? 6A 00 6A 00 6A 02 6A 02 6A 02 6A 00 6A 00 6A 05 6A 19");
		struct StatueClimbQTE
		{
			void operator()(injector::reg_pack& regs)
			{
				*(int32_t*)(regs.eax + 0x168) += 3;
			}
		};
		injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(0).get<uint32_t>(0), pattern.count(2).get(0).get<uint32_t>(6));
		injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));
	}

	if (bSkipIntroLogos)
	{
		pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);
	}

	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (bFixUltraWideAspectRatio)
	{
		auto pattern = hook::pattern("DB 05 ? ? ? ? 85 ? 79 ? D8 05 ? ? ? ? DC 35 ? ? ? ? 8B");
		ptrEngineWidthScale = *pattern.count(1).get(0).get<uint32_t*>(18);

		pattern = hook::pattern("D9 05 ? ? ? ? C7 45 ? ? ? ? ? D9 5C 24 ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? D9 81");
		ptrAspectRatio = *pattern.count(1).get(0).get<uint32_t*>(2);

		// Hook function that changes the resolution
		pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? 5E 5B 8B E5 5D C3");
		ptrResMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
		struct ResChange
		{
			void operator()(injector::reg_pack& regs)
			{
				#ifdef VERBOSE
				std::cout << "ResX = " << regs.eax << std::endl;
				std::cout << "ResY = " << regs.ecx << std::endl;
				#endif

				double fGameWidth = regs.eax;
				double fGameHeight = regs.ecx;

				double fGameDisplayAspectRatio = fGameWidth / fGameHeight;

				double fNewEngineAspectRatio = (fGameDisplayAspectRatio / fDefaultEngineAspectRatio);
				double fNewAspectRatio = fGameDisplayAspectRatio / fDefaultAspectRatio;
				double fNewEngineWidthScale = fNewEngineAspectRatio * fDefaultEngineWidthScale;

				// if ultrawide
				if (fGameDisplayAspectRatio > 2.2)
				{
					#ifdef VERBOSE
					std::cout << "fNewEngineWidthScale = " << fNewEngineWidthScale << std::endl;
					std::cout << "fNewAspectRatio = " << fNewAspectRatio << std::endl;
					#endif

					fNewInvItemNamePos = 385;
					fNewFilesTitlePos = 370;
					fNewFilesItemsPos = 375;
					fNewMapIconsPos = 336;
					fNewMerchItemListPos = 333;
					fNewMerchItemDescPos = 395;
					fNewMerchGreetingPos = 380;
					fNewTItemNamesPos = 385;
					fNewRadioNamesPos = 380;

					injector::WriteMemory(ptrEngineWidthScale, static_cast<double>(fNewEngineWidthScale), true);
					injector::WriteMemory(ptrAspectRatio, static_cast<float>(fNewAspectRatio), true);
				}
				else {
					#ifdef VERBOSE
					std::cout << "Wrote default aspect ratio values" << std::endl;
					#endif
					fNewInvItemNamePos = 320;
					fNewFilesTitlePos = 320;
					fNewFilesItemsPos = 320;
					fNewMapIconsPos = 320;
					fNewMerchItemListPos = 320;
					fNewMerchItemDescPos = 320;
					fNewMerchGreetingPos = 320;
					fNewTItemNamesPos = 320;
					fNewRadioNamesPos = 320;

					injector::WriteMemory(ptrEngineWidthScale, static_cast<double>(fDefaultEngineWidthScale), true);
					injector::WriteMemory(ptrAspectRatio, static_cast<float>(fDefaultAspectRatio), true);
				}
				*(int32_t*)(ptrResMovAddr) = regs.eax;
			}
		}; injector::MakeInline<ResChange>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(4));

		// Text offset fixes
		// Iventory item name
		pattern = hook::pattern("DC 05 ? ? ? ? DC 0D ? ? ? ? E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 89 45 ? DC 0D ? ? ? ? E8 ? ? ? ? 8B C8 0F BE 05 ? ? ? ? 99");
		injector::WriteMemory(pattern.get_first(2), &fNewInvItemNamePos, true);

		// "Files" title
		pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 0F B7 0D ? ? ? ? 0F B7 15 ? ? ? ? 51 52 6A 04 B9");
		injector::WriteMemory(pattern.get_first(2), &fNewFilesTitlePos, true);

		// "Files" item list
		pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 0F B7 0D ? ? ? ? 0F B7 15 ? ? ? ? 89 45 ? 8D 43 ? 51 0F B6 ? 52 56 B9");
		injector::WriteMemory(pattern.get_first(2), &fNewFilesItemsPos, true);

		// Map icons
		pattern = hook::pattern("DC 05 ? ? ? ? D9 45 ? D9 C0 DE CA D9 C9 D9 98 ? ? ? ? D9 45");
		injector::WriteMemory(pattern.get_first(2), &fNewMapIconsPos, true);

		// Merchant's greeting
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B6 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 0F B6 7D ? 8B 75 ? C7 43 28 ? ? ? ? E8");
		injector::WriteMemory(pattern.get_first(2), &fNewMerchGreetingPos, true);

		// Merchant's item "buy" list
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B9 ? ? ? ? 66 89 8E ? ? ? ? 8B 4D ? BA ? ? ? ? 66 89 ? ? ? ? ? 0F B7 ? 8B 89 ? ? ? ? 50 E8");
		injector::WriteMemory(pattern.get_first(2), &fNewMerchItemListPos, true);

		// Merchant's item "sell" list
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 ? ? 50 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B9 ? ? ? ? 66 89 ? ? ? ? ? 8D 83 ? ? ? ? 0F B6 ? 6A ?");
		injector::WriteMemory(pattern.get_first(2), &fNewMerchItemListPos, true);

		// Merchant's item "tune up" list
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 0F B7 ? ? 50 52 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B8 ? ? ? ? 66 89 86 ? ? ? ? 8B 45 A4 B9 ? ? ? ? 66 89 8E ? ? ? ? 0F B7");
		injector::WriteMemory(pattern.get_first(2), &fNewMerchItemListPos, true);

		// Merchant's item description
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 5E 5D C3");
		injector::WriteMemory(pattern.get_first(2), &fNewMerchItemDescPos, true);

		// Tresure name
		pattern = hook::pattern("DC 05 ? ? ? ? DC 0D ? ? ? ? E8 ? ? ? ? D9 83 ? ? ? ? DC 2D ? ? ? ? 89 45 ? DC 0D ? ? ? ? E8 ? ? ? ? 8B C8 0F BE 05");
		injector::WriteMemory(pattern.get_first(2), &fNewTItemNamesPos, true);

		// Radio names
		pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 8A 1D ? ? ? ?");
		injector::WriteMemory(pattern.get_first(2), &fNewRadioNamesPos, true);
	}

	// Fix camera after zooming with the sniper
	if (bFixSniperZoom)
	{
		auto pattern = hook::pattern("6A 7F 6A 81 6A 7F E8 ? ? ? ? 83 C4 0C A2");
		injector::MakeNOP(pattern.get_first(14), 5, true);
	}
	
	// Restore missing transparency in the item pickup screen
	if (bRestorePickupTransparency)
	{
		// Remove call to GXCopyTex inside ItemExamine::gxDraw
		auto pattern = hook::pattern("56 6A 00 6A 00 50 8B F1 E8");
		injector::MakeNOP(pattern.get_first(8), 5);
	}
	
	// Fix a problem related to a vertex buffer that caused the image to be slightly blurred
	if (bFixBlurryImage) 
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 6A ? 6A ? 52");
		uint32_t* ptrNewVertex = *pattern.count(1).get(0).get<uint32_t*>(7);

		pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 56 57 6A ? 6A");
		injector::WriteMemory(pattern.get_first(2), ptrNewVertex, true);

		pattern = hook::pattern("D9 5D ? FF D0 D9 E8 A1 ? ? ? ? 8B 08 8B 91 ? ? ? ? 6A ? 51 D9 1C ? 68");
		injector::WriteMemory(pattern.get_first(42), ptrNewVertex, true);
	}

	// Disable film grain
	if (bDisableFilmGrain)
	{
		auto pattern = hook::pattern("75 ? 0F B6 56 ? 52 68 ? ? ? ? 50 50 E8 ? ? ? ? 83 C4 ? 5E 8B 4D ? 33 CD E8 ? ? ? ? 8B E5 5D C3 53");
		injector::MakeNOP(pattern.get_first(0), 2, true);
	}

	if (bEnableGCBlur)
	{
		// Hook Filter01Render, first block (loop that's ran 4 times + 0.25 pass)
		auto pattern = hook::pattern("3C 01 0F 85 ? ? ? ? D9 85");
		uint8_t* filter01_end = pattern.count(1).get(0).get<uint8_t>(4);
		filter01_end += sizeof(int32_t) + *(int32_t*)filter01_end;

		injector::WriteMemory(pattern.get_first(8), uint8_t(0x56), true); // PUSH ESI (esi = Filter01Params*)
		injector::MakeCALL(pattern.get_first(9), Filter01Render_Hook1, true);
		injector::WriteMemory(pattern.get_first(14), uint8_t(0x58), true); // POP EAX (fixes esp)
		injector::MakeJMP(pattern.get_first(15), filter01_end, true); // JMP over code that was reimplemented

		// Hook Filter01Render second block (loop ran N times depending on AlphaLevel)
		pattern = hook::pattern("D9 46 ? B9 04 00 00 00 D9");

		injector::WriteMemory(pattern.get_first(0), uint8_t(0x56), true); // PUSH ESI (esi = Filter01Params*)
		injector::MakeCALL(pattern.get_first(1), Filter01Render_Hook2, true);
		injector::WriteMemory(pattern.get_first(6), uint8_t(0x58), true); // POP EAX (fixes esp)
		injector::MakeJMP(pattern.get_first(7), filter01_end, true); // JMP over code that was reimplemented
	}

	// QTE bindings and icons
	// KEY_1 binding hook
	intQTE_key_1 = getMapKey(QTE_key_1, "dik");
	pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
	struct QTEkey1
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.ebx = intQTE_key_1;
			regs.eax = *(int32_t*)(regs.eax + 0x1C);
		}
	}; injector::MakeInline<QTEkey1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// KEY_2 binding hook
	intQTE_key_2 = getMapKey(QTE_key_2, "dik");
	pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
	struct QTEkey2
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.edx = intQTE_key_2;
			regs.eax = *(int32_t*)(regs.eax + 0x1C);
		}
	}; injector::MakeInline<QTEkey2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// KEY_1 icon prompts
	pattern = hook::pattern("8D ? ? ? ? ? 6A 2D ? E8");
	injector::WriteMemory<uint8_t>(pattern.count(3).get(0).get<uint32_t>(7), intQTE_key_1, true);
	injector::WriteMemory<uint8_t>(pattern.count(3).get(1).get<uint32_t>(7), intQTE_key_1, true);
	injector::WriteMemory<uint8_t>(pattern.count(3).get(2).get<uint32_t>(7), intQTE_key_1, true);

	// KEY_2 icon prompts
	pattern = hook::pattern("8D ? ? ? ? ? 6A 2E ? E8");
	injector::WriteMemory<uint8_t>(pattern.count(4).get(0).get<uint32_t>(7), intQTE_key_2, true);
	injector::WriteMemory<uint8_t>(pattern.count(4).get(1).get<uint32_t>(7), intQTE_key_2, true);
	injector::WriteMemory<uint8_t>(pattern.count(4).get(2).get<uint32_t>(7), intQTE_key_2, true);

	// Inventory item flip binding
	pattern = hook::pattern("A1 ? ? ? ? 75 ? A8 ? 74 ? 6A ? 8B CE E8 ? ? ? ? BB");
	ptrInvMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct InvFlip
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.eax = *(int32_t*)ptrInvMovAddr;

			if (bShouldFlipX)
			{
				regs.eax = 0x00300000;
				bShouldFlipX = false;
			}

			if (bShouldFlipY)
			{
				regs.eax = 0x00C00000;
				bShouldFlipY = false;
			}
		}
	}; injector::MakeInline<InvFlip>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Enable MH hooks
	MH_EnableHook(MH_ALL_HOOKS);

	return true;
}

void LoadRealDLL(HMODULE hModule)
{
	char configname[MAX_PATH];
	GetModuleFileNameA(hModule, configname, MAX_PATH);
	WrapperName.assign(strrchr(configname, '\\') + 1);

	// Get wrapper mode
	const char* RealWrapperMode = Wrapper::GetWrapperName((WrapperMode.size()) ? WrapperMode.c_str() : WrapperName.c_str());

	proxy_dll = Wrapper::CreateWrapper((RealDllPath.size()) ? RealDllPath.c_str() : nullptr, (WrapperMode.size()) ? WrapperMode.c_str() : nullptr, WrapperName.c_str());
}

// Dll main function
bool APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:

#ifdef VERBOSE
		AllocConsole();
		freopen("CONIN$", "r", stdin);
		freopen("CONOUT$", "w", stdout);
		freopen("CONOUT$", "w", stderr);
#endif
		LoadRealDLL(hModule);

		if (Init())
		{
			CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)&WindowCheck, nullptr, 0, nullptr));
		}

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}