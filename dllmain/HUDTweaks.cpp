#include <iostream>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Game.h"
#include "Settings.h"
#include "HUDTweaks.h"

extern bool isQTEactive();

struct HUDEditData
{
	uint8_t unitNo;
	float posX, posY;
	float sizeW, sizeH;
};

#define DEF FLT_MIN
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

void ShrinkLifeMeter()
{
	static const HUDEditData SmallLifeMeter[] = {
		{ 0,  DEF, -136.0f, DEF, DEF },
		{ 1,  37.2f, DEF, DEF, DEF },
		{ 40, -15.0f, -16.0f, DEF, DEF },
		{ 25, -15.0f, -9.2f, 10.0f, 20.0f },
		{ 5,  -5.0f, -9.2f, 10.0f, 20.0f },
		{ 6,  5.0f, -9.2f, 10.0f, 20.0f },
		{ 2,  -4.8f, -1.6f, 52.8f, 52.8f },
		{ 48, -13.15f, -12.6f, 37.2f, 37.2f },
		{ 13, -13.4f, -13.7f, 29.92f, 29.92f },
		{ 16, -19.9f, DEF, DEF, DEF },
		{ 30, -6.6f, -1.1f, 48.85f, 48.85f },
		{ 14, -13.4f, -13.7f, 29.92f, 29.92f },
		{ 9,  -13.6f,  -12.6f, DEF, DEF },
		{ 3, -1.7f, -0.6f, DEF, DEF },
		{ 10, DEF, DEF, 32.5f, 32.5f },
		{ 11, DEF, DEF, 32.5f, 32.5f },
		{ 12, DEF, DEF, 32.5f, 32.5f },
		{ 41, 8.1f, 14.9f, 50.6f, 50.6f },
		{ 57, 8.1f, 14.9f, 50.6f, 50.6f },
		{ 24, 8.9f, -36.7f, 36.84f, 19.955f },
		{ 28, 0.475f, -37.6f, 51.7165f, 17.9f },
		{ 50, -14.7f, -15.3f, 31.82f, 31.82f },
		{ 8,  -5.25f, -19.8f, 76.202f, 72.79f },
		{ 29, 13.1f, -39.8f, 27.3f, 13.26f },
		{ 51, -12.6f, -14.0f, 30.52f, 30.52f },
		{ 59, -0.4f, -39.95f, 52.085f, 12.984f },
		{ 60, 2.8f, -40.4f, 48.425f, 12.665f },
		{ 58, 9.4f, -40.75f, 34.498f, 12.478f },
		{ 15, 0.0f, DEF, 107.03f, 67.32f },
		{ 26, -19.5f, -21.3f, 50.025f, 50.025f },
		{ 49, DEF, -29.0f, 1.64f, 8.2f },
		{ 23, DEF, 24.1f, 1.723f, 10.104f },
		{ 62, -12.712f, -12.712f, 31.8f, 31.8f },
		{ 63, -12.712f, -12.712f, 31.8f, 31.8f },
		{ 64, -12.712f, -12.712f, 31.8f, 31.8f },
		{ 55, -12.712f, -12.712f, 37.87f, 37.87f },
		{ 56, -12.712f, -12.712f, 37.87f, 37.87f },
		{ 53, -12.788f, -12.7f, 35.6f, 35.6f },
		{ 52, -12.788f, -12.7f, 35.6f, 35.6f }
	};

	ApplyHUDEdits(IDC_LIFE_METER, SmallLifeMeter, sizeof(SmallLifeMeter) / sizeof(*SmallLifeMeter));
}
                
void ShrinkPRLGauge()
{
	static const HUDEditData SmallPRLGauge[] = {
		{ 21, -3.6f, -6.4f, DEF, DEF },
		{ 22, -3.6f, -6.4f, DEF, DEF },
		{ 23, -4.0f, -5.3f, 50.5f, 50.5f },
		{ 34, -1.2f, -17.9f, 2.125f, 5.1f },
		{ 38, 2.75f, 1.0f, 13.0f, 13.0f },
		{ 32, DEF, DEF, 24.0f, 24.0f },
		{ 37, -9.0f, -11.1f, 16.0f, 10.0f },
		{ 35, -13.3f, -13.0f, 1.8f, 4.8f },
		{ 41, -9.0f, -11.1f, 16.0f, 10.0f },
		{ 29, DEF, DEF, 19.0f, 19.0f },
		{ 36, -16.9f, -0.394f, 2.1f, 9.45f },
		{ 26, DEF, DEF, 19.0f, 19.0f },
		{ 27, DEF, DEF, 19.0f, 19.0f },
		{ 30, DEF, DEF, 19.0f, 19.0f },
		{ 39, -9.0f, -11.1f, 16.0f, 10.0f },
		{ 33, -11.0f, 11.5f, 1.75f, 9.0f },
		{ 40, 2.75f, 1.0f, 13.0f, 13.0f },
		{ 28, DEF, DEF, 19.0f, 19.0f },
		{ 31, DEF, DEF, 19.0f, 19.0f }
	};

	ApplyHUDEdits(IDC_LASER_GAUGE, SmallPRLGauge, sizeof(SmallPRLGauge) / sizeof(*SmallPRLGauge));
}
              
void ShrinkBulletInfo()
{
	static const HUDEditData SmallBulletInfo[] = {
		// still not sure what these 3 are...
		//{ 4, DEF, DEF, 40.8f, 15.12f },
		//{ 6, DEF, -1.0f, 47.75f, 47.75f },
		//{ 11, DEF, DEF, 16.5f, 20.5f },
		{ 12, -6.9f, 7.5f, 36.5f, 19.388f },
		{ 3,  -6.6f, 8.7f, 34.0195f, 25.386f },
		{ 2,  -12.1f, 0.7f, 24.23f, 29.3415f },
		{ 5,  -6.9f, -1.7f, 45.879f, 9.324f },
		{ 7,  -7.0f, 8.2f, 36.0f, 28.5f },
		{ 8,  -5.0f, 8.4f, 30.5f, 20.33f },
		{ 9,  -6.7f, -1.0f, 46.48f, 8.13f },
		{ 13, -9.2f, 4.3f, 33.0f, 33.0f }
	};

	ApplyHUDEdits(IDC_BLLT_ICON, SmallBulletInfo, sizeof(SmallBulletInfo) / sizeof(*SmallBulletInfo));
}

void ResetLifeMeter()
{
	IDSystemPtr()->kill(0xFF, IDC_LIFE_METER);
	IDSystemPtr()->kill(0xFF, IDC_BLLT_ICON);
	IDSystemPtr()->kill(0xFF, IDC_LASER_GAUGE);
	Cckpt->m_LifeMeter_0.roomInit();
	Cckpt->m_BlltInfo_6C.roomInit();
}

void re4t::HUDTweaks::ToggleSmallLifeMeter()
{
	if (IDSystemPtr()->setCk(IDC_LIFE_METER))
	{
		if (re4t::cfg->bSmallerLifeMeter)
		{
			ShrinkLifeMeter();
			ShrinkPRLGauge();
			ShrinkBulletInfo();
		}
		else
			ResetLifeMeter();
	}
}

 // AspectRatioTweaks.cpp
extern bool bIsUltrawide, bIs16by10;
extern float fGameDisplayAspectRatio;

void SideAlignHud()
{
	float fHudPosOffset = ((360.0f * fGameDisplayAspectRatio) - 640.0f) / 2.0f;
	IDSystemPtr()->unitPtr(0u, IDC_LIFE_METER)->pos0_94.x += fHudPosOffset;
}

void re4t::HUDTweaks::ToggleSideAlignHUD()
{
	if (IDSystemPtr()->setCk(IDC_LIFE_METER))
	{
		if ((re4t::cfg->bSideAlignHUD && bIsUltrawide) || (re4t::cfg->bRemove16by10BlackBars && bIs16by10))
			SideAlignHud();
		else
			ResetLifeMeter();
	}
}

void re4t::init::HUDTweaks()
{
	// Life meter HUD tweaks
	{
		auto pattern = hook::pattern("C7 06 00 00 00 00 E8 ? ? ? ? 5E");
		struct LifeMeter__roomInit_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Side align life meter for ultrawide
				if ((re4t::cfg->bSideAlignHUD && bIsUltrawide) || (re4t::cfg->bRemove16by10BlackBars && bIs16by10))
					SideAlignHud();

				// Shrink the size of the life meter HUD
				if (re4t::cfg->bSmallerLifeMeter)
				{
					ShrinkLifeMeter();
					ShrinkPRLGauge();
				}

				// Code we overwrote
				*(uint32_t*)regs.esi = 0;
			}
		}; injector::MakeInline<LifeMeter__roomInit_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		pattern = hook::pattern("8B ? ? 51 50 B9 ? ? ? ? E8 ? ? ? ? EB");
		struct bulletInfo__move_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bSmallerLifeMeter)
					ShrinkBulletInfo();

				// Code we overwrote
				regs.ecx = (uint32_t)IDSystemPtr();
			}
		}; injector::MakeInline<bulletInfo__move_hook>(pattern.count(1).get(0).get<uint32_t>(5), pattern.count(1).get(0).get<uint32_t>(10));
	}

	// Shrink action button prompts
	{
		auto pattern = hook::pattern("03 C2 50 B9 ? ? ? ? E8 ? ? ? ? C7");
		static uint32_t* g_IDSystemSetException = *pattern.count(1).get(0).get<uint32_t*>(15);
		struct ActionButton__move_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				if (re4t::cfg->bSmallerActionPrompts)
				{
					static const int GrabUnitNo[] = {
						45, 33, 57, 63, 84, 91, // mouse
						33, 44, 58, 57, 63, 84, 91 // joystick
					};

					static const int QTEUnitNo[] = {
						184, 185, 188, 186, 187, 190, 191, // mashing prompt
						134, 135, 137, 139, 140, 149, 150, 153, 154, // QTE type 1
						8, 9, 10, 17, 25, 35, 36, 41, 42, 49, 50, 117, 152 // QTE type 2
					};

					static const float scale = 0.90f;

					// Ganado grab
					if (FlagIsSet(GlobalPtr()->flags_STATUS_0_501C, uint32_t(Flags_STATUS::STA_PL_CATCHED)))
					{
						IDSystemPtr()->unitPtr(6u, IDC_ACT_BUTTON)->pos0_94.y = -90.5f; // button anchor

						for (int i = 0; i < sizeof(GrabUnitNo) / sizeof(*GrabUnitNo); ++i)
						{
							ID_UNIT* tex = IDSystemPtr()->unitPtr2(GrabUnitNo[i], IDC_ACT_BUTTON);
							tex->size0_W_DC *= scale;
							tex->size0_H_E0 *= scale;
						}
					}
					// QTEs
					else if (isQTEactive())
					{
						for (int i = 0; i < sizeof(QTEUnitNo) / sizeof(*QTEUnitNo); ++i)
						{
							ID_UNIT* tex = IDSystemPtr()->unitPtr2(QTEUnitNo[i], IDC_ACT_BUTTON);
							tex->size0_W_DC *= scale;
							tex->size0_H_E0 *= scale;
						}
					}
					// Interact prompts
					else
					{
						// button anchor
						IDSystemPtr()->unitPtr(0x20u, IDC_ACT_BUTTON)->pos0_94.x = -55.8f;
						IDSystemPtr()->unitPtr(0x20u, IDC_ACT_BUTTON)->pos0_94.y = -148.1f;
						// button texture
						IDSystemPtr()->unitPtr2(181, IDC_ACT_BUTTON)->size0_W_DC *= scale;
						IDSystemPtr()->unitPtr2(181, IDC_ACT_BUTTON)->size0_H_E0 *= scale;
						// button glow
						IDSystemPtr()->unitPtr2(182, IDC_ACT_BUTTON)->size0_W_DC *= scale;
						IDSystemPtr()->unitPtr2(182, IDC_ACT_BUTTON)->size0_H_E0 *= scale;
					}
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
