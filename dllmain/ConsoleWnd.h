#pragma once

struct ConsoleOutput
{
	void ShowConsoleOutput();
	void AddLogChar(const char* fmt, ...);
	void AddLogHex(int fmt, ...);
	void AddLogInt(int fmt, ...);
	void AddLogFloat(float fmt, ...);
	void AddConcatLog(const char* fmt, int value, ...);
	void AddConcatLog(const char* fmt, const char* value, ...);
	void Clear();
};

void ConsoleBinding(UINT uMsg, WPARAM wParam);
bool ParseConsoleKeyCombo(std::string_view in_combo);

extern ConsoleOutput con;