#pragma once
#include "cUnit.h"

class cCoord : public cUnit
{
public:
	Mtx mat_C;
	Mtx l_mat_3C;
	cCoord* pParent_6C;
	Vec world_70;
	Vec world_old_7C;
	Vec world_old2_88;
	Vec pos_94;
	Vec ang_A0;
	Vec scale_AC;
	Vec r_scale_B8;

	virtual void matUpdate() = 0;
};
assert_size(cCoord, 0xC4);
