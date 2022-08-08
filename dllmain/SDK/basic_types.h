#pragma once

#include <stdint.h>

#define assert_size(mytype, size) \
static_assert( sizeof(mytype) <= (size), "Size of " #mytype " is greater than " #size "!" ); \
static_assert( sizeof(mytype) >= (size), "Size of " #mytype " is less than "    #size "!" )

typedef struct tagVec // PS2 seems to refer to these via the tag names
{
	float x, y, z;
} Vec, *VecPtr, Point3d, *Point3dPtr;
assert_size(Vec, 0xC);

struct SVEC
{
	int16_t x, y, z;
};
assert_size(SVEC, 0x6);

typedef struct tagQuaternion
{
	float x, y, z, w;
} Quaternion, *QuaternionPtr;
assert_size(Quaternion, 0x10);

typedef float Mtx[3][4];
assert_size(Mtx, 0x30);
typedef float Mtx44[4][4];
assert_size(Mtx44, 0x40);
typedef float ROMtx[4][3];
assert_size(ROMtx, 0x30);

#include "GXStruct.h"

typedef uint16_t ITEM_ID;
assert_size(ITEM_ID, 0x2);
