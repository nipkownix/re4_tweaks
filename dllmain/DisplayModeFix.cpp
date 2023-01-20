#include <iostream>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Game.h"
#include "Settings.h"

int iDesiredRefreshRate = 0;

struct GX_DisplayMode
{
	int width;
	int height;
	int refreshRate;
};

void FillDisplayModeVector(std::vector<GX_DisplayMode> *vect, bool OnlyHighestRefreshRates)
{
	std::vector<GX_DisplayMode> modes;

	DEVMODE devMode = {};
	ZeroMemory(&devMode, sizeof(DEVMODE));
	devMode.dmSize = sizeof(DEVMODE);

	HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

	MONITORINFOEXW monInfo = {};
	monInfo.cbSize = sizeof(monInfo);
	if (!GetMonitorInfoW(primaryMonitor, reinterpret_cast<MONITORINFO*>(&monInfo)))
	{
		spd::log()->info("{} -> Failed to query monitor info!", __FUNCTION__);
	}

	// Get all display modes from primary monitor
	int modeNum = 0;
	while (EnumDisplaySettingsW(monInfo.szDevice, modeNum, &devMode))
	{
		modes.push_back({ int(devMode.dmPelsWidth), int(devMode.dmPelsHeight), int(devMode.dmDisplayFrequency) });
		modeNum++;
	}

	// Sort display modes by resolution and refresh rate
	std::sort(modes.begin(), modes.end(), [](const GX_DisplayMode& lhs, const GX_DisplayMode& rhs)
		{
			if (lhs.width != rhs.width) return lhs.width < rhs.width;
			if (lhs.height != rhs.height) return lhs.height < rhs.height;
			return lhs.refreshRate < rhs.refreshRate; // Keep the lowest refresh rate
		});

	// Filter display modes
	std::vector<GX_DisplayMode> filteredModes;
	filteredModes.reserve(modes.size());

	GX_DisplayMode prevMode = {};
	for (const GX_DisplayMode& mode : modes)
	{
		if (mode.width != prevMode.width || mode.height != prevMode.height)
		{
			// This is a unique resolution, so add it to the filtered list
			filteredModes.push_back(mode);
		}
		else
		{
			if (!OnlyHighestRefreshRates)
			{
				// Duplicate resolution, but we'll populate every refresh rate available for each unique resolution
				if (mode.refreshRate != prevMode.refreshRate)
				{
					filteredModes.push_back(mode);
				}
			}
			else
			{
				// Duplicate resolution, only keep the highest refresh rate for this resolution
				if (mode.refreshRate > prevMode.refreshRate)
				{
					filteredModes.back() = mode;
				}
			}
		}

		prevMode = mode;
	}

	modes = std::move(filteredModes);

	// Log the display modes used
	if (re4t::cfg->bVerboseLog)
	{
		spd::log()->info("+-----------------+-----------------+");
		spd::log()->info("| FillDisplayModeVector: modes      |");
		spd::log()->info("+-----------------+-----------------+");
		spd::log()->info("+ Primary monitor: {:<16} +", WstrToStr(monInfo.szDevice));
		spd::log()->info("+-----------------+-----------------+");
		spd::log()->info("+ OnlyHighestRefreshRates = {:<11} +", OnlyHighestRefreshRates ? "true" : "false");
		spd::log()->info("+-----------------+-----------------+");
		for (const GX_DisplayMode& mode : modes)
		{
			spd::log()->info("| {:<5}x {:<5} {:>17} Hz |", mode.width, mode.height, mode.refreshRate);
		}
		spd::log()->info("+-----------------+-----------------+");
	}

	// Move values into vector
	*vect = std::move(modes);

	return;
}

DWORD* (__thiscall* bio4__vector_D3D_DisplayMode__push_back)(std::vector<GX_DisplayMode>* displayVec, GX_DisplayMode* displayMode);
char __cdecl D3D_FillDisplayModeVector_hook(std::vector<GX_DisplayMode>* vect)
{
	std::vector<GX_DisplayMode> dispModeVec;
	FillDisplayModeVector(&dispModeVec, re4t::cfg->bOnlyShowHighestRefreshRates);

	for (GX_DisplayMode& mode : dispModeVec)
	{
		bio4__vector_D3D_DisplayMode__push_back(vect, &mode);
	}

	return 0;
}

int sprintfCurrentHz = 0; // Filled in res_sprintf_getCurRefreshRate_hook
int sprintf_1_hook(char* Buffer, char* Format, ...)
{
	va_list args;
	va_start(args, Format);
	int result = vsprintf(Buffer, Format, args);
	va_end(args);

	// Append refresh rate for current resolution
	std::string hzStr = " " + std::to_string(sprintfCurrentHz) + " Hz";
	strcat(Buffer, hzStr.c_str());

	// Update iDesiredRefreshRate so our hooks know what is the refesh rate
	// of the resolution that is currently selected
	iDesiredRefreshRate = sprintfCurrentHz;

	return result;
}

void re4t::init::DisplayModeFix()
{
	// Game is hardcoded to only allow 60Hz display modes for some reason...
	if (re4t::cfg->bFixDisplayMode)
	{
		// Replace function that fills resolution list, normally it filters resolutions to only 60Hz modes, which is bad for non-60Hz players...
		// We now reimplement the entire function and use EnumDisplaySettings to get a list of all resolutions available, avoiding duplicates and only
		// showing the highest refresh rate for each unique width and height, if needed.
		// Using EnumDisplaySettings is what DXVK does as well, so this should hopefully be fine.
		auto pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 83 C4 08 3B C7");
		auto ptr_D3D_FillDisplayModeVector = injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int();
		InjectHook(ptr_D3D_FillDisplayModeVector, D3D_FillDisplayModeVector_hook);

		// Use the game's own std::vector<D3D_DisplayMode>::push_back function to push values to its display vector, othewise issue can happen.
		pattern = hook::pattern("E8 ? ? ? ? 8B 4D E8 41 89 4D E8 3B 4D E4");
		ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), bio4__vector_D3D_DisplayMode__push_back);

		// Hook D3D_GetDisplayModeForWidthHeight to manipulate its filter, so we can use our own refresh rate instead.
		// This function was originally only capable of returning 60 Hz resolutions, which is quite an oversight on QLOC's part.
		pattern = hook::pattern("8B 45 ? 83 F8 ? 75 ? 8B 4D ? 8B 7D ? 3B 4D");
		struct D3D_GetDisplayModeForWidthHeight_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				assert(iDesiredRefreshRate != 0);

				regs.eax = iDesiredRefreshRate;
				regs.ef |= (1 << regs.zero_flag);
			}
		}; injector::MakeInline<D3D_GetDisplayModeForWidthHeight_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Hook GXInit_Impl right after the game calls ConfigReadINI() so we can determine which refresh rate the game should
		// use on startup based on what is stored in it's config.ini.
		pattern = hook::pattern("8B 85 ? ? ? ? 8B 8D ? ? ? ? 8D 95 ? ? ? ? 52 50 51 E8 ? ? ? ? 83 C4");
		struct StartupRefreshRate_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				regs.eax = *(uint32_t*)(regs.ebp - 0x8B8);

				int iRefreshRate = 0;

				// Get vector of all display modes
				std::vector<GX_DisplayMode> dispModeVec;
				FillDisplayModeVector(&dispModeVec, false);

				// Get desired resolution and refresh rate from the game's config.ini
				GX_DisplayMode targetMode = { g_INIConfig()->resolutionWidth, g_INIConfig()->resolutionHeight, g_INIConfig()->resolutionRefreshRate };

				spd::log()->info("FixDisplayMode -> Checking if display mode \"{}x{} @ {} Hz\" is supported by the primary monitor...", targetMode.width, targetMode.height, targetMode.refreshRate);

				// Check if this mode is supported by the current primary display
				auto it = std::find_if(dispModeVec.begin(), dispModeVec.end(), [&](const GX_DisplayMode& mode)
					{
						return mode.width == targetMode.width && mode.height == targetMode.height && mode.refreshRate == targetMode.refreshRate;
					});

				// Display mode is supported, proceed
				if (it != dispModeVec.end())
				{
					spd::log()->info("FixDisplayMode -> Display mode is supported, proceeding...");
					iRefreshRate = g_INIConfig()->resolutionRefreshRate;
				}
				else // Display mode is not supported, try to use the desktop's instead
				{
					spd::log()->info("FixDisplayMode -> Display mode specified in the game's config.ini file isn't supported by the primary monitor!");
					spd::log()->info("FixDisplayMode -> Trying to use the current desktop resolution instead...");

					// Get the current refresh rate from Windows
					DEVMODE devMode = {};
					ZeroMemory(&devMode, sizeof(DEVMODE));
					devMode.dmSize = sizeof(DEVMODE);

					HMONITOR primaryMonitor = MonitorFromPoint({ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);

					MONITORINFOEXW monInfo = {};
					monInfo.cbSize = sizeof(monInfo);
					if (!GetMonitorInfoW(primaryMonitor, reinterpret_cast<MONITORINFO*>(&monInfo)))
					{
						spd::log()->info("FixDisplayMode -> Failed to query monitor info!");
					}

					if (!EnumDisplaySettingsW(monInfo.szDevice, ENUM_CURRENT_SETTINGS, &devMode))
					{
						#ifdef VERBOSE
						con.log("Couldn't read the display refresh rate. Using fallback value.");
						#endif

						spd::log()->info("FixDisplayMode -> Couldn't read the display refresh rate. Using \"60\" as fallback value.");

						iRefreshRate = 60; // Fallback value.
					}
					else
					{
						#ifdef VERBOSE
						con.log("Primary display device name: %s", WstrToStr(monInfo.szDevice).c_str());
						#endif

						spd::log()->info("FixDisplayMode -> Primary display device name: {}", WstrToStr(monInfo.szDevice));

						iRefreshRate = devMode.dmDisplayFrequency;

						// Replace game's current display mode with the one from the desktop
						g_INIConfig()->resolutionWidth = devMode.dmPelsWidth;
						g_INIConfig()->resolutionHeight = devMode.dmPelsHeight;
						g_INIConfig()->resolutionRefreshRate = iRefreshRate;

						regs.eax = devMode.dmPelsHeight; // Height param that will be passed to the next function call
					}
				}

				spd::log()->info("FixDisplayMode -> Starting game with a refresh rate of {} Hz", iRefreshRate);

				#ifdef VERBOSE
				con.log("New refresh rate = %d", iRefreshRate);
				#endif

				// Save chosen refresh rate
				iDesiredRefreshRate = iRefreshRate;
			}
		}; injector::MakeInline<StartupRefreshRate_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Hook D3D_SetupPresentationGlobals
		pattern = hook::pattern("89 35 ? ? ? ? 89 15 ? ? ? ? A3 ? ? ? ? 89");
		struct D3D_SetupPresentationGlobals_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// If we don't update g_D3D_RefreshRate and regs.esi (RefreshRate param passed to this func) here,
				// g_D3DPresentParams's FullScreen_RefreshRateInHz will not be updated when changing resolutions through the game's config menu.
				*bio4::g_D3D::RefreshRate = iDesiredRefreshRate;
				regs.esi = iDesiredRefreshRate;
			}
		}; injector::MakeInline<D3D_SetupPresentationGlobals_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));	
		
		// Hook OptionScreen::init to also take g_D3D_RefreshRate into consideration when initiating g_GfxSettings_ResolutionIdx
		pattern = hook::pattern("8B 15 ? ? ? ? 3B 14 ? 75 ? 8B 15 ? ? ? ? 3B 54 38");
		struct OptionScreen__init_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.edx = *bio4::g_D3D::Width_1;

				int curRefreshRate = *(uint32_t*)(regs.eax + regs.edi + 8);

				if (curRefreshRate != *bio4::g_D3D::RefreshRate)
					regs.edx = 0; // Easy way to make the game fail the check and move to the next resolution in the list...
			}
		}; injector::MakeInline<OptionScreen__init_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(6));

		// Hook sprinf_1 call inside graphics_menu so we can also display the refresh rate next to the resolution
		pattern = hook::pattern("E8 ? ? ? ? 8B 35 ? ? ? ? 83 C4 10 85 F6");
		InjectHook(pattern.count(1).get(0).get<uint32_t>(0), sprintf_1_hook);

		// Hook just before the previous sprinf_1 call to store the refresh rate of the currently shown/selected resolution
		pattern = hook::pattern("8B 48 04 8B 10 51 52 68 ? ? ? ? 68 ? ? ? ? E8");
		struct res_sprintf_getCurRefreshRate_hook
		{
			void operator()(injector::reg_pack& regs)
			{
				// Code we replaced
				regs.edx = *(uint32_t*)(regs.eax);
				regs.ecx = *(uint32_t*)(regs.eax + 0x4);

				// Save refresh rate for this resolution
				sprintfCurrentHz = *(uint32_t*)(regs.eax + 0x8);
			}
		}; injector::MakeInline<res_sprintf_getCurRefreshRate_hook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));

		// Hook ConfigWriteINI to make sure the current resolution is being saved correctly
		pattern = hook::pattern("8B 4E ? 8B 56 ? 8B 06 51 8B 8D ? ? ? ? 52 50 68");
		struct WriteIniHook
		{
			void operator()(injector::reg_pack& regs)
			{
				regs.eax = *bio4::g_D3D::Width_1;
				regs.edx = *bio4::g_D3D::Height_1;
				regs.ecx = *bio4::g_D3D::RefreshRate;
			}
		}; injector::MakeInline<WriteIniHook>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(8));

		// Log display modes for troubleshooting
		if (re4t::cfg->bVerboseLog)
		{
			DISPLAY_DEVICE device;
			ZeroMemory(&device, sizeof(DISPLAY_DEVICE));
			device.cb = sizeof(DISPLAY_DEVICE);

			DEVMODE devMode = {};
			ZeroMemory(&devMode, sizeof(DEVMODE));
			devMode.dmSize = sizeof(DEVMODE);

			spd::log()->info("+-----------------+-----------------+");
			spd::log()->info("| Display modes                     |");
			spd::log()->info("+-----------------+-----------------+");
			int devIndex = 0;
			while (EnumDisplayDevicesW(NULL, devIndex, &device, 0))
			{
				// Enumerate all of the display settings for the current display
				int setIndex = 0;
				while (EnumDisplaySettingsExW(device.DeviceName, setIndex, &devMode, 0))
				{
					spd::log()->info("| {:<12}: {:<5}x {:<5} {:>3} Hz |", WstrToStr(device.DeviceName), devMode.dmPelsWidth, devMode.dmPelsHeight, devMode.dmDisplayFrequency);

					setIndex++;
				}
				devIndex++;
			}
			spd::log()->info("+-----------------+-----------------+");
		}
	}
}