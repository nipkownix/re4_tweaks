#pragma once

typedef struct _GXColor
{
	uint8_t r, g, b, a;
} GXColor;
assert_size(GXColor, 0x4);

typedef struct _GXTexObj
{
	uint32_t texObjNum_0;
	uint32_t size_4; // used to tell type of tex obj, either size or maybe ID?

	// rest are unused?
	uint8_t unused_8[0x18];
} GXTexObj;
assert_size(GXTexObj, 0x20);

enum class GXAnisotropy : uint8_t
{
	GX_ANISO_1,
	GX_ANISO_2,
	GX_ANISO_4
};
assert_size(GXAnisotropy, 1);

enum class GXTevScale : uint32_t
{
	GX_CS_SCALE_1,
	GX_CS_SCALE_2,
	GX_CS_SCALE_4,
	GX_CS_DIVIDE_2,
	GX_MAX_TEVSCALE
};
assert_size(GXTevScale, 4);

enum class GXFogType : uint32_t
{
	GX_FOG_NONE = 0x0,
	GX_FOG_PERSP_LIN = 0x2,
	GX_FOG_PERSP_EXP = 0x4,
	GX_FOG_PERSP_EXP2 = 0x5,
	GX_FOG_PERSP_REVEXP = 0x6,
	GX_FOG_PERSP_REVEXP2 = 0x7,
	GX_FOG_ORTHO_LIN = 0xA,
	GX_FOG_ORTHO_EXP = 0xC,
	GX_FOG_ORTHO_EXP2 = 0xD,
	GX_FOG_ORTHO_REVEXP = 0xE,
	GX_FOG_ORTHO_REVEXP2 = 0xF,
	GX_FOG_LIN = 0x2,
	GX_FOG_EXP = 0x4,
	GX_FOG_EXP2 = 0x5,
	GX_FOG_REVEXP = 0x6,
	GX_FOG_REVEXP2 = 0x7,
};
assert_size(GXFogType, 4);
