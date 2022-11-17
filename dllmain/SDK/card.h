#pragma once
#include "basic_types.h"
#include "cDataSwap.h"

#define MAX_SAVE_SLOTS 20

struct CARD_SLOT_INFO_UnkStruct
{
	uint32_t flag_0[5];
};
assert_size(CARD_SLOT_INFO_UnkStruct, 0x14);

struct CARD_SLOT_INFO
{
	void* CARD_workarea_0;
	uint32_t field_4;
	uint32_t flags_8;
	uint32_t field_C;
	uint32_t field_10;
	uint8_t unk_14[8];
	CARD_SLOT_INFO_UnkStruct unk_struct[16];
	uint8_t unk_15C[52];
};
assert_size(CARD_SLOT_INFO, 0x190);

struct STR_INFO
{
	uint32_t snd_id_0;
	char vol_4;
	uint8_t pad5[3];
};
assert_size(STR_INFO, 8);

class cCard
{
public:
	enum class Routine0 : uint8_t
	{
		SlotSelect = 0,
		InSlotCheck = 1,
		DataSelect = 2,
		LoadSave = 3,
		Exit = 4,
		ErrorDisp = 5,
		CardFormat = 6,
		FileDelete = 7,
		CreateSysFile = 8,
	};

	uint32_t m_NeedMemSize_0;
	Routine0 m_Rno0_4;
	uint8_t m_Rno1_5;
	uint8_t m_Rno2_6;
	uint8_t m_Rno3_7;
	uint32_t m_SPFbak_8;
	uint32_t m_DPFbak_C;
	uint32_t m_Status_10;
	uint32_t m_aMode_14;
	uint32_t m_mMode_18;
	uint8_t m_SlotNo_1C;
	char m_SaveNo_1D;
	uint8_t m_SaveDataNum_1E;
	uint8_t m_RetryCtr_1F;
	int m_ErrCode_20;
	uint8_t unk_24[4];
	CARD_SLOT_INFO m_Slot_28[3];
	struct CARD_SAVE_DATA* m_pSaveData_4D8;
	uint32_t m_SaveSize_4DC;
	uint32_t field_4E0;
	void* ptr_4E4;
	char* system_savedata_workarea_4E8;
	uint32_t system_savedata_length_4EC;
	uint32_t field_4F0;
	uint32_t memcard_das_data_4F4;
	void* ptr_4F8;
	float m_Timer_4FC;
	float m_StrTimer_500;
	int m_ResultCode_504;
	STR_INFO m_StrInfo_508[4];
	uint32_t m_SndId_528;
	bool m_FormatDone_52C;
	bool m_ExitReq_52D;
	bool m_DispReq_52E;
	bool m_isSePause_52F;
	char m_Name_530[64];
	cDataSwap m_DataSwap_570;
	int m_Width_bak_588;
	uint32_t field_58C;
	void* ptr_sys_savedata_workareaPlus793_590;
	uint8_t unk_594[4];
	uint32_t field_598;
	uint32_t field_59C;
};
assert_size(cCard, 0x5A0);

struct cCard_MainLoop_tbl
{
	void(__fastcall* load_0)(cCard* thisptr, void* unused);
	void(__thiscall* save_4)(cCard* thisptr, void* unused);
	void(__thiscall* start_8)(cCard* thisptr, void* unused);
};
assert_size(cCard_MainLoop_tbl, 0xC);

namespace bio4 {
	extern bool(__cdecl* CardCheckDone)();
	extern bool(__cdecl* CardLoad)(uint8_t a1);
	extern void(__cdecl* CardSave)(uint8_t terminal_no, uint8_t attr);
};
