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
std::string SFD_DisplayResolution;
std::string flip_item_up;
std::string flip_item_down;
std::string flip_item_left;
std::string flip_item_right;

HMODULE wrapper_dll = nullptr;
HMODULE proxy_dll = nullptr;

//#define VERBOSE

DWORD _EAX;
DWORD _ECX;
DWORD _EDI;

float fFOVAdditional = 0.0f;
float fQTESpeedMult = 2.0f;

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

bool bShouldFlip;
bool bIsItemUp;
bool bFixSniperZoom;
bool bFixVsyncToggle;
bool bFixUltraWideAspectRatio;
bool bFixQTE;
bool bRestorePickupTransparency;
bool bDisableFXAA;
bool bDisableFilmGrain;
bool bIgnoreFPSWarning;
bool bMemOptimi;

uintptr_t jmpAddrQTE1icon1;
uintptr_t jmpAddrQTE1icon2;
uintptr_t jmpAddrQTE1icon3;
uintptr_t jmpAddrQTE2icon1;
uintptr_t jmpAddrQTE2icon2;
uintptr_t jmpAddrQTE2icon3;
uintptr_t jmpAddrChangedRes;
uintptr_t jmpAddrDoPostProcessing;

static uint32_t ptrGameState;
static uint32_t ptrIsInventoryOpen;
static uint32_t ptrAfterPostProcessing;
static uint32_t ptrFXAAProcedure;
static uint32_t* ptrGameFrameRate;
static uint32_t* ptrIsMotionBlur;
static uint32_t* ptrIsColorFilter;
static uint32_t* ptrMovState;
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

uint32_t intGameWidth;
uint32_t intGameHeight;

int intFlipDirection;
int intQTE_key_1;
int intQTE_key_2;
int intMovState;
int intGameState;
int intIsInventoryOpen;
int intIsMotionBlur;
int intIsColorFilter;

void HandleAspectRatio(uint32_t intGameWidth, uint32_t intGameHeight)
{
	double fGameWidth = intGameWidth;
	double fGameHeight = intGameHeight;

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
	} else {
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
}

// Called when the resolution is changed
void __declspec(naked) ChangedRes()
{
	_asm
	{
		mov intGameWidth, eax
		mov intGameHeight, ecx
	}

	// Call function to handle aspect ratio when res is changed
	HandleAspectRatio(intGameWidth, intGameHeight);

	_asm
	{
		pop esi
		pop ebx
		mov esp, ebp
		pop ebp
		jmp jmpAddrChangedRes
	}
}

// Check if variableframerate is valid
void CheckFPS(DWORD IniFrameRate)
{
	#ifdef VERBOSE
	std::cout << "Config.ini frame rate = " << IniFrameRate << std::endl;
	#endif

	int intCurFPS = IniFrameRate;

	if (intCurFPS != 30 && intCurFPS != 60) {

		int intNewFPS;
		
		// Calculate new fps
		if (intCurFPS > 45)
			intNewFPS = 60;
		else
			intNewFPS = 30;

		// Show warning
		std::string Msg = "ERROR: Invalid frame rate detected\n\nYour game appears to be configured to run at " + std::to_string(intCurFPS) + 
			" FPS, which isn't supported and will break multiple aspects of the game.\nThis could mean you edited the game's \"config.ini\" file" + 
			" and changed the \"variableframerate\" value to something other than 30 or 60.\n\nTo prevent the problems mentioned, re4_tweaks automatically" +
			" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' \"winmm.ini\" file.";

		MessageBoxA(0, Msg.c_str(), "re4_tweaks", 0);

		// Overwrite bad fps value
		injector::WriteMemory<int>(ptrGameFrameRate, intNewFPS, true);
	} else {
		// Write original fps value
		injector::WriteMemory<int>(ptrGameFrameRate, intCurFPS, true);
	}
}

// Called when the resolution is changed
DWORD IniFrameRate;
void __declspec(naked) ReadFPS()
{
	_asm 
	{
		mov IniFrameRate, ecx
		mov _EAX, eax
		mov _ECX, ecx
		mov _EDI, edi
	}

	CheckFPS(IniFrameRate);

	_asm 
	{
		mov eax, _EAX
		mov ecx, _ECX
		mov edi, _EDI
		ret
	}
}

// Calculate new FOV
void __declspec(naked) ScaleFOV()
{
	_asm
	{
		fld dword ptr[ebp - 0x34]
		fadd fFOVAdditional
		mov[esi + 0x4], ecx
		ret
	}
}

// Post processing
void __declspec(naked) DoPostProcessing()
{

	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _EDI, edi
	}

	intGameState = injector::ReadMemory<int>(ptrGameState, true);
	intIsInventoryOpen = injector::ReadMemory<uint16_t>(ptrIsInventoryOpen, true);
	intIsMotionBlur = injector::ReadMemory<uint8_t>(ptrIsMotionBlur, true);
	intIsColorFilter = injector::ReadMemory<int>(ptrIsColorFilter, true);

	if ((intIsMotionBlur != 0 || intIsColorFilter != 0) && intGameState == 3 && intIsInventoryOpen != 0xd7ff) {
		_asm
		{
			mov eax, _EAX
			mov ecx, _ECX
			mov edi, _EDI
			cmp ecx, edi
			je offset done
			jmp jmpAddrDoPostProcessing
			done :
			jmp ptrAfterPostProcessing
		}
	}
	else {
		_asm
		{
			mov eax, _EAX
			mov ecx, _ECX
			mov edi, _EDI
			jmp ptrAfterPostProcessing
		}
	}
}

void __declspec(naked) GXDrawScreenQuadLogic()
{
	_asm
	{
		mov _EAX, eax
		mov _ECX, ecx
		mov _EDI, edi
	}

	intIsMotionBlur = injector::ReadMemory<uint8_t>(ptrIsMotionBlur, true);
	intIsColorFilter = injector::ReadMemory<int>(ptrIsColorFilter, true);

	if (intIsMotionBlur == 1 && intIsColorFilter == 0) {
		_asm
		{
			mov eax, _EAX
			mov ecx, _ECX
			mov edi, _EDI
			ret
		}
	}
	else {
		_asm
		{
			mov eax, _EAX
			mov ecx, _ECX
			mov edi, _EDI
			jmp ptrFXAAProcedure
		}
	}
}

// Flip inventory item
void __declspec(naked) FlipInv()
{
	if (bShouldFlip)
	{
		bShouldFlip = false;
		_asm {mov eax, intFlipDirection}
	}
	_asm
	{
		test eax, 0xC00000
		ret
	}
}

void __declspec(naked) invItemDown()
{
	bIsItemUp = false;
	_asm
	{
		and byte ptr[eax + 0x8A], -0x10
		ret
	}
}

void __declspec(naked) invItemUp()
{
	bIsItemUp = true;
	_asm
	{
		and byte ptr[eax + 0x8A], 0x0F
		ret
	}
}

// QTE stuff
void __declspec(naked) QTE1Binding()
{
	_asm
	{
		mov ebx, intQTE_key_1
		mov eax, [eax + 0x1C]
		ret
	}
}

void __declspec(naked) QTE2Binding()
{
	_asm
	{
		mov edx, intQTE_key_2
		mov eax, [eax + 0x1C]
		ret
	}
}

void __declspec(naked) QTE1Icon1()
{
	_asm
	{
		lea edx, [ebp - 0x3B4]
		push intQTE_key_1
		jmp jmpAddrQTE1icon1
	}
}

void __declspec(naked) QTE1Icon2()
{
	_asm
	{
		lea ecx, [ebp - 0x4CC]
		push intQTE_key_1
		jmp jmpAddrQTE1icon2
	}
}

void __declspec(naked) QTE1Icon3()
{
	_asm
	{
		lea ecx, [ebp - 0x280]
		push intQTE_key_1
		jmp jmpAddrQTE1icon3
	}
}

void __declspec(naked) QTE2Icon1()
{
	_asm
	{
		lea ecx, [ebp - 0x37C]
		push intQTE_key_2
		jmp jmpAddrQTE2icon1
	}
}

void __declspec(naked) QTE2Icon2()
{
	_asm
	{
		lea ecx, [ebp - 0x76C]
		push intQTE_key_2
		jmp jmpAddrQTE2icon2
	}
}

void __declspec(naked) QTE2Icon3()
{
	_asm
	{
		lea ecx, [ebp - 0x2F0]
		push intQTE_key_2
		jmp jmpAddrQTE2icon3
	}
}

void __declspec(naked) QTEspd_Boulders()
{
	_asm
	{
		fld[edi + 0x418]
		fld    dword ptr ds : [fQTESpeedMult]
		fmul
		ret
	}
}

void __declspec(naked) QTEspd_MinecartPullUp()
{
	_asm
	{
		fld	   dword ptr[esi + 0x40C]
		fld    dword ptr ds : [fQTESpeedMult]
		fdiv
		ret
	}
}

void __declspec(naked) QTEspd_StatueRun()
{
	_asm
	{
		mov		edi, [ebp + 0x14]
		fld		dword ptr[edi]
		fld    dword ptr ds : [fQTESpeedMult]
		fmul
		ret
	}
}

void __declspec(naked) QTEspd_StatuePullUp()
{
	_asm
	{
		add[eax + 0x168], 0x3
		ret
	}
}

// 21:9 Text positions. Couldn't find a better way to implement this.
void __declspec(naked) LoadPos_InvItemName()
{
	_asm
	{
		fadd fNewInvItemNamePos
		ret
	}
}

void __declspec(naked) LoadPos_FilesTitle()
{
	_asm
	{
		fadd fNewFilesTitlePos
		ret
	}
}

void __declspec(naked) LoadPos_FilesItems()
{
	_asm
	{
		fadd fNewFilesItemsPos
		ret
	}
}

void __declspec(naked) LoadPos_MapIcons()
{
	_asm
	{
		fadd fNewMapIconsPos
		ret
	}
}

void __declspec(naked) LoadPos_MerchItemList()
{
	_asm
	{
		fadd fNewMerchItemListPos
		ret
	}
}

void __declspec(naked) LoadPos_MerchItemDesc()
{
	_asm
	{
		fadd fNewMerchItemDescPos
		ret
	}
}

void __declspec(naked) LoadPos_MerchGreeting()
{
	_asm
	{
		fadd fNewMerchGreetingPos
		ret
	}
}

void __declspec(naked) LoadPos_TItemNames()
{
	_asm
	{
		fadd fNewTItemNamesPos
		ret
	}
}

void __declspec(naked) LoadPos_RadioNames()
{
	_asm
	{
		fadd fNewRadioNamesPos
		ret
	}
}

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

void ReadSettings()
{
	CIniReader iniReader("");
	fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", 0.0f);
	bFixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", true);
	bFixSniperZoom = iniReader.ReadBoolean("DISPLAY", "FixSniperZoom", true);
	bFixVsyncToggle = iniReader.ReadBoolean("DISPLAY", "FixVsyncToggle", true);
	bRestorePickupTransparency = iniReader.ReadBoolean("MISC", "RestorePickupTransparency", true);
	bDisableFXAA = iniReader.ReadBoolean("MISC", "DisableFXAA", false);
	bFixQTE = iniReader.ReadBoolean("MISC", "FixQTE", true);
	bDisableFilmGrain = iniReader.ReadBoolean("MISC", "DisableFilmGrain", false);
	flip_item_up = iniReader.ReadString("KEYBOARD", "flip_item_up", "HOME");
	flip_item_down = iniReader.ReadString("KEYBOARD", "flip_item_down", "END");
	flip_item_left = iniReader.ReadString("KEYBOARD", "flip_item_left", "INSERT");
	flip_item_right = iniReader.ReadString("KEYBOARD", "flip_item_right", "PAGEUP");
	QTE_key_1 = iniReader.ReadString("KEYBOARD", "QTE_key_1", "D");
	QTE_key_2 = iniReader.ReadString("KEYBOARD", "QTE_key_2", "A");
	bIgnoreFPSWarning = iniReader.ReadBoolean("FRAME RATE", "IgnoreFPSWarning", false);
	SFD_DisplayResolution = iniReader.ReadString("MOVIE", "SFD_DisplayResolution", "512x336");
	bMemOptimi = iniReader.ReadBoolean("EXPERIMENTAL", "MemOptimi", false);
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
	static uint32_t* HealthBase;
	static uint32_t* StateBase;

	// HealthBase pointer
	auto pattern = hook::pattern("A1 ? ? ? ?  83 C0 60  6A 10  50  E8");
	HealthBase = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Current game state
	ptrGameState = injector::ReadMemory<int>(HealthBase, true) + 0x20;

	// StateBase pointer
	pattern = hook::pattern("A1 ? ? ? ? F7 40 ? ? ? ? ? 75 ? E8 ? ? ? ? 85 C0 74 ? 80 3D ? ? ? ? ? 75 ? 32 DB EB ? B3 ? E8 ? ? ? ? 85 C0 74 ? 32 DB");
	StateBase = *pattern.count(1).get(0).get<uint32_t*>(1);

	// Is inventory open
	ptrIsInventoryOpen = injector::ReadMemory<int>(StateBase, true) + 0x58;

	// Is motion blur enabled
	pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? 84 DB 75 ? 8B 0D ? ? ? ? 8B 91 ? ? ? ? C1 EA");
	ptrIsMotionBlur = *pattern.count(1).get(0).get<uint32_t*>(2), true;

	// Is color filter enabled
	pattern = hook::pattern("A3 ? ? ? ? 5D C3 CC CC CC A1 ? ? ? ? C3 CC CC CC CC CC CC CC CC CC CC 55 8B EC 8A 45");
	ptrIsColorFilter = *pattern.count(1).get(0).get<uint32_t*>(1), true;

	// SFD
	pattern = hook::pattern("89 3D ? ? ? ? E8 ? ? ? ? 8B ? ? 51 E8 ? ? ? ? A1 ? ? ? ? 83 ? ? 2B C7 89");
	ptrMovState = *pattern.count(1).get(0).get<uint32_t*>(2);

	pattern = hook::pattern("56 57 8B F9 6A ? 8D 77 ? 6A ? 56 E8 ? ? ? ? C7 46 ? ? ? ? ? C7 06");
	ptrcSofdec__startApp = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B 74 24 ? 85 F6 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 33 C0 5E C3 8B 06");
	ptrmwPlyCalcWorkCprmSfd = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("56 8B F1 57 8D 86 ? ? ? ? 50 33 FF 68 ? ? ? ? 89 3D ? ? ? ? E8 ? ? ? ? 8B 4E");
	ptrcSofdec__finishMovie = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("8B 15 ? ? ? ? 81 C2 ? ? ? ? 6A ? 51 89 56 ? E8 ? ? ? ? 8B 4E");
	ptrMemPoolMovie = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(2), true);

	// Inventory screen mem
	pattern = hook::pattern("A1 ? ? ? ? 56 D9 58 ? FF 15 ? ? ? ? 50 FF D7 8B 8D ? ? ? ? 8B 95 ? ? ? ? 89 0D");
	ptrpG = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(1), true);

	pattern = hook::pattern("05 ? ? ? ? F6 C1 ? 8B 0D ? ? ? ? A3 ? ? ? ? 8D 91 ? ? ? ? 75 ? 8D 91 ? ? ? ? 52 03 C1");
	ptrpzzl_size = pattern.count(1).get(0).get<uint32_t>(1);

	pattern = hook::pattern("55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 ? A1 ? ? ? ? 8A 80");
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
	if (game_version == "1.0.6")
		ptrp_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(2);
	else
		ptrp_MemPool_SubScreen5 = pattern.count(1).get(0).get<uint32_t>(-4);

	pattern = hook::pattern("8B 15 ? ? ? ? A1 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? 6A ? 6A ? 6A ? 8D 8C");
	if (game_version == "1.0.6")
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
}

void HandleLimits()
{
	// vertex buffers
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

	// Inventory screen mem
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

	#define SS_MEM_OFFSET 0x1CB5400 // SubScreenAramRead adds this offset to the memory addr, could probably be patched out, but meh
	injector::WriteMemory<int>(ptrg_MemPool_SubScreen1, (uint32_t)(sizeof(g_MemPool_SubScreen) - SS_MEM_OFFSET), true); // SubScreenExec MemorySwap size
	injector::WriteMemory<int>(ptrg_MemPool_SubScreen2, (uint32_t)(sizeof(g_MemPool_SubScreen) - SS_MEM_OFFSET), true); // SubScreenExitCore MemorySwap size

	injector::WriteMemory<int>(ptrg_MemPool_SubScreen3, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x1000000), true); // SubScreenExec, some heap size
	injector::WriteMemory<int>(ptrg_MemPool_SubScreen4, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true); // SubScreenExec, some heap size
	injector::WriteMemory<int>(ptrg_MemPool_SubScreen5, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true);
	injector::WriteMemory<int>(ptrg_MemPool_SubScreen6, (uint32_t)(sizeof(g_MemPool_SubScreen) - 0x2000000), true);
}

bool Init()
{
	std::cout << "Big ironic thanks to QLOC S.A." << std::endl;

	// Detect game version
	auto pattern = hook::pattern("31 2E ? ? ? 00 00 00 6D 6F 76 69 65 2F 64 65 6D 6F 30 65 6E 67 2E 73 66 64");
	int ver = injector::ReadMemory<int>(pattern.get_first(2));

	std::cout << std::hex << pattern.get_first(0) << std::endl;

	std::cout << std::hex << ver << std::endl;

	if (ver == 0x362E30) {
		game_version = "1.0.6";
	} else if (ver == 0x302E31) {
		game_version = "1.1.0";
	} else {
		MessageBoxA(NULL, "This version of RE4 is not supported.\nre4_tweaks will be disabled.", "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		return false;
	}

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
		auto pattern = hook::pattern("D9 45 ? 89 4E ? D9 5E ? 8B 8D ? ? ? ? 89 46 ? 8B 85 ? ? ? ? 8D 7E");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), ScaleFOV, true);
	}
	
	// Fix vsync option
	if (bFixVsyncToggle)
	{
		auto pattern = hook::pattern("50 E8 ? ? ? ? C7 07 01 00 00 00 E9");
		injector::MakeNOP(pattern.get_first(6), 6);
	}

	// SFD size
	if (SFD_DisplayResolution != "512x336")
	{
		// Create mem hooks
		MH_CreateHook(ptrcSofdec__startApp, cSofdec__startApp_Hook, (LPVOID*)&cSofdec__startApp_Orig);
		MH_CreateHook(ptrmwPlyCalcWorkCprmSfd, mwPlyCalcWorkCprmSfd_Hook, (LPVOID*)&mwPlyCalcWorkCprmSfd_Orig);
		MH_CreateHook(ptrcSofdec__finishMovie, cSofdec__finishMovie_Hook, (LPVOID*)&cSofdec__finishMovie_Orig);

		// Parse resolution using an array
		char* ResString[] = {SFD_DisplayResolution.data()};
		char* ResArray[2];

		ResArray[0] = strtok_s(*ResString, "x", &ResArray[1]);

		int MovPosX;
		int MovPosY;
		assert(sscanf_s(ResArray[0], "%d", &MovPosX) > 0);
		assert(sscanf_s(ResArray[1], "%d", &MovPosY) > 0);

		// Calculate new resolution
		int newMovPosX = MovPosX / 2;
		int newMovNegX = -newMovPosX;

		int newMovPosY = MovPosY / 1.5;
		int newMovNegY = -newMovPosY;

		#ifdef VERBOSE
		std::cout << "newMovPosX = " << newMovPosX << std::endl;
		std::cout << "newMovNegX = " << newMovNegX << std::endl;

		std::cout << "newMovPosY = " << newMovPosY << std::endl;
		std::cout << "newMovNegY = " << newMovNegY << std::endl;
		#endif

		auto pattern = hook::pattern("D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 5C 24 ? D9 05 ? ? ? ? D9 1C ? 50 E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 56 E8 ? ? ? ? 8B 4D ? 33 CD");
		static uint32_t* ptrMovPosX = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(2), true);
		static uint32_t* ptrMovNegX = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(12), true);

		static uint32_t* ptrMovPosY = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(32), true);
		static uint32_t* ptrMovNegY = injector::ReadMemory<uint32_t*>(pattern.count(1).get(0).get<uint32_t*>(22), true);

		// Write new values
		injector::WriteMemory<float>(ptrMovPosX, newMovPosX, true);
		injector::WriteMemory<float>(ptrMovNegX, newMovNegX, true);
		injector::WriteMemory<float>(ptrMovPosY, newMovPosY, true);
		injector::WriteMemory<float>(ptrMovNegY, newMovNegY, true);
	}

	// Check if the game is running at a valid frame rate
	if (!bIgnoreFPSWarning)
	{
		auto pattern = hook::pattern("89 0D ? ? ? ? 0F 95 ? 88 15 ? ? ? ? D9 1D ? ? ? ? A3 ? ? ? ? DB 46 ? D9 1D ? ? ? ? 8B 4E ? 89 0D ? ? ? ? 8B 4D ? 5E");
		ptrGameFrameRate = *pattern.count(1).get(0).get<uint32_t*>(2);
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), ReadFPS, true);
	}

	// Replace MemorySwap with memcpy
	if (bMemOptimi)
	{
		//MH_CreateHook((LPVOID)(GameAddress + 0x724D), memcpy, NULL);
		auto pattern = hook::pattern("8b 49 14 33 c0 a3 ? ? ? ? 85 c9 74 ? 8b ff f6 81 ? ? ? ? 20 8b 49 ? 74 ? b8 01 00 00 00 a3 ? ? ? ? 85 c9 75 ? c3");
		injector::MakeJMP(pattern.get_first(0), memcpy, true);
	}

	// Fix QTE mashing speed issues
	if (bFixQTE)
	{
		// Running from boulders
		auto pattern = hook::pattern("D9 87 ? ? ? ? D8 87 ? ? ? ? D9 9F ? ? ? ? D9 EE D9 9F ? ? ? ? D9 05 ? ? ? ? D8 97 ? ? ? ? DF E0 F6 C4 ? 7A 20");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), QTEspd_Boulders, true);

		// Climbing up after the minecart ride
		pattern = hook::pattern("D9 86 ? ? ? ? 8B 0D ? ? ? ? D8 61 ? D9 9E ? ? ? ? 6A ? 56 E8 ? ? ? ? D9 EE");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), QTEspd_MinecartPullUp, true);

		// Running from Salazar's statue
		pattern = hook::pattern("8B 7D 14 D9 07 E8 ? ? ? ? 0F AF 5D 24 D9 EE 01 06 8D 43 FF D9 1F 39 06");
		injector::MakeNOP(pattern.get_first(0), 5, true);
		injector::MakeCALL(pattern.get_first(0), QTEspd_StatueRun, true);

		// Climbing up after running from Salazar's statue
		pattern = hook::pattern("FF 80 ? ? ? ? 6A 00 6A 00 6A 02 6A 02 6A 02 6A 00 6A 00 6A 05 6A 19");
		injector::MakeNOP(pattern.count(2).get(0).get<uint32_t>(0), 6, true); 
		injector::MakeNOP(pattern.count(2).get(1).get<uint32_t>(0), 6, true); 
		injector::MakeCALL(pattern.count(2).get(0).get<uint32_t>(0), QTEspd_StatuePullUp, true);
		injector::MakeCALL(pattern.count(2).get(1).get<uint32_t>(0), QTEspd_StatuePullUp, true);
	}

	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (bFixUltraWideAspectRatio)
	{
		auto pattern = hook::pattern("DB 05 ? ? ? ? 85 ? 79 ? D8 05 ? ? ? ? DC 35 ? ? ? ? 8B");
		ptrEngineWidthScale = *pattern.count(1).get(0).get<uint32_t*>(18);

		pattern = hook::pattern("D9 05 ? ? ? ? C7 45 ? ? ? ? ? D9 5C 24 ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? D9 81");
		ptrAspectRatio = *pattern.count(1).get(0).get<uint32_t*>(2);

		pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? 5E 5B 8B E5 5D C3");
		injector::MakeNOP(pattern.get_first(11), 5, true);
		injector::MakeJMP(pattern.get_first(11), ChangedRes, true);
		jmpAddrChangedRes = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(16);

		// Iventory item name position
		pattern = hook::pattern("DC 05 ? ? ? ? DC 0D ? ? ? ? E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 89 45 ? DC 0D ? ? ? ? E8 ? ? ? ? 8B C8 0F BE 05 ? ? ? ? 99");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_InvItemName, true);

		// "Files" title position
		pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 0F B7 0D ? ? ? ? 0F B7 15 ? ? ? ? 51 52 6A 04 B9");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_FilesTitle, true);

		// "Files" item list position
		pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 0F B7 0D ? ? ? ? 0F B7 15 ? ? ? ? 89 45 ? 8D 43 ? 51 0F B6 ? 52 56 B9");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_FilesItems, true);

		// Map icons position
		pattern = hook::pattern("DD 05 ? ? ? ? DC C9 D9 C9 D8 6D ? DC 05 ? ? ? ? D9");
		injector::MakeNOP(pattern.get_first(13), 6, true);
		injector::MakeCALL(pattern.get_first(13), LoadPos_MapIcons, true);

		// Merchant's greeting position
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B6 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 0F B6 7D ? 8B 75 ? C7 43 28 ? ? ? ? E8");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_MerchGreeting, true);

		// Merchant's item "buy" list position
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B9 ? ? ? ? 66 89 8E ? ? ? ? 8B 4D ? BA ? ? ? ? 66 89 ? ? ? ? ? 0F B7 ? 8B 89 ? ? ? ? 50 E8");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_MerchItemList, true);

		// Merchant's item "sell" list position
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 ? ? 50 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B9 ? ? ? ? 66 89 ? ? ? ? ? 8D 83 ? ? ? ? 0F B6 ? 6A ?");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_MerchItemList, true);

		// Merchant's item "tune up" list position
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 0F B7 ? ? 50 52 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B8 ? ? ? ? 66 89 86 ? ? ? ? 8B 45 A4 B9 ? ? ? ? 66 89 8E ? ? ? ? 0F B7");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_MerchItemList, true);

		// Merchant's item description position
		pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 5E 5D C3");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_MerchItemDesc, true);

		// Tresure name position
		pattern = hook::pattern("DC 05 ? ? ? ? DC 0D ? ? ? ? E8 ? ? ? ? D9 83 ? ? ? ? DC 2D ? ? ? ? 89 45 ? DC 0D ? ? ? ? E8 ? ? ? ? 8B C8 0F BE 05");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_TItemNames, true);

		// Radio names position
		pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 8A 1D ? ? ? ?");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_RadioNames, true);
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
	
	// Disable the game's forced FXAA
	if (bDisableFXAA) 
	{
		// Decides if post processing should be done
		auto pattern = hook::pattern("80 3D ? ? ? ? ? 75 ? 39 3D ? ? ? ? 75 ? A1");
		ptrAfterPostProcessing = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(0);

		pattern = hook::pattern("0F 84 ? ? ? ? ? ? ? ? ? 8B 10 51 57 50 8B 82 ? ? ? ?");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeJMP(pattern.get_first(0), DoPostProcessing, true);
		jmpAddrDoPostProcessing = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(6);

		// Tweak GXDrawScreenQuad's logic
		pattern = hook::pattern("55 8B EC 8B 15 ? ? ? ? A1 ? ? ? ? 8B 08 56 57 6A ? 6A ? 52 6A ? 50 8B 81 ? ? ? ? FF D0 A1 ? ? ? ? 8B 0D");
		ptrFXAAProcedure = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(0);

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? E8 ? ? ? ? D9 E8 A1 ? ? ? ? 8B 10 57 51 D9 1C ? 68 ? ? ? ? 6A ? 57 57 50 8B 82");
		injector::MakeNOP(pattern.get_first(0), 5, true);
		injector::MakeCALL(pattern.get_first(0), GXDrawScreenQuadLogic, true);

		// Disable motion blur's conditional jump
		pattern = hook::pattern("74 ? 68 ? ? ? ? 6A ? E8 ? ? ? ? A1 ? ? ? ? 8B 0D ? ? ? ? 50 51 E8 ? ? ? ? 8B 0D");
		injector::MakeNOP(pattern.get_first(0), 2, true);

		// Disable second IDirect3DDevice9::Clear call
		pattern = hook::pattern("FF D0 D9 05 ? ? ? ? D9 5D ? 83 EC ? D9 05 ? ? ? ? 8B C4 D9 18 D9 45 ? D9 58 ? D9 E8");
		injector::MakeNOP(pattern.get_first(0), 2, true);

		// Disable second FXAA procedure call
		pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 8B 10 83 C4 ? 50 8B 42 ? FF D0 89 3D ? ? ? ? 80 3D ? ? ? ? ? 75 ? 39 3D");
		injector::MakeNOP(pattern.get_first(0), 5, true);
	}

	// Disable film grain
	if (bDisableFilmGrain)
	{
		auto pattern = hook::pattern("75 ? 0F B6 56 ? 52 68 ? ? ? ? 50 50 E8 ? ? ? ? 83 C4 ? 5E 8B 4D ? 33 CD E8 ? ? ? ? 8B E5 5D C3 53");
		injector::MakeNOP(pattern.get_first(0), 2, true);
	}

	// QTE bindings and icons
	// KEY_1 binding hook
	intQTE_key_1 = getMapKey(QTE_key_1, "dik");
	pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
	injector::MakeNOP(pattern.get_first(0), 6, true);
	injector::MakeCALL(pattern.get_first(0), QTE1Binding, true);

	// KEY_2 binding hook
	intQTE_key_2 = getMapKey(QTE_key_2, "dik");
	pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
	injector::MakeNOP(pattern.get_first(0), 6, true);
	injector::MakeCALL(pattern.get_first(0), QTE2Binding, true);

	// KEY_1 icon hook (1)
	pattern = hook::pattern("8D 95 ? ? ? ? 6A 2D 52 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ?");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), QTE1Icon1, true);
	jmpAddrQTE1icon1 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_1 icon hook (2)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2D 51 E8 ? ? ? ? B9 07 00 00 00 8D BD ? ? ? ? 8B F0 F3 A5");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), QTE1Icon2, true);
	jmpAddrQTE1icon2 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_1 icon hook (3)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2D 51 E8 ? ? ? ? 8B F0 B9 07 00 00 00 8D BD ? ? ? ? F3 A5");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), QTE1Icon3, true);
	jmpAddrQTE1icon3 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_2 icon hook (1)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ? 8B F0 F3 A5 D9");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), QTE2Icon1, true);
	jmpAddrQTE2icon1 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_2 icon hook (2)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ? 8B F0 F3 A5 83 C4 0C");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), QTE2Icon2, true);
	jmpAddrQTE2icon2 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_2 icon hook (3)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? 8B F0 B9 07 00 00 00 8D BD ? ? ? ? F3 A5 D9");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), QTE2Icon3, true);
	jmpAddrQTE2icon3 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// Inventory item flip binding
	pattern = hook::pattern("A9 ? ? ? ? 74 ? 6A 01 8B CE E8 ? ? ? ? BB 02 00 00 00");
	injector::MakeNOP(pattern.get_first(0), 5, true);
	injector::MakeCALL(pattern.get_first(0), FlipInv, true);
	
	pattern = hook::pattern("80 A0 8A 00 00 00 F0 5D C3 80 88 8A 00 00 00 0F 5D C3");
	injector::MakeNOP(pattern.get_first(0), 7, true);
	injector::MakeCALL(pattern.get_first(0), invItemDown, true);
	
	injector::MakeNOP(pattern.get_first(9), 7, true);
	injector::MakeCALL(pattern.get_first(9), invItemUp, true);

	// Enable MH hooks
	MH_EnableHook(MH_ALL_HOOKS);

	return true;
}

DWORD WINAPI ContinuousThread(LPVOID)
{
	// Continuous operations
	while (true)
	{
		// Decide if we should grain. Terrible hacky way to achieve this, but this engine is a mess... and I'm not the most briliant person.
		if (bDisableFilmGrain)
			EvaluateGrain();

		// Inventory bindings
		if (GetAsyncKeyState(getMapKey(flip_item_left, "vk")) & 1 || GetAsyncKeyState(getMapKey(flip_item_right, "vk")) & 1)
		{
			if (bIsItemUp)
			{
				bShouldFlip = true;
				intFlipDirection = 0x300000;
			}
		}

		if (GetAsyncKeyState(getMapKey(flip_item_up, "vk")) & 1 || GetAsyncKeyState(getMapKey(flip_item_down, "vk")) & 1)
		{
			if (bIsItemUp)
			{
				bShouldFlip = true;
				intFlipDirection = 0x400000;
			}
		}
		#ifdef VERBOSE
		if (GetAsyncKeyState(VK_NUMPAD3) & 1)
		{
			std::cout << "Sono me... dare no me?" << std::endl;
		}
		#endif
		Sleep(50);
	}
	return S_OK;
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
			CloseHandle(CreateThread(nullptr, 0, ContinuousThread, nullptr, 0, nullptr));
		}

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}