#pragma once

#include <stdint.h>

#define assert_size(mytype, size) \
static_assert( sizeof(mytype) <= (size), "Size of " #mytype " is greater than " #size "!" ); \
static_assert( sizeof(mytype) >= (size), "Size of " #mytype " is less than "    #size "!" )

struct Vec // aka tagVec, Point3d
{
public:
    float x, y, z;
    Vec() : x(0.f), y(0.f), z(0.f) {}
    Vec(float x, float y, float z) : x(x), y(y), z(z) {}

    // Overload syntactic sugar
    Vec operator+(const Vec& vector) const
    {
        return Vec(x + vector.x, y + vector.y, z + vector.z);
    }

    void operator+=(const Vec& vector)
    {
        x += vector.x;
        y += vector.y;
        z += vector.z;
    }

    Vec operator-(const Vec& vector) const
    {
        return Vec(x - vector.x, y - vector.y, z - vector.z);
    }

    void operator-=(const Vec& vector)
    {
        x -= vector.x;
        y -= vector.y;
        z -= vector.z;
    }

    Vec operator*(const Vec& vector) const
    {
        return Vec(x * vector.x, y * vector.y, z * vector.z);
    }

    Vec operator*(const float& value) const
    {
        return Vec(x * value, y * value, z * value);
    }

    void operator*=(const float& value)
    {
        x *= value;
        y *= value;
        z *= value;
    }

    Vec operator/(const Vec& vector) const
    {
        return Vec(x / vector.x, y / vector.y, z / vector.z);
    }

    Vec operator/(const float& value) const
    {
        return Vec(x / value, y / value, z / value);
    }

    void operator/=(const float& value)
    {
        x /= value;
        y /= value;
        z /= value;
    }

    void normalize(); // MathReimpl.cpp
};
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
