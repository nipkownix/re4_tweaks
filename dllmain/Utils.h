#pragma once
#include <string>

std::string StrToUTF8(std::string const& str);
std::string WstrToStr(const std::wstring& wstr);

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);