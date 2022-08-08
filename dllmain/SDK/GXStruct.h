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
