#pragma once
#include "basic_types.h"

class cDataSwap
{
	uint32_t m_be_flag_0;
	uint32_t m_SwapMaddr_4;
	uint32_t m_SwapAaddr_8;
	uint32_t m_SwapSize_C;
	uint32_t m_CurHeapNo_10;
	void* m_alloc_addr_14;
};
assert_size(cDataSwap, 0x18);
