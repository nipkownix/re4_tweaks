#pragma once
#define WIN32_LEAN_AND_MEAN
#define _USE_MATH_DEFINES

#pragma warning(push)
#pragma warning(disable: 4005)

#include <iomanip>
#include <IniReader.h>
#include <injector\injector.hpp>
#include <injector\calling.hpp>
#include <injector\hooking.hpp>
#include <injector\assembly.hpp>
#include <injector\utility.hpp>
#include <Patterns.h>
#include <MemoryMgr.h>
#include <log.h>
#include <filesystem>
#include <string>

#pragma warning(pop)

using namespace Memory::VP;

extern bool TweaksDevMode; // CommandLine.cpp, enabled in DEBUG build & with -dev command-line param

#define VERBOSE