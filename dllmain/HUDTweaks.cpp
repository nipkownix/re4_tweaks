#include <iostream>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Game.h"
#include "Settings.h"
#include "HUDTweaks.h"

extern bool isQTEactive(); // QTEFixes.cpp

float fDefaultHUDPosX;
float fDefaultHUDPosY;

#define DEF FLT_MIN
struct HUDEditData
{
	uint8_t unitNo;
	float posX = DEF, posY = DEF;
	float sizeW = DEF, sizeH = DEF;
};

void ApplyHUDEdits(ID_CLASS idClass, const HUDEditData* data, int size)
{
	for (int i = 0; i < size; ++i)
	{
		ID_UNIT* tex = IDSystemPtr()->unitPtr2(data[i].unitNo, idClass);
		if (data[i].posX != DEF) tex->pos0_94.x = data[i].posX;
		if (data[i].posY != DEF) tex->pos0_94.y = data[i].posY;
		if (data[i].sizeW != DEF) tex->size0_W_DC = data[i].sizeW;
		if (data[i].sizeH != DEF) tex->size0_H_E0 = data[i].sizeH;
	}
}

void ShrinkIDClass(ID_CLASS idClass, float scale)
{
	ID_UNIT* tex = IDSystemPtr()->m_IdUnit_50;
	for (int i = 0; i < IDSystemPtr()->m_maxId_0; ++i)
	{
		if (tex->classNo_2 == idClass && tex->texId_78 != 0xFF)
		{
			tex->size0_W_DC *= scale;
			tex->size0_H_E0 *= scale;
		}

		tex++;
	}
}

void ShrinkLifeMeter()
{
	static const HUDEditData SmallLifeMeter[] = {
		{ 0, 232.97f, -140.5f },
		{ 1,  30.4f, 36.4f },
		{ 16, -10.7f, -21.0f },
		{ 49, DEF, -28.9f },
		{ 4, 33.1f, 34.1f },
		{ 26, -9.0f, -23.0f },
		{ 27, 9.4f, -1.2f },
		{ 30, -6.7f, -0.3f },
		{ 40, 2.2f, -1.4f },
		{ 3, 6.2f, DEF },
		{ 2, 3.4f, -1.8f },
		{ 54, 2.8f, -2.7f },
		{ 48, 2.8f, -2.7f },
		{ 9, 2.8f, -2.7f },
		{ 23, DEF, 24.5f },
		// names
		{ 24, 18.1f, -37.1f },
		{ 28, 9.3f, -38.0f },
		{ 29, 22.1f, -40.4f },
		{ 59, 7.9f, -40.0f },
		{ 60, 11.8f, -40.2f },
		{ 58, 18.1f, -40.2f },
		// ammo digits
		{ 25, -6.0f, -9.2f, 10.0f, 20.0f },
		{ 5,  4.0f, -9.2f, 10.0f, 20.0f },
		{ 6,  14.0f, -9.2f, 10.0f, 20.0f },
	};

	ShrinkIDClass(IDC_LIFE_METER, 0.7484f); 
	ApplyHUDEdits(IDC_LIFE_METER, SmallLifeMeter, sizeof(SmallLifeMeter) / sizeof(*SmallLifeMeter));
}

void ShrinkPRLGauge()
{
	static const HUDEditData SmallPRLGauge[] = {
		{ 0, -1.2f, 1.0f },
		{ 34, -0.6f, -19.2f },
		{ 35, -12.6f, -12.7f },
		{ 36, -15.2f, -0.7f },
		{ 33, -10.8f, 9.2f },
		{ 38, 3.7f, 1.3f },
		{ 40, 3.7f, 1.3f },
		{ 21, -3.3f, -5.9f },
		{ 22, -3.3f, -5.9f },
		{ 23, -3.7f, -5.0f },
	};

	ShrinkIDClass(IDC_LASER_GAUGE, 0.7484f);
	ApplyHUDEdits(IDC_LASER_GAUGE, SmallPRLGauge, sizeof(SmallPRLGauge) / sizeof(*SmallPRLGauge));
}

void ShrinkBulletInfo()
{
	static const HUDEditData SmallBulletInfo[] = {
		{ 6,  -6.7f, -0.1f, 49.39f, 49.39f},
		{ 4,  -6.3f, -1.0f },
		{ 11, -14.9f, 2.4f },
		{ 12, -7.5f, 9.2f },
		{ 3,  -7.0f, 10.6f },
		{ 2,  -12.4f, 2.0f },
		{ 5,  -7.3f, -0.8f },
		{ 7,  -7.4f, 10.0f },
		{ 8,  -5.5f, 10.3f },
		{ 9,  -6.7f, -0.9f },
		{ 13, -9.9f, 5.6f }
	};

	ShrinkIDClass(IDC_BLLT_ICON, 0.7484f);
	ApplyHUDEdits(IDC_BLLT_ICON, SmallBulletInfo, sizeof(SmallBulletInfo) / sizeof(*SmallBulletInfo));
}

void re4t::HUDTweaks::ResetLifeMeter()
{
	if (IDSystemPtr()->setCk(IDC_LIFE_METER))
	{
		IDSystemPtr()->kill(0xFF, IDC_LIFE_METER);
		IDSystemPtr()->kill(0xFF, IDC_LASER_GAUGE);
		Cckpt->m_LifeMeter_0.roomInit();
		Cckpt->m_BlltInfo_6C.roomInit();
	}
}

 // AspectRatioTweaks.cpp
extern bool bIsUltrawide, bIs16by10;
extern float fGameDisplayAspectRatio;

void SideAlignHUD()
{
	float fHudPosOffset = ((360.0f * fGameDisplayAspectRatio) - 640.0f) / 2.0f;
	IDSystemPtr()->unitPtr(0u, IDC_LIFE_METER)->pos0_94.x += fHudPosOffset;
}

void OffsetHUD()
{
	IDSystemPtr()->unitPtr(0u, IDC_LIFE_METER)->pos0_94.x = fDefaultHUDPosX + re4t::cfg->fHUDOffsetX;
	IDSystemPtr()->unitPtr(0u, IDC_LIFE_METER)->pos0_94.y = fDefaultHUDPosY + re4t::cfg->fHUDOffsetY;
}

void re4t::HUDTweaks::UpdateHUDOffsets()
{
	if (IDSystemPtr()->setCk(IDC_LIFE_METER))
		OffsetHUD();
}

void re4t::init::HUDTweaks()
{
	// Life meter HUD tweaks
	{
		auto pattern = hook::pattern("56 50 B9 ? ? ? ?  E8 ? ? ? ? 80 26 F7");
		struct BulletInfo__roomInit_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Shrink the size of the life meter HUD
				if (re4t::cfg->bSmallerHUD)
				{
					ShrinkLifeMeter();
					ShrinkPRLGauge();
				}

				// Side align HUD for ultrawide and 16:10
				if ((re4t::cfg->bSideAlignHUD && bIsUltrawide) || (re4t::cfg->bRemove16by10BlackBars && bIs16by10))
					SideAlignHUD();

				// Save the default HUD position
				fDefaultHUDPosX = IDSystemPtr()->unitPtr(0u, IDC_LIFE_METER)->pos0_94.x;
				fDefaultHUDPosY = IDSystemPtr()->unitPtr(0u, IDC_LIFE_METER)->pos0_94.y;

				// Reposition the HUD
				if (re4t::cfg->bRepositionHUD)
					OffsetHUD();

				// Code we overwrote
				regs.ecx = (uint32_t)IDSystemPtr();
			}
		}; injector::MakeInline<BulletInfo__roomInit_hook>(pattern.count(1).get(0).get<uint32_t>(2), pattern.count(1).get(0).get<uint32_t>(7));

		pattern = hook::pattern("8B ? ? 51 50 B9 ? ? ? ? E8 ? ? ? ? EB");
		struct BulletInfo__move_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bSmallerHUD)
					ShrinkBulletInfo();

				// Code we overwrote
				regs.ecx = (uint32_t)IDSystemPtr();
			}
		}; injector::MakeInline<BulletInfo__move_hook>(pattern.count(1).get(0).get<uint32_t>(5), pattern.count(1).get(0).get<uint32_t>(10));
	}

	// ShrinkIDClass action button prompts
	{
		auto pattern = hook::pattern("03 C2 50 B9 ? ? ? ? E8 ? ? ? ? C7");
		static uint32_t* g_IDSystemSetException = *pattern.count(1).get(0).get<uint32_t*>(15);
		struct ActionButton__move_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bSmallerActionPrompts)
				{
					ShrinkIDClass(IDC_ACT_BUTTON, 0.9f);

					// grab prompt anchor
					IDSystemPtr()->unitPtr(6u, IDC_ACT_BUTTON)->pos0_94.y = -90.5f;
					// interact prompt anchor
					IDSystemPtr()->unitPtr(0x20u, IDC_ACT_BUTTON)->pos0_94.x = -55.8f;
					IDSystemPtr()->unitPtr(0x20u, IDC_ACT_BUTTON)->pos0_94.y = -148.1f;
				}

				// Code we overwrote
				*g_IDSystemSetException = 0;
			}
		}; injector::MakeInline<ActionButton__move_hook>(pattern.count(1).get(0).get<uint32_t>(13), pattern.count(1).get(0).get<uint32_t>(23));

		pattern = hook::pattern("8D 53 16 52 B9 ? ? ? ? E8");
		struct cActionButton__disp_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bSmallerActionPrompts)
					cMes->setFontSize(1, 21, 29); // default font size: 23 x 32

				// Code we overwrote
				regs.ecx = (uint32_t)cMes;
			}
		}; injector::MakeInline<cActionButton__disp_hook>(pattern.count(1).get(0).get<uint32_t>(4), pattern.count(1).get(0).get<uint32_t>(9));
	}

	// Hide zoom control hints from the weapon scope and binocular HUDs
	{
		auto pattern = hook::pattern("80 B8 C0 4F 00 00 0E 56 57 8B F9 0F");
		struct IdScope__move_HideZoomHints
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bHideZoomControlHints)
				{
					IDSystemPtr()->unitPtr(0x20u, IDC_SCOPE)->be_flag_0 &= ~ID_BE_FLAG_VISIBLE;
					IDSystemPtr()->unitPtr(0x21u, IDC_SCOPE)->be_flag_0 &= ~ID_BE_FLAG_VISIBLE;

					IdScope* thisptr = (IdScope*)regs.ecx;
					thisptr->m_zoom_disp_C = 0; // hides the text
				}

				// Code we overwrote
				if (GlobalPtr()->weapon_no_4FC0 == 14)
					regs.ef |= (1 << regs.zero_flag);
				else
					regs.ef &= ~(1 << regs.zero_flag);
			}
		}; injector::MakeInline<IdScope__move_HideZoomHints>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(7));

		pattern = hook::pattern("F7 81 1C 50 00 00 00 10 00 00");
		struct IdBinocular__move_HideZoomHints
		{
			void operator()(injector::reg_pack& regs)
			{
				bool isEvent = FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_EVENT));
				if (re4t::cfg->bHideZoomControlHints || isEvent)
				{
					IDSystemPtr()->unitPtr(0x30u, IDC_BINOCULAR)->be_flag_0 &= ~ID_BE_FLAG_VISIBLE;
					IDSystemPtr()->unitPtr(0x1Bu, IDC_BINOCULAR)->be_flag_0 &= ~ID_BE_FLAG_VISIBLE;
					regs.ef &= ~(1 << regs.zero_flag);
				}
				else
				{
					IDSystemPtr()->unitPtr(0x30u, IDC_BINOCULAR)->be_flag_0 |= ID_BE_FLAG_VISIBLE;
					IDSystemPtr()->unitPtr(0x1Bu, IDC_BINOCULAR)->be_flag_0 |= ID_BE_FLAG_VISIBLE;
					regs.ef |= (1 << regs.zero_flag);
				}
			}
		}; injector::MakeInline<IdBinocular__move_HideZoomHints>(pattern.count(2).get(1).get<uint32_t>(0), pattern.count(2).get(1).get<uint32_t>(10));
	}
}
