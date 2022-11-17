#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include "Game.h"
#include "Settings.h"

// HDProject.cpp: Used to apply changes that only apply to the re4 HD project.
// At the moment, this only changed the scaling of textures contained inside ".fix" files.
// (".fix" seems to be a simple container, just like the usual .pack format)

const float fOptionsScale = 2.0f;
const int iFixImagesScale = 4;

void Init_HDProject()
{
	// BIO4\option\xxx_option.fix
	{
		// The textures from this .fix seem to be handled a bit differently.
		// The vanilla game scales their display size by half, for some reason.
		// We just divide the original value by how many times it was upscled (eg: 2x)
		float fOrigScale = 0.5f;
		static double dNewScale = fOrigScale / fOptionsScale;

		auto pattern = hook::pattern("DC 0D ? ? ? ? D9 C0 DD 05 ? ? ? ? DC C9 D9 07 DE C2 D9 C9 E8 ? ? ? ? DB 86");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(2), &dNewScale, true);

		pattern = hook::pattern("DC 0D ? ? ? ? D9 C0 DE CA D9 47 ? DE C2 D9 C9 E8 ? ? ? ? D9 C9 0F B7 ? E8");
		injector::WriteMemory(pattern.count(1).get(0).get<uint32_t>(2), &dNewScale, true);

		if (pConfig->bVerboseLog)
		{
			spd::log()->info("HDProject: \"option\\xxx_option.fix\": dNewScale = {}", dNewScale);
		}
	}

	// BIO4\option\NewOption\howto\xxx_NewOption.fix
	// BIO4\option\NewOption\howto\xxx_hd_HowTo.fix
	// BIO4\option\NewOption\howto\xxx_sd_HowTo.fix
	// BIO4\Ranking\Ranking_xxx.fix
	// BIO4\Ranking\UploadRanking_xxx.fix
	// BIO4\SS\cmn\NowLoading.fix
	// BIO4\Title\xxx_press_Start.fix
	{
		auto pattern = hook::pattern("89 4E ? 89 56 ? 85 FF 75 ? 43 83 C6 ? 3B 5D ? 0F 8C ? ? ? ? 8B");
		struct sub_9E6580_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// This function stores the resolution of the .fix textures.
				// Seems the game uses these to calculate their display size later, so we store modified image sizes.
				// To maintain their inteded display size, we just divide their actual size by how many times it was upscled (eg: 4x)
				*(uint32_t*)(regs.esi + 0x4) = regs.ecx / iFixImagesScale;
				*(uint32_t*)(regs.esi + 0x8) = regs.edx / iFixImagesScale;

				if (pConfig->bVerboseLog)
				{
					spd::log()->info("+-------------------------+-------------------------+");
					spd::log()->info("HDProject: ***.fix: originalX = {}, scaledX = {}", regs.ecx, regs.ecx / iFixImagesScale);
					spd::log()->info("HDProject: ***.fix: originalY = {}, scaledY = {}", regs.edx, regs.edx / iFixImagesScale);
					spd::log()->info("+-------------------------+-------------------------+");
				}
			}
		}; injector::MakeInline<sub_9E6580_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));
	}
}