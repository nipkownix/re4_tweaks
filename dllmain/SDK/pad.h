#pragma once
#include "basic_types.h"

enum class KEY_BTN : uint64_t
{
	KEY_FORWARD = 0x1,
	KEY_BACK = 0x2,
	KEY_RIGHT = 0x4,
	KEY_LEFT = 0x8,
	KEY_KAMAE = 0x10,
	KEY_RELOCKON = 0x20,
	KEY_RUN = 0x40,
	KEY_FIRE = 0x80,
	KEY_180TURN = 0x100,
	KEY_ASHLEY = 0x200,
	KEY_CK = 0x400,
	KEY_KAMAE_KNIFE = 0x800,
	KEY_ST = 0x1000,
	KEY_OPTION = 0x2000,
	KEY_C_U = 0x4000,
	KEY_C_D = 0x8000,
	KEY_Y = 0x10000,
	KEY_X = 0x20000,
	KEY_B = 0x40000,
	KEY_A = 0x80000,
	KEY_SSCRN = 0x100000,
	KEY_MAP = 0x200000,
	KEY_LB = 0x400000,
	KEY_RB = 0x800000,
	KEY_U = 0x1000000,
	KEY_D = 0x2000000,
	KEY_R = 0x4000000,
	KEY_L = 0x8000000,
	KEY_UDRL = 0xF000000,
	KEY_Z = 0x10000000,
	KEY_EV_CANCEL = 0x20000000,
	KEY_CANCEL = 0x40000000,
	KEY_OK = 0x80000000,
	KEY_LL = 0x100000000,
	KEY_LR = 0x200000000,
	KEY_LD = 0x400000000,
	KEY_LU = 0x800000000,
	KEY_C_R = 0x1000000000,
	KEY_C_L = 0x2000000000,
	KEY_PICK_UP = 0x4000000000,
	KEY_MAP_MENU = 0x8000000000,
	KEY_ACT_A = 0x10000000000,
	KEY_QUICK = 0x20000000000,
	KEY_ACT_B = 0x40000000000,
	KEY_MAP_ZOOM = 0x80000000000,
	KEY_PLUS = 0x100000000000,
	KEY_MINUS = 0x200000000000,
	KEY_1 = 0x400000000000,
	KEY_2 = 0x800000000000,
	KEY_KNIFE = 0x1000000000000,
	KEY_KNIFE2 = 0x2000000000000,
	KEY_CRANK = 0x4000000000000,
	KEY_SU = 0x10000000000000,
	KEY_SD = 0x20000000000000,
	KEY_SR = 0x40000000000000,
	KEY_SL = 0x80000000000000,
	KEY_SUDRL = 0xF0000000000000,
};

enum class InputDevices
{
	// 0 = Keyboard?
	// 1 = Mouse
	// 2 = Xinput Controller
	// 3 = Dinput Controller

	Keyboard,
	Mouse,
	XinputController,
	DinputController
};

enum class MouseAimingModes : uint8_t
{
	Classic,
	Modern
};

enum class keyConfigTypes : uint8_t
{
	TypeI,
	TypeII,
	TypeIII
};

struct PAD_MOTOR
{
	uint16_t flag_0;
	uint16_t delay_2;
	uint16_t time_4;
	uint16_t dummy_6;
	int level_8;
	int fade_C;
};
assert_size(PAD_MOTOR, 0x10);

enum JOY_BTN : uint32_t
{
	JOY_DPAD_LEFT = 0x1,
	JOY_DPAD_RIGHT = 0x2,
	JOY_DPAD_DOWN = 0x4,
	JOY_DPAD_UP = 0x8,
	JOY_RT = 0x10,
	JOY_RB = 0x20,
	JOY_LB = 0x40,
	JOY_LT = 0x80,
	JOY_A = 0x100,
	JOY_B = 0x200,
	JOY_X = 0x400,
	JOY_Y = 0x800,
	JOY_START = 0x1000,
	JOY_BACK = 0x2000,
	JOY_RS = 0x4000,
	JOY_LS = 0x8000,
	JOY_LS_LEFT = 0x10000,
	JOY_LS_RIGHT = 0x20000,
	JOY_LS_DOWN = 0x40000,
	JOY_LS_UP = 0x80000,
	JOY_RS_LEFT = 0x100000,
	JOY_RS_RIGHT = 0x200000,
	JOY_RS_DOWN = 0x400000,
	JOY_RS_UP = 0x800000,
};

struct JOY
{
	int leftStick_X_0;
	int leftStick_Y_4;
	int8_t rightStick_X_8;
	int8_t rightStick_Y_9;
	uint8_t triggerLeft_A;
	uint8_t triggerRight_B;
	uint8_t analogA_C;
	uint8_t analogB_D;
	int8_t err_E;
	JOY_BTN old_10;
	JOY_BTN on_14;
	JOY_BTN trg_18;
	JOY_BTN rel_1C;
	JOY_BTN rep_20;
	JOY_BTN rep2_24;
	int8_t rep_timer_28[32];
	int8_t rep2_timer_48[32];
	uint8_t motor_state_68;
	uint8_t pad_6A[3];
	PAD_MOTOR motor_6C[10];

	// extra fields added in Wii/UHD ver, not fully mapped out yet:
	uint8_t unk_10C[12];
	int pc_leftStick_X_118;
	int pc_leftStick_Y_11C;
	int8_t pc_rightStick_X_120;
	int8_t pc_rightStick_Y_121;
	__int16 pc_unk_A_122;
	uint8_t unk_124[4];
	int pc_buttonStates0_128;
	int pc_buttonsOn_12C;
	int pc_buttonStates2_130;
	uint8_t unk_134[312];
	int8_t wpad_rightStick_X_26C;
	int8_t wpad_rightStick_Y_26D;
};
assert_size(JOY, 0x270);

namespace bio4
{
	extern bool(__cdecl* KeyOnCheck_0)(KEY_BTN a1);
	extern void(__cdecl* KeyStop)(uint64_t un_stop_bit);
	extern bool(__cdecl* joyFireOn)();
}