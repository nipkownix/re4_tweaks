#include <iostream>
#include "..\includes\stdafx.h"
#include "dllmain.h"
#include "ConsoleWnd.h"

double fDefaultEngineWidthScale = 1280.0;
double fDefaultEngineAspectRatio = 1.777777791;
double fDefaultAspectRatio = 1.333333373;

double fNewInvItemNamePos;
double fNewFilesTitlePos;
double fNewFilesItemsPos;
double fNewMapIconsPos;
double fNewMerchItemListPos;
double fNewMerchItemDescPos;
double fNewMerchGreetingPos;
double fNewTItemNamesPos;
double fNewRadioNamesPos;

uintptr_t* ptrResMovAddr;

static uint32_t* ptrEngineWidthScale;
static uint32_t* ptrAspectRatio;

void Init_UltraWideFix()
{
	auto pattern = hook::pattern("DB 05 ? ? ? ? 85 ? 79 ? D8 05 ? ? ? ? DC 35 ? ? ? ? 8B");
	ptrEngineWidthScale = *pattern.count(1).get(0).get<uint32_t*>(18);

	pattern = hook::pattern("D9 05 ? ? ? ? C7 45 ? ? ? ? ? D9 5C 24 ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? C7 45 ? ? ? ? ? D9 81");
	ptrAspectRatio = *pattern.count(1).get(0).get<uint32_t*>(2);

	// Hook function that changes the resolution
	pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? 5E 5B 8B E5 5D C3");
	ptrResMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct ResChange
	{
		void operator()(injector::reg_pack& regs)
		{
			#ifdef VERBOSE
			con.AddConcatLog("ResX = ", regs.eax);
			con.AddConcatLog("ResY = ", regs.ecx);
			#endif

			double fGameWidth = regs.eax;
			double fGameHeight = regs.ecx;

			double fGameDisplayAspectRatio = fGameWidth / fGameHeight;

			double fNewEngineAspectRatio = (fGameDisplayAspectRatio / fDefaultEngineAspectRatio);
			double fNewAspectRatio = fGameDisplayAspectRatio / fDefaultAspectRatio;
			double fNewEngineWidthScale = fNewEngineAspectRatio * fDefaultEngineWidthScale;

			// if ultrawide
			if (fGameDisplayAspectRatio > 2.2)
			{
				#ifdef VERBOSE
				con.AddConcatLog("fNewEngineWidthScale = ", fNewEngineWidthScale);
				con.AddConcatLog("fNewAspectRatio = ", fNewAspectRatio);
				#endif

				fNewInvItemNamePos = 385;
				fNewFilesTitlePos = 370;
				fNewFilesItemsPos = 375;
				fNewMapIconsPos = 336;
				fNewMerchItemListPos = 333;
				fNewMerchItemDescPos = 395;
				fNewMerchGreetingPos = 380;
				fNewTItemNamesPos = 385;
				fNewRadioNamesPos = 380;

				injector::WriteMemory(ptrEngineWidthScale, static_cast<double>(fNewEngineWidthScale), true);
				injector::WriteMemory(ptrAspectRatio, static_cast<float>(fNewAspectRatio), true);
			}
			else {
				#ifdef VERBOSE
				con.AddLogChar("Wrote default aspect ratio values");
				#endif
				fNewInvItemNamePos = 320;
				fNewFilesTitlePos = 320;
				fNewFilesItemsPos = 320;
				fNewMapIconsPos = 320;
				fNewMerchItemListPos = 320;
				fNewMerchItemDescPos = 320;
				fNewMerchGreetingPos = 320;
				fNewTItemNamesPos = 320;
				fNewRadioNamesPos = 320;

				injector::WriteMemory(ptrEngineWidthScale, static_cast<double>(fDefaultEngineWidthScale), true);
				injector::WriteMemory(ptrAspectRatio, static_cast<float>(fDefaultAspectRatio), true);
			}
			*(int32_t*)(ptrResMovAddr) = regs.eax;
		}
	}; injector::MakeInline<ResChange>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(4));

	// Text offset fixes
	// Iventory item name
	pattern = hook::pattern("DC 05 ? ? ? ? DC 0D ? ? ? ? E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 89 45 ? DC 0D ? ? ? ? E8 ? ? ? ? 8B C8 0F BE 05 ? ? ? ? 99");
	injector::WriteMemory(pattern.get_first(2), &fNewInvItemNamePos, true);

	// "Files" title
	pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 0F B7 0D ? ? ? ? 0F B7 15 ? ? ? ? 51 52 6A 04 B9");
	injector::WriteMemory(pattern.get_first(2), &fNewFilesTitlePos, true);

	// "Files" item list
	pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 0F B7 0D ? ? ? ? 0F B7 15 ? ? ? ? 89 45 ? 8D 43 ? 51 0F B6 ? 52 56 B9");
	injector::WriteMemory(pattern.get_first(2), &fNewFilesItemsPos, true);

	// Map icons
	pattern = hook::pattern("DD 05 ? ? ? ? DC C9 D9 C9 D8 6D ? DC 05 ? ? ? ? D9");
	injector::WriteMemory(pattern.get_first(15), &fNewMapIconsPos, true);

	// Merchant's greeting
	pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B6 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 0F B6 7D ? 8B 75 ? C7 43 28 ? ? ? ? E8");
	injector::WriteMemory(pattern.get_first(2), &fNewMerchGreetingPos, true);

	// Merchant's item "buy" list
	pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B9 ? ? ? ? 66 89 8E ? ? ? ? 8B 4D ? BA ? ? ? ? 66 89 ? ? ? ? ? 0F B7 ? 8B 89 ? ? ? ? 50 E8");
	injector::WriteMemory(pattern.get_first(2), &fNewMerchItemListPos, true);

	// Merchant's item "sell" list
	pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 ? ? 50 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B9 ? ? ? ? 66 89 ? ? ? ? ? 8D 83 ? ? ? ? 0F B6 ? 6A ?");
	injector::WriteMemory(pattern.get_first(2), &fNewMerchItemListPos, true);

	// Merchant's item "tune up" list
	pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 0F B7 ? ? 50 52 B9 ? ? ? ? E8 ? ? ? ? C1 E6 ? B8 ? ? ? ? 66 89 86 ? ? ? ? 8B 45 A4 B9 ? ? ? ? 66 89 8E ? ? ? ? 0F B7");
	injector::WriteMemory(pattern.get_first(2), &fNewMerchItemListPos, true);

	// Merchant's item description
	pattern = hook::pattern("DC 05 ? ? ? ? 50 DE C9 E8 ? ? ? ? 50 0F B7 45 ? 50 B9 ? ? ? ? E8 ? ? ? ? 5E 5D C3");
	injector::WriteMemory(pattern.get_first(2), &fNewMerchItemDescPos, true);

	// Tresure name
	pattern = hook::pattern("DC 05 ? ? ? ? DC 0D ? ? ? ? E8 ? ? ? ? D9 83 ? ? ? ? DC 2D ? ? ? ? 89 45 ? DC 0D ? ? ? ? E8 ? ? ? ? 8B C8 0F BE 05");
	injector::WriteMemory(pattern.get_first(2), &fNewTItemNamesPos, true);

	// Radio names
	pattern = hook::pattern("DC 05 ? ? ? ? DD 05 ? ? ? ? DC C9 D9 C9 E8 ? ? ? ? D9 86 ? ? ? ? DC 2D ? ? ? ? 8B F8 DE C9 E8 ? ? ? ? 8A 1D ? ? ? ?");
	injector::WriteMemory(pattern.get_first(2), &fNewRadioNamesPos, true);
}