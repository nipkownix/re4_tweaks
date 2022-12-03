#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"

uint32_t ModelForceRenderAll_EndTick = 0;

// Called by AddOtModelPosRadius to check if model is in view of camera
bool(*collision_sphere_hexahedron)(void*, void*);
bool collision_sphere_hexahedron_Hook(void* a1, void* a2)
{
	if (ModelForceRenderAll_EndTick > GlobalPtr()->frameCounter_530C)
		return true;
	return collision_sphere_hexahedron(a1, a2);
}

void(*NowLoadingOff)();
void NowLoadingOff_Hook()
{
	NowLoadingOff();

	// Enable model hack for 1 tick after loading
	// Forces game to fully process all models (rendering etc)
	// Without this game could lag significantly when many newly-seen models are on screen
	// By forcing this processing for all models, it seems something is cached
	// which stops lag from occurring when the models are viewed later on
	// (by enabling this hack after load screen, the player will also view any perf drop from it as part of loading too :)
	if (re4t::cfg->bPrecacheModels)
		ModelForceRenderAll_EndTick = GlobalPtr()->frameCounter_530C + 1;
}

void re4t::init::FrameRateFixes()
{
	// Fix the speed of falling items
	{
		// Treasure (emItem_R1_Drop) (Items from bird nests are handled by another function that already has deltaTime_70 applied to it)
		{
			auto pattern = hook::pattern("DC 25 ? ? ? ? 6A ? 83 EC ? 8D BE ? ? ? ? D9 9E ? ? ? ? B9");
			struct emItem_R1_Drop_hook
			{
				void operator()(injector::reg_pack& regs)
				{
					double vanillaSub = 10.0;
					
					if (re4t::cfg->bFixFallingItemsSpeed)
					{
						double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;

						// How many frames to wait until newSub is subtracted from pEm->Spd_62C.y.
						// On 30 fps, subtract every frame. On 60 fps, subtract every two frames, etc.
						int interval = (int)std::round(CurrentFrameRate() / 30);

						// Interval must be at least 1
						if (interval < 1)
							interval = 1;

						// Only apply the speed changes if the frame interval has passed
						if (GlobalPtr()->frameCounter_530C % interval == 0)
						{
							_asm {fsub newSub}
						}
					}
					else
					{
						_asm {fsub vanillaSub}
					}
				}
			}; injector::MakeInline<emItem_R1_Drop_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}

		// Falling door locks (cObj12::fallMove, might affect a few other falling things too)
		{
			// Ajust the speed of the Y axis
			auto pattern = hook::pattern("DC 25 ? ? ? ? 50 8D 56 ? 52 50 D9 1E E8 ? ? ? ? C7 46");
			struct cObj12__fallMove_hook
			{
				void operator()(injector::reg_pack& regs)
				{
					double vanillaSub = 20.0;

					if (re4t::cfg->bFixFallingItemsSpeed)
					{
						double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;

						int interval = (int)std::round(CurrentFrameRate() / 30);

						// Interval must be at least 1
						if (interval < 1)
							interval = 1;

						// Only apply the speed changes if the frame interval has passed
						if (GlobalPtr()->frameCounter_530C % interval == 0)
						{
							_asm {fsub newSub}
						}
					}
					else
					{
						_asm {fsub vanillaSub}
					}
				}
			}; injector::MakeInline<cObj12__fallMove_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

			// Ajust the speed of all axis. Seems to be indepent from the calculation above.
			struct cObj12__setFall_hook
			{
				void operator()(injector::reg_pack& regs)
				{
					double vanillaMulti = 10.0;

					if (re4t::cfg->bFixFallingItemsSpeed)
					{
						double newMulti = GlobalPtr()->deltaTime_70 * vanillaMulti;
						_asm {fmul newMulti}
					}
					else
					{
						_asm {fmul vanillaMulti}
					}
				}
			}; 
			
			// First 3
			pattern = hook::pattern("DC 0D ? ? ? ? E8 ? ? ? ? 66 89 87");
			pattern.count(3).for_each_result([&](hook::pattern_match match) {
				injector::MakeInline<cObj12__setFall_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			});

			// 4, 5
			pattern = hook::pattern("DC 0D ? ? ? ? E8 ? ? ? ? 8B ? ? 66 89 ? ? E8 ? ? ? ? D8");
			pattern.count(2).for_each_result([&](hook::pattern_match match) {
				injector::MakeInline<cObj12__setFall_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			});

			// Last
			pattern = hook::pattern("DC 0D ? ? ? ? E8 ? ? ? ? 8D 54 76");
			injector::MakeInline<cObj12__setFall_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}
	}

	// Fix the speed of opening drawers/boxes/cabinets/etc
	{
		// r104_openShelf_main, r129_openShelf_main_mb, r12a_openShelf_main_mb
		auto pattern = hook::pattern("DC 35 ? ? ? ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? D9 5D ? E8 ? ? ? ? 83 C4 ? BF ? ? ? ? EB");
		struct r104_r129_r12a_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newMulti = (double)CurrentFrameRate();
					_asm {fdiv newMulti}
				}
				else
				{
					double vanillaMulti = 30.0;
					_asm {fdiv vanillaMulti}
				}
			}
		};

		struct r104_r129_r12a_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.edi = CurrentFrameRate();
				}
				else
				{
					regs.edi = 30; // Vanilla loop count
				}
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r104_r129_r12a_hook_1>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r104_r129_r12a_hook_2>(match.get<uint32_t>(29), match.get<uint32_t>(33));
		});

		// r106_openShelf_main, r128_openShelf_main_mb, r507_openShelf_main_mb
		pattern = hook::pattern("DD 05 ? ? ? ? 6A ? DC F9 6A ? 6A ? 6A ? 6A ? 6A ? D9 C9 D9");
		struct r106_r128_r507_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newMulti = (double)CurrentFrameRate();
					_asm {fld newMulti}
				}
				else
				{
					double vanillaMulti = 30.0;
					_asm {fld vanillaMulti}
				}
			}
		};

		struct r106_r128_r507_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.ebx = CurrentFrameRate();
				}
				else
					regs.ebx = 30; // Vanilla loop count
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r106_r128_r507_hook_1>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r106_r128_r507_hook_2>(match.get<uint32_t>(39), match.get<uint32_t>(43));
		});

		// r20d_openShelf_main
		pattern = hook::pattern("BB ? ? ? ? 8D A4 24 ? ? ? ? 8B 86 ? ? ? ? D9 80");
		struct r20d_openShelf_main_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.ebx = CurrentFrameRate();
				}
				else
					regs.ebx = 30; // Vanilla loop count
			}
		}; injector::MakeInline<r20d_openShelf_main_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		pattern = hook::pattern("DC 25 ? ? ? ? D9 98 ? ? ? ? 8B 87");
		struct r20d_openShelf_main_hook_2_pos
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaSub = 0.07400000095367432;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;
					_asm {fsub newSub}
				}
				else
				{
					_asm {fsub vanillaSub}
				}
			}
		}; injector::MakeInline<r20d_openShelf_main_hook_2_pos>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct r20d_openShelf_main_hook_2_neg
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaSub = -0.07400000095367432;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;
					_asm {fsub newSub}
				}
				else
				{
					_asm {fsub vanillaSub}
				}
			}
		}; injector::MakeInline<r20d_openShelf_main_hook_2_neg>(pattern.count(1).get(0).get<uint32_t>(24), pattern.count(1).get(0).get<uint32_t>(30));

		// r20e_openShelf_main
		pattern = hook::pattern("BB ? ? ? ? EB ? 8D 9B ? ? ? ? 8B 86 ? ? ? ? DD 05");
		struct r20e_openShelf_main_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.ebx = CurrentFrameRate();
				}
				else
					regs.ebx = 30; // Vanilla loop count
			}
		}; injector::MakeInline<r20e_openShelf_main_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		struct r20e_openShelf_main_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 0.07400000095367432;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		}; injector::MakeInline<r20e_openShelf_main_hook_2>(pattern.count(1).get(0).get<uint32_t>(19), pattern.count(1).get(0).get<uint32_t>(25));

		// r104_openBox_main, r129_openBox_main_mb, r12a_openBox_main_mb, r200_openBox_main, r22e_openBox_main_mb
		pattern = hook::pattern("BF ? ? ? ? 8B 86 ? ? ? ? D9 80 ? ? ? ? 6A ? DC 05 ? ? ? ? D9 98 ? ? ? ? E8");
		struct r104_r129_r12a_r200_r22e_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.edi = CurrentFrameRate();
				}
				else
					regs.edi = 30; // Vanilla loop count
			}
		};

		struct r104_r129_r12a_r200_r22e_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 0.05666666850447655;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;
					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r104_r129_r12a_r200_r22e_hook_1>(match.get<uint32_t>(0), match.get<uint32_t>(5));
			injector::MakeInline<r104_r129_r12a_r200_r22e_hook_2>(match.get<uint32_t>(19), match.get<uint32_t>(25));
		});

		// r200_openBox_main and r22e_openBox_main_mb uses another pattern, but the same hooks
		pattern = hook::pattern("BF ? ? ? ? 8D A4 24 ? ? ? ? 8B 86 ? ? ? ? D9 80 ? ? ? ? 6A");
		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r104_r129_r12a_r200_r22e_hook_1>(match.get<uint32_t>(0), match.get<uint32_t>(5));
			injector::MakeInline<r104_r129_r12a_r200_r22e_hook_2>(match.get<uint32_t>(26), match.get<uint32_t>(32));
		});

		// r202_openBox_main, r20e_openBox_main
		pattern = hook::pattern("D9 05 ? ? ? ? 57 51 8D 45 ? D9 1C ? 50 8D 4D");
		struct r202_r20e_openBox_main_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 0.033333335;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		}; injector::MakeInline<r202_r20e_openBox_main_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("BF ? ? ? ? EB ? 8D A4 24 ? ? ? ? 8B FF 8B 86");
		struct r202_r20e_openBox_main_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.edi = CurrentFrameRate();
				}
				else
					regs.edi = 30; // Vanilla loop count
			}
		}; injector::MakeInline<r202_r20e_openBox_main_hook_2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// r20e_openBox_main uses the same hooks
		pattern = hook::pattern("D9 05 ? ? ? ? 57 51 8D 55 ? D9 1C ? 52 8D 45");
		injector::MakeInline<r202_r20e_openBox_main_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("BF ? ? ? ? 8D 49 ? 8B 86 ? ? ? ? 05 ? ? ? ? 50");
		injector::MakeInline<r202_r20e_openBox_main_hook_2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// r326_openBox_main, r51b_openBox_main_mb
		pattern = hook::pattern("D9 05 ? ? ? ? 51 8D 45 ? D9 1C ? 50 8D 4D ? 51 E8 ? ? ? ? D9 05 ? ? ? ? 83 C4");
		struct r326_r51b_openBox_main_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 0.033333335;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		};
		
		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r326_r51b_openBox_main_hook_1>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r326_r51b_openBox_main_hook_1>(match.get<uint32_t>(23), match.get<uint32_t>(29));
		});

		pattern = hook::pattern("C7 45 ? ? ? ? ? 53 8D 4D ? 51 53 E8 ? ? ? ? 8B 87 ? ? ? ? 05 ? ? ? ? 50");
		struct r326_r51b_openBox_main_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					*(int32_t*)(regs.ebp - 0x38) = CurrentFrameRate();
				}
				else
					*(int32_t*)(regs.ebp - 0x38) = 30; // Vanilla loop count
			}
		};

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r326_r51b_openBox_main_hook_2>(match.get<uint32_t>(0), match.get<uint32_t>(7));
		});

		// r20d_openDrawer_main 
		pattern = hook::pattern("D9 05 ? ? ? ? 53 51 8D 45 ? D9 1C ? 50 8D 4D");
		struct r20d_openDrawer_main_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 0.033333335;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		}; injector::MakeInline<r20d_openDrawer_main_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("BB ? ? ? ? 56 8D 55 ? 52 56 E8 ? ? ? ? 83 C4 ? 85 FF");
		struct r20d_openDrawer_main_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.ebx = CurrentFrameRate();
				}
				else
				regs.ebx = 30; // Vanilla loop count
			}
		}; injector::MakeInline<r20d_openDrawer_main_hook_2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// r107_openKiln_main
		pattern = hook::pattern("DC 35 ? ? ? ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? D9");
		struct r107_openKiln_main_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newMulti = (double)CurrentFrameRate();
					_asm {fdiv newMulti}
				}
				else
				{
					double vanillaMulti = 30.0;
					_asm {fdiv vanillaMulti}
				}
			}
		}; injector::MakeInline<r107_openKiln_main_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct r107_openKiln_main_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.ebx = CurrentFrameRate();
				}
				else
				{
					regs.ebx = 30; // Vanilla loop count
				}
			}
		}; injector::MakeInline<r107_openKiln_main_2>(pattern.count(1).get(0).get<uint32_t>(29), pattern.count(1).get(0).get<uint32_t>(34));

		// r315_TanaOpen, r515_TanaOpen
		pattern = hook::pattern("D9 05 ? ? ? ? 83 C4 ? D9 5D ? 83 FE ? 75 ? D9 05 ? ? ? ? D9 5D ? BF");
		struct r315_r515_TanaOpen_hook_1_pos
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 26.666666;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		};

		struct r315_r515_TanaOpen_hook_1_neg
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = -26.666666;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		};

		struct r315_r515_TanaOpen_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					regs.edi = CurrentFrameRate();
				}
				else
				{
					regs.edi = 30; // Vanilla loop count
				}
			}
		};

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r315_r515_TanaOpen_hook_1_pos>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r315_r515_TanaOpen_hook_1_neg>(match.get<uint32_t>(17), match.get<uint32_t>(23));
			injector::MakeInline<r315_r515_TanaOpen_hook_2>(match.get<uint32_t>(26), match.get<uint32_t>(31));
		});
	}

	// Fix character backwards turning speed
	{
		struct TurnSpeedSubtract
		{
			void operator()(injector::reg_pack& regs)
			{
				float cPlayer__SPEED_WALK_TURN = 0.04188790545f; // game calcs this on startup, always seems to be same value
				float newTurnSpeed = GlobalPtr()->deltaTime_70 * cPlayer__SPEED_WALK_TURN;

				if (re4t::cfg->bFixTurningSpeed)
					_asm {fsub newTurnSpeed}
				else
					_asm {fsub cPlayer__SPEED_WALK_TURN}
			}
		};

		auto pattern = hook::pattern("D8 25 ? ? ? ? D9 ? A4 00 00 00");

		// 0x7636d9 - pl_R1_Back
		// 0x766069 - pl_R1_Crouch
		// 0x7660b6 - pl_R1_Crouch
		// 0x766167 - pl_R1_Crouch
		// 0x7661b4 - pl_R1_Crouch
		// 0x9001a3 - pl_R1_KlauserAttack
		for (int i = 0; i < 6; i++)
			injector::MakeInline<TurnSpeedSubtract>(pattern.count(6).get(i).get<uint32_t>(0), pattern.count(6).get(i).get<uint32_t>(6));

		struct TurnSpeedAdd
		{
			void operator()(injector::reg_pack& regs)
			{
				float cPlayer__SPEED_WALK_TURN = 0.04188790545f; // game calcs this on startup, always seems to be same value
				float newTurnSpeed = GlobalPtr()->deltaTime_70 * cPlayer__SPEED_WALK_TURN;

				if (re4t::cfg->bFixTurningSpeed)
					_asm {fadd newTurnSpeed}
				else
					_asm {fadd cPlayer__SPEED_WALK_TURN}
			}
		};

		pattern = hook::pattern("D8 05 ? ? ? ? D9 ? A4 00 00 00");

		// 0x7636fa - pl_R1_Back
		// 0x76607b - pl_R1_Crouch
		// 0x7660a4 - pl_R1_Crouch
		// 0x766179 - pl_R1_Crouch
		// 0x7661a2 - pl_R1_Crouch
		for (int i = 0; i < 5; i++)
			injector::MakeInline<TurnSpeedAdd>(pattern.count(5).get(i).get<uint32_t>(0), pattern.count(5).get(i).get<uint32_t>(6));

		struct TurnSpeedLoad
		{
			void operator()(injector::reg_pack& regs)
			{
				float cPlayer__SPEED_WALK_TURN = 0.04188790545f; // game calcs this on startup, always seems to be same value
				float newTurnSpeed = GlobalPtr()->deltaTime_70 * cPlayer__SPEED_WALK_TURN;

				if (re4t::cfg->bFixTurningSpeed)
					_asm {fld newTurnSpeed}
				else
					_asm {fld cPlayer__SPEED_WALK_TURN}
			}
		};

		pattern = hook::pattern("D9 05 ? ? ? ? D8 86 A4 00 00 00 D9 9E A4 00 00 00");

		// 0x9001C0 - pl_R1_KlauserAttack
		injector::MakeInline<TurnSpeedLoad>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}

	// Mercenaries: fix doubling of the village stage's passive difficulty gain in 60fps NTSC mode
	{
		static float fMercsDeltaTimer = 0.0F;

		auto pattern = hook::pattern("A1 40 ? ? ? 80 ? ? 00 74 ? 6A 0E"); // R400Main());
		struct MercsModeFPSFix
		{
			void operator()(injector::reg_pack& regs)
			{
				fMercsDeltaTimer += GlobalPtr()->deltaTime_70;
				if (fMercsDeltaTimer >= 1.0F)
				{
					fMercsDeltaTimer -= 1.0F;
					regs.ef |= (1 << regs.zero_flag); // call j_GameAddPoint(LVADD_TIMECOUNT);
				}
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<MercsModeFPSFix>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(9));
	}

	// Copy delta-time related code from cSubChar::moveBust to cPlAshley::moveBust
	// Seems to make Ashley bust physics speed match between 30 & 60FPS
	{
		auto pattern = hook::pattern("57 E8 ? ? ? ? 8B 06 8B 50 0C 00 1D");

		// Hook struct
		struct AshleyBustFrametimeFix
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bFixAshleyBustPhysics)
				{
					// Update ebx to make use of delta-time from pG+0x70
					float ebx = float(regs.ebx);
					ebx *= GlobalPtr()->deltaTime_70;
					regs.ebx = uint32_t(ebx);
				}

				// Code that we overwrote
				regs.eax = *(uint32_t*)regs.esi;
				regs.edx = *(uint32_t*)(regs.eax + 0xC);
			}
		}; injector::MakeInline<AshleyBustFrametimeFix>(pattern.count(1).get(0).get<uint32_t>(6), pattern.count(1).get(0).get<uint32_t>(11));
	}

	// Workaround for lag spike when many models are first shown on screen
	// forces game to treat all models as on-screen for a single tick after loading
	// allowing game to process/cache whatever data was causing lag spike
	auto pattern = hook::pattern("8B 4F 08 50 8D 55 ? 52 89 4D ? E8 ? ? ? ?"); // AddOtModelPosRadius
	auto caller = pattern.count(2).get(1).get<uint8_t>(0xB);
	ReadCall(caller, collision_sphere_hexahedron);
	InjectHook(caller, collision_sphere_hexahedron_Hook, PATCH_CALL);

	pattern = hook::pattern("39 91 80 84 00 00 0F 85 ? ? ? ? E8 ? ? ? ?"); // gameStageInit
	caller = (uint8_t*)injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xC)).as_int();
	ReadCall(caller, NowLoadingOff);
	InjectHook(caller, NowLoadingOff_Hook, PATCH_JUMP);

	pattern = hook::pattern("D8 D1 DF E0 DD D9 F6 C4 41 75 ? D9 C0 D9 EE DA E9 DF E0 F6 C4 44 7A");

	struct ModelRenderDistHack
	{
		void operator()(injector::reg_pack& regs)
		{
			// original code we patched over...
			__asm
			{
				fcom st(1)
				fnstsw ax
				fstp st(1)
				mov ecx, [regs]
				mov[ecx]regs.eax, eax
			}
			// distance check skip
			if (ModelForceRenderAll_EndTick > GlobalPtr()->frameCounter_530C)
				regs.eax = 0x3125; // passes "test ah, 41h"
		}
	}; injector::MakeInline<ModelRenderDistHack>(pattern.count(2).get(1).get<uint8_t>(0), pattern.count(2).get(1).get<uint8_t>(6));

	spd::log()->info("{} -> FPS fixes applied", __FUNCTION__);
}
