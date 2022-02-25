#include <iostream>
#include "stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "ConsoleWnd.h"
#include "Logging/Logging.h"

uint32_t* ptrKEY_1_icon1;
uint32_t* ptrKEY_1_icon2;
uint32_t* ptrKEY_1_icon3;
uint32_t* ptrKEY_2_icon1;
uint32_t* ptrKEY_2_icon2;
uint32_t* ptrKEY_2_icon3;
uint32_t* ptrKEY_2_icon4;

uintptr_t jmpAddrKEY_1_icon1;
uintptr_t jmpAddrKEY_1_icon2;
uintptr_t jmpAddrKEY_1_icon3;
uintptr_t jmpAddrKEY_2_icon1;
uintptr_t jmpAddrKEY_2_icon2;
uintptr_t jmpAddrKEY_2_icon3;
uintptr_t jmpAddrKEY_2_icon4;

int intQTE_key_1;
int intQTE_key_2;

float fQTESpeedMult = 1.5f;

// QTE Key 1 icons
void __declspec(naked) KEY_1_icon1()
{
	intQTE_key_1 = cfg.KeyMap(cfg.sQTE_key_1.data(), false);
	_asm
	{
		lea edx, [ebp - 0x3B4]
		push intQTE_key_1
		jmp jmpAddrKEY_1_icon1
	}
}

void __declspec(naked) KEY_1_icon2()
{
	intQTE_key_1 = cfg.KeyMap(cfg.sQTE_key_1.data(), false);
	_asm
	{
		lea ecx, [ebp - 0x4CC]
		push intQTE_key_1
		jmp jmpAddrKEY_1_icon2
	}
}

void __declspec(naked) KEY_1_icon3()
{
	intQTE_key_1 = cfg.KeyMap(cfg.sQTE_key_1.data(), false);
	_asm
	{
		lea ecx, [ebp - 0x280]
		push intQTE_key_1
		jmp jmpAddrKEY_1_icon3
	}
}

// QTE Key 2 icos

void __declspec(naked) KEY_2_icon1()
{
	intQTE_key_2 = cfg.KeyMap(cfg.sQTE_key_2.data(), false);
	_asm
	{
		lea ecx, [ebp - 0x37C]
		push intQTE_key_2
		jmp jmpAddrKEY_2_icon1
	}
}

void __declspec(naked) KEY_2_icon2()
{
	intQTE_key_2 = cfg.KeyMap(cfg.sQTE_key_2.data(), false);
	_asm
	{
		lea ecx, [ebp - 0x76C]
		push intQTE_key_2
		jmp jmpAddrKEY_2_icon2
	}
}

void __declspec(naked) KEY_2_icon3()
{
	intQTE_key_2 = cfg.KeyMap(cfg.sQTE_key_2.data(), false);
	_asm
	{
		lea edx, [ebp - 0x494]
		push intQTE_key_2
		jmp jmpAddrKEY_2_icon3
	}
}

void __declspec(naked) KEY_2_icon4()
{
	intQTE_key_2 = cfg.KeyMap(cfg.sQTE_key_2.data(), false);
	_asm
	{
		lea ecx, [ebp - 0x2F0]
		push intQTE_key_2
		jmp jmpAddrKEY_2_icon4
	}
}

void Init_QTEfixes()
{
	// Fix QTE mashing speed issues
	// Each one of these uses a different way to calculate how fast you're pressing the QTE key.
	//
	// Running from boulders
	auto pattern = hook::pattern("D9 87 ? ? ? ? D8 87 ? ? ? ? D9 9F ? ? ? ? D9 EE D9 9F ? ? ? ? D9 05 ? ? ? ? D8 97 ? ? ? ? DF E0 F6 C4 ? 7A 20");
	struct BouldersQTE
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaSpeed = *(float*)(regs.edi + 0x418);
			_asm {fld  vanillaSpeed}
			if (cfg.bFixQTE)
				_asm {fmul fQTESpeedMult}
		}
	}; injector::MakeInline<BouldersQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Climbing up after the minecart ride
	pattern = hook::pattern("D9 86 ? ? ? ? 8B 0D ? ? ? ? D8 61 ? D9 9E ? ? ? ? 6A ? 56 E8 ? ? ? ? D9 EE");
	struct MinecartQTE
	{
		void operator()(injector::reg_pack& regs)
		{
			float vanillaSpeed = *(float*)(regs.esi + 0x40C);
			_asm {fld  vanillaSpeed}
			if (cfg.bFixQTE)
				_asm {fdiv fQTESpeedMult}
		}
	}; injector::MakeInline<MinecartQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Running from Salazar's statue
	pattern = hook::pattern("8B 7D 14 D9 07 E8 ? ? ? ? 0F AF 5D 24 D9 EE 01 06 8D 43 FF D9 1F 39 06");
	struct StatueRunQTE
	{
		void operator()(injector::reg_pack& regs)
		{
			regs.edi = *(int32_t*)(regs.ebp + 0x14);
			float vanillaSpeed = *(float*)(regs.edi);
			_asm {fld  vanillaSpeed}
			if (cfg.bFixQTE)
				_asm {fmul fQTESpeedMult}
		}
	}; injector::MakeInline<StatueRunQTE>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

	// Climbing up after running from Salazar's statue
	pattern = hook::pattern("FF 80 ? ? ? ? 6A 00 6A 00 6A 02 6A 02 6A 02 6A 00 6A 00 6A 05 6A 19");
	struct StatueClimbQTE
	{
		void operator()(injector::reg_pack& regs)
		{
			if (cfg.bFixQTE)
				*(int32_t*)(regs.eax + 0x168) += 3;
			else
				*(int32_t*)(regs.eax + 0x168) += 1;
		}
	};
	injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(0).get<uint32_t>(0), pattern.count(2).get(0).get<uint32_t>(6));
	injector::MakeInline<StatueClimbQTE>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(6));

	Logging::Log() << __FUNCTION__ << " -> QTE speed changes applied";

	// QTE bindings and icons
	//
	// KEY_1 binding hook
	pattern = hook::pattern("8B 58 ? 8B 40 ? 8D 8D ? ? ? ? 51");
	struct QTEkey1
	{
		void operator()(injector::reg_pack& regs)
		{
			intQTE_key_1 = cfg.KeyMap(cfg.sQTE_key_1.data(), false);
			regs.ebx = intQTE_key_1;
			regs.eax = *(int32_t*)(regs.eax + 0x1C);
		}
	}; injector::MakeInline<QTEkey1>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// KEY_2 binding hook
	pattern = hook::pattern("8B 50 ? 8B 40 ? 8B F3 0B F1 74 ?");
	struct QTEkey2
	{
		void operator()(injector::reg_pack& regs)
		{
			intQTE_key_2 = cfg.KeyMap(cfg.sQTE_key_2.data(), false);
			regs.edx = intQTE_key_2;
			regs.eax = *(int32_t*)(regs.eax + 0x1C);
		}
	}; injector::MakeInline<QTEkey2>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// KEY_1 icon prompts
	pattern = hook::pattern("8D ? ? ? ? ? 6A 2D ? E8");
	ptrKEY_1_icon1 = pattern.count(3).get(0).get<uint32_t>(0);
	ptrKEY_1_icon2 = pattern.count(3).get(1).get<uint32_t>(0);
	ptrKEY_1_icon3 = pattern.count(3).get(2).get<uint32_t>(0);

	injector::MakeNOP(ptrKEY_1_icon1, 8, true);
	injector::MakeJMP(ptrKEY_1_icon1, KEY_1_icon1, true);
	jmpAddrKEY_1_icon1 = (uintptr_t)pattern.count(3).get(0).get<uint32_t>(6);

	injector::MakeNOP(ptrKEY_1_icon2, 8, true);
	injector::MakeJMP(ptrKEY_1_icon2, KEY_1_icon2, true);
	jmpAddrKEY_1_icon2 = (uintptr_t)pattern.count(3).get(1).get<uint32_t>(6);

	injector::MakeNOP(ptrKEY_1_icon3, 8, true);
	injector::MakeJMP(ptrKEY_1_icon3, KEY_1_icon3, true);
	jmpAddrKEY_1_icon3 = (uintptr_t)pattern.count(3).get(2).get<uint32_t>(6);

	// KEY_2 icon prompts
	pattern = hook::pattern("8D ? ? ? ? ? 6A 2E ? E8");
	ptrKEY_2_icon1 = pattern.count(4).get(0).get<uint32_t>(0);
	ptrKEY_2_icon2 = pattern.count(4).get(1).get<uint32_t>(0);
	ptrKEY_2_icon3 = pattern.count(4).get(2).get<uint32_t>(0);
	ptrKEY_2_icon4 = pattern.count(4).get(3).get<uint32_t>(0);

	injector::MakeNOP(ptrKEY_2_icon1, 8, true);
	injector::MakeJMP(ptrKEY_2_icon1, KEY_2_icon1, true);
	jmpAddrKEY_2_icon1 = (uintptr_t)pattern.count(4).get(0).get<uint32_t>(6);

	injector::MakeNOP(ptrKEY_2_icon2, 8, true);
	injector::MakeJMP(ptrKEY_2_icon2, KEY_2_icon2, true);
	jmpAddrKEY_2_icon2 = (uintptr_t)pattern.count(4).get(1).get<uint32_t>(6);

	injector::MakeNOP(ptrKEY_2_icon3, 8, true);
	injector::MakeJMP(ptrKEY_2_icon3, KEY_2_icon3, true);
	jmpAddrKEY_2_icon3 = (uintptr_t)pattern.count(4).get(2).get<uint32_t>(6);

	injector::MakeNOP(ptrKEY_2_icon4, 8, true);
	injector::MakeJMP(ptrKEY_2_icon4, KEY_2_icon4, true);
	jmpAddrKEY_2_icon4 = (uintptr_t)pattern.count(4).get(3).get<uint32_t>(6);

	Logging::Log() << __FUNCTION__ << " -> Keyboard QTE bindings applied";
}
