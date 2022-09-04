#pragma once
#include "basic_types.h"
#include "camera.h"
#include "light.h"
#include "item.h"
#include "objWep.h"
#include "puzzle.h"

enum SS_OPEN_FLAG
{
	SS_OPEN_NULL = 0x0,
	SS_OPEN_NORMAL = 0x1,
	SS_OPEN_MAP = 0x2,
	SS_OPEN_PZZL = 0x4,
	SS_OPEN_FLAG_UNK8 = 0x8,
	SS_OPEN_SHOP = 0x10,
	SS_OPEN_TERM = 0x20,
	SS_OPEN_FILE = 0x40,
	SS_OPEN_ITEM = 0x80,
	SS_OPEN_CAP = 0x100,
};

enum SS_ATTR_FLAG
{
	SS_ATTR_NULL = 0x0,
	SS_ATTR_EVENT = 0x1,
	SS_ATTR_BOAT = 0x2,
	SS_ATTR_ASHLEY = 0x4,
};

struct SUB_SCREEN
{
	struct wep_que
	{
		uint32_t be_flag_0;
		uint16_t no_4;
		uint16_t type_6;
	};
	assert_size(SUB_SCREEN::wep_que, 8);

	char filename_0[32];
	int Wsize_20;
	int Hsize_24;
	uint8_t Loop_28;
	uint8_t unk_29[3];
	SS_OPEN_FLAG open_flag_2C;
	SS_ATTR_FLAG attr_flag_30;
	uint32_t close_flag_34;
	uint32_t model_flag_38;
	float wait_cnt_3C;
	bool item_get_flag_40;
	bool trans_off_41;
	uint8_t unk_42[2];
	uint32_t str_id_44;
	void* scrn_out_func_48;
	uint32_t backup_Flags_STOP_4C;
	uint32_t backup_Flags_DISP_50;
	CAMERA camera_bak_54;
	Mtx pl_mat_14C;
	Mtx sub_mat_17C;
	int8_t stage_no_1AC;
	uint8_t unk_1AD[1];
	uint16_t room_no_1AE;
	int8_t scope_flag_1B0;
	int8_t binocular_flag_1B1;
	int8_t suspend_flag_1B2;
	int8_t swep_flag_1B3;
	int8_t jacket_flag_1B4;
	uint8_t unk_1B5[3];
	uint32_t pieceCommand_field_1B8;
	void* pG_0x3C_value;
	uint32_t dataCurOffset_1C0;
	uint32_t execPzzlOffset_1C4;
	uint32_t dataStartOffset_1C8;
	uint32_t cmmnCurOffset_1CC;
	uint32_t pzzlCurOffset_1D0;
	uint8_t* dataStartPointer_1D4;
	uint8_t* cmmnPointer_1D8;
	uint8_t* pzzlPointer_1DC;
	uint8_t* ssPzzlMain_1E0;
	uint8_t* pItemDat_1E4;
	uint8_t* pTermDat_1E8;
	uint8_t* pTermMes_1EC;
	uint8_t* pMapDat_1F0;
	uint8_t* pMapObj_1F4;
	uint8_t* pFileDat_1F8;
	uint8_t* pCapDat_1FC;
	uint8_t* pShopDat_200;
	uint8_t* pTelDat_204;
	uint8_t* pTplDat_208;
	uint8_t* pWepDat_20C;
	uint32_t eff_addr_210;
	uint32_t uwf_addr_214;
	cLight* p_light[8];
	uint8_t* pExamDat_238;
	uint8_t* pItemBin_23C;
	uint8_t* pItemTpl_240;
	cItem* p_exam_item_244;
	cModel* p_exam_model_248;
	int8_t wep_rno_24C;
	int8_t wep_idx_24D;
	int16_t wep_cnt_24E;
	SUB_SCREEN::wep_que wep_que_250[2];
	int8_t menu_no_260;
	int8_t menu_next_261;
	int8_t menu_old_262;
	uint8_t cursor_mode_263;
	uint8_t cursor_flag_264;
	uint8_t alpha_flag_265;
	int16_t alpha_cnt_266;
	int8_t cmd_menu_no_268;
	uint8_t unk_269[3];
	Mtx pl_mat_map_26C;
	int8_t map_help_29C;
	int8_t map_obj_num_29D;
	int8_t floor_no_29E;
	uint8_t unk_29F[1];
	int timer_2A0;
	int cursor_model_no_2A4;
	int8_t zoom_flag_2A8;
	int8_t Key_disable_2A9;
	int8_t board_size_2AA;
	int8_t board_next_2AB;
	pzlPlayer* puzzlePlayer_2AC;
	uint8_t back2_2B0;
	uint8_t unk_2B1[3];
	struct ID_UNIT* pCmdMenu_2B4[16];
	int8_t cmd_num_2F4;
	uint8_t unk_2F5[1];
	uint16_t get_item_id_2F6;
	uint16_t get_item_num_2F8;
	uint8_t unk_2FA[2];
	cItem* p_get_item_2FC;
	class SSCRN_ITEM* item_300;
	class SSCRN_MAP* map_304;
	class SSCRN_FILE* file_308;
	class SSCRN_CAP* cap_30C;
	class SSCRN_SHOP* shop_310;
	class Merchant* merchant_314;
	uint32_t opeMdtNo_318;
	int sndId_31C;
	cObjWep* pObjWep_320;
	Vec posBak_324;
	Vec angBak_330;
	bool opeCancelFlag_33C;
	uint8_t unk_33D[3];
	uint32_t opeFlag_340;
	int8_t WhoNoOld_344;
	uint8_t unk_345[3];
	struct OSModuleHeader* p_module_348;
	uint32_t mapModeSelect_flags_34C;
	uint8_t pieceCommand_field_350;
	uint8_t pieceCommand_field_351;
	uint8_t unk_352[2];
	float field_354;
};
assert_size(SUB_SCREEN, 0x358);

extern SUB_SCREEN* SubScreenWk; // extern from Game.cpp

namespace bio4
{
	extern bool(__cdecl* SubScreenOpen)(SS_OPEN_FLAG open_flag, SS_ATTR_FLAG attr_flag);
}
