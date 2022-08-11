#pragma once
#include "basic_types.h"

extern uint32_t* roomInfoAddr;

struct CRoomInfo
{
	uint16_t flag_0;
	uint16_t roomNo_2;
	Vec pos_4;
	float r_10;

	// These start as offsets in the roomInfo file
	// But opening area jump menu converts them to pointers...
	uint32_t name_14;
	uint32_t person_18;
	uint32_t person2_1C;

	char* getName()
	{
		if ((uintptr_t)this < (uintptr_t)name_14)
			return (char*)name_14;
		if (roomInfoAddr)
			return (char*)(*roomInfoAddr + name_14);
		// can't work it out, offset needs to be added to _start_ of cRoomJmp pData...
		return nullptr; 
	}

	char* getPerson()
	{
		if ((uintptr_t)this < (uintptr_t)person_18)
			return (char*)person_18;
		if (roomInfoAddr)
			return (char*)(*roomInfoAddr + person_18);
		// can't work it out, offset needs to be added to _start_ of cRoomJmp pData...
		return nullptr; 
	}

	char* getPerson2()
	{
		if ((uintptr_t)this < (uintptr_t)person2_1C)
			return (char*)person2_1C;
		if (roomInfoAddr)
			return (char*)(*roomInfoAddr + person2_1C);
		// can't work it out, offset needs to be added to _start_ of cRoomJmp pData...
		return nullptr; 
	}
};

struct cRoomJmp_stage
{
	uint32_t nData_0;
	CRoomInfo pData[1];

	CRoomInfo* GetRoom(int idx)
	{
		return &pData[idx];
	}
};

struct cRoomJmp_data
{
	uint32_t nData_0;
	uint32_t pData_4[1]; // [nData]

	cRoomJmp_stage* GetStage(int stageNo)
	{
		return (cRoomJmp_stage*)(((uintptr_t)this) + pData_4[stageNo]);
	}

	static cRoomJmp_data* get()
	{
		if (roomInfoAddr)
			return (cRoomJmp_data*)*roomInfoAddr;
		return nullptr;
	}
};
