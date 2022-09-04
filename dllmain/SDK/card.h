#pragma once
#include "basic_types.h"

namespace bio4 {
	extern bool(__cdecl* CardCheckDone)();
	extern void(__cdecl* CardSave)(uint8_t terminal_no, uint8_t attr);
};
