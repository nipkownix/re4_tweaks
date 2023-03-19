#include <iostream>
#include <optional>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"

struct ADA_WEAPON_LEVEL
{
	ITEM_ID id_0;
	int8_t power_2;
	int8_t speed_3;
	int8_t reload_4;
	int8_t bullet_5;
};

struct FILE_MSG_TBL_mb
{
	uint8_t top_0;
	uint8_t color_1;
	uint8_t attr_2;
	uint8_t layout_3;
};
FILE_MSG_TBL_mb* file_msg_tbl_35 = nullptr;

typedef void(__cdecl* wepXX_routine)(cPlayer* a1);
wepXX_routine wep17_r3_ready00 = nullptr;
wepXX_routine wep17_r3_ready10 = nullptr;
wepXX_routine wep02_r3_ready10 = nullptr;

bool wep17_justFired = false;

float(__cdecl* CameraControl__getCameraDirection)();
void __cdecl wep17_r3_ready00_Hook(cPlayer* a1)
{
	// Update weapon direction to match camera if allowing quickturn
	if (re4t::cfg->bAllowMatildaQuickturn)
		a1->Wep_7D8->m_CamAdjY_30 = CameraControl__getCameraDirection();

	wep17_r3_ready00(a1);
}

void __cdecl wep17_r3_ready10_Hook(cPlayer* a1)
{
	// Jump to wep02_r3_ready10 to allow Mathilda to quickturn character
	if (re4t::cfg->bAllowMatildaQuickturn)
		wep02_r3_ready10(a1);
	else
		wep17_r3_ready10(a1);
}

void(__fastcall* cPlayer__weaponInit)(cPlayer* thisptr, void* unused);
void __fastcall cPlayer__weaponInit_Hook(cPlayer* thisptr, void* unused)
{
	// Fix Ditman glitch by resetting player anim speed to 1 when weapon changing (weaponInit is called during change)
	// Ditman glitch seems to be caused by changing weapon while in-between different weapon states
	// The Striker has unique code inside wep07_r3_ready00 state which increases this speed var to 1.4
	// (maybe as a workaround to increase the anim speed without redoing anims, no other weps have similar code)
	// Normally this would then be nearly-instantly reverted back to 1.0 by the wep07_r3_ready10 state, but changing weapons can interrupt that
	// We fix that here by resetting speed to 1 whenever weapon change is occurring, pretty simple fix

	if (re4t::cfg->bFixDitmanGlitch)
		thisptr->Motion_1D8.Seq_speed_C0 = 1.0f;

	cPlayer__weaponInit(thisptr, unused);
}

bool bShouldDropChicagoAmmo = false;
void ChicagoAmmoDropCheck() noexcept
{
	bool hasChicago = ItemMgr->num((ITEM_ID)EItemId::Thompson) /* || ItemMgr->num((ITEM_ID)EItemId::Ada_Machine_Gun) */;
	if (!hasChicago)
	{
		bShouldDropChicagoAmmo = false;
		return;
	}

	auto chicagoPtr = ItemMgr->search((ITEM_ID)EItemId::Thompson);
	if (!chicagoPtr)
	{
		bShouldDropChicagoAmmo = false;
		return;
	}

	// Don't drop ammo if the weapon has unlimited ammo already
	if ((chicagoPtr->getCapacity() + 1) == 7)
	{
		bShouldDropChicagoAmmo = false;
		return;
	}
	
	int curChicagoAmmo = ItemMgr->num((ITEM_ID)EItemId::Bullet_45in_M);
	bool rnd = GetRandomInt(0, 31) == 5; // 5/31 probability (16%)

	bShouldDropChicagoAmmo = (rnd || (curChicagoAmmo <= 35));
}

// A bit ugly, but I couldn't find a better way to do this without reimplementing the entire func...
uintptr_t ChicagoAmmoDrop_finish;
void __declspec(naked) ChicagoAmmoDrop()
{
	_asm
	{
		pushad
		pushfd
	}

	ChicagoAmmoDropCheck();

	if (bShouldDropChicagoAmmo)
	{
		bShouldDropChicagoAmmo = false;
		static int ret_id = int(EItemId::Bullet_45in_M);
		static int ret_num = 70;

		_asm
		{
			popfd
			popad

			mov eax, ret_id
			mov ebx, ret_num
			mov edx, [ebp + 0x8]
			mov ecx, [ebp + 0xC]
			mov dword ptr[edx], eax
			mov dword ptr[ecx], ebx
			mov eax, 0x1
			mov esp, ebp
			pop ebp
			ret
		}
	}
	else
	{
		_asm
		{
			popfd
			popad

			mov ebx, [ebp - 0x10]
			cmp[ebp - 0x14], ebx
			jmp ChicagoAmmoDrop_finish
		}
	}
}

void re4t::init::Gameplay()
{
	// Make the Chicago Typewriter not upgraded by default and try to balance it more for normal gameplay.
	// This mostly works fine for Ada too (minus the fact the Merchant has no upgrades in SW...), but the bigger problem
	// is that Ada's Chicago Typewriter never calls its reload func. Haven't figured out why that is. For now, we'll just
	// not enable any of this for her, so I've left everyting I got so far commented out.
	if (re4t::cfg->bBalancedChicagoTypewriter)
	{
		// Hook GetDropBullet to make the game drop the unused Chicago Typewriter ammo
		auto pattern = hook::pattern("8B 5D ? 39 5D ? 72 ? E8 ? ? ? ? 0F B6 ? 81 E2 ? ? ? ? 79 ? 4A 83 CA ? 42 83 FA ? 0F 85");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 6, true);
		ChicagoAmmoDrop_finish = (uintptr_t)pattern.count(1).get(0).get<uint32_t>(6);
		injector::MakeJMP(pattern.count(1).get(0).get<uint32_t>(0), ChicagoAmmoDrop, true);

		// Get WeaponLevelTbl to change the Chicago Typewriter firepower stats.
		// Originally it is always 10.0f regardless of what upgrade level you have. We now make it grow over time, with
		// the max upgrade level bringing it to the vanilla firepower of 10.0f.
		pattern = hook::pattern("D9 04 8D ? ? ? ? D9 5D ? 75 ? 8B CE 83 E9");
		auto WeaponLevelTbl = *pattern.count(1).get(0).get<float(*)[49][7]>(3);

		// Get wep num of Leon's Chicago
		auto TypewriterNumLeon = bio4::WeaponId2WeaponNo(ITEM_ID(EItemId::Thompson));

		/*
		// Get wep num of Ada's Chicago
		auto TypewriterNumAda = bio4::WeaponId2WeaponNo(ITEM_ID(EItemId::Ada_Machine_Gun));
		*/

		// Our new fire power array
		float NewChicagoFirepower[] = { 0.9f, 1.5f, 1.7f, 2.0f, 2.5f, 3.5f, 10.0f };

		// Write new values
		std::copy(std::begin(NewChicagoFirepower), std::end(NewChicagoFirepower), std::begin((*WeaponLevelTbl)[TypewriterNumLeon]));
		/* std::copy(std::begin(NewChicagoFirepower), std::end(NewChicagoFirepower), std::begin((*WeaponLevelTbl)[TypewriterNumAda])); */

		// Nop special case inside dispBuyItemList that makes the weapon stats be shown as fully upgraded
		pattern = hook::pattern("BE ? ? ? ? F7 42 ? ? ? ? ? 74 ? 83 C0 ? 83 F8 ? 77 ? 0F B6 90");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 5, true);

		// Prevent cItemMgr::construct from forcefully applying upgrades the chicago when bought
		pattern = hook::pattern("B9 ? ? ? ? 66 89 4E ? 8B 15 ? ? ? ? F7 42");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(1), uint32_t(0), true);

		/*
		// Get ada_weapon_level to change Ada's Chicago Typewriter fire power stats.
		pattern = hook::pattern("0F BE 8C 12 ? ? ? ? 03 D2 66 ? 66 33 4E");
		auto tbl_addr = *pattern.count(1).get(0).get<uintptr_t>(4);
		tbl_addr -= 2;

		auto ada_weapon_level = (ADA_WEAPON_LEVEL(*)[7])tbl_addr;

		(*ada_weapon_level)[5].power_2 = 1;
		(*ada_weapon_level)[5].bullet_5 = 1;
		*/

		// Add chicago to the Merchant's upgrade list
		pattern = hook::pattern("68 ? ? ? ? E8 ? ? ? ? 6A 00 68 ? ? ? ? EB 54");
		static auto merchantData = *pattern.count(1).get(0).get<MERCHANT_DATA(*)[1]>(1);

		static LEVEL_INFO level_ext_tompson[2] = {
			{ 0x34, {7, 1, 1, 7}, {0, 0} },
			{ 0xFFFF, {0, 0, 0, 0}, {0, 0} }
		};
		
		pattern = hook::pattern("8B 0D ? ? ? ? F7 41 ? ? ? ? ? 74 ? 6A");
		struct MerchantRoomInit_hook_chicagolvl
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.ecx = uint32_t(SystemSavePtr());

				bio4::levelDataAdd(*merchantData, level_ext_tompson, 2);
			}
		}; injector::MakeInline<MerchantRoomInit_hook_chicagolvl>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		/* <- Works, but it isn't really necessary. Meh.
		* 
		// Add new value to level_price
		pattern = hook::pattern("C7 05 ? ? ? ? ? ? ? ? 0F B7 81 ? ? ? ? 8D");
		static auto level_price = *pattern.count(1).get(0).get<LEVEL_PRICE(*)[16]>(6);

		// define the new element
		LEVEL_PRICE chicagoLvlUpPrice = {
			0x34, // id
			{ 700, 1400, 1800, 2400, 3500, 10000, 0 },
			{ 0, 0, 0 },
			{ 500, 1500, 0 },
			{ 700, 1500, 2000, 2500, 3500, 0, 0 }
		};

		// copy all the elements from the original array into a array
		static LEVEL_PRICE newArray[18];
		std::copy(std::begin(*level_price), std::end(*level_price), newArray);

		// add the new element to the end of the array
		newArray[17] = chicagoLvlUpPrice;

		memset(*level_price, 0, sizeof(*level_price));

		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(6), &newArray, true);
		*/

		spd::log()->info("BalancedChicagoTypewriter enabled");
	}

	// Unlock JP-only classic camera angle during Ashley segment
	{
		static uint8_t* pSys = nullptr;
		struct UnlockAshleyJPCameraAngles
		{
			void operator()(injector::reg_pack& regs)
			{
				bool unlock = *(uint32_t*)(pSys + 8) == 0 // pSys->language_8
					|| re4t::cfg->bAshleyJPCameraAngles;

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

		if (re4t::cfg->bAshleyJPCameraAngles)
			spd::log()->info("AshleyJPCameraAngles enabled");
	}

	// NTSC mode
	// Enables difficulty modifiers previously exclusive to the NTSC console versions of RE4.
	// These were locked behind checks for pSys->language_8 == 1 (NTSC English). Since RE4 UHD uses PAL English (language_8 == 2), PC players never saw these.
	if (re4t::cfg->bEnableNTSCMode)
	{
		// Normal mode and Separate Ways: increased starting difficulty (3500->5500)
		auto pattern = hook::pattern("05 7C 15 00 00 6A 00 89 81 94 4F 00 00");
		struct GamePointInit_NormalGameRank
		{
			void operator()(injector::reg_pack& regs)
			{
				*(uint32_t*)(regs.ecx + 0x4F94) = SystemSavePtr()->language_8 == 1 || re4t::cfg->bEnableNTSCMode ? 5500 : 3500;
			}
		}; injector::MakeInline<GamePointInit_NormalGameRank>(pattern.count(1).get(0).get<uint32_t>(7), pattern.count(1).get(0).get<uint32_t>(13));

		// Assignment Ada: increased difficulty (4500->6500)
		pattern = hook::pattern("8B ? ? ? ? ? 80 7E 08 01 74");
		struct GameAddPoint_AAdaGameRank
		{
			void operator()(injector::reg_pack& regs)
			{
				if (SystemSavePtr()->language_8 == 1 || re4t::cfg->bEnableNTSCMode)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<GameAddPoint_AAdaGameRank>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));

		// Shooting range: increased bottle cap score requirements (1000->3000)
		pattern = hook::pattern("8B F9 8A ? ? 8B ? ? FE C9");
		Patch(pattern.count(1).get(0).get<uint32_t>(2), { 0xB1, 0x01, 0x90 }); // cCap::check, { mov cl, 1 }

		// Shooting range: use NTSC strings for the game rules note
		// (only supports English for now, as only eng/ss_file_01.MDT contains the additional strings necessary for this)
		pattern = hook::pattern("? 00 01 00 46 00 01 00");
		file_msg_tbl_35 = pattern.count(1).get(0).get<FILE_MSG_TBL_mb>(0);
		// update the note's message index whenever we load into r22c
		pattern = hook::pattern("89 41 78 83 C1 7C E8");
		struct R22cInit_UpdateMsgIdx
		{
			void operator()(injector::reg_pack& regs)
			{
				file_msg_tbl_35[0].top_0 = SystemSavePtr()->language_8 == 2 ? 0x9D : 0x3F;

				// code we overwrote
				*(uint32_t*)(regs.ecx + 0x78) = regs.eax;
				regs.ecx += 0x7C;
			}
		}; injector::MakeInline<R22cInit_UpdateMsgIdx>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Shooting range: only check for bottle cap reward once per results screen
		pattern = hook::pattern("8B 15 ? ? ? ? 80 7A ? 01 74");
		Patch(pattern.count(2).get(1).get<uint32_t>(10), { 0xEB }); // shootResult, jz -> jmp

		// Mercenaries: unlock village stage difficulty, requires 60fps fix
		Patch(pattern.count(2).get(0).get<uint32_t>(10), { 0xEB }); // GameAddPoint, jz -> jmp

		// remove Easy mode from the difficulty menu
		pattern = hook::pattern("A1 ? ? ? ? 80 78 ? 01 75");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(9), 2); // titleLevelInit

		// Swap NEW GAME texture for START on a fresh system save
		pattern = hook::pattern("89 51 68 8B 57 68 8B");
		struct titleMenuInit_StartTex
		{
			void operator()(injector::reg_pack& regs)
			{
				bool newSystemSave = !FlagIsSet(SystemSavePtr()->flags_EXTRA_4, uint32_t(Flags_EXTRA::EXT_HARD_MODE));
				if (newSystemSave)
				{
					float texW;

					// texW = aspect ratio of image file * size0_H_E0 (14)
					switch (SystemSavePtr()->language_8)
					{
					case 4: // French
						texW = 131.0f;
						break;
					case 5: // Spanish
						texW = 70.0f;
						break;
					case 6: // Traditional Chinese / Italian
						texW = GameVersion() == "1.1.0" ? 66.0f : 68.0f;
						break;
					case 8: // Italian
						texW = 68.0f;
						break;
					default: // English, German, Japanese, Simplified Chinese
						texW = 66.0f;
						break;
					}

					IdSysPtr()->unitPtr(0x1u, IDC_TITLE_MENU_0)->texId_78 = 164;
					IdSysPtr()->unitPtr(0x1u, IDC_TITLE_MENU_0)->size0_W_DC = texW;
					IdSysPtr()->unitPtr(0x2u, IDC_TITLE_MENU_0)->texId_78 = 164;
					IdSysPtr()->unitPtr(0x2u, IDC_TITLE_MENU_0)->size0_W_DC = texW;
				}

				// Code we overwrote
				*(uint32_t*)(regs.ecx + 0x68) = regs.edx;
				regs.edx = *(uint32_t*)(regs.edi + 0x68);
			}
		}; injector::MakeInline<titleMenuInit_StartTex>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Patches for Japanese language support (language_8 == 0):

		// repurpose the hide Professional mode block to hide Amateur mode instead
		pattern = hook::pattern("C7 46 30 01 00 00 00");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(7), 2); // titleLevelInit
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(16), 2);
		Patch(pattern.count(1).get(0).get<uint32_t>(21), { 0x09 });
		Patch(pattern.count(1).get(0).get<uint32_t>(38), { 0x0A });
		// erase the rest of the block
		pattern = hook::pattern("C7 46 ? 03 00 00 00 85 DB");
		injector::MakeNOP(pattern.get_first(0), 37);

		// disable JP difficulty select confirmation prompts
		pattern = hook::pattern("A1 ? ? ? ? 38 58 08 75");
		Patch(pattern.count(1).get(0).get<uint32_t>(8), { 0xEB }); // titleMain, jnz -> jmp

		// remove JP only 20% damage armor from Mercenaries mode
		pattern = hook::pattern("F7 46 54 00 00 00 40");
		Patch(pattern.count(1).get(0).get<uint32_t>(7), { 0xEB }); // LifeDownSet2, jz -> jmp

		spd::log()->info("NTSC mode enabled");
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
				if (!re4t::cfg->bAllowAshleySuplex && playerType == 1)
				{
					regs.eax = 0;
				}
			}
		}; injector::MakeInline<SuplexCheckPlayerAshley>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));
	}

	// Hooks to allow quickturning character when wielding Matilda
	// (credits to qingsheng8848 for finding out this method!)
	{
		// Get pointer to wep02_r2_ready func table
		auto pattern = hook::pattern("89 45 FC 53 56 8B 75 08 0F B6 86 FF 00 00 00 8B 0C 85");
		uint32_t* wep02_r2_ready_funcTbl = *pattern.count(1).get(0).get<uint32_t*>(0x12);

		wep02_r3_ready10 = (wepXX_routine)wep02_r2_ready_funcTbl[1];

		// Get pointer to wep17_r2_ready (mathilda) func table
		pattern = hook::pattern("C6 40 24 01 0F B6 86 FF 00 00 00 8B 0C 85 ? ? ? ? 56 FF D1 83 C4 04 83 3D");
		uint32_t* wep17_r2_ready_funcTbl = *pattern.count(1).get(0).get<uint32_t*>(0xE);

		wep17_r3_ready00 = (wepXX_routine)wep17_r2_ready_funcTbl[0];
		wep17_r3_ready10 = (wepXX_routine)wep17_r2_ready_funcTbl[1];

		// Hook wep17_r3_ready00 & ready10
		wep17_r2_ready_funcTbl[0] = (uint32_t)&wep17_r3_ready00_Hook;
		wep17_r2_ready_funcTbl[1] = (uint32_t)&wep17_r3_ready10_Hook;

		// Fetch CameraControl::getCameraDirection addr
		pattern = hook::pattern("E8 ? ? ? ? 8B 96 D8 07 00 00 D9 5A 30");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), CameraControl__getCameraDirection);
	}

	// Hook cPlayer::weaponInit so we can add code to fix ditman glitch
	{
		auto pattern = hook::pattern("83 C4 0C E8 ? ? ? ? D9 EE 8B 06 D9 9E 44 05 00 00");

		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(3)).as_int(), cPlayer__weaponInit);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(3)).as_int(), cPlayer__weaponInit_Hook, PATCH_JUMP);
	}

	// Disable Automatic Reload
	{
		auto pattern = hook::pattern("8B 86 D8 07 00 00 8B 48 34 8B 11 8B 42 4C FF D0");
		struct DisableReloadCheck
		{
			void operator()(injector::reg_pack& regs)
			{
				if (PlayerPtr()->Wep_7D8->m_pWep_34->reloadable() && !re4t::cfg->bDisableAutomaticReload)
					regs.ef &= ~(1 << regs.zero_flag);
				else
					regs.ef |= (1 << regs.zero_flag);
			}
		};
		// for Handguns, Matilda, TMP, Shotguns
		injector::MakeInline<DisableReloadCheck>(pattern.count(5).get(0).get<uint32_t>(0), pattern.count(5).get(0).get<uint32_t>(18));
		injector::MakeInline<DisableReloadCheck>(pattern.count(5).get(1).get<uint32_t>(0), pattern.count(5).get(1).get<uint32_t>(18));
		injector::MakeInline<DisableReloadCheck>(pattern.count(5).get(2).get<uint32_t>(0), pattern.count(5).get(2).get<uint32_t>(18));
		injector::MakeInline<DisableReloadCheck>(pattern.count(5).get(4).get<uint32_t>(0), pattern.count(5).get(4).get<uint32_t>(18));
		// for Rifles, Mine Thrower
		pattern = hook::pattern("8B 8E D8 07 00 00 8B 49 34 8B 11 8B 42 4C FF");
		injector::MakeInline<DisableReloadCheck>(pattern.count(3).get(1).get<uint32_t>(0), pattern.count(3).get(1).get<uint32_t>(18));
		injector::MakeInline<DisableReloadCheck>(pattern.count(3).get(2).get<uint32_t>(0), pattern.count(3).get(2).get<uint32_t>(18));
	}

	// Limit the Matilda to one three round burst per trigger pull
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 85 C0 74 ? 8B 96 D8 07 00 00 8B 4A 34 E8");
		struct wep17_r2_set_joyFireTrig
		{
			void operator()(injector::reg_pack& regs)
			{
				if (bio4::joyFireTrg())
				{
					wep17_justFired = true;
					regs.ef &= ~(1 << regs.zero_flag);
				}
				else
					regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<wep17_r2_set_joyFireTrig>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(7));

		pattern = hook::pattern("E8 ? ? ? ? 85 C0 74 ? 8B 8E D8 07 00 00 8B 49 34 E8 ? ? ? ? 84 C0 0F ? ? ? ? ? 8B");
		struct wep17_r2_set_joyFireOn 
		{
			void operator()(injector::reg_pack& regs)
			{
				if ((!re4t::cfg->bLimitMatildaBurst || !wep17_justFired) && bio4::joyFireOn())
				{
					wep17_justFired = true;
					regs.ef &= ~(1 << regs.zero_flag);
				}
				else
					regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<wep17_r2_set_joyFireOn>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		pattern = hook::pattern("55 8B EC 56 8B ? ? 0F B6 86 FE 00 00 00 8B ? ? ? ? ? ? 56 FF ? 8B 8E D8 07 00 00 83 C4 04 E8 ? ? ? ? 83");
		struct wep17_move_LimitMatildaBurst
		{
			void operator()(injector::reg_pack& regs)
			{
				// reset justFired flag anytime the player lifts up on the trigger
				if (!bio4::joyFireOn())
					wep17_justFired = false;

				// Code we overwrote
				regs.eax = *(uint8_t*)(regs.esi + 0xFE);
			}
		}; injector::MakeInline<wep17_move_LimitMatildaBurst>(pattern.count(3).get(2).get<uint32_t>(7), pattern.count(3).get(2).get<uint32_t>(14));
	}
}