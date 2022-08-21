#pragma once 
#include "Game.h"

void UI_NewEmManager(int selectedEmIndex = -1);
void UI_NewGlobalsViewer();
void UI_NewAreaJump();
void UI_NewFilterTool();

class UI_Window
{
	std::string origWindowTitle;

protected:
	std::string windowTitle;

public:
	UI_Window(std::string_view title) : origWindowTitle(title) { UpdateWindowTitle(); };
	virtual bool Init() = 0;
	virtual bool Render() = 0;

	void UpdateWindowTitle();
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

	uint16_t AtariFlagBackup = 0;
	bool AtariFlagBackupSet = false;

	float addPosition[3] = { 0 };
public:
	UI_EmManager(std::string_view title, int selectedIndex = -1) : UI_Window(title), emIdx(selectedIndex) {}
	static std::string EmDisplayString(int i, cEm& em, bool showEmPointers);
	bool Init() { return true; }
	bool Render();
};

class UI_Globals : public UI_Window
{
public:
	UI_Globals(std::string_view title) : UI_Window(title) {}
	bool Init() { return true; }
	bool Render();
};

class UI_AreaJump : public UI_Window
{
	int curStage = 0;
	int curRoomIdx = 0;

	char curRoomNo[256] = { 0 };
	Vec curRoomPosition = { 0 };
	float curRoomRotation = 0;

	std::string RoomDisplayString(int stage, int room);
	void UpdateRoomInfo();
public:
	UI_AreaJump(std::string_view title) : UI_Window(title) { Init(); }
	bool Init();
	bool Render();
};

class UI_FilterTool : public UI_Window
{
public:
	UI_FilterTool(std::string_view title) : UI_Window(title) { Init(); }
	bool Init() { return true; }
	bool Render();
};