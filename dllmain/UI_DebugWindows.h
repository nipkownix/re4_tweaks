#pragma once 
#include "Game.h"

class UI_Window
{
public:
	virtual bool Render() = 0;
};

class UI_EmManager : public UI_Window
{
	int emIdx = -1;
	bool onlyShowValidEms = true;
	bool onlyShowESLSpawned = false;
#ifdef _DEBUG
	bool showEmPointers = true;
#else
	bool showEmPointers = false;
#endif

	Vec copyPosition = { 0 };
	uint16_t AtariFlagBackup = 0;
	bool AtariFlagBackupSet = false;

	float addPosition[3] = { 0 };

	int windowId = 0;

	std::string EmDisplayString(int i, cEm& em);
public:
	UI_EmManager(int id) : windowId(id) {};
	bool Render();
};
