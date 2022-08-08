#pragma once
#include "basic_types.h"
#include "model.h"

class cMotBase
{
public:
	cModel* pMod_0;
	Vec pos_4;
	Vec ang_10;
	Vec pos_old_1C;
	Vec ang_old_28;
	uint8_t hokan_34;
	uint8_t dummy61_35;
	uint8_t dummy62_36;
	uint8_t dummy63_37;
};
assert_size(cMotBase, 0x38);
