#pragma once
#include "basic_types.h"

// These funcs are available via MathReimpl.cpp, so don't need bio4 namespace
void MTXConcat(const Mtx a, const Mtx b, Mtx out);
void MTXMultVec(const Mtx m, const Vec* v, Vec* out);
void MTXMultVecSR(const Mtx m, const Vec* v, Vec* out);

void VECAdd(const Vec* a, const Vec* b, Vec* out);
void VECScale(const Vec* a, Vec* out, float scale);
void VECSubtract(const Vec* a, const Vec* b, Vec* out);
float VECDotProduct(const Vec* a, const Vec* b);
