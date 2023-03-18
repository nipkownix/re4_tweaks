#pragma once
#include <string>

class dxvk_cfg {
public:

	bool bUseVulkanRenderer = true;
	bool bShowFPS = false;
	bool bShowGPULoad = false;
	bool bShowDeviceInfo = false;

	std::string DXVK_HUD = ""; // Will override the previous HUD options if set in the ini.
	std::string DXVK_FILTER_DEVICE_NAME = "";
};

namespace re4t::dxvk
{
	inline dxvk_cfg* cfg = new dxvk_cfg;
}