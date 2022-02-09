#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "Settings.h"

void Init_HandleLimits()
{
	// vertex buffers
	if (cfg.bRaiseVertexAlloc)
	{
		auto pattern = hook::pattern("68 80 1A 06 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? 46");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68000C3500, true);  // 400000 -> 800000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68000C3500, true);

		pattern = hook::pattern("68 00 09 3D 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? A1 ? ? ? ? 8B 08");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68007A1200, true);  // 4000000 -> 8000000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68007A1200, true);

		pattern = hook::pattern("68 80 84 1E 00 50 8B 41 ? FF D0 85 C0 0F 85 ? ? ? ? A1");
		injector::WriteMemory<int>(pattern.count(2).get(0).get<uint32_t>(1), 0x68003D0900, true);  // 2000000 -> 4000000
		injector::WriteMemory<int>(pattern.count(2).get(1).get<uint32_t>(1), 0x68003D0900, true);

		pattern = hook::pattern("68 80 8D 5B 00 50 8B 41 ? FF D0 85 C0 0F 85");
		injector::WriteMemory<int>(pattern.count(4).get(0).get<uint32_t>(1), 0x6800B71B00, true);  // 6000000 -> 12000000
		injector::WriteMemory<int>(pattern.count(4).get(1).get<uint32_t>(1), 0x6800B71B00, true);
		injector::WriteMemory<int>(pattern.count(4).get(2).get<uint32_t>(1), 0x6800B71B00, true);
		injector::WriteMemory<int>(pattern.count(4).get(3).get<uint32_t>(1), 0x6800B71B00, true);

		// Make it so models don't disappear when a certain polygon limit is reached
		pattern = hook::pattern("81 80 ? ? ? ? ? ? ? ? A1 ? ? ? ? 8B 90 ? ? ? ? 03 D2");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(6), 0x00340000, true);
	}

	// Inventory screen mem
	if (cfg.bRaiseInventoryAlloc)
	{
		const int SSOldSize = 0x34AC00;
		const int SSNewSize = 0x2000000;
		const int WepDataSize = 0x400000; // space for loading ss wep models
		const int PzzlNewSize = SSNewSize - WepDataSize;

		const int Mem1OrigSize = 0x1D00000;
		const int Mem2OrigSize = 0x3700000;

		// (SubScreenExec) Not sure what this is, changes offset into SS memory depending on 0x20 flag
		auto pattern = hook::pattern("05 ? ? ? ? F6 C1 ? 8B 0D ? ? ? ? A3 ? ? ? ? 8D 91 ? ? ? ? 75 ? 8D 91 ? ? ? ? 52 03 C1");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(1), 0x00600000, true); // 0x007681d7

		// (SubScreenExec) Change offset to pzzl data inside mem2 to end of original mem2 space
		// and change size of pzzl data to new size
		pattern = hook::pattern("68 ? ? ? ? 81 c1 ? ? ? ? 51 50");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(1), PzzlNewSize, true); // 0x007680fc
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(7), Mem2OrigSize, true); // 0x00768102

		// (SubScreenExec) update pzzl data end size (used for ss-wep.dat load?), and skip pointless check
		pattern = hook::pattern("8d 91 ? ? ? ? 75 ? 8d 91 ? ? ? ? 52 03 c1");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(2), PzzlNewSize, true); // 0x007681eb
		injector::WriteMemory<uint8_t>(pattern.count(1).get(0).get<uint32_t>(6), uint8_t(0xEB), true); // 0x007681ef

		// (SubScreenAramRead) Change offset to pzzl data inside mem2 to end of original mem2 space
		pattern = hook::pattern("05 ? ? ? ? 50 6a ? 68 ? ? ? ? c7 05 ? ? ? ? ? ? ? ? e8 ? ? ? ? 8b 0d");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(1), Mem2OrigSize, true); // 0x00768ded
		pattern = hook::pattern("8d 94 ? ? ? ? ? 52 6a ? 68 ? ? ? ? e8 ? ? ? ? 8b 0d");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(3), Mem2OrigSize, true); // 0x00768e78 - ss_cmmn.dat load
		pattern = hook::pattern("8d 8c ? ? ? ? ? 51 6a ? 68 ? ? ? ? e8 ? ? ? ? 8b 15");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(3), Mem2OrigSize, true); // 0x00768fb2 - ss_pzzl.dat load

		// (SubScreenExitCore) Change offset to pzzl data inside mem2...
		pattern = hook::pattern("81 c2 ? ? ? ? 52 50 e8 ? ? ? ? 6a");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(2), Mem2OrigSize, true); // 0x0076938b

		// (SubScreenExit) Change offset to pzzl data inside mem2...
		pattern = hook::pattern("8d 94 08 ? ? ? ? 52 53");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(3), Mem2OrigSize, true); // 0x007695b3

		// (SubScreenTask) Change offset to ss_wep data inside mem2 (end of pzzl data)
		pattern = hook::pattern("81 c2 ? ? ? ? 89 15 ? ? ? ? 80 b8");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(2), PzzlNewSize, true); // 0x00774e20 - ss-wep.dat load?

		// (MemAlloc) Increase mem2 size to include space for our SSNewSize
		pattern = hook::pattern("68 ? ? ? ? 89 35 ? ? ? ? e8 ? ? ? ? 68");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(1), Mem2OrigSize + SSNewSize, true); // 0x0097485f

		// (MemAlloc) Increase mem1 area size
		pattern = hook::pattern("68 00 00 D0 01");
		injector::WriteMemory<int>(pattern.count(3).get(0).get<uint32_t>(1), Mem1OrigSize + SSNewSize - SSOldSize, true); // 0x0097483a
		injector::WriteMemory<int>(pattern.count(3).get(1).get<uint32_t>(1), Mem1OrigSize + SSNewSize - SSOldSize, true); // 0x0097484e
		injector::WriteMemory<int>(pattern.count(3).get(2).get<uint32_t>(1), Mem1OrigSize + SSNewSize - SSOldSize, true); // 0x0097486f

		// (SystemMemInit) Increase heap 0 end ptr offset 
		pattern = hook::pattern("8D 8E 00 00 D0 01");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(2), Mem1OrigSize + SSNewSize - SSOldSize, true); // 0x006562cd
		
		// (MessageControl::stageInit) Increase size given to mem_alloc for pG+0x3C buffer 
		pattern = hook::pattern("6A 0D 6A 01 6A 00 6A 00 68 00 AC 34 00");
		injector::WriteMemory<int>(pattern.count(1).get(0).get<uint32_t>(9), SSNewSize, true); // 0x00719817
	}
}