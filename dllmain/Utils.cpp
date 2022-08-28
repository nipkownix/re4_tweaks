#include <iostream>
#include "dllmain.h"
#include <random>

std::string WstrToStr(const std::wstring& wstr)
{
	size_t len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);

	if (len <= 1)
		return "";

	len -= 1;

	std::string result;
	result.resize(len);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result.at(0), len, nullptr, nullptr);
	return result;
}

std::wstring StrToWstr(const std::string& str) 
{
	size_t len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);

    if (len <= 1)
        return L"";

    len -= 1;

    std::wstring result;
    result.resize(len);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result.at(0), len);
    return result;
}

std::string StrToUpper(std::string str)
{
	std::transform(str.begin(), str.end(), str.begin(), ::toupper);
	return str;
}

float GetRandomFloat(float min, float max)
{
	static std::mt19937 engi;
	std::uniform_real_distribution<float> num(min, max); // range min - max
	return num(engi);
}

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	CloseHandle(CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId));
}