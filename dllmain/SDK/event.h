#pragma once
#include "basic_types.h"

struct DatTblWork
{
	char Name_0[48];
	uint8_t FlagBe8_30;
	uint8_t Etc_31;
	int16_t Count_32;
	uint8_t* Dat_34;
	void* Unit_38;
};
assert_size(DatTblWork, 0x3C);

class DatTbl
{
public:
	int NumDatTbl_0;
	DatTblWork* PDatTbl_4;
};
assert_size(DatTbl, 8);
