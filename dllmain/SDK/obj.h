#pragma once
#include "basic_types.h"
#include "model.h"
#include "cManager.h"

class cObj : public cModel
{
public:
	uint32_t m_StatusFlag_324;
	int32_t m_BlockNo_328;
};
assert_size(cObj, 0x32C);

class cObjMgr : public cManager<cObj>
{
	uint32_t m_nMaxActiveWork_1C; // TODO: unsure if this is part of cManager or maybe the cxxxMgr (cEmMgr etc) classes that inherit it
};
assert_size(cObjMgr, 0x20);
