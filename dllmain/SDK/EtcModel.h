#pragma once
#include "basic_types.h"

struct ETS_DATA
{
	uint16_t EtcModelId_0;

	// normally uint16_t, but game only allows values 0x40 and below
	// we patch game to read it as byte, which gives us a free byte to use as flag :)
	union
	{
		uint16_t EtcModelNo_2; // vanilla
		struct
		{
			uint8_t expansion_EtcModelNo_2;
			uint8_t expansion_Flag_3;
		};
	};
	union
	{
		Vec scale_4; // vanilla
		struct
		{
			SVEC expansion_PercentScaleModel_4;
			SVEC expansion_PercentScaleCollision_A;
		};
	};
	Vec rotation_10;
	Vec position_1C;
};
assert_size(ETS_DATA, 0x28);
