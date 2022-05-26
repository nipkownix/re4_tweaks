// Reimplemented versions of certain game functions, for improved framerate & other bonuses

#include <iostream>
#include <intrin.h>
#include <xmmintrin.h>
#include "stdafx.h"
#include "dllmain.h"
#include "Settings.h"
#include "Game.h"

void* j_PSVECAdd;
void(__cdecl* PSVECAdd)(const Vec*, const Vec*, Vec*);
void* j_PSVECSubtract;
void(__cdecl* PSVECSubtract)(const Vec*, const Vec*, Vec*);
void* j_PSVECScale;
void(__cdecl* PSVECScale)(const Vec*, Vec*, float);
void* j_PSVECNormalize;
void(__cdecl* PSVECNormalize)(const Vec*, Vec*);
void* j_PSVECDotProduct;
float(__cdecl* PSVECDotProduct)(const Vec*, const Vec*);
void* j_PSVECCrossProduct;
void(__cdecl* PSVECCrossProduct)(const Vec*, const Vec*, Vec*);
void* j_PSVECMag;
float(__cdecl* PSVECMag)(const Vec*);
void* j_PSVECSquareDistance;
float(__cdecl* PSVECSquareDistance)(const Vec* a, const Vec* b);
void* j_GetDistance;
float(__cdecl* GetDistance)(const Vec* a, const Vec* b);
void* j_PSVECSquareMag;
float(__cdecl* PSVECSquareMag)(const Vec*);
void* j_PSVECDistance;
float(__cdecl* PSVECDistance)(const Vec* a, const Vec* b);
void* j_GetDistance3;
float(__cdecl* GetDistance3)(const Vec* a, const Vec* b);
void* j_PSMTXMultVec;
void(__cdecl* PSMTXMultVec)(const Mtx, const Vec*, Vec*);
void* j_PSMTXMultVecSR;
void(__cdecl* PSMTXMultVecSR)(const Mtx, const Vec*, Vec*);
void* j_SQRTF;
double(__cdecl* SQRTF)(float);

typedef void(*penClothAtCkParallel_Fn)(Vec*); // func uses custom call convention, params likely incorrect here
penClothAtCkParallel_Fn penClothAtCkParallel;

// forward defs
double SQRTF_new(float in);
void VECSubtract(const Vec* a, const Vec* b, Vec* out);
void VECNormalize(const Vec* vec, Vec* out);
void MTXMultVec(const Mtx m, const Vec* v, Vec* out);

// Uncomment to allow comparing values between original CkParallel & our reimpl.
// (see a2_bad/a3_bad usages inside penClothAtCkParallel_hook)
// Only works with 1.1.0 atm, and by default re4_tweaks will always have different values, due to using SSE/AVX which is less precise than x87
// Use /arch:IA32 when using this option to make sure values actually match up!
// (C/C++ properties -> Code Generation -> Enable Enhanced Instruction Set)
//#define TEST_CKPARALLEL_REIMPL 1

Vec a3_orig;
Vec a2_orig;

Vec* CkParallel_a3 = nullptr;
Vec* CkParallel_a2 = nullptr;
PenClothAtTable* CkParallel_a1 = nullptr;

void penClothAtCkParallel_hook(Vec* a3_ignore)
{
	PenClothAtTable* a1 = CkParallel_a1;
	Vec* a2 = CkParallel_a2;
	Vec* a3 = CkParallel_a3;

	if (!a1)
		return;

#ifdef TEST_CKPARALLEL_REIMPL
	Vec prev_result_a2 = *a2;
	Vec prev_result_a3 = *a3;

	Vec a = a3_orig;
	Vec b = a2_orig;
#else
	Vec a = *a3;
	Vec b = *a2;
#endif

	Vec src;
	VECSubtract(&a, &b, &src);

	// original code overwrote the returned values in src with its own redundant a - b calculation here instead, weird

	if (0.0 == src.x && 0.0 == src.y && src.z == 0.0)
		src.y = 1.0;

	Vec unit;
	VECNormalize(&src, &unit);
	float v37 = src.y * src.y + src.x * src.x + src.z * src.z;
	if (SQRTF_new(v37) == 0.0f)
	{
		for (int i = 0; i < a1->count_0; i++)
		{
			PenClothAtData* v12 = &a1->data_ptr_4[i];
			float v43 = v12->field_7C * v12->field_7C;
			if (v12->field_0)
			{
				Vec dst;
				Vec v63;
				MTXMultVec(v12->mtx_34, &b, &dst);
				MTXMultVec(v12->mtx_34, &a, &v63);
				Vec v36;
				v36.x = v63.x - dst.x;
				v36.y = v63.y - dst.y;
				v36.z = v63.z - dst.z;
				float v58 = v36.z * v36.z + v36.x * v36.x;
				float v40 = -dst.x * v36.x + -dst.z * v36.z;
				float v51 = dst.x * dst.x + dst.z * dst.z;
				float v41 = v63.x * v63.x + v63.z * v63.z;
				if (v51 <= v43
					|| v41 <= v43
					|| (v40 >= 0.0 && v58 > v40))
				{
					v40 = v40 / v58;
					VECNormalize(&v36, &v36);
					Vec v67;
					v67.x = v36.x * v40 + dst.x;
					v67.y = v36.y * v40 + dst.y;
					v67.z = v36.z * v40 + dst.z;

					if (v67.y >= 0.0 && v12->field_80 >= v67.y)
					{
						v67.y = 0.0;
						float v59 = v67.z * v67.z + v67.x * v67.x;
						if (v59 < v43)
						{
							if (0.0 != v67.x || 0.0 != v67.y || 0.0 != v67.z)
							{
								VECNormalize(&v67, &v67);
								float v60 = (float)(v12->field_7C - SQRTF_new(v59));
								v67.x = v60 * v67.x;
								v67.y = v60 * v67.y;
								v67.z = v60 * v67.z;
								dst.x = v67.x + dst.x;
								dst.y = v67.y + dst.y;
								dst.z = v67.z + dst.z;
								v63.x = v67.x + v63.x;
								v63.y = v67.y + v63.y;
								v63.z = v67.z + v63.z;
								MTXMultVec(v12->mtx_4, &dst, &b);
								MTXMultVec(v12->mtx_4, &v63, &a);
							}
						}
					}
				}
				continue;
			}
			float v54 = (v12->field_64.z - b.z) * src.z + ((v12->field_64.y - b.y) * src.y + ((v12->field_64.x - b.x) * src.x));
			float v55 = float(v54 * (1.0 / v37));
			Vec v68;
			v68.x = unit.x * v55 + b.x - v12->field_64.x;
			v68.y = unit.y * v55 + b.y - v12->field_64.y;
			v68.z = unit.z * v55 + b.z - v12->field_64.z;
			float v56 = v68.y * v68.y + v68.x * v68.x + v68.z * v68.z;

			if (v43 <= (double)v56)
				continue;

			Vec tmp;
			tmp.x = v12->field_64.x - b.x;
			tmp.y = v12->field_64.y - b.y;
			tmp.z = v12->field_64.z - b.z;
			float dist_b = tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z;

			tmp.x = v12->field_64.x - a.x;
			tmp.y = v12->field_64.y - a.y;
			tmp.z = v12->field_64.z - a.z;
			float dist_a = tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z;

			// weird if statement tree so that NaNs can pass without continuing, like the original func
			if (dist_b > v43)
				if (dist_a > v43)
					if ((v54 < 0.0f || v37 <= v54))
						continue;

			if (0.0 == v68.x && 0.0 == v68.y && v68.z == 0.0)
				v68.y = 1.0;
			VECNormalize(&v68, &v68);
			float v57 = (float)(v12->field_7C - SQRTF_new(v56));
			v68.x = v57 * v68.x;
			v68.y = v57 * v68.y;
			v68.z = v57 * v68.z;
			b.x = v68.x + b.x;
			b.y = v68.y + b.y;
			b.z = v68.z + b.z;
			a.x = v68.x + a.x;
			a.y = v68.y + a.y;
			a.z = v68.z + a.z;
		}
		a2->x = b.x;
		a2->y = b.y;
		a2->z = b.z;
		a3->x = a.x;
		a3->y = a.y;
		a3->z = a.z;
	}

#ifdef TEST_CKPARALLEL_REIMPL
	bool a2_bad = false;
	bool a3_bad = false;
	if (prev_result_a2.x != a2->x || prev_result_a2.y != a2->y || prev_result_a2.z != a2->z)
		if (a2->x == a2->x && a2->y == a2->y && a2->z == a2->z)
			a2_bad = true;
	if (prev_result_a3.x != a3->x || prev_result_a3.y != a3->y || prev_result_a3.z != a3->z)
		if (a3->x == a3->x && a3->y == a3->y && a3->z == a3->z)
			a3_bad = true;

	if (a2_bad || a3_bad)
		a2_bad = a3_bad; // breakpoint here to test...
#endif
}

#ifdef TEST_CKPARALLEL_REIMPL
void CkParallel_BackupVecs()
{
	a2_orig = *CkParallel_a2;
	a3_orig = *CkParallel_a3;

	// breakpoint these to test for NaN beforehand...
	if (a2_orig.x != a2_orig.x || a2_orig.y != a2_orig.y || a2_orig.z != a2_orig.z)
		a2_orig.x = a2_orig.x;
	if (a3_orig.x != a3_orig.x || a3_orig.y != a3_orig.y || a3_orig.z != a3_orig.z)
		a3_orig.x = a3_orig.x;
}
#endif

__declspec(naked) void penClothAtCkParallel_trampoline()
{
	// penClothAtCkParallel uses custom calling convention (fastcall sadly doesn't work)
	// need to copy pointers out of registers first so we can use them...
	__asm
	{
#ifndef TEST_CKPARALLEL_REIMPL
		mov al, [cfg.bEnableReimplementedCloth]
		cmp al, 0
		je originalCode
#endif

		mov eax, [esp + 4]
		mov[CkParallel_a3], eax
		mov[CkParallel_a2], ecx
		mov[CkParallel_a1], edx
#ifndef TEST_CKPARALLEL_REIMPL
		jmp penClothAtCkParallel_hook
#else
		call CkParallel_BackupVecs

		mov edx, [CkParallel_a1]
		mov ecx, [CkParallel_a2]
		mov eax, [CkParallel_a3]
		mov[esp + 4], eax

		// jmp to orig func first
#endif
	originalCode:
		// code we overwrote with jmp..
		push ebp
		mov ebp, esp
		sub esp, 0x9C
		jmp penClothAtCkParallel
	}
}

__m128 DotProduct_SSE1(__m128 a, __m128 b)
{
	__m128 r1 = _mm_mul_ps(a, b);
	__m128 shuf = _mm_shuffle_ps(r1, r1, _MM_SHUFFLE(2, 3, 0, 1));
	__m128 sums = _mm_add_ps(r1, shuf);
	shuf = _mm_movehl_ps(shuf, sums);
	sums = _mm_add_ss(sums, shuf);
	return _mm_shuffle_ps(sums, sums, 0); // copies [0] to all elements
}

inline float DotProduct_ss(__m128 a, __m128 b)
{
	return (a.m128_f32[0] * b.m128_f32[0]) + (a.m128_f32[1] * b.m128_f32[1]) + (a.m128_f32[2] * b.m128_f32[2]) + (a.m128_f32[3] * b.m128_f32[3]);
}

// SSE1 seems faster than compiler VECNormalize & SSE41!
void VECNormalize_SSE1(const Vec* vec, Vec* out)
{
	const __m128 tmp = _mm_set_ps(0.f, vec->z, vec->y, vec->x);
	const __m128 outp = _mm_div_ps(tmp, _mm_sqrt_ps(DotProduct_SSE1(tmp, tmp)));

	out->x = outp.m128_f32[0];
	out->y = outp.m128_f32[1];
	out->z = outp.m128_f32[2];
}

void VECNormalize(const Vec* vec, Vec* out)
{
	float w = sqrt(vec->x * vec->x + vec->y * vec->y + vec->z * vec->z);
	out->x = vec->x / w;
	out->y = vec->y / w;
	out->z = vec->z / w;
}

float VECDotProduct(const Vec* a, const Vec* b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

// compiler made much faster version of MTXMultVec/VECScale/VECSubtract than handmade SSE vers...

void MTXMultVec(const Mtx m, const Vec* v, Vec* out)
{
	const __m128 tmpB = { v->x, v->y, v->z, 1.f };

	out->x = DotProduct_ss(tmpB, { m[0][0], m[0][1], m[0][2], m[0][3] });
	out->y = DotProduct_ss(tmpB, { m[1][0], m[1][1], m[1][2], m[1][3] });
	out->z = DotProduct_ss(tmpB, { m[2][0], m[2][1], m[2][2], m[2][3] });
}

void MTXMultVecSR(const Mtx m, const Vec* v, Vec* out)
{
	const __m128 tmpB = { v->x, v->y, v->z, 1.f };

	out->x = DotProduct_ss(tmpB, { m[0][0], m[0][1], m[0][2], 0.0f });
	out->y = DotProduct_ss(tmpB, { m[1][0], m[1][1], m[1][2], 0.0f });
	out->z = DotProduct_ss(tmpB, { m[2][0], m[2][1], m[2][2], 0.0f });
}

void VECAdd(const Vec* a, const Vec* b, Vec* out)
{
	out->x = a->x + b->x;
	out->y = a->y + b->y;
	out->z = a->z + b->z;
}

void VECSubtract(const Vec* a, const Vec* b, Vec* out)
{
	out->x = a->x - b->x;
	out->y = a->y - b->y;
	out->z = a->z - b->z;
}

void VECScale(const Vec* a, Vec* out, float scale)
{
	out->x = a->x * scale;
	out->y = a->y * scale;
	out->z = a->z * scale;
}

void VECCrossProduct_SSE1(const Vec* a, const Vec* b, Vec* axb)
{
	const __m128 tmpA = _mm_set_ps(1.f, a->z, a->y, a->x);
	const __m128 tmpB = _mm_set_ps(1.f, b->z, b->y, b->x);

	const __m128 a_yzx = _mm_shuffle_ps(tmpA, tmpA, _MM_SHUFFLE(3, 0, 2, 1));
	const __m128 b_yzx = _mm_shuffle_ps(tmpB, tmpB, _MM_SHUFFLE(3, 0, 2, 1));
	const __m128 c = _mm_sub_ps(_mm_mul_ps(tmpA, b_yzx), _mm_mul_ps(a_yzx, tmpB));
	const __m128 result = _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));

	axb->x = result.m128_f32[0];
	axb->y = result.m128_f32[1];
	axb->z = result.m128_f32[2];
}

// slightly faster than original, slower than SSE1
void VECCrossProduct(const Vec* a, const Vec* b, Vec* axb)
{
	Vec* out = axb;
	Vec tmp;

	// if output is same as one of inputs, write to tmp buffer till we're done
	if (a == axb || b == axb)
		out = &tmp;

	out->x = a->y * b->z - a->z * b->y;
	out->y = a->z * b->x - b->z * a->x;
	out->z = b->y * a->x - a->y * b->x;

	if (a == axb || b == axb)
		*axb = *out;
}

float __cdecl VECMag(const Vec* v)
{
	return (float)SQRTF(v->y * v->y + v->x * v->x + v->z * v->z);
}

float VECSquareDistance(const Vec* a, const Vec* b)
{
	float v3 = a->x - b->x;
	float v4 = a->y - b->y;
	float v5 = a->z - b->z;
	return v4 * v4 + v3 * v3 + v5 * v5;
}

float GetDistance_new(const Vec* a, const Vec* b)
{
	float v3 = b->x - a->x;
	float v4 = b->y - a->y;
	float v5 = b->z - a->z;
	return v4 * v4 + v3 * v3 + v5 * v5;
}

float VECSquareMag(const Vec* v)
{
	return v->y * v->y + v->x * v->x + v->z * v->z;
}

float __cdecl VECDistance_SSE1(const Vec* a, const Vec* b)
{
	const __m128 tmpA = _mm_set_ps(0.f, a->z, a->y, a->x);
	const __m128 tmpB = _mm_set_ps(0.f, b->z, b->y, b->x);
	__m128 res = _mm_sub_ps(tmpA, tmpB);
	return _mm_cvtss_f32(_mm_sqrt_ps(DotProduct_SSE1(res, res)));
}

// much faster than orig, slower than SSE1
float VECDistance(const Vec* a, const Vec* b)
{
	float v3 = a->x - b->x;
	float v4 = a->y - b->y;
	float v5 = a->z - b->z;
	float v6 = v4 * v4 + v3 * v3 + v5 * v5;
	return (float)SQRTF(v6);
}

float __cdecl GetDistance3_SSE1(const Vec* a, const Vec* b)
{
	const __m128 tmpA = _mm_set_ps(0.f, a->z, a->y, a->x);
	const __m128 tmpB = _mm_set_ps(0.f, b->z, b->y, b->x);
	__m128 res = _mm_sub_ps(tmpB, tmpA);
	return _mm_cvtss_f32(_mm_sqrt_ps(DotProduct_SSE1(res, res)));
}

// much faster than orig, slower than SSE1
float GetDistance3_new(const Vec* a, const Vec* b)
{
	float v3 = b->x - a->x;
	float v4 = b->y - a->y;
	float v5 = b->z - a->z;
	float v6 = v4 * v4 + v3 * v3 + v5 * v5;
	return (float)SQRTF(v6);
}

// fastest sqrtf so far...
double SQRTF_new(float in)
{
	if (in > 0.00001f)
	{
		__m128 in128 = _mm_load_ss(&in);
		return _mm_cvtss_f32(_mm_mul_ss(in128, _mm_rsqrt_ss(in128)));
	}
	return 0.0f;
}

void (*PSMTXConcat)(const Mtx a, const Mtx b, Mtx out);
void MTXConcat(const Mtx a, const Mtx b, Mtx out)
{
	const __m128 row0 = _mm_loadu_ps(b[0]);
	const __m128 row1 = _mm_loadu_ps(b[1]);
	const __m128 row2 = _mm_loadu_ps(b[2]);
	const __m128 row3 = _mm_set_ps(1, 0, 0, 0);

	__m128 newRow0 = _mm_mul_ps(row0, _mm_set_ps1(a[0][0]));
	newRow0 = _mm_add_ps(newRow0, _mm_mul_ps(row1, _mm_set_ps1(a[0][1])));
	newRow0 = _mm_add_ps(newRow0, _mm_mul_ps(row2, _mm_set_ps1(a[0][2])));
	newRow0 = _mm_add_ps(newRow0, _mm_mul_ps(row3, _mm_set_ps1(a[0][3])));

	__m128 newRow1 = _mm_mul_ps(row0, _mm_set_ps1(a[1][0]));
	newRow1 = _mm_add_ps(newRow1, _mm_mul_ps(row1, _mm_set_ps1(a[1][1])));
	newRow1 = _mm_add_ps(newRow1, _mm_mul_ps(row2, _mm_set_ps1(a[1][2])));
	newRow1 = _mm_add_ps(newRow1, _mm_mul_ps(row3, _mm_set_ps1(a[1][3])));

	__m128 newRow2 = _mm_mul_ps(row0, _mm_set_ps1(a[2][0]));
	newRow2 = _mm_add_ps(newRow2, _mm_mul_ps(row1, _mm_set_ps1(a[2][1])));
	newRow2 = _mm_add_ps(newRow2, _mm_mul_ps(row2, _mm_set_ps1(a[2][2])));
	newRow2 = _mm_add_ps(newRow2, _mm_mul_ps(row3, _mm_set_ps1(a[2][3])));

	__m128 newRow3 = _mm_mul_ps(row0, _mm_set_ps1(row3.m128_f32[0]));
	newRow3 = _mm_add_ps(newRow3, _mm_mul_ps(row1, _mm_set_ps1(row3.m128_f32[1])));
	newRow3 = _mm_add_ps(newRow3, _mm_mul_ps(row2, _mm_set_ps1(row3.m128_f32[2])));
	newRow3 = _mm_add_ps(newRow3, _mm_mul_ps(row3, _mm_set_ps1(row3.m128_f32[3])));

	_mm_storeu_ps(out[0], newRow0);
	_mm_storeu_ps(out[1], newRow1);
	_mm_storeu_ps(out[2], newRow2);
}

void (*PSMTXTranspose)(const Mtx a, Mtx out);
void MTXTranspose(const Mtx a, Mtx out)
{
	const __m128 row0 = _mm_loadu_ps(a[0]);
	const __m128 row1 = _mm_loadu_ps(a[1]);
	const __m128 row2 = _mm_loadu_ps(a[2]);
	const __m128 row3 = _mm_set_ps(0, 0, 0, 0);

	const __m128 tmp0 = _mm_unpacklo_ps(row0, row1);
	const __m128 tmp2 = _mm_unpacklo_ps(row2, row3);
	const __m128 tmp1 = _mm_unpackhi_ps(row0, row1);
	const __m128 tmp3 = _mm_unpackhi_ps(row2, row3);

	const __m128 col0 = _mm_movelh_ps(tmp0, tmp2);
	const __m128 col1 = _mm_movehl_ps(tmp2, tmp0);
	const __m128 col2 = _mm_movelh_ps(tmp1, tmp3);

	_mm_storeu_ps(out[0], col0);
	_mm_storeu_ps(out[1], col1);
	_mm_storeu_ps(out[2], col2);
}

void (*PSMTXScale)(Mtx m, float x, float y, float z);
void MTXScale(Mtx m, float x, float y, float z)
{
	m[0][0] = x;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][0] = 0.0f;
	m[1][1] = y;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;
	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = z;
	m[2][3] = 0.0f;
}

void (*PSMTXScaleApply)(const Mtx src, Mtx dst, float xS, float yS, float zS);
void MTXScaleApply(const Mtx src, Mtx dst, float xS, float yS, float zS)
{
	dst[0][0] = src[0][0] * xS;
	dst[0][1] = src[0][1] * xS;
	dst[0][2] = src[0][2] * xS;
	dst[0][3] = src[0][3] * xS;

	dst[1][0] = src[1][0] * yS;
	dst[1][1] = src[1][1] * yS;
	dst[1][2] = src[1][2] * yS;
	dst[1][3] = src[1][3] * yS;

	dst[2][0] = src[2][0] * zS;
	dst[2][1] = src[2][1] * zS;
	dst[2][2] = src[2][2] * zS;
	dst[2][3] = src[2][3] * zS;
}

void MTXScaleApply_SSE(const Mtx src, Mtx dst, float xS, float yS, float zS)
{
	const __m128 row0 = _mm_loadu_ps(src[0]);
	const __m128 row1 = _mm_loadu_ps(src[1]);
	const __m128 row2 = _mm_loadu_ps(src[2]);

	_mm_storeu_ps(dst[0], _mm_mul_ps(row0, _mm_set_ps1(xS)));
	_mm_storeu_ps(dst[1], _mm_mul_ps(row1, _mm_set_ps1(yS)));
	_mm_storeu_ps(dst[2], _mm_mul_ps(row2, _mm_set_ps1(zS)));
}

void (*PSMTXIdentity)(Mtx m);
void MTXIdentity(Mtx m)
{
	m[0][0] = 1.0f;
	m[0][1] = 0.0f;
	m[0][2] = 0.0f;
	m[0][3] = 0.0f;
	m[1][0] = 0.0f;
	m[1][1] = 1.0f;
	m[1][2] = 0.0f;
	m[1][3] = 0.0f;
	m[2][0] = 0.0f;
	m[2][1] = 0.0f;
	m[2][2] = 1.0f;
	m[2][3] = 0.0f;
}

void (*PSMTXRotTrig)(Mtx m, char axis, float sinA, float cosA);
void MTXRotTrig(Mtx m, char axis, float sinA, float cosA)
{
	switch (axis)
	{
	case 'X':
	case 'x':
		_mm_storeu_ps(m[0], _mm_set_ps(0, 0, 0, 1.0f));
		_mm_storeu_ps(m[1], _mm_set_ps(0, -sinA, cosA, 0));
		_mm_storeu_ps(m[2], _mm_set_ps(0, cosA, sinA, 0));
		break;
	case 'Y':
	case 'y':
		_mm_storeu_ps(m[0], _mm_set_ps(0, sinA, 0, cosA));
		_mm_storeu_ps(m[1], _mm_set_ps(0, 0, 1.0f, 0));
		_mm_storeu_ps(m[2], _mm_set_ps(0, cosA, 0, -sinA));
		break;
	case 'Z':
	case 'z':
		_mm_storeu_ps(m[0], _mm_set_ps(0, 0, -sinA, cosA));
		_mm_storeu_ps(m[1], _mm_set_ps(0, 0, cosA, sinA));
		_mm_storeu_ps(m[2], _mm_set_ps(0, 1.0f, 0, 0));
		break;
	}
}

void (*PSMTXRotRad)(Mtx m, char axis, float rad);
void MTXRotRad(Mtx m, char axis, float rad)
{
	float sinA = sin(rad);
	float cosA = cos(rad);
	switch (axis)
	{
	case 'X':
	case 'x':
		_mm_storeu_ps(m[0], _mm_set_ps(0, 0, 0, 1.0f));
		_mm_storeu_ps(m[1], _mm_set_ps(0, -sinA, cosA, 0));
		_mm_storeu_ps(m[2], _mm_set_ps(0, cosA, sinA, 0));
		break;
	case 'Y':
	case 'y':
		_mm_storeu_ps(m[0], _mm_set_ps(0, sinA, 0, cosA));
		_mm_storeu_ps(m[1], _mm_set_ps(0, 0, 1.0f, 0));
		_mm_storeu_ps(m[2], _mm_set_ps(0, cosA, 0, -sinA));
		break;
	case 'Z':
	case 'z':
		_mm_storeu_ps(m[0], _mm_set_ps(0, 0, -sinA, cosA));
		_mm_storeu_ps(m[1], _mm_set_ps(0, 0, cosA, sinA));
		_mm_storeu_ps(m[2], _mm_set_ps(0, 1.0f, 0, 0));
		break;
	}
}

void (*PSMTXCopy)(const Mtx src, Mtx dst);
void MTXCopy_SSE(const Mtx src, Mtx dst)
{
	if (src == dst)
		return;
	_mm_storeu_ps(dst[0], _mm_loadu_ps(src[0]));
	_mm_storeu_ps(dst[1], _mm_loadu_ps(src[1]));
	_mm_storeu_ps(dst[2], _mm_loadu_ps(src[2]));
}

void MTXCopy(const Mtx src, Mtx dst)
{
	if (src == dst)
		return;
	dst[0][0] = src[0][0];
	dst[0][1] = src[0][1];
	dst[0][2] = src[0][2];
	dst[0][3] = src[0][3];
	dst[1][0] = src[1][0];
	dst[1][1] = src[1][1];
	dst[1][2] = src[1][2];
	dst[1][3] = src[1][3];
	dst[2][0] = src[2][0];
	dst[2][1] = src[2][1];
	dst[2][2] = src[2][2];
	dst[2][3] = src[2][3];
}

void (*PSMTXRotAxisRad)(Mtx m, const Vec* axis, float rad);
void MTXRotAxisRad(Mtx m, const Vec* axis, float rad)
{
	float sinA = sin(rad);
	float cosA = cos(rad);
	float invCosA = 1.0f - cosA;

	Vec unit;
	VECNormalize_SSE1(axis, &unit);

	float x_sinA = (unit.x * sinA);
	float y_sinA = (unit.y * sinA);
	float z_sinA = (unit.z * sinA);

	float y_mult = (unit.y * (unit.x * invCosA));
	float y_mult2 = (unit.y * invCosA * unit.z);
	float x_mult = (unit.x * invCosA * unit.z);

	m[0][0] = (unit.x * unit.x) * invCosA + cosA;
	m[0][1] = y_mult - z_sinA;
	m[0][2] = y_sinA + x_mult;
	m[0][3] = 0;

	m[1][0] = z_sinA + y_mult;
	m[1][1] = (unit.y * unit.y) * invCosA + cosA;
	m[1][2] = y_mult2 - x_sinA;
	m[1][3] = 0;

	m[2][0] = x_mult - y_sinA;
	m[2][1] = y_mult2 + x_sinA;
	m[2][2] = (unit.z * unit.z) * invCosA + cosA;
	m[2][3] = 0;
}

void Init_MathReimpl()
{
	if (cfg.bEnableFastMath)
	{
		int cpuinfo[4];
		__cpuid(cpuinfo, 1);

		// pretty much everything should have sse1, but we'll still check just in case
		// really too bad they didn't compile with it enabled...
		bool sse1 = cpuinfo[3] & (1 << 25) || false;
		bool sse2 = cpuinfo[3] & (1 << 26) || false;
		bool sse3 = cpuinfo[2] & (1 << 0) || false;
		bool ssse3 = cpuinfo[2] & (1 << 9) || false;
		bool sse4_1 = cpuinfo[2] & (1 << 19) || false;
		bool sse4_2 = cpuinfo[2] & (1 << 20) || false;

		// Replace vector/matrix math functions with our own 2022-optimized/SSE-powered equivalents

		// VECAdd
		{
			auto pattern = hook::pattern("A1 ? ? ? ? 8D 4D ? 51 8B D1 52 05 94 00 00 00 50 E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0x12)).as_int();
			ReadCall(caller, PSVECAdd);
			// alternate version
			pattern = hook::pattern("56 8D 55 ? 52 56 E8 ? ? ? ? 8B 4D ? 83 C4 ? 5F");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(6)).as_int();

			InjectHook(caller, VECAdd, PATCH_JUMP);
			InjectHook(caller_0, VECAdd, PATCH_JUMP);
		}

		// VECSubtract
		{
			auto pattern = hook::pattern("51 8D 55 ? 52 E8 ? ? ? ? D9 45 ? D8 65 ?");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(5)).as_int();
			ReadCall(caller, PSVECSubtract);
			// alternate version
			pattern = hook::pattern("8D 0C 40 8D 14 8B 57 52 E8 ? ? ? ? 8D 45");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(8)).as_int();

			InjectHook(caller, VECSubtract, PATCH_JUMP);
			InjectHook(caller_0, VECSubtract, PATCH_JUMP);
		}

		// VECScale
		{
			auto pattern = hook::pattern("83 C4 08 8D 55 ? D9 1C ? 52 8B C2 50 E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xD)).as_int();
			ReadCall(caller, PSVECScale);
			// alternate version
			pattern = hook::pattern("8B 7D ? 51 8D 4D ? D9 1C ? 51 50 E8");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xC)).as_int();

			InjectHook(caller, VECScale, PATCH_JUMP);
			InjectHook(caller_0, VECScale, PATCH_JUMP);
		}

		// VECNormalize
		{
			auto pattern = hook::pattern("8D 45 ? 50 8D 4D ? 51 E8 ? ? ? ? D9 45 ? D9 45 ? 83 C4 04 D9 45 ?");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(8)).as_int();
			ReadCall(caller, PSVECNormalize);

			if (sse1)
				InjectHook(caller, VECNormalize_SSE1, PATCH_JUMP);
			else
				InjectHook(caller, VECNormalize, PATCH_JUMP);
		}

		// VECDotProduct
		{
			auto pattern = hook::pattern("8B B5 ? ? ? ? 8D 4D ? 56 51 E8 ? ? ? ? D9 9D ? ? ? ? D9 85");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xB)).as_int();
			ReadCall(caller, PSVECDotProduct);
			// alternate version
			pattern = hook::pattern("8D 55 ? 52 8D 45 ? 50 E8 ? ? ? ? D8 1D");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(8)).as_int();

			InjectHook(caller, VECDotProduct, PATCH_JUMP);
			InjectHook(caller_0, VECDotProduct, PATCH_JUMP);
		}

		// VECCrossProduct
		{
			auto pattern = hook::pattern("DD D9 8D 45 ? DD D8 56 50 E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(9)).as_int();
			ReadCall(caller, PSVECCrossProduct);
			// alternate version
			pattern = hook::pattern("8D 55 ? 52 8D 45 ? 50 8D 4D ? 51 E8 ? ? ? ? 8D 55 ? 52 8D 45 ? 50 E8");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xC)).as_int();

			if (sse1)
			{
				InjectHook(caller, VECCrossProduct_SSE1, PATCH_JUMP);
				InjectHook(caller_0, VECCrossProduct_SSE1, PATCH_JUMP);
			}
			else
			{
				InjectHook(caller, VECCrossProduct, PATCH_JUMP);
				InjectHook(caller_0, VECCrossProduct, PATCH_JUMP);
			}
		}

		// VECMag
		{
			auto pattern = hook::pattern("E8 ? ? ? ? 8D 55 ? 52 E8 ? ? ? ? D9 5D ? D9 45 ?");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(9)).as_int();
			ReadCall(caller, PSVECMag);

			InjectHook(caller, VECMag, PATCH_JUMP);
		}

		// VECSquareDistance
		{
			auto pattern = hook::pattern("51 8D 95 ? ? ? ? 52 DD 9D ? ? ? ? E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xE)).as_int();
			ReadCall(caller, PSVECSquareDistance);
			// alternate version
			pattern = hook::pattern("D9 45 ? 8B 55 ? DD 5D ? 8D 4D ? 51 52 E8");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xE)).as_int();

			InjectHook(caller, VECSquareDistance, PATCH_JUMP);
			InjectHook(caller_0, VECSquareDistance, PATCH_JUMP);
		}

		// GetDistance (VECSquareDistance with reversed params...)
		{
			auto pattern = hook::pattern("D9 5D ? 8B 18 8D 45 ? 50 E8 ? ? ? ? 0F AF DB");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0x9)).as_int();
			ReadCall(caller, GetDistance);

			// alternate version
			pattern = hook::pattern("D9 5D ? D9 5D ? D9 5D ? D9 5D ? E8 ? ? ? ? D9 45 ? 83 C4 08");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xC)).as_int();

			InjectHook(caller, GetDistance_new, PATCH_JUMP);
			InjectHook(caller_0, GetDistance_new, PATCH_JUMP);
		}

		// VECSquareMag
		{
			auto pattern = hook::pattern("E8 ? ? ? ? D9 9D ? ? ? ? 8D 45 ? 83 C4 40 50 E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0x12)).as_int();
			ReadCall(caller, PSVECSquareMag);
			// alt ver
			pattern = hook::pattern("E8 ? ? ? ? 8D 45 ? 50 E8 ? ? ? ? D9 5D ? 8D 4D ? 51 8D 55");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(9)).as_int();

			InjectHook(caller, VECSquareMag, PATCH_JUMP);
			InjectHook(caller_0, VECSquareMag, PATCH_JUMP);
		}

		// VECDistance
		{
			auto pattern = hook::pattern("8D 55 ? 52 8D 85 ? ? ? ? 50 E8 ? ? ? ? DD 5D ? 8D 4D ?");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xB)).as_int();
			ReadCall(caller, PSVECDistance);

			if (sse1)
				InjectHook(caller, VECDistance_SSE1, PATCH_JUMP);
			else
				InjectHook(caller, VECDistance, PATCH_JUMP);
		}

		// GetDistance3 (VECDistance with reversed params...)
		{
			auto pattern = hook::pattern("81 C1 94 00 00 00 51 8D 96 ? ? ? ? 52 E8 ? ? ? ? DC 25");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xE)).as_int();
			ReadCall(caller, GetDistance3);

			if (sse1)
				InjectHook(caller, GetDistance3_SSE1, PATCH_JUMP);
			else
				InjectHook(caller, GetDistance3_new, PATCH_JUMP);
		}

		// MTXMultVec
		{
			auto pattern = hook::pattern("DD D8 51 8D 55 ? 52 8D 7E ? 57 E8 ? ? ? ? 8D 45");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xB)).as_int();
			ReadCall(caller, PSMTXMultVec);

			InjectHook(caller, MTXMultVec, PATCH_JUMP);
		}

		// MTXMultVecSR
		{
			auto pattern = hook::pattern("D9 96 54 07 00 00 D9 5D ? D9 55 ? D9 5D ? E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xF)).as_int();
			ReadCall(caller, PSMTXMultVecSR);

			InjectHook(caller, MTXMultVecSR, PATCH_JUMP);
		}

		// SQRTF - replacing this seems to fix the low-fps r104s00 cutscene issue?
		{
			auto pattern = hook::pattern("D9 46 ? DD 5D ? D9 1C ? E8 ? ? ? ? DC 6D ? 83 C4 04");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(9)).as_int();
			ReadCall(caller, SQRTF);
			if (sse1)
				InjectHook(caller, SQRTF_new, PATCH_JUMP);
		}

		// MTXConcat
		{
			auto pattern = hook::pattern("8D 47 3C 50 8D 55 ? 52 50 E8 ? ? ? ? 83 C4 18");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(9)).as_int();
			ReadCall(caller, PSMTXConcat);
			if (sse1)
				InjectHook(caller, MTXConcat, PATCH_JUMP);
		}

		// MTXTranspose
		{
			auto pattern = hook::pattern("8D 45 ? 50 8D 4D ? 51 E8 ? ? ? ? 83 C4 40 8D 55");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(8)).as_int();
			ReadCall(caller, PSMTXTranspose);
			if (sse1)
				InjectHook(caller, MTXTranspose, PATCH_JUMP);
		}

		// MTXScale
		{
			auto pattern = hook::pattern("D9 5C 24 ? D9 46 0C D9 1C ? 52 E8 ? ? ? ? 8D 45");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xB)).as_int();
			ReadCall(caller, PSMTXScale);
			if (sse1)
				InjectHook(caller, MTXScale, PATCH_JUMP);
		}

		// MTXScaleApply
		{
			auto pattern = hook::pattern("D9 00 8B 45 ? D9 1C ? 50 50 E8 ? ? ? ? 83 C4 14");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xA)).as_int();
			ReadCall(caller, PSMTXScaleApply);
			if (sse1)
				InjectHook(caller, MTXScaleApply_SSE, PATCH_JUMP);
			else
				InjectHook(caller, MTXScaleApply, PATCH_JUMP);
		}

		// MTXIdentity
		{
			auto pattern = hook::pattern("38 86 FD 00 00 00 75 ? 8D 45 ? 50 E8 ? ? ? ? D9 E8");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xC)).as_int();
			ReadCall(caller, PSMTXIdentity);
			// alt ver
			pattern = hook::pattern("8D 45 ? 50 E8 ? ? ? ? 8D 8E A0 00 00 00 51 8D 55");
			auto caller_0 = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(4)).as_int();

			if (sse1)
			{
				InjectHook(caller, MTXIdentity, PATCH_JUMP);
				InjectHook(caller_0, MTXIdentity, PATCH_JUMP);
			}
		}

		// MTXRotTrig
		{
			auto pattern = hook::pattern("51 8B 4D ? D9 1C ? 50 51 E8 ? ? ? ? 83 C4 10");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(9)).as_int();
			ReadCall(caller, PSMTXRotTrig);
			if (sse1)
				InjectHook(caller, MTXRotTrig, PATCH_JUMP);
		}

		// MTXRotRad
		{
			auto pattern = hook::pattern("6A 79 51 E8 ? ? ? ? 8D 55");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(3)).as_int();
			ReadCall(caller, PSMTXRotRad);
			if (sse1)
				InjectHook(caller, MTXRotRad, PATCH_JUMP);
		}

		// MTXRotAxisRad
		{
			auto pattern = hook::pattern("83 C4 08 8D 4D ? D9 1C ? 51 8D 55 ? 52 E8 ? ? ? ?");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0xE)).as_int();
			ReadCall(caller, PSMTXRotAxisRad);
			if (sse1) // calls into VECNormalize_SSE1
				InjectHook(caller, MTXRotAxisRad, PATCH_JUMP);
		}

		// MTXCopy
		{
			auto pattern = hook::pattern("50 8D 4E 0C 51 E8 ? ? ? ? 83 C7 70");
			auto caller = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(5)).as_int();
			ReadCall(caller, PSMTXCopy);
			if (sse1)
				InjectHook(caller, MTXCopy_SSE, PATCH_JUMP);
			else
				InjectHook(caller, MTXCopy, PATCH_JUMP);
		}
	}

	// Fix cloth-related lag by replacing penClothAtCkParallel with our own reimplementation
	// besides 1 redundant subtraction, the code is essentially the same, except somehow seems to allow running at 60FPS without a sweat
	// seems to be returning same values as the original function too - set TEST_CKPARALLEL_REIMPL to test (and set breakpoints where instructed...)
	{
		auto pattern = hook::pattern("8D B8 28 01 00 00 8D 83 5C 01 00 00 8D 4F 34 50 E8 ? ? ? ?");
		auto ckParallel = injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0x10), false).as_int();

		// overwrite start of func with jump to our hook...
		injector::MakeNOP(ckParallel, 9, true);
		penClothAtCkParallel = (penClothAtCkParallel_Fn)(ckParallel + 9);

		InjectHook(ckParallel, penClothAtCkParallel_trampoline, PATCH_JUMP);
	}

	//speedtest();

#ifdef TEST_CKPARALLEL_REIMPL
	InjectHook(0x678AED, penClothAtCkParallel_trampoline, PATCH_CALL);
	InjectHook(0x678C16, penClothAtCkParallel_trampoline, PATCH_CALL);
	InjectHook(0x678D41, penClothAtCkParallel_trampoline, PATCH_CALL);
	InjectHook(0x678EE9, penClothAtCkParallel_trampoline, PATCH_CALL);
	InjectHook(0x67908C, penClothAtCkParallel_trampoline, PATCH_CALL);

	// overwrite ret of the original func to jump to our reimpl
	InjectHook(0x674A49, penClothAtCkParallel_hook, PATCH_JUMP);
#endif
}