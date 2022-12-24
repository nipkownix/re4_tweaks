#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"
#include "input.hpp"

void __cdecl titleLoop_hook(TITLE_WORK* pT)
{
	bool UIPause = FlagIsSet(GlobalPtr()->flags_STOP_0_170, uint32_t(Flags_STOP::SPF_ID_SYSTEM));
	if (UIPause)
		return;

	ID_UNIT* tex = IDSystemPtr()->unitPtr(0x6u, IDC_TITLE);

	if (re4t::cfg->bRestoreAnalogTitleScroll)
	{
		if (isController())
		{
			// prefer to use fAnalogR over AnalogR here, because it lets us to make the deadzone smaller
			const float fDeadZone = 0.30f;

			if (abs(*fAnalogRX) >= fDeadZone)
				pT->scroll_add_5C = *fAnalogRX * 3.0f;

			if (abs(*fAnalogRY) >= fDeadZone)
			{
				if (*fAnalogRY < 0)
					tex->pos0_94.z -= 5.0f * GlobalPtr()->deltaTime_70;
				else
					tex->pos0_94.z += 5.0f * GlobalPtr()->deltaTime_70;

				tex->pos0_94.z = std::clamp(tex->pos0_94.z, 0.0f, 170.0f);
			}
		}
		else
		{
			// Using our own KB/M input here instead of the game's, since the game's function to handle input (PadRead) is a mess.
			if (pInput->is_key_down(VK_CONTROL))
			{
				float fDeltaX = float(pInput->raw_mouse_delta_x());
				float fDeltaY = float(pInput->raw_mouse_delta_y());

				if (abs(fDeltaX) > 0)
				{
					tex->pos0_94.x -= fDeltaX * 0.2f;

					if (fDeltaX < 0)
						pT->scroll_add_5C = -(fabsf(pT->scroll_add_5C));
					else
						pT->scroll_add_5C = fabsf(pT->scroll_add_5C);
				}

				if (abs(fDeltaY) > 0)
				{

					tex->pos0_94.z -= fDeltaY * 0.2f;
					tex->pos0_94.z = std::clamp(tex->pos0_94.z, 0.0f, 170.0f);
				}
			}
		}
	}

	if (!re4t::cfg->bRestoreAnalogTitleScroll || !pInput->is_key_down(VK_CONTROL))
		tex->pos0_94.x -= pT->scroll_add_5C * GlobalPtr()->deltaTime_70;
	if (tex->pos0_94.x > 1800.0f)
		tex->pos0_94.x -= 2700.0f;
	else if (tex->pos0_94.x < -900.0f)
		tex->pos0_94.x += 2700.0f;
}

void re4t::init::TitleMenu()
{
	// Restore Gamecube title menu pan and zoom controls
	{
		// hook the titleLoop call with our own reimplementation
		auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 E8 ? ? ? ? 33 DB 53");
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), titleLoop_hook, PATCH_JUMP);

		// don't reset scroll_add_5C in titleMenuInit, otherwise scrolling will reset when leaving submenus like 'Help & Options'
		pattern = hook::pattern("D9 5E 5C C6 46 58 00 39");
		injector::MakeNOP(pattern.count(1).get(0).get<uint8_t>(0), 7);

		// reset scroll_add_5C when leaving Assignment Ada & Mercenaries 
		pattern = hook::pattern("C7 06 01 00 00 00 E8 ? ? ? ? 57");
		struct titleSub_resetScrollAdd
		{
			void operator()(injector::reg_pack& regs)
			{
				TitleWorkPtr()->scroll_add_5C = 1.5f;

				// Code we overwrote
				__asm { mov dword ptr[esi], 0x1 }
			}
		}; injector::MakeInline<titleSub_resetScrollAdd>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// reset scroll_add_5C when leaving Separate Ways
		pattern = hook::pattern("C7 45 F4 00 00 00 FF 8B ? ? 52 8B C8 51 50");
		struct titleAda_resetScrollAdd
		{
			void operator()(injector::reg_pack& regs)
			{
				TitleWorkPtr()->scroll_add_5C = 1.5f;

				// Code we overwrote
				__asm { mov dword ptr[ebp - 0xC], 0xFF000000 }
			}
		}; injector::MakeInline<titleAda_resetScrollAdd>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
	}

	// Force the original title menu background on completed saves
	{
		auto pattern = hook::pattern("F7 41 ? ? ? ? ? 6A ? 6A ? 6A ? 0F 85 ? ? ? ? 8B 46");
		struct titleSet_bgCheckHook
		{
			void operator()(injector::reg_pack& regs)
			{
				bool hasFinishedGame = FlagIsSet(SystemSavePtr()->flags_EXTRA_4, uint32_t(Flags_EXTRA::EXT_COSTUME));

				if (!hasFinishedGame || re4t::cfg->bAlwaysShowOriginalTitleBackground)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<titleSet_bgCheckHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
	}

	// Skip difficulty select on a fresh system save while in NTSC mode, or when OverrideDifficulty is enabled
	{
		auto pattern = hook::pattern("B8 04 00 00 00 5B 8B E5");
		struct titleMenuSelect_SkipLevelSelect
		{
			void operator()(injector::reg_pack& regs)
			{
				bool hasFinishedGame = FlagIsSet(SystemSavePtr()->flags_EXTRA_4, uint32_t(Flags_EXTRA::EXT_COSTUME));

				if ((re4t::cfg->bEnableNTSCMode && !hasFinishedGame) || re4t::cfg->bOverrideDifficulty)
					regs.eax = TTL_CMD_START;
				else
					regs.eax = TTL_CMD_LEVEL;
			}
		}; injector::MakeInline<titleMenuSelect_SkipLevelSelect>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// Japanese: ignore language_8 check in the TTL_CMD_START case
		pattern = hook::pattern("38 59 08 0F 84 09 0B 00 00");
		injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(3), 6); // titleMain
	}

	// Add Professional mode select to Separate Ways
	// TODO: support Assignment Ada as well. Not possible with this method currently, as all the difficulty select textures get unloaded in titleSub.
	{
		static const int def_texId[4] = { 170, 170, 187, 187 };
		static const int jpn_texId[4] = { 220, 220, 187, 187 };

		static const float eng_texW[4] = { 149.0f, 149.0f, 87.0f, 87.0f };
		static const float deu_texW[4] = {  60.0f,  60.0f, 87.0f, 87.0f };
		static const float fra_texW[4] = { 160.0f, 160.0f, 87.0f, 87.0f };
		static const float esp_texW[4] = { 140.0f, 140.0f, 87.0f, 87.0f };
		static const float ita_texW[4] = {  90.0f,  90.0f, 99.0f, 99.0f };

		static int texId_orig[4] = {};
		static float texW_orig[4] = {};
		static float texH_orig[4] = {};

		static bool insideLevelMenu = false;

		static uint8_t* mouseMenuNum = hook::pattern("03 7C ? BA 01 00 00 00 8B").count(1).get(0).get<uint8_t>(0);
		static uint32_t* snd_id_1654 = *hook::pattern("A3 ? ? ? ? 5F 5E 8B E5 5D").count(1).get(0).get<uint32_t*>(1);

		auto pattern_begin = hook::pattern("8B ? ? ? ? ? 8B ? ? ? ? ? 8B C1 25 00 00 00 40 81 E7 00 20 00 00 0B C7 74 ? 33");
		auto pattern_end = hook::pattern("C6 86 80 00 00 00 08 88 4E 03");
		struct titleAda_levelMenu
		{
			enum TitleAda
			{
				MenuExit = 6,
				CardLoad = 7,
				GameStart = 8
			};

			static const float* getLangW()
			{
				switch (SystemSavePtr()->language_8)
				{
				case 3: // German
					return deu_texW;
				case 4: // French
					return fra_texW;
				case 5: // Spanish
					return esp_texW;
				case 6: // Traditional Chinese / Italian
					return GameVersion() == "1.1.0" ? eng_texW : ita_texW;
				case 8: // Italian
					return ita_texW;
				default: // English, Japanese, Simplified Chinese
					return eng_texW;
				}
			}

			static void enterLevelMenu()
			{
				insideLevelMenu = true;
				injector::WriteMemory(mouseMenuNum, uint8_t(0x2), true); // disable mouse interaction with the MenuExit texture
				TitleWorkPtr()->omk_menu_no_6C = 1;

				bio4::SndCall(0, 4u, 0, 0, 0, 0);

				const int* texId = SystemSavePtr()->language_8 ? def_texId : jpn_texId;
				const float* texW = getLangW();

				for (int i = 0; i < 4; ++i)
				{
					ID_UNIT* tex = IDSystemPtr()->unitPtr(i + 1, IDC_OPTION_BG);

					texId_orig[i] = tex->texId_78;
					texW_orig[i] = tex->size0_W_DC;
					texH_orig[i] = tex->size0_H_E0;

					tex->texId_78 = texId[i];
					tex->size0_W_DC = texW[i];
					tex->size0_H_E0 = 14;
					tex->pos0_94.y -= 14;
					IDSystemPtr()->setTime(tex, 0); // fade in
				}

				IDSystemPtr()->unitPtr(0x5u, IDC_OPTION_BG)->be_flag_0 &= ~ID_BE_FLAG_VISIBLE;
				IDSystemPtr()->unitPtr(0x6u, IDC_OPTION_BG)->be_flag_0 &= ~ID_BE_FLAG_VISIBLE;
			}

			static void exitLevelMenu()
			{
				insideLevelMenu = false;
				injector::WriteMemory(mouseMenuNum, uint8_t(0x3), true);
				TitleWorkPtr()->omk_menu_no_6C = 0;

				bio4::SndCall(0, 5u, 0, 0, 0, 0);

				for (int i = 0; i < 4; ++i)
				{
					ID_UNIT* tex = IDSystemPtr()->unitPtr(i + 1, IDC_OPTION_BG);

					tex->texId_78 = texId_orig[i];
					tex->size0_W_DC = texW_orig[i];
					tex->size0_H_E0 = texH_orig[i];
					tex->pos0_94.y += 14;
					IDSystemPtr()->setTime(tex, 0);
				}

				IDSystemPtr()->unitPtr(0x5u, IDC_OPTION_BG)->be_flag_0 |= ID_BE_FLAG_VISIBLE;
				IDSystemPtr()->setTime(IDSystemPtr()->unitPtr(0x5u, IDC_OPTION_BG), 0);
				IDSystemPtr()->unitPtr(0x6u, IDC_OPTION_BG)->be_flag_0 |= ID_BE_FLAG_VISIBLE;
				IDSystemPtr()->setTime(IDSystemPtr()->unitPtr(0x6u, IDC_OPTION_BG), 0);
			}

			// replaces most of the input code in titleAda routine 5
			void operator()(injector::reg_pack& regs)
			{
				TITLE_WORK* pT = TitleWorkPtr();

				if ((Key_btn_trg() & (uint64_t)KEY_BTN::KEY_MINUS) == (uint64_t)KEY_BTN::KEY_MINUS ||
					(Key_btn_trg() & (uint64_t)KEY_BTN::KEY_CANCEL) == (uint64_t)KEY_BTN::KEY_CANCEL)
				{
					if (!insideLevelMenu)
					{
						pT->Rno1_1 = TitleAda::MenuExit;
						FadeWorkPtr(FADE_NO_SYSTEM)->FadeSet(FADE_NO_SYSTEM, 5u, 0);
						bio4::SndCall(0, 5u, 0, 0, 0, 0);
						bio4::SndStrReq_0(*(uint32_t*)snd_id_1654, 4, 0x3C, 0); // cancel music
					}
					else
						exitLevelMenu();
				}
				else if ((Key_btn_trg() & (uint64_t)KEY_BTN::KEY_OK) == (uint64_t)KEY_BTN::KEY_OK ||
					(Key_btn_trg() & (uint64_t)KEY_BTN::KEY_Y) == (uint64_t)KEY_BTN::KEY_Y)
				{
					switch (pT->omk_menu_no_6C)
					{
					case 0:
						if (!re4t::cfg->bSeparateWaysDifficultyMenu || re4t::cfg->bOverrideDifficulty)
							pT->Rno1_1 = TitleAda::GameStart;
						else if (!insideLevelMenu)
							enterLevelMenu();
						else
						{
							pT->Rno1_1 = TitleAda::GameStart;
							GlobalPtr()->gameDifficulty_847C = GameDifficulty::Pro;
							insideLevelMenu = false;
							injector::WriteMemory(mouseMenuNum, uint8_t(0x3), true);
						}
						break;
					case 1:
						if (!insideLevelMenu)
							pT->Rno1_1 = TitleAda::CardLoad;
						else
						{
							pT->Rno1_1 = TitleAda::GameStart;
							insideLevelMenu = false;
							injector::WriteMemory(mouseMenuNum, uint8_t(0x3), true);
						}
						break;
					case 2:
						pT->Rno1_1 = TitleAda::MenuExit;
						FadeWorkPtr(FADE_NO_SYSTEM)->FadeSet(FADE_NO_SYSTEM, 5u, 0);
						bio4::SndCall(0, 5u, 0, 0, 0, 0);
						bio4::SndStrReq_0(*(uint32_t*)snd_id_1654, 4, 0x3C, 0);
						break;
					}
				}
				else if ((Key_btn_trg() & (uint64_t)KEY_BTN::KEY_U) == (uint64_t)KEY_BTN::KEY_U ||
					(Key_btn_trg() & (uint64_t)KEY_BTN::KEY_D) == (uint64_t)KEY_BTN::KEY_D)
				{
					int omk_menu_no_6C = pT->omk_menu_no_6C;
					if ((Key_btn_trg() & (uint64_t)KEY_BTN::KEY_U) == (uint64_t)KEY_BTN::KEY_U)
						omk_menu_no_6C--;
					else
						omk_menu_no_6C++;

					if (!insideLevelMenu)
						omk_menu_no_6C = std::clamp(omk_menu_no_6C, 0, 2);
					else
					{
						if (omk_menu_no_6C < 0)
							omk_menu_no_6C = 1;
						else if (omk_menu_no_6C > 1)
							omk_menu_no_6C = 0;
					}

					if (omk_menu_no_6C != pT->omk_menu_no_6C)
					{
						bio4::SndCall(0, 0xAu, 0, 0, 0, 0);
						pT->omk_menu_no_6C = omk_menu_no_6C;
					}
				}
			}
		}; injector::MakeInline<titleAda_levelMenu>(pattern_begin.count(1).get(0).get<uint32_t>(0), pattern_end.count(1).get(0).get<uint32_t>(14));

		// gameInit: don't overwrite Professional mode difficulty for Separate Ways or Assignment Ada
		auto pattern = hook::pattern("F7 40 54 00 10 00 C0 74 ? C6 80 7C 84 00 00 05 A1");
		struct gameInit_AdaPro
		{
			void operator()(injector::reg_pack& regs)
			{
				// Don't do anything here if OverrideDifficulty is enabled. OverrideDifficulty uses another hook earlier in gameInit,
				// so there's no need to do anything here if that is enabled.
				if (!re4t::cfg->bOverrideDifficulty)
				{
					bool isSeparateWays = FlagIsSet(GlobalPtr()->Flags_SYSTEM_0_54, uint32_t(Flags_SYSTEM::SYS_PS2_ADA_GAME));
					bool isAssignmentAda = FlagIsSet(GlobalPtr()->Flags_SYSTEM_0_54, uint32_t(Flags_SYSTEM::SYS_OMAKE_ADA_GAME));
					bool isMercenaries = FlagIsSet(GlobalPtr()->Flags_SYSTEM_0_54, uint32_t(Flags_SYSTEM::SYS_OMAKE_ETC_GAME));

					if (isMercenaries ||
						((isSeparateWays || isAssignmentAda) && GlobalPtr()->gameDifficulty_847C != GameDifficulty::Pro))
					{
						GlobalPtr()->gameDifficulty_847C = GameDifficulty::Medium;
					}
				}
			}
		}; injector::MakeInline<gameInit_AdaPro>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(21));
	}
}

