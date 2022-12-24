#include <iostream>
#include <optional>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"

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

void re4t::init::Gameplay()
{
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

	// Silence armored Ashley
	{
		auto pattern = hook::pattern("83 C4 ? 80 BA ? ? ? ? 02 75 ? 83 FF ? 77 ? 0F B6 87 ? ? ? ? FF 24 85 ? ? ? ? BE");
		struct ClankClanklHook
		{
			void operator()(injector::reg_pack& regs)
			{
				int AshleyCostumeID = int(GlobalPtr()->subCostume_4FCB);

				// Mimic what CMP does, since we're overwriting it.
				if (AshleyCostumeID > 2)
				{
					// Clear both flags
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}
				else if (AshleyCostumeID < 2)
				{
					// ZF = 0, CF = 1
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef |= (1 << regs.carry_flag);
				}
				else if (AshleyCostumeID == 2)
				{
					// ZF = 1, CF = 0
					regs.ef |= (1 << regs.zero_flag);
					regs.ef &= ~(1 << regs.carry_flag);
				}

				if (re4t::cfg->bSilenceArmoredAshley)
				{
					// Make the game think Ashley isn't using the clanky costume
					regs.ef &= ~(1 << regs.zero_flag);
					regs.ef |= (1 << regs.carry_flag);
				}
			}
		}; injector::MakeInline<ClankClanklHook>(pattern.count(1).get(0).get<uint32_t>(3), pattern.count(1).get(0).get<uint32_t>(10));

		spd::log()->info("SilenceArmoredAshley applied");
	}

	// NTSC mode
	// Enables difficulty modifiers previously exclusive to the NTSC console versions of RE4.
	// These were locked behind checks for pSys->language_8 == 1 (NTSC English). Since RE4 UHD uses PAL English (language_8 == 2), PC players never saw these.
	if (re4t::cfg->bEnableNTSCMode)
	{
		// Normal mode and Separate Ways: increased starting difficulty (3500->5500)
		auto pattern = hook::pattern("8A 50 ? FE CA 0F B6 C2");
		Patch(pattern.count(1).get(0).get<uint32_t>(0), { 0xB2, 0x01, 0x90 }); // GamePointInit, { mov dl, 1 }

		// Assignment Ada: increased difficulty (4500->6500)
		pattern = hook::pattern("66 39 B1 ? ? 00 00 75 10");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(7), 2); // GameAddPoint

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

					IDSystemPtr()->unitPtr(0x1u, IDC_TITLE_MENU)->texId_78 = 164;
					IDSystemPtr()->unitPtr(0x1u, IDC_TITLE_MENU)->size0_W_DC = texW;
					IDSystemPtr()->unitPtr(0x2u, IDC_TITLE_MENU)->texId_78 = 164;
					IDSystemPtr()->unitPtr(0x2u, IDC_TITLE_MENU)->size0_W_DC = texW;
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

	// Limit the Matilda to one three round burst per trigger pull
	{
		auto pattern = hook::pattern("E8 ? ? ? ? 85 C0 74 ? 8B 8E D8 07 00 00 8B 49 34 E8 ? ? ? ? 84 C0 0F ? ? ? ? ? 8B");
		struct wep17_r2_set_LimitMatildaBurst
		{
			void operator()(injector::reg_pack& regs)
			{
				if (!re4t::cfg->bLimitMatildaBurst && bio4::joyFireOn())
					regs.ef &= ~(1 << regs.zero_flag);
				else
					regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<wep17_r2_set_LimitMatildaBurst>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
	}
}