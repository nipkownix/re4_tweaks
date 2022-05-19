#include <iostream>
#include "stdafx.h"
#include "Game.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "Logging/Logging.h"
#include "Patches.h"

static uint32_t* ptrGameFrameRate;

void(__stdcall* setLanguage_Orig)();
void __stdcall setLanguage_Hook()
{
	// Update violence level on game launch / new game
	setLanguage_Orig();
	if (cfg.iViolenceLevelOverride >= 0)
	{
		auto* SystemSave = SystemSavePtr();
		if (SystemSave)
			SystemSave->violenceLevel_9 = uint8_t(cfg.iViolenceLevelOverride);
	}
}

typedef void(__fastcall* cCard__firstCheck10_Fn)(void* thisptr, void* unused);
cCard__firstCheck10_Fn cCard__firstCheck10_Orig;
void __fastcall cCard__firstCheck10_Hook(void* thisptr, void* unused)
{
	// pSys gets overwritten with data from gamesave during first loading screen, so update violence level after reading it
	cCard__firstCheck10_Orig(thisptr, unused);
	if (cfg.iViolenceLevelOverride >= 0)
	{
		auto* SystemSave = SystemSavePtr();
		if (SystemSave)
			SystemSave->violenceLevel_9 = uint8_t(cfg.iViolenceLevelOverride);
	}
}

void(__cdecl* PSVECSubtract)(const Vec*, const Vec*, Vec*);
void(__cdecl* PSVECNormalize)(const Vec*, Vec*);
void(__cdecl* PSMTXMultVec)(const Mtx*, const Vec*, Vec*);
typedef void(*penClothAtCkParallel_Fn)(Vec*); // func uses custom call convention, params likely incorrect here
penClothAtCkParallel_Fn penClothAtCkParallel;
double(__cdecl* SQRTF)(float);

// Uncomment to allow comparing values between original CkParallel & our reimpl.
// (see a2_bad/a3_bad usages inside penClothAtCkParallel_hook)
// Only works with 1.1.0 atm, and by default re4_tweaks will always have different values, due to using SSE/AVX which is less precise than x87
// Use /arch:IA32 when using this option to make sure values actually match up!
// (C/C++ properties -> Code Generation -> Enable Enhanced Instruction Set)
//#define TEST_CKPARALLEL_REIMPL 1

Vec a3_orig;
Vec a2_orig;

Vec* CkParallel_a3 = nullptr;
Vec* CkParallel_a2 = nullptr;
PenClothAtTable* CkParallel_a1 = nullptr;

void penClothAtCkParallel_hook(Vec* a3_ignore)
{
	PenClothAtTable* a1 = CkParallel_a1;
	Vec* a2 = CkParallel_a2;
	Vec* a3 = CkParallel_a3;

	if (!a1)
		return;

#ifdef TEST_CKPARALLEL_REIMPL
	Vec prev_result_a2 = *a2;
	Vec prev_result_a3 = *a3;

	Vec a = a3_orig;
	Vec b = a2_orig;
#else
	Vec a = *a3;
	Vec b = *a2;
#endif

	Vec src;
	PSVECSubtract(&a, &b, &src);

	// original code overwrote the returned values in src with its own redundant a - b calculation here instead, weird

	if (0.0 == src.x && 0.0 == src.y && src.z == 0.0)
		src.y = 1.0;

	Vec unit;
	PSVECNormalize(&src, &unit);
	float v37 = src.y * src.y + src.x * src.x + src.z * src.z;
	if (sqrtf(v37) == 0.0f)
	{
		for (int i = 0; i < a1->count_0; i++)
		{
			PenClothAtData* v12 = &a1->data_ptr_4[i];
			float v43 = v12->field_7C * v12->field_7C;
			if (v12->field_0)
			{
				Vec dst;
				Vec v63;
				PSMTXMultVec(&v12->mtx_34, &b, &dst);
				PSMTXMultVec(&v12->mtx_34, &a, &v63);
				Vec v36;
				v36.x = v63.x - dst.x;
				v36.y = v63.y - dst.y;
				v36.z = v63.z - dst.z;
				float v58 = v36.z * v36.z + v36.x * v36.x;
				float v40 = -dst.x * v36.x + -dst.z * v36.z;
				float v51 = dst.x * dst.x + dst.z * dst.z;
				float v41 = v63.x * v63.x + v63.z * v63.z;
				if (v51 <= v43
					|| v41 <= v43
					|| (v40 >= 0.0 && v58 > v40))
				{
					v40 = v40 / v58;
					PSVECNormalize(&v36, &v36);
					Vec v67;
					v67.x = v36.x * v40 + dst.x;
					v67.y = v36.y * v40 + dst.y;
					v67.z = v36.z * v40 + dst.z;

					if (v67.y >= 0.0 && v12->field_80 >= v67.y)
					{
						v67.y = 0.0;
						float v59 = v67.z * v67.z + v67.x * v67.x;
						if (v59 < v43)
						{
							float v60 = v12->field_7C - sqrtf(v59);
							if (0.0 != v67.x || 0.0 != v67.y || 0.0 != v67.z)
							{
								PSVECNormalize(&v67, &v67);
								v67.x = v60 * v67.x;
								v67.y = v60 * v67.y;
								v67.z = v60 * v67.z;
								dst.x = v67.x + dst.x;
								dst.y = v67.y + dst.y;
								dst.z = v67.z + dst.z;
								v63.x = v67.x + v63.x;
								v63.y = v67.y + v63.y;
								v63.z = v67.z + v63.z;
								PSMTXMultVec(&v12->mtx_4, &dst, &b);
								PSMTXMultVec(&v12->mtx_4, &v63, &a);
							}
						}
					}
				}
				continue;
			}
			float v54 = (v12->field_64.z - b.z) * src.z + ((v12->field_64.y - b.y) * src.y + ((v12->field_64.x - b.x) * src.x));
			float v55 = float(v54 * (1.0 / v37));
			Vec v68;
			v68.x = unit.x * v55 + b.x - v12->field_64.x;
			v68.y = unit.y * v55 + b.y - v12->field_64.y;
			v68.z = unit.z * v55 + b.z - v12->field_64.z;
			float v56 = v68.y * v68.y + v68.x * v68.x + v68.z * v68.z;

			if (v43 <= (double)v56)
				continue;

			Vec tmp;
			tmp.x = v12->field_64.x - b.x;
			tmp.y = v12->field_64.y - b.y;
			tmp.z = v12->field_64.z - b.z;
			float dist_b = tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z;

			tmp.x = v12->field_64.x - a.x;
			tmp.y = v12->field_64.y - a.y;
			tmp.z = v12->field_64.z - a.z;
			float dist_a = tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z;

			// weird if statement tree so that NaNs can pass without continuing, like the original func
			if (dist_b > v43)
				if (dist_a > v43)
					if ((v54 < 0.0f || v37 <= v54))
						continue;

			float v57 = v12->field_7C - sqrtf(v56);
			if (0.0 == v68.x && 0.0 == v68.y && v68.z == 0.0)
				v68.y = 1.0;
			PSVECNormalize(&v68, &v68);
			v68.x = v57 * v68.x;
			v68.y = v57 * v68.y;
			v68.z = v57 * v68.z;
			b.x = v68.x + b.x;
			b.y = v68.y + b.y;
			b.z = v68.z + b.z;
			a.x = v68.x + a.x;
			a.y = v68.y + a.y;
			a.z = v68.z + a.z;
		}
		a2->x = b.x;
		a2->y = b.y;
		a2->z = b.z;
		a3->x = a.x;
		a3->y = a.y;
		a3->z = a.z;
	}

#ifdef TEST_CKPARALLEL_REIMPL
	bool a2_bad = false;
	bool a3_bad = false;
	if (prev_result_a2.x != a2->x || prev_result_a2.y != a2->y || prev_result_a2.z != a2->z)
		if (a2->x == a2->x && a2->y == a2->y && a2->z == a2->z)
			a2_bad = true;
	if (prev_result_a3.x != a3->x || prev_result_a3.y != a3->y || prev_result_a3.z != a3->z)
		if (a3->x == a3->x && a3->y == a3->y && a3->z == a3->z)
			a3_bad = true;

	if (a2_bad || a3_bad)
		a2_bad = a3_bad; // breakpoint here to test...
#endif
}

#ifdef TEST_CKPARALLEL_REIMPL
void CkParallel_BackupVecs()
{
	a2_orig = *CkParallel_a2;
	a3_orig = *CkParallel_a3;

	// breakpoint these to test for NaN beforehand...
	if (a2_orig.x != a2_orig.x || a2_orig.y != a2_orig.y || a2_orig.z != a2_orig.z)
		a2_orig.x = a2_orig.x;
	if (a3_orig.x != a3_orig.x || a3_orig.y != a3_orig.y || a3_orig.z != a3_orig.z)
		a3_orig.x = a3_orig.x;
}
#endif

__declspec(naked) void penClothAtCkParallel_trampoline()
{
	// penClothAtCkParallel uses custom calling convention (fastcall sadly doesn't work)
	// need to copy pointers out of registers first so we can use them...
	__asm
	{
#ifndef TEST_CKPARALLEL_REIMPL
		mov al, [cfg.bEnableReimplementedCloth]
		cmp al, 0
		je originalCode
#endif

		mov eax, [esp+4]
		mov [CkParallel_a3], eax
		mov [CkParallel_a2], ecx
		mov [CkParallel_a1], edx
#ifndef TEST_CKPARALLEL_REIMPL
		jmp penClothAtCkParallel_hook
#else
		call CkParallel_BackupVecs

		mov edx, [CkParallel_a1]
		mov ecx, [CkParallel_a2]
		mov eax, [CkParallel_a3]
		mov [esp+4], eax
		
		// jmp to orig func first
#endif
	originalCode:
		// code we overwrote with jmp..
		push ebp
		mov ebp, esp
		sub esp, 0x9C
		jmp penClothAtCkParallel
	}
}

void Init_Misc()
{
	// Remove savegame SteamID checks, allows easier save transfers
	{
		auto pattern = hook::pattern("8B 88 40 1E 00 00 3B 4D ? 0F 85 ? ? ? ?");
		Nop(pattern.count(1).get(0).get<uint8_t>(0x9), 6);
		Nop(pattern.count(1).get(0).get<uint8_t>(0x18), 6);
	}

	// Mafia Leon on cutscenes
	if (cfg.bAllowMafiaLeonCutscenes)
	{
		auto pattern = hook::pattern("80 B9 ? ? ? ? 04 6A ? 6A ? 6A ? 6A ? 0F 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), (uint8_t)-1, true); // Allow the correct models to be used

		pattern = hook::pattern("8B 7D 18 75 ? 80 B8 ? ? ? ? 02 75 ? 6A ? 68");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(11), (uint8_t)-1, true); // Allow the correct animations to be used

		Logging::Log() << "AllowMafiaLeonCutscenes enabled";
	}

	// Silence armored Ashley
	if (cfg.bSilenceArmoredAshley)
	{
		auto pattern = hook::pattern("CB 4F 00 00 02 75 ? 83 FF ? 77 ? 0F B6 87 ? ? ? ? FF 24 85");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(5), (uint8_t)0xEB, true); // jne -> jmp

		Logging::Log() << "SilenceArmoredAshley enabled";
	}

	// Check if the game is running at a valid frame rate
	if (!cfg.bIgnoreFPSWarning)
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
				con.AddConcatLog("Config.ini frame rate = ", intIniFPS);
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
						" changed the FPS to " + std::to_string(intNewFPS) + ".\n\nThis warning and the automatic change can be disabled by editing re4_tweaks' .ini file.";

					MessageBoxA(NULL, Msg.c_str(), "re4_tweaks", MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND);

					// Use new FPS value
					*(int32_t*)(ptrGameFrameRate) = intNewFPS;
				}
				else {
					// Use .ini FPS value.
					*(int32_t*)(ptrGameFrameRate) = intIniFPS;
				}
			}
		}; injector::MakeInline<ReadFPS>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
	else
		Logging::Log() << "User decided to ignore the FPS warning";

	// Unlock JP-only classic camera angle during Ashley segment
	{
		static uint8_t* pSys = nullptr;
		struct UnlockAshleyJPCameraAngles
		{
			void operator()(injector::reg_pack& regs)
			{
				bool unlock = *(uint32_t*)(pSys + 8) == 0 // pSys->language_8
					|| cfg.bAshleyJPCameraAngles;

				// set zero-flag if we're unlocking the camera, for the jz game uses after this hook
				if (unlock)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag); // clear zero_flag if unlock is false, in case it was set by something previously
			}
		};

		auto pattern = hook::pattern("8B 0D ? ? ? ? 80 79 08 00 75 ? 8A 80 A3 4F 00 00");
		pSys = *pattern.count(1).get(0).get<uint8_t*>(2);

		injector::MakeInline<UnlockAshleyJPCameraAngles>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));

		if (cfg.bAshleyJPCameraAngles)
			Logging::Log() << "AshleyJPCameraAngles enabled";
	}

	// Allow changing games level of violence to users choice
	{
		// find cCard functbl (tbl.1654)
		auto pattern = hook::pattern("0F B6 46 04 8D 14 47 03 D0 8B 04 95 ? ? ? ?");
		uint8_t** tbl_1654 = *pattern.count(1).get(0).get<uint8_t**>(12);

		// Update cCard::firstCheck10 entry to use our hook instead
		cCard__firstCheck10_Orig = (cCard__firstCheck10_Fn)tbl_1654[5];
		injector::WriteMemory(&tbl_1654[5], &cCard__firstCheck10_Hook, true);

		// Hook setLanguage func
		pattern = hook::pattern("A3 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? A9 00 00 00 F0");
		if (!pattern.empty())
		{
			ReadCall(pattern.count(1).get(0).get<uint32_t>(15), setLanguage_Orig);
			InjectHook(pattern.count(1).get(0).get<uint32_t>(15), setLanguage_Hook);
		}
		else
		{
			// JP build is missing whole setLanguage func, hook systemStartInit instead
			pattern = hook::pattern("53 57 E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 8B 35");
			ReadCall(pattern.count(1).get(0).get<uint32_t>(7), setLanguage_Orig);
			InjectHook(pattern.count(1).get(0).get<uint32_t>(7), setLanguage_Hook);
		}

		// Remove stupid check of EXE violence level against savegame
		// (might not be in all versions?)
		// Without this, game decides not to read in savegames for some reason
		pattern = hook::pattern("3A 4A 09 74");
		if (!pattern.empty())
			injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint8_t(0xeb), true);
	}

	// Option to skip the intro logos when starting up the game
	if (cfg.bSkipIntroLogos)
	{
		auto pattern = hook::pattern("81 7E 24 E6 00 00 00");
		// Overwrite some kind of timer check to check for 0 seconds instead
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(3), uint32_t(0), true);
		// After that timer, move to stage 0x1E instead of 0x2, making the logos end early
		injector::WriteMemory(pattern.count(1).get(0).get<uint8_t>(17), uint8_t(0x1E), true);

		Logging::Log() << "SkipIntroLogos enabled";
	}

	// Enable what was leftover from the dev's debug menu (called "ToolMenu")
	if (cfg.bEnableDebugMenu)
	{
		Init_ToolMenu();
		Init_ToolMenuDebug(); // mostly hooks for debug-build tool menu, but also includes hooks to slow down selection cursor

		Logging::Log() << "EnableDebugMenu applied";
	}

	// Add Handgun silencer to merchant sell list
	if (cfg.bAllowSellingHandgunSilencer)
	{
		auto pattern = hook::pattern("DB 00 AC 0D 01 00 FF FF 00 00 00 00 00 00 00 00 00 00 0B 01");

		struct PRICE_INFO // name from PS2/VR
		{
			unsigned short item_id_0;
			unsigned short price_2; // gets multiplied by 100 or 50
			unsigned short valid_4; // set to 1 on valid items? not sure if checked
		};

		// g_item_price_tbl has just enough room at the end to include 1 more item, so we'll add silencer to it there
		PRICE_INFO* g_item_price_tbl_130 = pattern.count(1).get(0).get<PRICE_INFO>(0);
		g_item_price_tbl_130[1].item_id_0 = 0x3F;
		g_item_price_tbl_130[1].price_2 = 3000; // buy price will be 30000, sell price should end up as 15000 - fair for a easter-egg item like this?
		g_item_price_tbl_130[1].valid_4 = 1;
		g_item_price_tbl_130[2].item_id_0 = 0xFFFF; // add new record end marker
		g_item_price_tbl_130[2].price_2 = 0;
		g_item_price_tbl_130[2].valid_4 = 0;

		// Add 1 to price table count
		pattern = hook::pattern("83 00 00 00 78 00 00 00");
		uint32_t* g_item_price_tbl_num = pattern.count(1).get(0).get<uint32_t>(0);
		*g_item_price_tbl_num += 1;

		Logging::Log() << "AllowSellingHandgunSilencer enabled";
	}

	// Allow physics to apply to tactical vest outfit
	// Some reason they specifically excluded that outfit inside the physics functions, maybe there's an issue with it somewhere
	// (Raz0r trainer overwrites this patch every frame for some reason, too bad)
	{
		auto pattern = hook::pattern("80 B8 C9 4F 00 00 02 74");
		injector::MakeNOP(pattern.count(2).get(0).get<uint8_t>(7), 2);
		injector::MakeNOP(pattern.count(2).get(1).get<uint8_t>(7), 2);
	}

	// Patch Ashley suplex glitch back into the game
	// They originally fixed this by adding a check for Ashley player-type inside em10ActEvtSetFS
	// However, the caller of that function (em10_R1_Dm_Small) already has player-type checking inside it, which will demote the suplex to a Kick for certain characters
	// Because of the way they patched it, enemies that are put into Suplex-state won't have any actions at all while playing as Ashley, while other chars are able to Kick
	// So we'll fix this by first removing the patch they added inside em10ActEvtSetFS, then add checks inside em10_R1_Dm_Small instead so that Ashley can Kick
	// (or if user wants we'll leave it with patch removed, so that they can play with the Ashley suplex :)
	// TODO: some reason the below doesn't actually let Ashley use SetKick, might be player-type checks inside it, so right now this just disables Suplex if bAllowAshleySuplex isn't set
	{
		// Remove player-type check from em10ActEvtSetFS
		auto pattern = hook::pattern("0F 8E ? ? ? ? A1 ? ? ? ? 80 B8 C8 4F 00 00 01 0F 84");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0x12), 6, true);

		pattern = hook::pattern("0F B6 81 C8 4F 00 00 83 C0 FE 83 F8 03 77");
		struct SuplexCheckPlayerAshley
		{
			void operator()(injector::reg_pack& regs)
			{
				int playerType = *(uint8_t*)(regs.ecx + 0x4FC8);

				// code we patched over
				regs.eax = playerType;
				regs.eax = regs.eax - 2;

				// Add Ashley player-type check, make it use Ada/Hunk/Wesker case which calls SetKick
				// TODO: some reason SetKick doesn't work for Ashley? might be player-type checks inside it, so right now this just disables Suplex if bAllowAshleySuplex isn't set
				if (!cfg.bAllowAshleySuplex && playerType == 1)
				{
					regs.eax = 0;
				}
			}
		}; injector::MakeInline<SuplexCheckPlayerAshley>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));
	}

	// Fix cloth-related lag by replacing penClothAtCkParallel with our own reimplementation
	// besides 1 redundant subtraction, the code is essentially the same, except somehow seems to allow running at 60FPS without a sweat
	// seems to be returning same values as the original function too - set TEST_CKPARALLEL_REIMPL to test (and set breakpoints where instructed...)
	{
		// Find PSVECSubtract/PSVECNormalize/PSMTXMultVec/SQRTF funcs
		auto pattern = hook::pattern("51 8D 55 ? 52 E8 ? ? ? ? D9 45 ? D8 65 ?");
		ReadCall(pattern.count(1).get(0).get<uint8_t>(5), PSVECSubtract);

		pattern = hook::pattern("8D 45 ? 50 8D 4D ? 51 E8 ? ? ? ? D9 45 ? D9 45 ? 83 C4 04 D9 45 ?");
		auto res = pattern.count(1).get(0).get<uint8_t>(8);
		ReadCall(res, PSVECNormalize);

		pattern = hook::pattern("DD D8 51 8D 55 ? 52 8D 7E ? 57 E8 ? ? ? ? 8D 45");
		ReadCall(pattern.count(1).get(0).get<uint8_t>(0xB), PSMTXMultVec);

		pattern = hook::pattern("D9 46 ? DD 5D ? D9 1C ? E8 ? ? ? ? DC 6D ? 83 C4 04");
		ReadCall(pattern.count(1).get(0).get<uint8_t>(9), SQRTF);

		// Get penClothAtCkParallel ptr
		pattern = hook::pattern("8D B8 28 01 00 00 8D 83 5C 01 00 00 8D 4F 34 50 E8 ? ? ? ?");
		auto ckParallel = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0x10), false).as_int();

		// overwrite start of func with jump to our hook...
		injector::MakeNOP(ckParallel, 9, true);
		penClothAtCkParallel = (penClothAtCkParallel_Fn)(ckParallel + 9);

		InjectHook(ckParallel, penClothAtCkParallel_trampoline, PATCH_JUMP);

#ifdef TEST_CKPARALLEL_REIMPL
		InjectHook(0x678AED, penClothAtCkParallel_trampoline, PATCH_CALL);
		InjectHook(0x678C16, penClothAtCkParallel_trampoline, PATCH_CALL);
		InjectHook(0x678D41, penClothAtCkParallel_trampoline, PATCH_CALL);
		InjectHook(0x678EE9, penClothAtCkParallel_trampoline, PATCH_CALL);
		InjectHook(0x67908C, penClothAtCkParallel_trampoline, PATCH_CALL);

		// overwrite ret of the original func to jump to our reimpl
		InjectHook(0x674A49, penClothAtCkParallel_hook, PATCH_JUMP);
#endif
	}
}