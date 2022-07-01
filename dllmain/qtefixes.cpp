#include <iostream>
#include "stdafx.h"
#include "Patches.h"
#include "Settings.h"
#include "Logging/Logging.h"
#include "Game.h"
#include "input.hpp"

uintptr_t* ptrKrauserQTEMovAddr;
static uint32_t* ptrQTEactive;
static uint32_t* ptrleftTrigger_A;
static uint32_t* ptrrightTrigger_B;

bool isAutoQTE;

bool isQTEactive()
{
	if (*(int8_t*)ptrQTEactive != 0)
		return true;
	else
		return false;
}

// Auto-QTE
bool (__stdcall* cActionButton_checkButton_orig)(int a1); // Used for the quick reaction QTEs
bool __stdcall cActionButton_checkButton_hook(int a1)
{
	// We run the original function first since that's what sets QTEactive to 0 or 1
	bool orig = cActionButton_checkButton_orig(a1);

	if (pConfig->bDisableQTE && isQTEactive())
	{
		isAutoQTE = true;
		return true;
	}
	else
	{
		isAutoQTE = false;
		return orig;
	}
}

unsigned int(__cdecl* KeyTrgCheck_orig)(KEY_BTN a1); // Used for the mashing QTEs
unsigned int __cdecl KeyTrgCheck_hook(KEY_BTN a1)
{
	if (pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE)
	{
		isAutoQTE = true;
		return 1;
	}
	else
	{
		isAutoQTE = false;
		return KeyTrgCheck_orig(a1);
	}
}

int (*sub_859810_orig)(); // Check for both QTE keys when running from Salazar's statue
int sub_859810_hook()
{
	if (pConfig->bDisableQTE)
		return 1;
	else
		return sub_859810_orig();
}

int (*sub_8064B0_orig)(); // Last QTE from the laser room after Krauser
int sub_8064B0_hook()
{
	if (pConfig->bDisableQTE)
		return 1;
	else
		return sub_8064B0_orig();
}

// QTE Key icons
int* (__cdecl* sub_41E600_orig)(int* a1, __int64 a2); // Not sure what to call this func
int* __cdecl KEY1prompt_hook(int* a1, __int64 a2)
{
	UNREFERENCED_PARAMETER(a2);

	int newKey = pInput->KeyMap_getDIK(pConfig->sQTE_key_1);

	return sub_41E600_orig(a1, newKey);
}

int* __cdecl KEY2prompt_hook(int* a1, __int64 a2)
{
	UNREFERENCED_PARAMETER(a2);

	int newKey = pInput->KeyMap_getDIK(pConfig->sQTE_key_2);

	return sub_41E600_orig(a1, newKey);
}

void Init_QTEfixes()
{
	// Automatic QTEs
	{
		// QTEactive pointer
		auto pattern = hook::pattern("c6 05 ? ? ? ? ? e8 ? ? ? ? 83 c4 ? 84 c0 0f 84");
		ptrQTEactive = *pattern.count(1).get(0).get<uint32_t*>(2);

		// Hook cActionButton_checkButton
		pattern = hook::pattern("E8 ? ? ? ? 3C ? 0F 85 ? ? ? ? 8B 4E");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cActionButton_checkButton_orig);
		InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cActionButton_checkButton_hook);

		// Hook cActionButton::move to hide the prompts if the QTE is automatic
		pattern = hook::pattern("F7 40 ? ? ? ? ? 75 ? F6 46");
		struct ActionButtonPrompt
		{
			void operator()(injector::reg_pack& regs)
			{
				if (pConfig->bDisableQTE && (!pConfig->bAutomaticMashingQTE || pConfig->bAutomaticMashingQTE))
				{
					if (isAutoQTE || isQTEactive())
						regs.ef &= ~(1 << regs.zero_flag);
					else
						regs.ef |= (1 << regs.zero_flag);
				}
				else
					regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<ActionButtonPrompt>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		// Running from boulders
		{
			auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 85 C0 74 4F");
			ReadCall(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_orig);
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);
		}

		// Del Lago
		{
			// Swimming back to the boat
			auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 85 C0 74 ? A1 ? ? ? ? 66 83 B8 ? ? ? ? ? 7F ? D9 86");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);

			// Cutting the rope
			pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 85 C0 74 06 FF 86 ? ? ? ? 5E");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);
		}

		// Knifing El Gigante's parasite
		{
			// First
			auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 85 C0 74 06 FF 86 ? ? ? ? 0F B6 96 ? ? ? ?");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);

			// Second
			pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 85 C0 74 06 FF 86 ? ? ? ? 57");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);

			// Second
			pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 85 C0 74 ? 8B 15 ? ? ? ? 8A 82");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);
		}

		// Climbing up after the minecart ride
		{
			auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 85 C0 74 24 D9 EE");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);
		}

		// Running from Salazar's statue
		{
			// Mashing
			auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 85 C0 74 1D 8B 7D 14");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), KeyTrgCheck_hook);

			// Quick reaction
			{
				// Key.leftTrigger_A
				auto pattern = hook::pattern("80 3D ? ? ? ? ? 0F 84 ? ? ? ? A1 ? ? ? ? 81 A0");
				ptrleftTrigger_A = *pattern.count(1).get(0).get<uint32_t*>(2);
				struct StatueRunTrigger_A
				{
					void operator()(injector::reg_pack& regs)
					{
						int leftTrigger_A = *(int8_t*)(ptrleftTrigger_A);

						if (!pConfig->bDisableQTE)
						{
							// Mimic what CMP does, since we're overwriting it.
							if (leftTrigger_A > 0)
							{
								// Clear both flags
								regs.ef &= ~(1 << regs.zero_flag);
								regs.ef &= ~(1 << regs.carry_flag);
							}
							else if (leftTrigger_A < 0)
							{
								// ZF = 0, CF = 1
								regs.ef &= ~(1 << regs.zero_flag);
								regs.ef |= (1 << regs.carry_flag);
							}
							else if (leftTrigger_A == 0)
							{
								// ZF = 1, CF = 0
								regs.ef |= (1 << regs.zero_flag);
								regs.ef &= ~(1 << regs.carry_flag);
							}
						}
						else
						{
							// Clear both flags
							regs.ef &= ~(1 << regs.zero_flag);
							regs.ef &= ~(1 << regs.carry_flag);
						}

					}
				}; injector::MakeInline<StatueRunTrigger_A>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
				
				// Key.rightTrigger_B
				pattern = hook::pattern("80 3D ? ? ? ? 00 EB ? 6A 0E 6A ? 6A ? 6A ? 6A ? 6A");
				ptrrightTrigger_B = *pattern.count(1).get(0).get<uint32_t*>(2);
				struct StatueRunTrigger_B
				{
					void operator()(injector::reg_pack& regs)
					{
						int rightTrigger_B = *(int8_t*)(ptrrightTrigger_B);

						if (!pConfig->bDisableQTE)
						{
							// Mimic what CMP does, since we're overwriting it.
							if (rightTrigger_B > 0)
							{
								// Clear both flags
								regs.ef &= ~(1 << regs.zero_flag);
								regs.ef &= ~(1 << regs.carry_flag);
							}
							else if (rightTrigger_B < 0)
							{
								// ZF = 0, CF = 1
								regs.ef &= ~(1 << regs.zero_flag);
								regs.ef |= (1 << regs.carry_flag);
							}
							else if (rightTrigger_B == 0)
							{
								// ZF = 1, CF = 0
								regs.ef |= (1 << regs.zero_flag);
								regs.ef &= ~(1 << regs.carry_flag);
							}
						}
						else
						{
							// Clear both flags
							regs.ef &= ~(1 << regs.zero_flag);
							regs.ef &= ~(1 << regs.carry_flag);
						}

					}
				}; injector::MakeInline<StatueRunTrigger_B>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
				
				// Both keys
				pattern = hook::pattern("E8 ? ? ? ? 85 C0 EB ? 6A ? 6A ? 6A ? 6A ? 6A");
				ReadCall(pattern.count(1).get(0).get<uint32_t>(0), sub_859810_orig);
				InjectHook(pattern.count(1).get(0).get<uint32_t>(0), sub_859810_hook);
			
				pattern = hook::pattern("E8 ? ? ? ? 85 C0 0F 84 ? ? ? ? 6A ? 6A ? 6A ? 8D 8E");
				InjectHook(pattern.count(1).get(0).get<uint32_t>(0), sub_859810_hook);
			}
		}

		// Climbing up after running from Salazar's statue
		{
			auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 ? 85 C0 74 ? A1 ? ? ? ? ? ? ? ? ? ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? B9");
			InjectHook(pattern.count(4).get(2).get<uint32_t>(0), KeyTrgCheck_hook);
			InjectHook(pattern.count(4).get(3).get<uint32_t>(0), KeyTrgCheck_hook);
		}

		// Last QTE from the Krauser knife fight
		{
			auto pattern = hook::pattern("83 FF ? 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 85 C0");
			InjectHook(pattern.count(1).get(0).get<uint32_t>(10), KeyTrgCheck_hook);
		}

		// Lasers after Krauser
		{
			// Key.leftTrigger_A
			auto pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? 66 C7 05 ? ? ? ? ? ? A1 ? ? ? ? 81 88 ? ? ? ? ? ? ? ? 68");
			struct LaserTrigger_A
			{
				void operator()(injector::reg_pack& regs)
				{
					int leftTrigger_A = *(int8_t*)(ptrleftTrigger_A);

					if (!pConfig->bDisableQTE)
					{
						// Mimic what CMP does, since we're overwriting it.
						if (leftTrigger_A > 0)
						{
							// Clear both flags
							regs.ef &= ~(1 << regs.zero_flag);
							regs.ef &= ~(1 << regs.carry_flag);
						}
						else if (leftTrigger_A < 0)
						{
							// ZF = 0, CF = 1
							regs.ef &= ~(1 << regs.zero_flag);
							regs.ef |= (1 << regs.carry_flag);
						}
						else if (leftTrigger_A == 0)
						{
							// ZF = 1, CF = 0
							regs.ef |= (1 << regs.zero_flag);
							regs.ef &= ~(1 << regs.carry_flag);
						}
					}
					else
					{
						// Clear both flags
						regs.ef &= ~(1 << regs.zero_flag);
						regs.ef &= ~(1 << regs.carry_flag);
					}

				}
			}; injector::MakeInline<LaserTrigger_A>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

			pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? 66 C7 05 ? ? ? ? ? ? A1 ? ? ? ? 81 88 ? ? ? ? ? ? ? ? 8B C7");
			injector::MakeInline<LaserTrigger_A>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

			// Key.rightTrigger_B
			pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? ? ? ? ? ? ? ? 74 ? 66 C7 05 ? ? ? ? ? ? A1 ? ? ? ? 81 88 ? ? ? ? ? ? ? ? 68");
			struct LaserTrigger_B
			{
				void operator()(injector::reg_pack& regs)
				{
					int rightTrigger_B = *(int8_t*)(ptrrightTrigger_B);

					if (!pConfig->bDisableQTE)
					{
						// Mimic what CMP does, since we're overwriting it.
						if (rightTrigger_B > 0)
						{
							// Clear both flags
							regs.ef &= ~(1 << regs.zero_flag);
							regs.ef &= ~(1 << regs.carry_flag);
						}
						else if (rightTrigger_B < 0)
						{
							// ZF = 0, CF = 1
							regs.ef &= ~(1 << regs.zero_flag);
							regs.ef |= (1 << regs.carry_flag);
						}
						else if (rightTrigger_B == 0)
						{
							// ZF = 1, CF = 0
							regs.ef |= (1 << regs.zero_flag);
							regs.ef &= ~(1 << regs.carry_flag);
						}
					}
					else
					{
						// Clear both flags
						regs.ef &= ~(1 << regs.zero_flag);
						regs.ef &= ~(1 << regs.carry_flag);
					}

				}
			}; injector::MakeInline<LaserTrigger_B>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
			
			pattern = hook::pattern("80 3D ? ? ? ? ? 74 ? ? ? ? ? ? ? ? 74 ? 66 C7 05 ? ? ? ? ? ? A1 ? ? ? ? 81 88 ? ? ? ? ? ? ? ? 8B C7");
			injector::MakeInline<LaserTrigger_B>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));
		
			// Last QTE
			pattern = hook::pattern("E8 ? ? ? ? 85 C0 74 ? FE 86 ? ? ? ? 5F 5E");
			ReadCall(pattern.count(1).get(0).get<uint32_t>(0), sub_8064B0_orig);
			InjectHook(pattern.count(1).get(0).get<uint32_t>(0), sub_8064B0_hook);
		}
	}

	// Fix QTE mashing speed issues
	// Each one of these uses a different way to calculate how fast you're pressing the QTE key.
	{
		// Running from boulders
		{
			auto pattern = hook::pattern("D9 87 ? ? ? ? D8 87 ? ? ? ? D9 9F ? ? ? ? D9 EE");
			struct BouldersQTE
			{
				void operator()(injector::reg_pack& regs)
				{
					float mashSpeed = *(float*)(regs.edi + 0x418);

					if (pConfig->bFixQTE && !(pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE))
						mashSpeed /= GlobalPtr()->deltaTime_70;

					_asm {fld mashSpeed}
				}
			}; injector::MakeInline<BouldersQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}

		// Knifing El Gigante's parasite
		{
			// First
			auto pattern = hook::pattern("FF 86 ? ? ? ? 0F B6 96 ? ? ? ? 8B 04 95 ? ? ? ? 53");
			struct ElGiganteParasite1
			{
				void operator()(injector::reg_pack& regs)
				{
					if (pConfig->bFixQTE && !(pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE))
						*(int32_t*)(regs.esi + 0x414) += (int32_t)(1 / GlobalPtr()->deltaTime_70);
					else
						*(int32_t*)(regs.esi + 0x414) += 1;
				}
			};
			injector::MakeInline<ElGiganteParasite1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
			
			// Second
			pattern = hook::pattern("FF 86 ? ? ? ? 57 56 E8 ? ? ? ? 83 C4 ? 85 C0 0F 84");
			struct ElGiganteParasite2
			{
				void operator()(injector::reg_pack& regs)
				{
					if (pConfig->bFixQTE && !(pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE))
						*(int32_t*)(regs.esi + 0x428) += (int32_t)(1 / GlobalPtr()->deltaTime_70);
					else
						*(int32_t*)(regs.esi + 0x428) += 1;
				}
			};
			injector::MakeInline<ElGiganteParasite2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}

		// Climbing up after the minecart ride
		{
			auto pattern = hook::pattern("D9 86 ? ? ? ? 8B 0D ? ? ? ? D8 61 ? D9 9E ? ? ? ? 6A");
			struct MinecartQTE
			{
				void operator()(injector::reg_pack& regs)
				{
					float mashSpeed = *(float*)(regs.esi + 0x40C);

					if (pConfig->bFixQTE || pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE)
						mashSpeed *= GlobalPtr()->deltaTime_70;

					_asm {fld mashSpeed}
				}
			}; injector::MakeInline<MinecartQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}

		// Running from Salazar's statue
		{
			auto pattern = hook::pattern("8B 7D ? D9 07 E8 ? ? ? ? 0F AF 5D 24 D9 EE 01 06");
			struct StatueRunQTE
			{
				void operator()(injector::reg_pack& regs)
				{
					regs.edi = *(int32_t*)(regs.ebp + 0x14);

					float mashSpeed = *(float*)(regs.edi);

					if (pConfig->bFixQTE || pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE)
						mashSpeed /= GlobalPtr()->deltaTime_70;

					regs.eax = (int32_t)mashSpeed;
				}
			}; injector::MakeInline<StatueRunQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(10));
		}

		// Climbing up after running from Salazar's statue
		{
			auto pattern = hook::pattern("FF 80 ? ? ? ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? 6A ? B9");
			struct StatueClimbQTE
			{
				void operator()(injector::reg_pack& regs)
				{
					if (pConfig->bFixQTE || pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE)
						*(int32_t*)(regs.eax + 0x168) += (int32_t)(1 / GlobalPtr()->deltaTime_70);
					else
						*(int32_t*)(regs.eax + 0x168) += 1;
				}
			};
			injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(0).get<uint32_t>(0), pattern.count(2).get(0).get<uint32_t>(6));
			injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));
		}

		// Last QTE from the Krauser knife fight
		{
			auto pattern = hook::pattern("A1 ? ? ? ? 74 ? FF 00 39 30 0F 8C ? ? ? ? A1 ? ? ? ? 81 A0");
			ptrKrauserQTEMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
			struct KrauserQTE
			{
				void operator()(injector::reg_pack& regs)
				{
					bool pressed = (bool)regs.eax;

					regs.eax = *(int32_t*)ptrKrauserQTEMovAddr;

					if (pressed)
					{
						con.AddLogInt(*(int32_t*)(regs.eax));
						if (pConfig->bFixQTE || pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE)
							*(int32_t*)(regs.eax) += (int32_t)(1 / GlobalPtr()->deltaTime_70);
						else
							*(int32_t*)(regs.eax) += 1;
						con.AddLogInt(*(int32_t*)(regs.eax));
					}
				}
			}; injector::MakeInline<KrauserQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

			pattern = hook::pattern("FF 00 39 30 0F 8C ? ? ? ? A1 ? ? ? ? 81 A0");
			injector::MakeNOP(pattern.count(1).get(0).get<uint32_t>(0), 2, true);
		}

		if (pConfig->bFixQTE && !(pConfig->bDisableQTE || pConfig->bAutomaticMashingQTE))
			Logging::Log() << __FUNCTION__ << " -> QTE speed changes applied";
	}

	// QTE bindings and icons
	{
		// KEY_1 binding hook
		{
			auto pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
			struct QTEkey1
			{
				void operator()(injector::reg_pack& regs)
				{
					regs.ebx = pInput->KeyMap_getDIK(pConfig->sQTE_key_1);
					regs.eax = *(int32_t*)(regs.eax + 0x1C);
				}
			}; injector::MakeInline<QTEkey1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}

		// KEY_2 binding hook
		{
			auto pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
			struct QTEkey2
			{
				void operator()(injector::reg_pack& regs)
				{
					regs.edx = pInput->KeyMap_getDIK(pConfig->sQTE_key_2);
					regs.eax = *(int32_t*)(regs.eax + 0x1C);
				}
			}; injector::MakeInline<QTEkey2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
		}

		// KEY_1 icon prompts
		auto pattern = hook::pattern("8D ? ? ? ? ? 6A 2D ? E8");
		ReadCall(pattern.count(3).get(0).get<uint32_t>(9), sub_41E600_orig);
		InjectHook(pattern.count(3).get(0).get<uint32_t>(9), KEY1prompt_hook);
		InjectHook(pattern.count(3).get(1).get<uint32_t>(9), KEY1prompt_hook);
		InjectHook(pattern.count(3).get(2).get<uint32_t>(9), KEY1prompt_hook);

		// KEY_2 icon prompts
		pattern = hook::pattern("8D ? ? ? ? ? 6A 2E ? E8");
		InjectHook(pattern.count(4).get(0).get<uint32_t>(9), KEY2prompt_hook);
		InjectHook(pattern.count(4).get(1).get<uint32_t>(9), KEY2prompt_hook);
		InjectHook(pattern.count(4).get(2).get<uint32_t>(9), KEY2prompt_hook);
		InjectHook(pattern.count(4).get(3).get<uint32_t>(9), KEY2prompt_hook);

		Logging::Log() << __FUNCTION__ << " -> Keyboard QTE bindings applied";
	}
}
