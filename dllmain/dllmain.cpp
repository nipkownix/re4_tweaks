#include <iostream>
#include "..\includes\stdafx.h"
#include "..\Wrappers\wrapper.h"

std::string RealDllPath;
std::string WrapperMode;
std::string WrapperName;
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
float fFOVScale = 0.2f;

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

bool bShouldDoGrain;
bool shouldflip;
bool isItemUp;
bool FixSniperZoom;
bool FixUltraWideAspectRatio;
bool RestorePickupTransparency;
bool DisablePostProcessing;
bool DisableFilmGrain;

uintptr_t jmpAddrqte1icon1;
uintptr_t jmpAddrqte1icon2;
uintptr_t jmpAddrqte1icon3;
uintptr_t jmpAddrqte2icon1;
uintptr_t jmpAddrqte2icon2;
uintptr_t jmpAddrqte2icon3;
uintptr_t jmpAddrChangedRes;

static uint32_t* ptrMovState;
static uint32_t* ptrEngineWidthScale;
static uint32_t* ptrAspectRatio;

uint32_t intGameWidth;
uint32_t intGameHeight;

int flipdirection;
int intQTE_key_1;
int intQTE_key_2;
int igstate;
int intMovState;

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
		fNewMapIconsPos = 340;
		fNewMerchItemListPos = 333;
		fNewMerchItemDescPos = 400;
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

// Call function to handle aspect ratio when res is changed
void __declspec(naked) ChangedRes()
{
	_asm
	{
		mov intGameWidth, eax
		mov intGameHeight, ecx
	}

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

// Calculate new FOV
void __declspec(naked) ScaleFOV()
{
	_asm
	{
		fld    dword ptr[ebx + 0x11C]
		fld    dword ptr ds : [fFOVAdditional]
		fld    dword ptr ds : [fFOVScale]
		fmul
		fadd
		ret
	}
}

// Flip inventory item
void __declspec(naked) FlipInv()
{
	if (shouldflip)
	{
		shouldflip = false;
		_asm {mov eax, flipdirection}
	}
	_asm
	{
		test eax, 0xC00000
		ret
	}
}

void __declspec(naked) invItemDown()
{
	isItemUp = false;
	_asm
	{
		and byte ptr[eax + 0x8A], -0x10
		ret
	}
}

void __declspec(naked) invItemUp()
{
	isItemUp = true;
	_asm
	{
		and byte ptr[eax + 0x8A], 0x0F
		ret
	}
}

void __declspec(naked) qte1Binding()
{
	_asm
	{
		mov ebx, intQTE_key_1
		mov eax, [eax + 0x1C]
		ret
	}
}

void __declspec(naked) qte2Binding()
{
	_asm
	{
		mov edx, intQTE_key_2
		mov eax, [eax + 0x1C]
		ret
	}
}

void __declspec(naked) qte1Icon1()
{
	_asm
	{
		lea edx, [ebp - 0x3B4]
		push intQTE_key_1
		jmp jmpAddrqte1icon1
	}
}

void __declspec(naked) qte1Icon2()
{
	_asm
	{
		lea ecx, [ebp - 0x4CC]
		push intQTE_key_1
		jmp jmpAddrqte1icon2
	}
}

void __declspec(naked) qte1Icon3()
{
	_asm
	{
		lea ecx, [ebp - 0x280]
		push intQTE_key_1
		jmp jmpAddrqte1icon3
	}
}

void __declspec(naked) qte2Icon1()
{
	_asm
	{
		lea ecx, [ebp - 0x37C]
		push intQTE_key_2
		jmp jmpAddrqte2icon1
	}
}

void __declspec(naked) qte2Icon2()
{
	_asm
	{
		lea ecx, [ebp - 0x76C]
		push intQTE_key_2
		jmp jmpAddrqte2icon2
	}
}

void __declspec(naked) qte2Icon3()
{
	_asm
	{
		lea ecx, [ebp - 0x2F0]
		push intQTE_key_2
		jmp jmpAddrqte2icon3
	}
}

void __declspec(naked) gstate()
{
	_asm 
	{
		mov igstate, edx
		movzx eax, word ptr[ebp - 02]
		or eax, 0xC00
		ret
	}
}

void __declspec(naked) DoGrain()
{
	_asm {fstp dword ptr[ebp - 04]}
	if (bShouldDoGrain)
	{
		_asm {fld dword ptr[ebp - 04]}
	}
	_asm {ret}
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

void ReadSettings()
{
	CIniReader iniReader("");
	fFOVAdditional = iniReader.ReadFloat("DISPLAY", "FOVAdditional", 0.0f);
	FixUltraWideAspectRatio = iniReader.ReadBoolean("DISPLAY", "FixUltraWideAspectRatio", true);
	FixSniperZoom = iniReader.ReadBoolean("DISPLAY", "FixSniperZoom", true);
	RestorePickupTransparency = iniReader.ReadBoolean("MISC", "RestorePickupTransparency", true);
	DisablePostProcessing = iniReader.ReadBoolean("MISC", "DisablePostProcessing", false);
	DisableFilmGrain = iniReader.ReadBoolean("MISC", "DisableFilmGrain", false);
	flip_item_up = iniReader.ReadString("KEYBOARD", "flip_item_up", "HOME");
	flip_item_down = iniReader.ReadString("KEYBOARD", "flip_item_down", "END");
	flip_item_left = iniReader.ReadString("KEYBOARD", "flip_item_left", "INSERT");
	flip_item_right = iniReader.ReadString("KEYBOARD", "flip_item_right", "PAGEUP");
	QTE_key_1 = iniReader.ReadString("KEYBOARD", "QTE_key_1", "D");
	QTE_key_2 = iniReader.ReadString("KEYBOARD", "QTE_key_2", "A");
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

void EvaluateGrain()
{
	intMovState = injector::ReadMemory<int>(ptrMovState, true);

	if (intMovState == 1 || igstate != 0x0F)
		bShouldDoGrain = true;
	else
		bShouldDoGrain = false;
}

DWORD WINAPI Init(LPVOID)
{
	std::cout << "Big ironic thanks to QLOC S.A." << std::endl;

	ReadSettings();

	HandleAppID();

	// FOV
	if (fFOVAdditional != 0.0f)
	{
		auto pattern = hook::pattern("DC ? D9 ? DE ? D9 ? D9 ? ? ? ? ? D9 ? ? ? ? ? DE");
		injector::MakeNOP(pattern.get_first(14), 6, true);
		injector::MakeCALL(pattern.get_first(14), ScaleFOV, true);
	}
	
	// Fix aspect ratio when playing in ultra-wide. Only 21:9 was tested.
	if (FixUltraWideAspectRatio)
	{
		auto pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? 5E 5B 8B E5 5D C3");
		injector::MakeNOP(pattern.get_first(11), 5, true);
		injector::MakeJMP(pattern.get_first(11), ChangedRes, true);
		jmpAddrChangedRes = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(16);

		pattern = hook::pattern("DB 05 ? ? ? ? 85 ? 79 ? D8 05 ? ? ? ? DC 35 ? ? ? ? 8B");
		ptrEngineWidthScale = *pattern.count(1).get(0).get<uint32_t*>(18);

		pattern = hook::pattern("D9 5C ? ? C7 45 ? ? ? ? ? D9 05 ? ? ? ? C7 45 ? ? ? ? ? D9 5C ? ? C7");
		ptrAspectRatio = *pattern.count(1).get(0).get<uint32_t*>(13);

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
		pattern = hook::pattern("DC 05 ? ? ? ? D9 ? ? D9 C0 DE CA D9 C9 D9 98 ? ? ? ? D9 45 ? D9 C3 DE CB DE E2 D9 C9 DC 05 ? ? ? ? D9 45 FC");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), LoadPos_MapIcons, true);

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
	if (FixSniperZoom)
	{
		auto pattern = hook::pattern("6A 7F 6A 81 6A 7F E8 ? ? ? ? 83 C4 0C A2");
		injector::MakeNOP(pattern.get_first(14), 5, true);
	}
	
	// Restore missing transparency in the item pickup screen
	if (RestorePickupTransparency)
	{
		auto pattern = hook::pattern("C7 40 58 FF FF FF FF A1 ? ? ? ? 81 60 58 FF FF FE FF A1 ? ? ? ? 81 60 58 FF FF FF FB A1 ? ? ? ? 81 60 58 FF DF FF FF A1 ? ? ? ? 81 60 58 FF F7 FF FF 8B 45 D8");
		injector::MakeNOP(pattern.get_first(0), 7, true);
		injector::MakeNOP(pattern.get_first(94), 8, true);
	}
	
	// Disable the game's built in post processing
	if (DisablePostProcessing) 
	{
		auto pattern = hook::pattern("0F 84 ? ? ? ? ? ? ? ? ? 8B 10 51 57 50 8B 82 ? ? ? ?");
		injector::MakeNOP(pattern.get_first(0), 1, true);
		injector::WriteMemory<uint8_t>(pattern.count(1).get(0).get<uint32_t>(1), 0xE9, true); // jmp
	}

	// Disable film grain
	if (DisableFilmGrain)
	{
		auto pattern = hook::pattern("0F B7 45 FE 0D 00 0C 00 00 89 45 F8 89 15 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? DC 0D ? ? ? ? D9 6D F8 DF 7D F4 8B 4D F4");
		injector::MakeNOP(pattern.get_first(0), 9, true);
		injector::MakeCALL(pattern.get_first(0), gstate, true);

		pattern = hook::pattern("D9 5D FC D9 45 FC D9 C0 8B C1 C1 E0 04 03 C1 0F BF 4D 08 89 4D 08 0F BF 4D 10 DB 45 08 03 C0 03 C0 DE C9 89 55 08");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), DoGrain, true);

		pattern = hook::pattern("89 3D ? ? ? ? E8 ? ? ? ? 8B ? ? 51 E8 ? ? ? ? A1 ? ? ? ? 83 ? ? 2B C7 89");
		ptrMovState = *pattern.count(1).get(0).get<uint32_t*>(2);
	}

	// QTE bindings and icons
	// KEY_1 binding hook
	intQTE_key_1 = getMapKey(QTE_key_1, "dik");
	auto pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
	injector::MakeNOP(pattern.get_first(0), 6, true);
	injector::MakeCALL(pattern.get_first(0), qte1Binding, true);

	// KEY_2 binding hook
	intQTE_key_2 = getMapKey(QTE_key_2, "dik");
	pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
	injector::MakeNOP(pattern.get_first(0), 6, true);
	injector::MakeCALL(pattern.get_first(0), qte2Binding, true);

	// KEY_1 icon hook (1)
	pattern = hook::pattern("8D 95 ? ? ? ? 6A 2D 52 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ?");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte1Icon1, true);
	jmpAddrqte1icon1 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_1 icon hook (2)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2D 51 E8 ? ? ? ? B9 07 00 00 00 8D BD ? ? ? ? 8B F0 F3 A5");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte1Icon2, true);
	jmpAddrqte1icon2 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_1 icon hook (3)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2D 51 E8 ? ? ? ? 8B F0 B9 07 00 00 00 8D BD ? ? ? ? F3 A5");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte1Icon3, true);
	jmpAddrqte1icon3 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_2 icon hook (1)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ? 8B F0 F3 A5 D9");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte2Icon1, true);
	jmpAddrqte2icon1 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_2 icon hook (2)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ? 8B F0 F3 A5 83 C4 0C");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte2Icon2, true);
	jmpAddrqte2icon2 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// KEY_2 icon hook (3)
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? 8B F0 B9 07 00 00 00 8D BD ? ? ? ? F3 A5 D9");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte2Icon3, true);
	jmpAddrqte2icon3 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	// Inventory item flip binding
	pattern = hook::pattern("A9 ? ? ? ? 74 ? 6A 01 8B CE E8 ? ? ? ? BB 02 00 00 00");
	injector::MakeNOP(pattern.get_first(0), 5, true);
	injector::MakeCALL(pattern.get_first(0), FlipInv, true);
	
	pattern = hook::pattern("80 A0 8A 00 00 00 F0 5D C3 80 88 8A 00 00 00 0F 5D C3");
	injector::MakeNOP(pattern.get_first(0), 7, true);
	injector::MakeCALL(pattern.get_first(0), invItemDown, true);
	
	injector::MakeNOP(pattern.get_first(9), 7, true);
	injector::MakeCALL(pattern.get_first(9), invItemUp, true);
	
	// Continuous operations
	while (true)
	{
		// Decide if we should grain. Terrible hacky way to achieve this, but this engine is a mess... and I'm not the most briliant person.
		if (DisableFilmGrain)
			EvaluateGrain();

		// Inventory bindings
		if (GetAsyncKeyState(getMapKey(flip_item_left, "vk")) & 1 || GetAsyncKeyState(getMapKey(flip_item_right, "vk")) & 1)
		{
			if (isItemUp)
			{
				shouldflip = true;
				flipdirection = 0x300000;
			}
		}
	
		if (GetAsyncKeyState(getMapKey(flip_item_up, "vk")) & 1 || GetAsyncKeyState(getMapKey(flip_item_down, "vk")) & 1)
		{
			if (isItemUp)
			{
				shouldflip = true;
				flipdirection = 0x400000;
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

		CloseHandle(CreateThread(nullptr, 0, Init, nullptr, 0, nullptr));

		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return true;
}