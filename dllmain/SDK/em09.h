// filename is likely wrong...
#pragma once
#include "cloth.h"
#include "obj.h"
#include "player.h"

// Unused Tyrant enemy, seems to be based on REmake
// UHD contains code & data for it, but code doesn't seem to match up with REmake em09/em0b exactly - maybe was rewritten/cut-down when originally added to RE4
class cEm09 : public cEm
{
public:
	uint32_t Be_flg_408;
	float Timer_mb_40C;
	YARARE_INFO YarareTbl_410[5];
	MOTION_INFO Sub_mot_514;
	float Pl_dir_mb_5E4;
	float Pl_rot_mb_5E8;
	uint8_t unk_5EC[8];
	float field_5F4;
	float Go_rot_mb_5F8;
	float L_go_mb_5FC;
	Vec Pl_pos_mb_600;
	uint8_t unk_60C[12];
	Vec Go_pos_mb_618;
	cPlayer* cPlayer_624;
	float Neck_dir_y_mbunsure_628;
	uint8_t Atk_ck_mb_62C;
};
assert_size(cEm09, 0x630);
