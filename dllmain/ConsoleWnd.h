#pragma once

struct ConsoleOutput
{
	std::string TitleKeyCombo;
	void ShowConsoleOutput();
	void AddLogChar(const char* fmt, ...);
	void AddLogHex(int fmt, ...);
	void AddLogInt(int fmt, ...);
	void AddLogFloat(float fmt, ...);
	void AddConcatLog(const char* fmt, int value, ...);
	void AddConcatLog(const char* fmt, float value, ...);
	void AddConcatLog(const char* fmt, double value, ...);
	void AddConcatLog(const char* fmt, const char* value, ...);
	void Clear();
};

void ConsoleBinding();
bool ParseConsoleKeyCombo(std::string_view in_combo);

extern ConsoleOutput con;