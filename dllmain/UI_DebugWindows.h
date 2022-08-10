#pragma once 
#include "Game.h"

void UI_NewEmManager(int selectedEmIndex = -1);
void UI_NewGlobalsViewer();

class UI_Window
{
protected:
	std::string windowTitle;

public:
	UI_Window(std::string_view title) : windowTitle(title) {};
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

	bool WindowResized = false;

	Vec copyPosition = { 0 };
	uint16_t AtariFlagBackup = 0;
	bool AtariFlagBackupSet = false;

	float addPosition[3] = { 0 };

	std::string EmDisplayString(int i, cEm& em);
public:
	UI_EmManager(std::string_view title, int selectedIndex = -1) : UI_Window(title), emIdx(selectedIndex) {}
	bool Render();
};

class UI_Globals : public UI_Window
{
public:
	UI_Globals(std::string_view title) : UI_Window(title) {}
	bool Render();
};
