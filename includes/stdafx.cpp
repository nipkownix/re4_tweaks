#include "stdafx.h"
#include <iostream>
#include <string>
#include <map>

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
    CloseHandle(CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId));
}

std::wstring GetModuleFileNameW(HMODULE hModule)
{
	static constexpr auto INITIAL_BUFFER_SIZE = MAX_PATH;
	static constexpr auto MAX_ITERATIONS = 7;
	std::wstring ret;
	auto bufferSize = INITIAL_BUFFER_SIZE;
	for (size_t iterations = 0; iterations < MAX_ITERATIONS; ++iterations)
	{
		ret.resize(bufferSize);
		auto charsReturned = GetModuleFileNameW(hModule, &ret[0], bufferSize);
		if (charsReturned < ret.length())
		{
			ret.resize(charsReturned);
			return ret;
		}
		else
		{
			bufferSize *= 2;
		}
	}
	return L"";
}