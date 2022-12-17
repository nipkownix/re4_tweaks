#pragma once

struct ROOM_SAVE_DATA
{
	uint16_t RoomNo_0;
	uint8_t passed_flg_2;
	uint8_t _padding_3;
	uint32_t save_flg_4[1];
	uint32_t item_flg_8[4];
	uint32_t item_find_flg_18[4];
	uint16_t EtcModelFlg[64];
	uint32_t BgmTable[6];
	uint32_t StrTable[6];
};

struct ROOM_SAVE_HEAD
{
	uint32_t data_size_0;
	uint32_t room_num_4;
	uint32_t padding_8[2];
	//ROOM_SAVE_DATA data_10[];
};

class cRoomData;
typedef ROOM_SAVE_DATA* (__fastcall* cRoomData__getRoomSavePtr_Fn)(cRoomData* thisptr, void* unused, uint16_t room_no);
extern cRoomData__getRoomSavePtr_Fn cRoomData__getRoomSavePtr;

class cRoomData
{
public:
	uint16_t m_RoomNum;
	uint16_t m_SaveNum[5];
	uint16_t m_CtrlFlag_C;
	uint16_t m_RoomNo_E;
	struct OSModuleHeader* m_pModule_10;
	void* m_pModule_bss_14;
	void* m_pModule_bss_bak_18;
	ROOM_SAVE_HEAD* m_pRoomSaveHead_1C;
	ROOM_SAVE_DATA* m_pRoomSaveData_20;
	uint16_t m_RelNo_24;

	inline ROOM_SAVE_DATA* getRoomSavePtr(uint16_t room_no)
	{
		return cRoomData__getRoomSavePtr(this, nullptr, room_no);
	}
};

extern cRoomData* RoomData;
