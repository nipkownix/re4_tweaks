#pragma once

enum class LAADialogState
{
	NotShowing,
	Showing,
	Finished // show dialog telling user that LAA patch is complete, etc
};

struct LAApatch
{
	void LAARender();
	bool GameIsLargeAddressAware();
	LAADialogState LAA_State;
};

extern LAApatch laa;