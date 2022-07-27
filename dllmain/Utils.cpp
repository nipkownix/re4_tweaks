#include <iostream>
#include "dllmain.h"

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


void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	CloseHandle(CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId));
}