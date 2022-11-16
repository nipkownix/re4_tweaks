#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"

bool bIsUltrawide;
bool bIs16by10;

float fDefaultEngineWidthScale = 1280.0f;
float fDefaultEngineAspectRatio = 1.777777791f;
float fDefaultAspectRatio = 1.333333373f;
float fDefaultEsp18Height = 0.375f;
float fDefaultHudPosX = 0.0f; // 217.0f in the vanilla game, but since mods can change it, we'll just store whatever the game gives us.

double fDefaultMapIconsPos = 320.0;
double fNewMapIconsPos;

float fGameWidth;
float fGameHeight;
float fGameDisplayAspectRatio;

void __cdecl C_MTXOrtho_messageTrans_hook(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far)
{
	if (bIsUltrawide || bIs16by10)
	{
		// I think this should be 1.777777791f, but 1.666667f looks better for some reason?
		float fNewAspectRatio = (fGameDisplayAspectRatio / 1.666667f);

		float fUIPosOffset = (((480.0f * fNewAspectRatio) - 512.0f) / 2.0f);

		PosX += fUIPosOffset;
		NegX -= fUIPosOffset;
	}

	return game_C_MTXOrtho(mtx, PosY, NegY, NegX, PosX, Near, Far);
}

void __cdecl C_MTXOrtho_sub_1484C50_hook(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far)
{
	if (bIsUltrawide || bIs16by10)
	{
		float fUIPosOffset = (fGameWidth - fGameHeight * (16.0f / 9.0f)) / 2.0f;

		PosX += fUIPosOffset;
		NegX -= fUIPosOffset;
	}

	return game_C_MTXOrtho(mtx, PosY, NegY, NegX, PosX, Near, Far);
}

void __cdecl C_MTXOrtho_DrawTexture_hook(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far)
{
	if (bIsUltrawide || bIs16by10)
	{
		// Game's default behavior is stretching, so we simply return
		if (re4t::cfg->bStretchFullscreenImages)
			return game_C_MTXOrtho(mtx, PosY, NegY, NegX, PosX, Near, Far);

		float fNewAspectRatio = (fGameDisplayAspectRatio / 1.666667f);

		float fUIPosOffset = (((480.0f * fNewAspectRatio) - 512.0f) / 2.0f);

		PosX += fUIPosOffset;
		NegX -= fUIPosOffset;
	}

	return game_C_MTXOrtho(mtx, PosY, NegY, NegX, PosX, Near, Far);
}

void __cdecl C_MTXOrtho_cSofdec_hook(Mtx44 mtx, float PosY, float NegY, float NegX, float PosX, float Near, float Far)
{
	if (bIsUltrawide || bIs16by10)
	{
		// Game's default behavior is stretching, so we simply return
		if (re4t::cfg->bStretchVideos)
			return game_C_MTXOrtho(mtx, PosY, NegY, NegX, PosX, Near, Far);

		float fNewAspectRatio = (fGameDisplayAspectRatio / 1.666667f);

		float fUIPosOffset = (((480.0f * fNewAspectRatio) - 512.0f) / 2.0f);

		PosX += fUIPosOffset;
		NegX -= fUIPosOffset;
	}

	return game_C_MTXOrtho(mtx, PosY, NegY, NegX, PosX, Near, Far);
}

void re4t::init::AspectRatioTweaks()
{
	// Get original C_MTXOrtho
	auto pattern = hook::pattern("E8 ? ? ? ? 8D 4D BC 6A 01 51 E8 ? ? ? ? 8D 55 98 52 8D 45 B0 50 8D 4D A4 51 56 E8 ? ? ? ? 8B 4D FC 83 C4 34");
	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), game_C_MTXOrtho);

	// messageTrans_messageCamera -> Used in most (all?) text drawn in the user interface
	pattern = hook::pattern("E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 E8 ? ? ? ? 8D 45 ? 6A ? 50 E8 ? ? ? ? 6A ? E8 ? ? ? ? 8B 4D");
	InjectHook(pattern.count(2).get(1).get<uint32_t>(0), C_MTXOrtho_messageTrans_hook);

	// sub_1146150 -> Only used the scale the mouse cursor area, afaik. Same logic as before, so lets just reuse that func
	InjectHook(pattern.count(2).get(0).get<uint32_t>(0), C_MTXOrtho_messageTrans_hook);

	// sub_1484C50 -> Used to scale some textures that are drawn in the user interface (main menu only?)
	pattern = hook::pattern("E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 56 E8 ? ? ? ? 8B 4D ? 83 C4");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), C_MTXOrtho_sub_1484C50_hook);

	// DrawTexture -> Used for fullscreen images, such as those that show up when you're reading Files
	pattern = hook::pattern("E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 95 ? ? ? ? 52");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), C_MTXOrtho_DrawTexture_hook);
	
	// cSofdec::setCamera -> Used for Sofdec videos
	pattern = hook::pattern("E8 ? ? ? ? 8D 4D ? 6A ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 56 E8 ? ? ? ? 8B 4D ? 33 CD");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), C_MTXOrtho_cSofdec_hook);

	// Hook LifeMeter::move so we can override the HUD pos
	pattern = hook::pattern("D9 98 ? ? ? ? 0F BE 46 ? 89 45 ? 6A ? B9 ? ? ? ? DB 45");
	struct LifeMeter__move_hook
	{
		void operator()(injector::reg_pack& regs)
		{
			// Code we replaced
			uint32_t orig_eax = regs.eax;
			__asm {
				mov eax, [orig_eax]
				fstp dword ptr[eax + 0x10C]
			}

			float* fCurHudPosX = (float*)(regs.eax - 0x1DCC);
			
			// Store default HUD Pos X
			if (fDefaultHudPosX == 0.0f)
				fDefaultHudPosX = *fCurHudPosX;

			// Calculate new X pos if needed
			if ((re4t::cfg->bSideAlignHUD && bIsUltrawide) || (re4t::cfg->bRemove16by10BlackBars && bIs16by10))
			{
				float fHudPosOffset = ((360.0f * fGameDisplayAspectRatio) - 640.0f) / 2.0f;
			
				*fCurHudPosX = fDefaultHudPosX + fHudPosOffset;
			}
			else
				*fCurHudPosX = fDefaultHudPosX;
		}
	}; injector::MakeInline<LifeMeter__move_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

	// Hook function that sets the BG color (bio4_AddBgColor), so we can make sure it is black when playing videos
	pattern = hook::pattern("F6 82 ? ? ? ? ? 75 ? B9 ? ? ? ? E8 ? ? ? ? 8B 48");
	struct AddBgColor_hook
	{
		void operator()(injector::reg_pack& regs)
		{
			// Game was originally checking for this flag
			bool origFlagSet = FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_SET_BG_COLOR));

			bool isPlayingSofdec = (FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_MOVIE_ON)) ||
				FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_MOVIE2_ON)));

			if ((origFlagSet || isPlayingSofdec))
				regs.ef &= ~(1 << regs.zero_flag);
			else
				regs.ef |= (1 << regs.zero_flag);
		}
	}; injector::MakeInline<AddBgColor_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));


	pattern = hook::pattern("DC 0D ? ? ? ? DE F9 D9 5D F8 8B 10 8B 92 ? ? ? ? FF D2 EB 22 8B 0D ? ? ? ? 8D 70 04");
	static uint32_t* ptrEngineWidthScale = *pattern.count(1).get(0).get<uint32_t*>(2);

	pattern = hook::pattern("D9 05 ? ? ? ? D9 5C 24 04 D9 80 ? ? ? ? D9 1C 24 E8 ? ? ? ? 8B CE E8 ? ? ? ? 8B 46 04");
	static uint32_t* ptrAspectRatio = *pattern.count(1).get(0).get<uint32_t*>(2);

	pattern = hook::pattern("DC 0D ? ? ? ? D9 9D ? ? ? ? D9 85 ? ? ? ? D9 5C 24 ? D9 5C 24");
	static uint32_t* ptrEsp18Height = *pattern.count(1).get(0).get<uint32_t*>(2);

	static uint32_t* ptrMapIconsPos = hook::pattern("DD 05 ? ? ? ? DC C9 D9 C9 D8 6D ? DC 05 ? ? ? ? D9").count(1).get(0).get<uint32_t>(15);

	// Hook D3D_SetupPresentationGlobals so we can calculate and write the new aspect ratio values
	pattern = hook::pattern("A3 ? ? ? ? 89 0D ? ? ? ? 5E 5B 8B E5 5D C3");
	static uintptr_t* ptrResMovAddr = *pattern.count(1).get(0).get<uint32_t*>(1);
	struct ResChange
	{
		void operator()(injector::reg_pack& regs)
		{
			spd::log()->info("Changing game resolution to: {0}x{1} {2}Hz", (int)regs.eax, (int)regs.ecx, (int)regs.esi);

			#ifdef VERBOSE
			con.AddConcatLog("ResX = ", (int)regs.eax);
			con.AddConcatLog("ResY = ", (int)regs.ecx);
			con.AddConcatLog("Refrash rate = ", (int)regs.esi);
			#endif

			fGameWidth = (float)regs.eax;
			fGameHeight = (float)regs.ecx;
			fGameDisplayAspectRatio = fGameWidth / fGameHeight;

			float fNewEngineAspectRatio = fGameDisplayAspectRatio / fDefaultEngineAspectRatio;
			float fNewAspectRatio = fGameDisplayAspectRatio / fDefaultAspectRatio;
			float fNewEngineWidthScale = fNewEngineAspectRatio * fDefaultEngineWidthScale;

			double fMapIconsPosOffset = 0.040 * ((double)fGameWidth - ((double)fDefaultEngineAspectRatio * (double)fGameHeight));
			fNewMapIconsPos = fDefaultMapIconsPos + fMapIconsPosOffset;

			double fNewEsp18Height = 0.50068 / fNewAspectRatio;

			// if ultrawide/super ultrawide or 16:10
			if (((fGameDisplayAspectRatio > 2.2f) && re4t::cfg->bUltraWideAspectSupport) ||
				((fGameDisplayAspectRatio == 1.6f) && re4t::cfg->bRemove16by10BlackBars))
			{
				bIsUltrawide = fGameDisplayAspectRatio > 2.2f;
				bIs16by10 = fGameDisplayAspectRatio == 1.6f;

				#ifdef VERBOSE
				con.AddConcatLog("fNewEngineWidthScale = ", fNewEngineWidthScale);
				con.AddConcatLog("fNewAspectRatio = ", fNewAspectRatio);
				con.AddConcatLog("fNewEsp18Height = ", fNewEsp18Height);
				con.AddConcatLog("fMapIconsPosOffset = ", fMapIconsPosOffset);
				con.AddConcatLog("fNewMapIconsPos = ", fNewMapIconsPos);
				#endif

				injector::WriteMemory(ptrMapIconsPos, &fNewMapIconsPos, true);
				injector::WriteMemory(ptrEsp18Height, static_cast<double>(-fNewEsp18Height), true);
				injector::WriteMemory(ptrEngineWidthScale, static_cast<double>(fNewEngineWidthScale), true);
				injector::WriteMemory(ptrAspectRatio, static_cast<float>(fNewAspectRatio), true);
			}
			else
			{
				bIsUltrawide = false;
				bIs16by10 = false;

				#ifdef VERBOSE
				con.AddLogChar("Wrote default aspect ratio values");
				#endif

				injector::WriteMemory(ptrMapIconsPos, &fDefaultMapIconsPos, true);
				injector::WriteMemory(ptrEsp18Height, static_cast<double>(-fDefaultEsp18Height), true);
				injector::WriteMemory(ptrEngineWidthScale, static_cast<double>(fDefaultEngineWidthScale), true);
				injector::WriteMemory(ptrAspectRatio, static_cast<float>(fDefaultAspectRatio), true);
			}

			*(int32_t*)(ptrResMovAddr) = regs.eax;
		}
	}; injector::MakeInline<ResChange>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(4));

	spd::log()->info("{} -> Aspect ratio tweaks applied", __FUNCTION__);
}