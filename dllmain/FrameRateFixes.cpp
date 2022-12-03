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

// QLOC dun goofed this one. This function is supposed to handle the opening of generic boxes/chests in the game.
// It handles both chests that are opening, and chests that are already open (if you save the game after opening one, for exemple.)
// Apparently they thought  it only handled chests that are opening, and applied GlobalPtr()->deltaTime_70 to correct the speed at higher fps,
// but they also applied it to the section of the function that sets up the position of boxes that are already open. This means that open chests would
// only by open half-way through at 60 fps, and even less at higher fps.
// Instead of hooking every single wrong multiplication here (there are many), we just temporarily 
// overwrite deltaTime_70 itself to 1 for the duration of the function, and restore it once it is done.
void(__cdecl* OpenBoxMain_orig)(int type, bool openedFlag, __int16 seId, int smdId0, int smdId1, int itemAtNo0);
void __cdecl OpenBoxMain_hook(int type, bool openedFlag, __int16 seId, int smdId0, int smdId1, int itemAtNo0)
{
	static float deltaTimeBackup = 1.0f;

	// Temporarily overwrite deltaTime_70 if openedFlag is set
	if (openedFlag)
	{
		deltaTimeBackup = GlobalPtr()->deltaTime_70;
		GlobalPtr()->deltaTime_70 = 1.0f;
	}

	OpenBoxMain_orig(type, openedFlag, seId, smdId0, smdId1, itemAtNo0);

	// Restore deltaTime_70
	if (openedFlag)
		GlobalPtr()->deltaTime_70 = deltaTimeBackup;
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
		struct r104_r129_r12a_openShelf_main_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaDiv = 30.0;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newDiv = vanillaDiv / GlobalPtr()->deltaTime_70;
					_asm {fdiv newDiv}
				}
				else
				{
					_asm {fdiv vanillaDiv}
				}
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r104_r129_r12a_openShelf_main_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
		});

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 4F 75 DE 5F 5E 8B E5 5D C3");
		struct r104_r129_r12a_openShelf_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					// The dec asm instruction later will set up the flag to get out of the loop when regs.edi is decreased to 0.
					// We replace the timer here with our own float, and when the loop should end, we make regs.edi "1", so the dec
					// instruction gets us out of the loop.
					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 30;
					}
				}
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r104_r129_r12a_openShelf_main_hook_new_timer>(match.get<uint32_t>(0), match.get<uint32_t>(5));
		});

		// r106_openShelf_main, r128_openShelf_main_mb, r507_openShelf_main_mb
		pattern = hook::pattern("DD 05 ? ? ? ? 6A ? DC F9 6A ? 6A ? 6A ? 6A ? 6A ? D9 C9 D9");
		struct r106_r128_r507_openShelf_main_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 30.0;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newNum = vanillaNum / GlobalPtr()->deltaTime_70;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r106_r128_r507_openShelf_main_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
		});

		pattern = hook::pattern("D9 45 ? D8 80 ? ? ? ? D9 98 ? ? ? ? E8 ? ? ? ? 83 C4 ? 4B 75 ? 5B 5F");
		struct r106_r128_r507_openShelf_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.ebx = 1;
					}
					else
					{
						regs.ebx = 30;
					}
				}
			}
		};

		pattern.count(3).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r106_r128_r507_openShelf_main_hook_new_timer>(match.get<uint32_t>(15), match.get<uint32_t>(20));
		});

		// r20d_openShelf_main
		pattern = hook::pattern("DC 25 ? ? ? ? D9 98 ? ? ? ? 8B 87");
		struct r20d_openShelf_main_hook_pos
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
		}; injector::MakeInline<r20d_openShelf_main_hook_pos>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct r20d_openShelf_main_hook_neg
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
		}; injector::MakeInline<r20d_openShelf_main_hook_neg>(pattern.count(1).get(0).get<uint32_t>(24), pattern.count(1).get(0).get<uint32_t>(30));

		struct r20d_openShelf_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.ebx = 1;
					}
					else
					{
						regs.ebx = 30;
					}
				}
			}
		}; injector::MakeInline<r20d_openShelf_main_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(36), pattern.count(1).get(0).get<uint32_t>(41));

		// r20e_openShelf_main
		pattern = hook::pattern("DD 05 ? ? ? ? D9 80 ? ? ? ? 6A 01 D8 C1 D9 98");
		struct r20e_openShelf_main_hook
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
		}; injector::MakeInline<r20e_openShelf_main_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct r20e_openShelf_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.ebx = 1;
					}
					else
					{
						regs.ebx = 30;
					}
				}
			}
		}; injector::MakeInline<r20e_openShelf_main_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(40), pattern.count(1).get(0).get<uint32_t>(45));

		// r104_openBox_main, r129_openBox_main_mb, r12a_openBox_main_mb, r200_openBox_main, r22e_openBox_main_mb
		pattern = hook::pattern("DC 05 ? ? ? ? D9 98 ? ? ? ? E8 ? ? ? ? 83 C4 ? 4F 75 ? 5F 5E 5D C3");
		struct r104_r129_r12a_r200_r22e_hook
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

		struct r104_r129_r12a_r200_r22e_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 30;
					}
				}
			}
		};

		pattern.count(5).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r104_r129_r12a_r200_r22e_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r104_r129_r12a_r200_r22e_hook_new_timer>(match.get<uint32_t>(12), match.get<uint32_t>(17));
		});

		// r202_openBox_main, r20e_openBox_main
		pattern = hook::pattern("D9 05 ? ? ? ? 57 51 8D 45 ? D9 1C ? 50 8D 4D");
		struct r202_r20e_openBox_main_hook
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
		}; injector::MakeInline<r202_r20e_openBox_main_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("D9 05 ? ? ? ? 57 51 8D 55 ? D9 1C ? 52 8D 45"); // r20e_openBox_main
		injector::MakeInline<r202_r20e_openBox_main_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 4F 75 DD 5F 8B 4D FC 33 CD 5E E8 ? ? ? ? 8B E5 5D C3");
		struct r202_r20e_openBox_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 30;
					}
				}
			}
		}; 

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r202_r20e_openBox_main_hook_new_timer>(match.get<uint32_t>(0), match.get<uint32_t>(5));
		});

		// r326_openBox_main, r51b_openBox_main_mb
		pattern = hook::pattern("D9 05 ? ? ? ? 51 8D 45 ? D9 1C ? 50 8D 4D ? 51 E8 ? ? ? ? D9 05 ? ? ? ? 83 C4");
		struct r326_r51b_openBox_main_hook
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
			injector::MakeInline<r326_r51b_openBox_main_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r326_r51b_openBox_main_hook>(match.get<uint32_t>(23), match.get<uint32_t>(29));
		});

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 FF 4D C8 75 9F 8B 4D FC 5F 5E 33 CD 5B E8 ? ? ? ? 8B E5 5D C3");
		struct r326_r51b_openBox_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						*(uint32_t*)(regs.ebp - 0x38) = 1;
					}
					else
					{
						*(uint32_t*)(regs.ebp - 0x38) = 30;
					}
				}
			}
		};

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r326_r51b_openBox_main_hook_new_timer>(match.get<uint32_t>(0), match.get<uint32_t>(5));
		});

		// r20d_openDrawer_main 
		pattern = hook::pattern("D9 05 ? ? ? ? 53 51 8D 45 ? D9 1C ? 50 8D 4D");
		struct r20d_openDrawer_main_hook
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
		}; injector::MakeInline<r20d_openDrawer_main_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 4B 75 CD 5B 8B 4D FC 5F 33 CD 5E E8 ? ? ? ? 8B E5");
		struct r20d_openDrawer_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.ebx = 1;
					}
					else
					{
						regs.ebx = 30;
					}
				}
			}
		}; injector::MakeInline<r20d_openDrawer_main_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// r107_openKiln_main
		pattern = hook::pattern("DC 35 ? ? ? ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? D9");
		struct r107_openKiln_main_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaDiv = 30.0;

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					double newDiv = vanillaDiv / GlobalPtr()->deltaTime_70;
					_asm {fdiv newDiv}
				}
				else
				{
					_asm {fdiv vanillaDiv}
				}
			}
		}; injector::MakeInline<r107_openKiln_main_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("8B 87 ? ? ? ? D8 80 ? ? ? ? D9 98 ? ? ? ? E8 ? ? ? ? 83 C4 ? 4B 75 ? 5B");
		struct r107_openKiln_main_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.ebx = 1;
					}
					else
					{
						regs.ebx = 30;
					}
				}
			}
		}; injector::MakeInline<r107_openKiln_main_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(18), pattern.count(1).get(0).get<uint32_t>(23));

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

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r315_r515_TanaOpen_hook_1_pos>(match.get<uint32_t>(0), match.get<uint32_t>(6));
			injector::MakeInline<r315_r515_TanaOpen_hook_1_neg>(match.get<uint32_t>(17), match.get<uint32_t>(23));
		});

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 08 4F 75 DE 5F 5E 5D C3");
		struct r315_r515_TanaOpen_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 30;
					}
				}
			}
		};

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<r315_r515_TanaOpen_hook_new_timer>(match.get<uint32_t>(0), match.get<uint32_t>(5));
		});

		// sub_7B49E0 (secret compartment in r50f's box)
		pattern = hook::pattern("DC 25 ? ? ? ? D9 9E ? ? ? ? 6A ? E8 ? ? ? ? 83");
		struct sub_7B49E0_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaSub = 0.05166666582226753;

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
		}; injector::MakeInline<sub_7B49E0_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct sub_7B49E0_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixCompartmentsOpeningSpeed)
				{
					// Set up new timer
					static float timer = 30.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 30.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 30;
					}
				}
			}
		}; injector::MakeInline<sub_7B49E0_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(14), pattern.count(1).get(0).get<uint32_t>(19));

		// OpenBoxMain
		pattern = hook::pattern("E8 ? ? ? ? 83 C4 18 5D E9 ? ? ? ? 3D ? ? ? ? 75 1A 6A FF 6A 36");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), OpenBoxMain_orig);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), OpenBoxMain_hook, PATCH_JUMP);
	}

	// Walls/etc/etc
	{
		// r20d_moveWall
		auto pattern = hook::pattern("DC 35 ? ? ? ? 6A ? D9 5D ? E8 ? ? ? ? 83 C4 ? A3");
		struct r20d_moveWall_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaDiv = 90.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newDiv = vanillaDiv / GlobalPtr()->deltaTime_70;
					_asm {fdiv newDiv}
				}
				else
				{
					_asm {fdiv vanillaDiv}
				}
			}
		}; injector::MakeInline<r20d_moveWall_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 47 89 7D E8");
		struct r20d_moveWall_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 0.0f;

					if (timer < 90.0f)
					{
						timer += GlobalPtr()->deltaTime_70;
						regs.edi = 1;
					}
					else
					{
						timer = 0.0f; // Reset timer
						regs.edi = 91;
					}
				}
			}
		}; injector::MakeInline<r20d_moveWall_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// r20e_moveCrestDoor
		pattern = hook::pattern("DC 05 ? ? ? ? D9 9E ? ? ? ? E8 ? ? ? ? 83 C4 04 4F 75 E1 A1");
		struct r20e_moveCrestDoor_opening_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 40.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;
					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		}; injector::MakeInline<r20e_moveCrestDoor_opening_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct r20e_moveCrestDoor_opening_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 75.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 75.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 75;
					}
				}
			}
		}; injector::MakeInline<r20e_moveCrestDoor_opening_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(12), pattern.count(1).get(0).get<uint32_t>(17));

		pattern = hook::pattern("DC 05 ? ? ? ? D9 5D ? D9 81 ? ? ? ? DE D9");
		struct r20e_moveCrestDoor_closing_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 15.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;

					// Applying deltaTime twice was the only way I managed to make the 240fps speed match the 30fps one. Not sure why.
					newAdd *= GlobalPtr()->deltaTime_70;

					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		}; injector::MakeInline<r20e_moveCrestDoor_closing_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("DC 05 ? ? ? ? D9 5D ? D9 86 ? ? ? ? D9 81");
		injector::MakeInline<r20e_moveCrestDoor_closing_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// r20e_moveArmorStatue
		pattern = hook::pattern("DD 05 ? ? ? ? 09 46 ? D8 F9 09 47");
		struct r20e_moveArmorStatue_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = 90.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newNum = vanillaNum / GlobalPtr()->deltaTime_70;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		}; injector::MakeInline<r20e_moveArmorStatue_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 4B 75 CA 8B 96 ? ? ? ? D9 05 ? ? ? ? D9 92");
		struct r20e_moveArmorStatue_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 90.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 90.0f; // Reset timer
						regs.ebx = 1;
					}
					else
					{
						regs.ebx = 90;
					}
				}
			}
		}; injector::MakeInline<r20e_moveArmorStatue_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// r21b_DoorOpen
		pattern = hook::pattern("DC 25 ? ? ? ? D9 9E ? ? ? ? E8 ? ? ? ? D9 05 ? ? ? ? 83 C4 ? D8 96");
		struct r21b_DoorOpen_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaSub = 60.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;

					_asm {fsub newSub}
				}
				else
				{
					_asm {fsub vanillaSub}
				}
			}
		}; injector::MakeInline<r21b_DoorOpen_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// r21b_GetDragonBall
		pattern = hook::pattern("DC 25 ? ? ? ? D9 1E E8 ? ? ? ? D9 05 ? ? ? ? 83 C4");
		struct r21b_GetDragonBall_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaSub = 50.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;

					_asm {fsub newSub}
				}
				else
				{
					_asm {fsub vanillaSub}
				}
			}
		}; injector::MakeInline<r21b_GetDragonBall_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// R31bExecFallRoom
		pattern = hook::pattern("33 C5 89 45 ? 53 8B 5D ? 8B 04 9D");
		struct R31bExecFallRoom_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we overwrote
				regs.eax ^= regs.ebp;
				*(uint32_t*)(regs.ebp - 0x4) = regs.eax;

				// The speed of the falling room is passed as an argument to this function, so we just manipulate it at the start.
				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					*(float*)(regs.ebp + 0xC) *= GlobalPtr()->deltaTime_70;
				}
			}
		}; injector::MakeInline<R31bExecFallRoom_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// cR31CDoor::close 
		pattern = hook::pattern("D9 05 ? ? ? ? 80 48 ? ? 8B 46 ? 80 48 ? ? D9 5E");
		struct cR31CDoor__close_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaNum = -60.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newNum = GlobalPtr()->deltaTime_70 * vanillaNum;
					_asm {fld newNum}
				}
				else
				{
					_asm {fld vanillaNum}
				}
			}
		}; injector::MakeInline<cR31CDoor__close_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));	
		
		// r327_DoorOpen
		pattern = hook::pattern("DC 05 ? ? ? ? D9 9E ? ? ? ? E8 ? ? ? ? D9 05 ? ? ? ? 83 C4 ? D8 96");
		struct r327_DoorOpen_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 40.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;
					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		}; injector::MakeInline<r327_DoorOpen_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// r501_openCover_mb
		pattern = hook::pattern("DC 35 ? ? ? ? D9 5D ? D9 80 ? ? ? ? D9 C9 DC 0D ? ? ? ? DE E9 D9 98");
		struct r501_openCover_mb_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaDiv = 25.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newDiv = vanillaDiv / GlobalPtr()->deltaTime_70;
					_asm {fdiv newDiv}
				}
				else
				{
					_asm {fdiv vanillaDiv}
				}
			}
		}; injector::MakeInline<r501_openCover_mb_hook>(pattern.count(2).get(0).get<uint32_t>(0), pattern.count(2).get(0).get<uint32_t>(6));

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 04 4F 75 DE 8B 86 ? ? ? ? D9");
		struct r501_openCover_mb_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 25.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 25.0f; // Reset timer
						regs.edi = 1;
					}
					else
					{
						regs.edi = 25;
					}
				}
			}
		}; injector::MakeInline<r501_openCover_mb_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// cR50fDoor::open
		pattern = hook::pattern("DC 05 ? ? ? ? D9 98 ? ? ? ? 8B 4E ? D9 46 ? D8 46 ? D9 81 ? ? ? ? D8 D9 DF E0 F6 C4 ? 75 ? D9 99 ? ? ? ? FE 46 ? 5E");
		struct cR50fDoor__open_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 40.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;
					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		}; injector::MakeInline<cR50fDoor__open_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		
		// R50fExecCageMain_mb
		pattern = hook::pattern("DE CA D9 C9 DC 35 ? ? ? ? DE E9 D9 5D ? D9");
		struct R50fExecCageMain_mb_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaDiv = 20.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newDiv = vanillaDiv / GlobalPtr()->deltaTime_70;
					_asm {fdiv newDiv}
				}
				else
				{
					_asm {fdiv vanillaDiv}
				}
			}
		}; 
		
		injector::MakeInline<R50fExecCageMain_mb_hook_1>(pattern.count(6).get(0).get<uint32_t>(4), pattern.count(6).get(0).get<uint32_t>(10));
		injector::MakeInline<R50fExecCageMain_mb_hook_1>(pattern.count(6).get(1).get<uint32_t>(4), pattern.count(6).get(1).get<uint32_t>(10));
		injector::MakeInline<R50fExecCageMain_mb_hook_1>(pattern.count(6).get(2).get<uint32_t>(4), pattern.count(6).get(2).get<uint32_t>(10));

		pattern = hook::pattern("89 75 ? 83 FE ? 0F 8C ? ? ? ? 8B 0D ? ? ? ? 6A");
		struct R50fExecCageMain_mb_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we overwrote
				*(uint32_t*)(regs.ebp - 0x2C) = regs.esi;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 0.0f;

					timer += GlobalPtr()->deltaTime_70;

					if (timer < 20.0f)
					{
						regs.ef |= (1 << regs.sign_flag); // The sign flag prevents the loop from breaking
					}
					else
					{
						timer = 0.0f; // Reset timer
					}
				}
				else if ((int)regs.esi < 20)
				{
					regs.ef |= (1 << regs.sign_flag);
				}
			}
		}; injector::MakeInline<R50fExecCageMain_mb_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// r405_em_set (Door opening)
		pattern = hook::pattern("DC 05 ? ? ? ? 6A 01 D9 98 ? ? ? ? E8 ? ? ? ? 83 C4 08");
		struct r405_em_set_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 120.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;
					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		}; injector::MakeInline<r405_em_set_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		struct r405_em_set_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 25.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 25.0f; // Reset timer
						regs.esi = 1;
					}
					else
					{
						regs.esi = 25;
					}
				}
			}
		}; injector::MakeInline<r405_em_set_hook_new_timer>(pattern.count(1).get(0).get<uint32_t>(14), pattern.count(1).get(0).get<uint32_t>(19));

		// r502_OpenCloseCover, r108_openCover
		pattern = hook::pattern("DC 05 ? ? ? ? 8B 15 ? ? ? ? 83 C4 ? D9 5D ? D9 45");
		struct r108_r502_Cover_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 10.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
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
			injector::MakeInline<r108_r502_Cover_hook>(match.get<uint32_t>(0), match.get<uint32_t>(6));
		});

		// sub_7A32B0 (r502's gate)
		pattern = hook::pattern("DC 25 ? ? ? ? 68 ? ? ? ? D9 98 ? ? ? ? E8");
		struct sub_7A32B0_hook_1
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaSub = 132.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newSub = GlobalPtr()->deltaTime_70 * vanillaSub;

					_asm {fsub newSub}
				}
				else
				{
					_asm {fsub vanillaSub}
				}
			}
		}; injector::MakeInline<sub_7A32B0_hook_1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("DC 05 ? ? ? ? 68 ? ? ? ? D9 98 ? ? ? ? E8");
		struct sub_7A32B0_hook_2
		{
			void operator()(injector::reg_pack& regs)
			{
				double vanillaAdd = 132.0;

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					double newAdd = GlobalPtr()->deltaTime_70 * vanillaAdd;
					_asm {fadd newAdd}
				}
				else
				{
					_asm {fadd vanillaAdd}
				}
			}
		}; injector::MakeInline<sub_7A32B0_hook_2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 4E 75 ? 8B 0D ? ? ? ? 8B 91 ? ? ? ? 56 52 E8");
		struct sub_7A32B0_hook_new_timer
		{
			void operator()(injector::reg_pack& regs)
			{
				// code we replaced
				bio4::SceSleep(1);

				if (re4t::cfg->bFixMovingGeometrySpeed)
				{
					// Set up new timer
					static float timer = 25.0f;

					timer -= GlobalPtr()->deltaTime_70;

					if (timer <= 0.0f)
					{
						timer = 25.0f; // Reset timer
						regs.esi = 1;
					}
					else
					{
						regs.esi = 25;
					}
				}
			}
		};

		pattern.count(2).for_each_result([&](hook::pattern_match match) {
			injector::MakeInline<sub_7A32B0_hook_new_timer>(match.get<uint32_t>(0), match.get<uint32_t>(5));
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
