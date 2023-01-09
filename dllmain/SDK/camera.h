#pragma once
#include "basic_types.h"

struct CAMERA_POINT
{
	Vec Campos_0;
	Vec Target_C;
	float Roll_18;
	float Fovy_1C;
};
assert_size(CAMERA_POINT, 0x20);

class CAMERA
{
public:
	Mtx mat_0;
	Mtx v_mat_30;
	int ProjType_60;
	Mtx44 ProjMat_64;
	CAMERA_POINT CamPoint_A4;
	Vec Up_C4;
	Vec Look_D0;
	Vec Right_DC;
	Vec Ang_E8;
	float Distance_F4;
};
assert_size(CAMERA, 0xF8);

struct ATTACH_CAMERA // maybe meant to be in cam_ctrl.h
{
	uint8_t cam_jnt_0[5];
	uint8_t cam_flag_5;
	uint8_t cam_int_6;
	Mtx* p_mat_8;
	Mtx mat_C;
	Vec camera_data_3C[5];
	uint16_t history_78[5][3];
};
assert_size(ATTACH_CAMERA, 0x98);

// might be part of cam_extra.h
class cCamera
{
public:
	CAMERA camData_4;

	virtual ~cCamera() = 0;
	virtual void move() = 0;
};

namespace bio4 {
	extern void(__cdecl* CameraCurrentProjection)();
	extern void(__cdecl* QuakeExec)(uint32_t No, uint32_t Delay, int Time, float Scale, uint32_t Axis);
};
