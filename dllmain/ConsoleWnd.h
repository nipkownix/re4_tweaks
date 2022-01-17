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

extern ConsoleOutput con;