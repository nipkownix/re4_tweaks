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

bool shouldflip;
bool isItemUp;
bool FixSniperZoom;
bool RestorePickupTransparency;
bool DisablePostProcessing;
bool DisableFilmGrain;

uintptr_t jmpAddrqte1icon;
uintptr_t jmpAddrqte1icon2;
uintptr_t jmpAddrqte2icon;

int flipdirection;
int intQTE_key_1;
int intQTE_key_2;
int igstate;

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

void __declspec(naked) qte1Icon()
{
	_asm
	{
		lea ecx, [ebp - 0x280]
		push intQTE_key_1
		jmp jmpAddrqte1icon
	}
}

void __declspec(naked) qte1Icon2()
{
	_asm
	{
		lea edx, [ebp - 0x3B4]
		push intQTE_key_1
		jmp jmpAddrqte1icon2
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

void __declspec(naked) qte2Icon()
{
	_asm
	{
		lea ecx, [ebp - 0x37C]
		push intQTE_key_2
		jmp jmpAddrqte2icon
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
	if (igstate != 0x0F)
	{
		_asm {fld dword ptr[ebp - 04]}
	}
	_asm {ret}
}

void ReadSettings()
{
	CIniReader iniReader("");
	fFOVAdditional = iniReader.ReadFloat("CAMERA", "FOVAdditional", 0.0f);
	FixSniperZoom = iniReader.ReadBoolean("CAMERA", "FixSniperZoom", true);
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

DWORD WINAPI Init(LPVOID)
{
	std::cout << "Big ironic thanks to QLOC S.A." << std::endl;

	ReadSettings();

	HandleAppID();

	//FOV
	if (fFOVAdditional != 0.0f)
	{
		auto pattern = hook::pattern("DC ? D9 ? DE ? D9 ? D9 ? ? ? ? ? D9 ? ? ? ? ? DE");
		injector::MakeNOP(pattern.get_first(14), 6, true);
		injector::MakeCALL(pattern.get_first(14), ScaleFOV, true);
	}
	
	//Fix camera after zooming with the sniper
	if (FixSniperZoom)
	{
		auto pattern = hook::pattern("6A 7F 6A 81 6A 7F E8 ? ? ? ? 83 C4 0C A2");
		injector::MakeNOP(pattern.get_first(14), 5, true);
	}
	
	//Restore missing transparency in the item pickup screen
	if (RestorePickupTransparency)
	{
		auto pattern = hook::pattern("C7 40 58 FF FF FF FF A1 ? ? ? ? 81 60 58 FF FF FE FF A1 ? ? ? ? 81 60 58 FF FF FF FB A1 ? ? ? ? 81 60 58 FF DF FF FF A1 ? ? ? ? 81 60 58 FF F7 FF FF 8B 45 D8");
		injector::MakeNOP(pattern.get_first(0), 7, true);
		injector::MakeNOP(pattern.get_first(94), 8, true);
	}
	
	//Disable the game's built in post processing
	if (DisablePostProcessing) 
	{
		auto pattern = hook::pattern("0F 84 ? ? ? ? ? ? ? ? ? 8B 10 51 57 50 8B 82 ? ? ? ?");
		injector::MakeNOP(pattern.get_first(0), 1, true);
		injector::WriteMemory<uint8_t>(pattern.count(1).get(0).get<uint32_t>(1), 0xE9, true); // jmp
	}

	//Disable film grain
	if (DisableFilmGrain)
	{
		auto pattern = hook::pattern("0F B7 45 FE 0D 00 0C 00 00 89 45 F8 89 15 ? ? ? ? C7 05 ? ? ? ? ? ? ? ? DC 0D ? ? ? ? D9 6D F8 DF 7D F4 8B 4D F4");
		injector::MakeNOP(pattern.get_first(0), 9, true);
		injector::MakeCALL(pattern.get_first(0), gstate, true);

		pattern = hook::pattern("D9 5D FC D9 45 FC D9 C0 8B C1 C1 E0 04 03 C1 0F BF 4D 08 89 4D 08 0F BF 4D 10 DB 45 08 03 C0 03 C0 DE C9 89 55 08");
		injector::MakeNOP(pattern.get_first(0), 6, true);
		injector::MakeCALL(pattern.get_first(0), DoGrain, true);
	}

	//QTE bindings and icons
	//KEY_1 binding hook
	intQTE_key_1 = getMapKey(QTE_key_1, "dik");
	auto pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
	injector::MakeNOP(pattern.get_first(0), 6, true);
	injector::MakeCALL(pattern.get_first(0), qte1Binding, true);

	//KEY_1 icon hook
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2D 51 E8 ? ? ? ? 8B F0 B9 07 00 00 00 8D BD ? ? ? ?");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte1Icon, true);
	jmpAddrqte1icon = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	//KEY_1 icon(2) hook
	pattern = hook::pattern("8D 95 ? ? ? ? 6A 2D 52 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ?");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte1Icon2, true);
	jmpAddrqte1icon2 = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(8);

	//KEY_2 binding hook
	intQTE_key_2 = getMapKey(QTE_key_2, "dik");
	pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
	injector::MakeNOP(pattern.get_first(0), 6, true);
	injector::MakeCALL(pattern.get_first(0), qte2Binding, true);

	//KEY_2 icon hook
	pattern = hook::pattern("8D 8D ? ? ? ? 6A 2E 51 E8 ? ? ? ? B9 07 00 00 00 BF ? ? ? ? 8B F0 F3 A5 D9");
	injector::MakeNOP(pattern.get_first(0), 8, true);
	injector::MakeJMP(pattern.get_first(0), qte2Icon, true);
	jmpAddrqte2icon = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(6);

	//Inventory item flip binding
	pattern = hook::pattern("A9 ? ? ? ? 74 ? 6A 01 8B CE E8 ? ? ? ? BB 02 00 00 00");
	injector::MakeNOP(pattern.get_first(0), 5, true);
	injector::MakeCALL(pattern.get_first(0), FlipInv, true);
	
	pattern = hook::pattern("80 A0 8A 00 00 00 F0 5D C3 80 88 8A 00 00 00 0F 5D C3");
	injector::MakeNOP(pattern.get_first(0), 7, true);
	injector::MakeCALL(pattern.get_first(0), invItemDown, true);
	
	injector::MakeNOP(pattern.get_first(9), 7, true);
	injector::MakeCALL(pattern.get_first(9), invItemUp, true);
	
	//Inventory bindings
	while (true)
	{
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
		Sleep(5);
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