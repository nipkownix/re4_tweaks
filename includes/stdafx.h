#pragma once
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES
#pragma warning(push)
#pragma warning(disable: 4178 4305 4309 4510 4996)
#include <iomanip>
#include "IniReader.h"
#include "injector\injector.hpp"
#include "injector\calling.hpp"
#include "injector\hooking.hpp"
#ifdef _M_IX86
#include "injector\assembly.hpp"
#endif
#include "injector\utility.hpp"
#include "Patterns.h"
#include "MemoryMgr.h"
#include <filesystem>
#include <string>
#pragma warning(pop)

using namespace Memory::VP;

void CreateThreadAutoClose(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId);

std::wstring GetModuleFileNameW(HMODULE hModule);