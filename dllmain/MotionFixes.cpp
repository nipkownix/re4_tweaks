#include <iostream>
#include "dllmain.h"
#include "Game.h"
#include "Settings.h"
#include <cmath>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <cstring>

// (hopefully) Better motion interpolation at 60fps (and above?)
//
// At 60fps, some animations play at 30fps (rifle reloads and fires, Ada's bowgun, a specific type of Plaga, etc.)
//
// Motions are originally keyed at 30fps, and GC only ever samples them at whole frames. The PC port advances frames by
// pG->Speed (0.5 at 60fps), so it samples in between, and QLOC's MotionMoveCore does so inconsistently: rotations are
// slerped between the two whole frames (except in the last frame), while positions, scale and IK rotations are read off
// the Hermite curves, whose tangents were never meant to be used between frames. Models that must fit another (a hand on
// a weapon, etc) fall apart. QLOC's workaround is to apply their new motion flag 0x4000, which truncates the frame,
// making those motions play at 30fps.
//
// Affects the in-between sampling of every motion. 
// Motions set with attr 0x4000 (30fps on PC):
//  - player's rifle reload and fire (bolt cycle), both rifles: wep09_r2_reload, wep09_r3_fire20;
//  - the bolt-action rifle's own reload and fire: cObjSniper::moveReload, cObjSniper::moveFire;
//  - Ada's bowgun fire (with the bolt reload): wep50_r3_fire00, and the bowgun's own: cObjAdaBow::moveFire;
//  - Ada's bowgun ready (non-Wii path): wep50_r3_ready00;
//  - the head-eating Plaga parasite (obj16): cObj16::setMotData (0x4004), obj16_R1_Critical (also 0x4080);
//  - Something related to Saddler human form on Separate Ways (cEm3f). Couldn't see anything wrong with it just by looking...
//  - r10c_TestPosMove (unsure what that even is)
//
// Steps we try to do to solve the problem:
//  1 -> Every channel's in-between value is the blend of the two whole frames around it (rotations slerped, positions and
//       scale lerped). Whole frames are the keys themselves, as in GC. The frame is never truncated.
//  2 -> The second whole frame never wraps past the end of a looping motion back to frame 0 (the last key instead), or the
//       end of looping motions heads back to their start.
//  3 -> Blending joint by joint still can't keep two models together between frames (the hand comes down one arm, the
//       weapon hangs off the other hand, and they only meet at whole frames). So models whose main motion has attr 0x4000
//       are posed at whole frames only (the next one, so nothing lags), and after every model has moved, before drawing
//       (Trans), each of their parts' final matrix is blended between the last two whole frames, relative to what the
//       model hangs off (the player's own matrix, for him and for the weapon in his hand). What fits at whole frames keeps
//       fitting in between.
//  4 -> Unflagged models hanging off a blended one (the auto-rifle in the player's hand) get their world matrices again
//       from the blend.
//  5 -> While a motion doesn't advance (paused, or held at its end), there's no blend: the saved matrices would leave the
//       model behind. Made Ada's bow gun float in space after firing.
//
// Steps 1 and 2 apply to every motion. Steps 3 to 5 only to QLOC's flagged ones.

// HERMITE_SET
struct HermiteSet
{
	float frame;    // +0
	float maxFrame; // +4
	uint32_t flags; // +8: 1 search backwards, 2 reverse, 4 loop (wraps when frame >= maxFrame, without 2), 8 no history
};

static int(__cdecl* HermiteInterpolation_orig)(HermiteSet* pH, Vec* res, uint16_t* hist, uint32_t linear, uint32_t absolute);

static const float c_pi = 3.14159265358979f;

// Samples at `frame` with the original HermiteInterpolation (never truncated), never wrapping past the end of the motion
// back to frame 0 (the last key instead)
static int MotionMoveCore_sample(HermiteSet* pH, float frame, Vec* res, uint16_t* hist, uint32_t linear)
{
	const float frameOrig = pH->frame;
	const uint32_t flags = pH->flags;
	pH->frame = frame;
	if (frame >= pH->maxFrame)
		pH->flags &= ~4u;
	int ret = HermiteInterpolation_orig(pH, res, hist, linear, 0);
	pH->flags = flags;
	pH->frame = frameOrig;
	return ret;
}

// Euler angles as used by RotMatrix (m = Rz * Ry * Rx) <-> quaternion (x, y, z, w)
static void MotionMoveCore_eulerToQuat(const Vec* e, float* q)
{
	const float cx = std::cos(e->x * 0.5f), sx = std::sin(e->x * 0.5f);
	const float cy = std::cos(e->y * 0.5f), sy = std::sin(e->y * 0.5f);
	const float cz = std::cos(e->z * 0.5f), sz = std::sin(e->z * 0.5f);
	q[0] = sx * cy * cz - cx * sy * sz;
	q[1] = cx * sy * cz + sx * cy * sz;
	q[2] = cx * cy * sz - sx * sy * cz;
	q[3] = cx * cy * cz + sx * sy * sz;
}

static void MotionMoveCore_quatToEuler(const float* q, Vec* e)
{
	const float x = q[0], y = q[1], z = q[2], w = q[3];
	e->x = std::atan2(2.0f * (w * x + y * z), 1.0f - 2.0f * (x * x + y * y));
	float s = 2.0f * (w * y - x * z);
	s = s > 1.0f ? 1.0f : (s < -1.0f ? -1.0f : s);
	e->y = std::asin(s);
	e->z = std::atan2(2.0f * (w * z + x * y), 1.0f - 2.0f * (y * y + z * z));
}

// Shortest path slerp of quaternions (x, y, z, w), normalized
static void MotionMoveCore_slerp(const float* qa, const float* qb, float t, float* q)
{
	float b[4] = { qb[0], qb[1], qb[2], qb[3] };
	float dot = qa[0] * b[0] + qa[1] * b[1] + qa[2] * b[2] + qa[3] * b[3];
	if (dot < 0.0f)
	{
		dot = -dot;
		for (int i = 0; i < 4; i++)
			b[i] = -b[i];
	}

	float w0 = 1.0f - t, w1 = t;
	if (dot < 0.9995f)
	{
		const float theta = std::acos(dot);
		const float sinTheta = std::sin(theta);
		w0 = std::sin((1.0f - t) * theta) / sinTheta;
		w1 = std::sin(t * theta) / sinTheta;
	}
	float len = 0.0f;
	for (int i = 0; i < 4; i++)
	{
		q[i] = qa[i] * w0 + b[i] * w1;
		len += q[i] * q[i];
	}
	len = std::sqrt(len);
	for (int i = 0; i < 4; i++)
		q[i] /= len;
}

// a - b, wrapped to [-pi, pi]
static float MotionMoveCore_angleDiff(float a, float b)
{
	float d = std::fmod(a - b, 2.0f * c_pi);
	if (d > c_pi)
		d -= 2.0f * c_pi;
	else if (d < -c_pi)
		d += 2.0f * c_pi;
	return d;
}

// Models whose main motion has attr 0x4000, posed at whole frames this update cycle
static std::unordered_set<cModel*> s_worldBlendMoved;
static cModel* s_worldBlendLastMoved = nullptr;

// True if the current MotionMoveCore's model has a main motion with attr 0x4000 (posed at whole frames, blended afterwards).
// pH is MotionMoveCore's own HERMITE_SET, a local at [ebp-90h] in every hooked call, so its frame (and arguments: +8 pEm,
// +0Ch pInfo) is right after it.
static bool MotionMoveCore_isWorldBlend(HermiteSet* pH)
{
	const uint8_t* frame = (const uint8_t*)pH + 0x90;
	cModel* pEm = *(cModel* const*)(frame + 8);
	if (!pEm || !(pEm->Motion_1D8.Mot_attr_40 & 0x4000))
		return false;

	// Only the main motion's frame decides the blend (the blend copy of cMot3 follows it)
	MOTION_INFO* pInfo = *(MOTION_INFO* const*)(frame + 0xC);
	if (pInfo == &pEm->Motion_1D8 && pEm != s_worldBlendLastMoved)
	{
		s_worldBlendMoved.insert(pEm);
		s_worldBlendLastMoved = pEm;
	}
	return true;
}

// The value at pH->frame: a whole frame as it is, an in-between frame as the blend of the whole frames around it
// (rotations slerped, positions and scale lerped)
static int MotionMoveCore_interpolate(HermiteSet* pH, Vec* res, uint16_t* hist, uint32_t linear, uint32_t absolute, bool rotation)
{
	if (!re4t::cfg->bMotionFixes)
		return HermiteInterpolation_orig(pH, res, hist, linear, absolute);

	const float frame = pH->frame;

	// Fitted to another model: the next whole frame, blended afterwards
	if (MotionMoveCore_isWorldBlend(pH))
		return MotionMoveCore_sample(pH, std::ceil(frame), res, hist, linear);

	const float frame0 = std::floor(frame);
	if (frame == frame0)
		return MotionMoveCore_sample(pH, frame0, res, hist, linear);

	Vec v0, v1;
	int ret = MotionMoveCore_sample(pH, frame0, &v0, hist, linear);
	ret |= MotionMoveCore_sample(pH, frame0 + 1.0f, &v1, hist, linear);
	const float t = frame - frame0;

	if (!rotation)
	{
		res->x = v0.x + (v1.x - v0.x) * t;
		res->y = v0.y + (v1.y - v0.y) * t;
		res->z = v0.z + (v1.z - v0.z) * t;
		return ret;
	}

	float q0[4], q1[4], q[4];
	MotionMoveCore_eulerToQuat(&v0, q0);
	MotionMoveCore_eulerToQuat(&v1, q1);
	MotionMoveCore_slerp(q0, q1, t, q);

	// Of the two Euler triples for this rotation, (x, y, z) and (x + pi, pi - y, z + pi), the one closest to the first
	// frame's, so anything that blends the angles themselves (motion blending, hokan) sees them continue smoothly
	Vec e;
	MotionMoveCore_quatToEuler(q, &e);
	const Vec alt = { e.x + c_pi, c_pi - e.y, e.z + c_pi };

	const float dx = MotionMoveCore_angleDiff(e.x, v0.x), dy = MotionMoveCore_angleDiff(e.y, v0.y), dz = MotionMoveCore_angleDiff(e.z, v0.z);
	const float ax = MotionMoveCore_angleDiff(alt.x, v0.x), ay = MotionMoveCore_angleDiff(alt.y, v0.y), az = MotionMoveCore_angleDiff(alt.z, v0.z);
	if (ax * ax + ay * ay + az * az < dx * dx + dy * dy + dz * dz)
		e = alt;

	// Relative to the first frame (VecRadLimit follows)
	res->x = v0.x + MotionMoveCore_angleDiff(e.x, v0.x);
	res->y = v0.y + MotionMoveCore_angleDiff(e.y, v0.y);
	res->z = v0.z + MotionMoveCore_angleDiff(e.z, v0.z);
	return ret;
}

// Positions (kind 4)
static int __cdecl MotionMoveCore_HermitePosition_hook(HermiteSet* pH, Vec* res, uint16_t* hist, uint32_t linear, uint32_t absolute)
{
	return MotionMoveCore_interpolate(pH, res, hist, linear, absolute, false);
}

// Scale (kind 8)
static int __cdecl MotionMoveCore_HermiteScale_hook(HermiteSet* pH, Vec* res, uint16_t* hist, uint32_t linear, uint32_t absolute)
{
	return MotionMoveCore_interpolate(pH, res, hist, linear, absolute, false);
}

// Rotations (kind 2 and the IK chains' kind 0x10/0x20)
static int __cdecl MotionMoveCore_HermiteRotation_hook(HermiteSet* pH, Vec* res, uint16_t* hist, uint32_t linear, uint32_t absolute)
{
	return MotionMoveCore_interpolate(pH, res, hist, linear, absolute, true);
}

// MotionMoveCore, rotation channels (kind 2), replacing "test ah, 5 ; jnp short loc_662029" (the "last frame" check).
// Enabled: always loc_662029, the plain HermiteInterpolation call for the rotation (hooked below), which pops the frame
// from the FPU stack. Disabled: the original check. Only eax (ah, the fcompp result) is live here.
static bool* s_pEnabled = nullptr;
static uintptr_t s_rotHermite = 0;  // loc_662029
static uintptr_t s_rotContinue = 0; // After the jnp: the 0x4000 / 0x4080 checks, then QLOC's slerp path

void __declspec(naked) MotionMoveCore_Rotation_hook()
{
	__asm
	{
		push ecx
		mov ecx, s_pEnabled
		cmp byte ptr [ecx], 0
		pop ecx
		jne hermite

		test ah, 5
		jnp hermite
		jmp s_rotContinue

	hermite:
		jmp s_rotHermite
	}
}

// ----- Blend of the models posed at whole frames (attr 0x4000) ------

struct MotionMtx
{
	float m[3][4];
};

// A model's parts' matrices at the last two whole frames
struct MotionWorldBlend
{
	uint32_t lastUpdate = 0;
	int32_t pMot = 0;
	bool hasCur = false;
	bool hasPrev = false;
	float curFrame = 0.0f;   // The whole frame of cur (prev is the one before)
	float lastFrame = -1.0f; // The frame of the last update
	cCoord* ref = nullptr;   // What the matrices are relative to (nullptr: world)
	std::vector<MotionMtx> cur;
	std::vector<MotionMtx> prev;
};

static std::unordered_map<cModel*, MotionWorldBlend> s_worldBlends;
static uint32_t s_worldBlendUpdate = 1;

// ObjMgr / EmMgr alive lists (units linked through +8), as Trans walks them
static cModel** s_objAlive = nullptr;
static cModel** s_emAlive = nullptr;

// Rotation matrix (normalized columns) -> quaternion (x, y, z, w)
static void MotionWorldBlend_mtxToQuat(const float r[3][3], float* q)
{
	const float trace = r[0][0] + r[1][1] + r[2][2];
	if (trace > 0.0f)
	{
		const float s = std::sqrt(trace + 1.0f) * 2.0f;
		q[3] = 0.25f * s;
		q[0] = (r[2][1] - r[1][2]) / s;
		q[1] = (r[0][2] - r[2][0]) / s;
		q[2] = (r[1][0] - r[0][1]) / s;
	}
	else if (r[0][0] > r[1][1] && r[0][0] > r[2][2])
	{
		const float s = std::sqrt(1.0f + r[0][0] - r[1][1] - r[2][2]) * 2.0f;
		q[3] = (r[2][1] - r[1][2]) / s;
		q[0] = 0.25f * s;
		q[1] = (r[0][1] + r[1][0]) / s;
		q[2] = (r[0][2] + r[2][0]) / s;
	}
	else if (r[1][1] > r[2][2])
	{
		const float s = std::sqrt(1.0f + r[1][1] - r[0][0] - r[2][2]) * 2.0f;
		q[3] = (r[0][2] - r[2][0]) / s;
		q[0] = (r[0][1] + r[1][0]) / s;
		q[1] = 0.25f * s;
		q[2] = (r[1][2] + r[2][1]) / s;
	}
	else
	{
		const float s = std::sqrt(1.0f + r[2][2] - r[0][0] - r[1][1]) * 2.0f;
		q[3] = (r[1][0] - r[0][1]) / s;
		q[0] = (r[0][2] + r[2][0]) / s;
		q[1] = (r[1][2] + r[2][1]) / s;
		q[2] = 0.25f * s;
	}
}

// Splits a world matrix into rotation (normalized columns), scale (column lengths) and translation; false when it can't
// be blended as a rotation (degenerate or mirrored)
static bool MotionWorldBlend_split(const MotionMtx& m, float r[3][3], float* scale, float* trans)
{
	for (int c = 0; c < 3; c++)
	{
		const float len = std::sqrt(m.m[0][c] * m.m[0][c] + m.m[1][c] * m.m[1][c] + m.m[2][c] * m.m[2][c]);
		if (len < 1e-6f)
			return false;
		scale[c] = len;
		for (int i = 0; i < 3; i++)
			r[i][c] = m.m[i][c] / len;
	}
	for (int i = 0; i < 3; i++)
		trans[i] = m.m[i][3];

	const float det = r[0][0] * (r[1][1] * r[2][2] - r[1][2] * r[2][1]) - r[0][1] * (r[1][0] * r[2][2] - r[1][2] * r[2][0])
		+ r[0][2] * (r[1][0] * r[2][1] - r[1][1] * r[2][0]);
	return det > 0.0f;
}

// a -> b by t: rotation slerped, scale and translation lerped
static void MotionWorldBlend_blend(const MotionMtx& a, const MotionMtx& b, float t, float out[3][4])
{
	float ra[3][3], rb[3][3], sa[3], sb[3], ta[3], tb[3];
	if (!MotionWorldBlend_split(a, ra, sa, ta) || !MotionWorldBlend_split(b, rb, sb, tb))
	{
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 4; j++)
				out[i][j] = b.m[i][j];
		return;
	}

	float qa[4], qb[4], q[4];
	MotionWorldBlend_mtxToQuat(ra, qa);
	MotionWorldBlend_mtxToQuat(rb, qb);
	MotionMoveCore_slerp(qa, qb, t, q);
	const float x = q[0], y = q[1], z = q[2], w = q[3];

	const float r[3][3] = {
		{ 1.0f - 2.0f * (y * y + z * z), 2.0f * (x * y - z * w), 2.0f * (x * z + y * w) },
		{ 2.0f * (x * y + z * w), 1.0f - 2.0f * (x * x + z * z), 2.0f * (y * z - x * w) },
		{ 2.0f * (x * z - y * w), 2.0f * (y * z + x * w), 1.0f - 2.0f * (x * x + y * y) }
	};
	for (int c = 0; c < 3; c++)
	{
		const float sc = sa[c] + (sb[c] - sa[c]) * t;
		for (int i = 0; i < 3; i++)
			out[i][c] = r[i][c] * sc;
	}
	for (int i = 0; i < 3; i++)
		out[i][3] = ta[i] + (tb[i] - ta[i]) * t;
}

// Affine 3x4 matrices: out = a * b
static void MotionWorldBlend_mul(const float a[3][4], const float b[3][4], float out[3][4])
{
	float r[3][4];
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
			r[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] + a[i][2] * b[2][j] + (j == 3 ? a[i][3] : 0.0f);
	}
	std::memcpy(out, r, sizeof(r));
}

// Affine 3x4 inverse; false when singular
static bool MotionWorldBlend_invert(const float m[3][4], float out[3][4])
{
	const float c00 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
	const float c01 = m[1][2] * m[2][0] - m[1][0] * m[2][2];
	const float c02 = m[1][0] * m[2][1] - m[1][1] * m[2][0];
	const float det = m[0][0] * c00 + m[0][1] * c01 + m[0][2] * c02;
	if (std::fabs(det) < 1e-12f)
		return false;
	const float inv = 1.0f / det;

	float r[3][4];
	r[0][0] = c00 * inv;
	r[1][0] = c01 * inv;
	r[2][0] = c02 * inv;
	r[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * inv;
	r[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * inv;
	r[2][1] = (m[0][1] * m[2][0] - m[0][0] * m[2][1]) * inv;
	r[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * inv;
	r[1][2] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]) * inv;
	r[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]) * inv;
	for (int i = 0; i < 3; i++)
		r[i][3] = -(r[i][0] * m[0][3] + r[i][1] * m[1][3] + r[i][2] * m[2][3]);
	std::memcpy(out, r, sizeof(r));
	return true;
}

// One model posed at a whole frame this update: keeps its parts' matrices at the last two whole frames, relative to
// "ref", and replaces them with the blend at the actual frame
static void MotionWorldBlend_model(cModel* pEm, cCoord* ref)
{
	MotionWorldBlend& blend = s_worldBlends[pEm];
	blend.lastUpdate = s_worldBlendUpdate;

	const MOTION_INFO& info = pEm->Motion_1D8;
	const float frame = info.Mot_frame_24;
	const float whole = std::ceil(frame); // The whole frame it was posed at

	size_t n = 0;
	for (cParts* p = pEm->childParts_F4; p; p = p->nextParts_F4)
		n++;

	float refInv[3][4];
	if (!ref || !MotionWorldBlend_invert(ref->mat_C, refInv))
		ref = nullptr;

	// Another motion, other parts, or another reference: start over
	if (blend.pMot != info.fcvDataPtr_0 || blend.cur.size() != n || blend.ref != ref)
	{
		blend.pMot = info.fcvDataPtr_0;
		blend.ref = ref;
		blend.hasCur = blend.hasPrev = false;
		blend.cur.resize(n);
		blend.prev.resize(n);
	}

	// The next whole frame: the current one becomes the previous one
	if (blend.hasCur && whole == blend.curFrame + 1.0f)
	{
		blend.prev.swap(blend.cur);
		blend.hasPrev = true;
	}
	else if (!blend.hasCur || whole != blend.curFrame)
		blend.hasPrev = false;

	size_t i = 0;
	for (cParts* p = pEm->childParts_F4; p; p = p->nextParts_F4, i++)
	{
		if (ref)
			MotionWorldBlend_mul(refInv, p->mat_C, blend.cur[i].m);
		else
			std::memcpy(blend.cur[i].m, p->mat_C, sizeof(MotionMtx));
	}
	blend.curFrame = whole;
	blend.hasCur = true;

	// Held (paused, or stopped at the end): the pose as it is
	const bool advanced = frame != blend.lastFrame;
	blend.lastFrame = frame;
	if (!advanced)
		blend.hasPrev = false;

	// At a whole frame (or without the previous one), the pose as it is
	const float t = frame - (whole - 1.0f);
	if (!blend.hasPrev || t >= 1.0f)
		return;

	i = 0;
	for (cParts* p = pEm->childParts_F4; p; p = p->nextParts_F4, i++)
	{
		MotionWorldBlend_blend(blend.prev[i], blend.cur[i], t, p->mat_C);
		if (ref)
			MotionWorldBlend_mul(ref->mat_C, p->mat_C, p->mat_C);
		p->world_70.x = p->mat_C[0][3];
		p->world_70.y = p->mat_C[1][3];
		p->world_70.z = p->mat_C[2][3];
	}
}

// cModel::partsWorldCalc (parts' world matrices from their parents' and their own local ones)
static void(__fastcall* cModel__partsWorldCalc)(cModel* thisptr, void* unused);

static void MotionWorldBlend_apply()
{
	if (re4t::cfg->bMotionFixes && !s_worldBlendMoved.empty())
	{
		cModel** lists[] = { s_objAlive, s_emAlive };

		// The models posed at whole frames this update, and which of them each of their parts belongs to
		std::vector<cModel*> models;
		std::unordered_map<cCoord*, cModel*> partOwner;
		for (cModel** list : lists)
		{
			if (!list)
				continue;
			for (cModel* pEm = *list; pEm; pEm = *(cModel**)((uint8_t*)pEm + 8))
			{
				if (!s_worldBlendMoved.count(pEm))
					continue;
				models.push_back(pEm);
				for (cParts* p = pEm->childParts_F4; p; p = p->nextParts_F4)
					partOwner[p] = pEm;
			}
		}

		// Each model is blended relative to what it hangs off, going up through the blended models: the player relative
		// to his own matrix (so his movement and turning are the current ones), the bolt-action rifle on his hand relative
		// to the player's matrix too (both blended in the same space, so they keep fitting), a flagged model on an
		// unflagged one's part (a parasite on a head) relative to that part (so it stays on it as it moves)
		for (cModel* pEm : models)
		{
			cCoord* ref = pEm->childParts_F4 ? pEm->childParts_F4->pParent_6C : nullptr;
			for (int depth = 0; ref && depth < 8; depth++)
			{
				auto owner = partOwner.find(ref);
				if (owner == partOwner.end())
					break;
				ref = owner->second->childParts_F4 ? owner->second->childParts_F4->pParent_6C : nullptr;
			}
			MotionWorldBlend_model(pEm, ref);
		}

		// Models hanging off a blended one with a motion of their own (the auto-rifle on the player's hand, reloading with
		// the player's flagged motion but an unflagged one of its own) took their world matrices from its whole frame
		// pose: again from the blend, or they'd move at 30fps
		for (cModel** list : lists)
		{
			if (!list)
				continue;
			for (cModel* pEm = *list; pEm; pEm = *(cModel**)((uint8_t*)pEm + 8))
			{
				if (s_worldBlendMoved.count(pEm))
					continue;
				for (cParts* p = pEm->childParts_F4; p; p = p->nextParts_F4)
				{
					if (partOwner.count(p->pParent_6C))
					{
						cModel__partsWorldCalc(pEm, nullptr);
						break;
					}
				}
			}
		}
	}

	// Models not posed this way this update (other motion, gone) start over next time
	for (auto it = s_worldBlends.begin(); it != s_worldBlends.end();)
	{
		if (it->second.lastUpdate != s_worldBlendUpdate)
			it = s_worldBlends.erase(it);
		else
			++it;
	}

	s_worldBlendMoved.clear();
	s_worldBlendLastMoved = nullptr;
	s_worldBlendUpdate++;
}

// Trans, after every model moved and before anything is drawn
static void(__cdecl* Trans_orig)();
static void __cdecl Trans_hook()
{
	MotionWorldBlend_apply();
	Trans_orig();
}

void re4t::init::MotionFixes()
{
	s_pEnabled = &re4t::cfg->bMotionFixes;

	// Rotation channels (kind 2):
	auto pattern = hook::pattern("8A 45 88 A8 02 0F 84 ? ? ? ? D9 85 70 FF FF FF DD 05 ? ? ? ? D8 C1 D9 85 74 FF FF FF DE D9 DF E0 F6 C4 05 7B ? 80 7D 90 00 74 ? 80 7D 8F 00 75");
	uintptr_t rot = (uintptr_t)pattern.count(1).get(0).get<uint8_t>(0);
	s_rotContinue = rot + 40;
	s_rotHermite = s_rotContinue + *(int8_t*)(rot + 39);
	injector::MakeJMP(rot + 35, MotionMoveCore_Rotation_hook, true);

	// loc_662029, the rotation's plain HermiteInterpolation:
	pattern = hook::pattern("8B 4D 90 DD D8 8B 55 98 8B 85 60 FF FF FF 51 52 50 6A 00 8B CE 8D 9E A0 00 00 00 E8 ? ? ? ? 50 8D 8D 70 FF FF FF 53 51 E8");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(41), HermiteInterpolation_orig);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(41), MotionMoveCore_HermiteRotation_hook, HookType::Call);

	// IK chain rotations (kind 0x10/0x20):
	pattern = hook::pattern("A8 30 74 ? 8B 4D 90 8B 55 98 8B 85 60 FF FF FF 51 52 50 6A 00 8B CE 8D 9E A0 00 00 00 E8 ? ? ? ? 50 8D 8D 70 FF FF FF 53 51 E8");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(43), MotionMoveCore_HermiteRotation_hook, HookType::Call);

	// Positions:
	pattern = hook::pattern("A8 04 74 ? 8B 55 90 8B 45 98 8B 8D 60 FF FF FF 52 50 51 6A 01 8B CE 8D 9E 94 00 00 00 E8 ? ? ? ? 50 8D 95 70 FF FF FF 53 52 E8");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(43), MotionMoveCore_HermitePosition_hook, HookType::Call);

	// Scale:
	pattern = hook::pattern("A8 08 74 ? 8B 45 90 8B 4D 98 8B 95 60 FF FF FF 50 51 52 6A 02 8B CE E8 ? ? ? ? 50 81 C6 AC 00 00 00 8D 85 70 FF FF FF 56 50 E8");
	InjectHook(pattern.count(1).get(0).get<uint32_t>(43), MotionMoveCore_HermiteScale_hook, HookType::Call);

	// WinMain's frame loop
	pattern = hook::pattern("85 72 54 75 05 E8 ? ? ? ? 8D 85 5C FE FF FF 50 FF D3");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(5), Trans_orig);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(5), Trans_hook, HookType::Call);

	// Trans, ObjMgr's alive list
	pattern = hook::pattern("56 8B 35 ? ? ? ? 85 F6 74 ? 8B C6 80 B8 2E 01 00 00 02 8B 76 08");
	s_objAlive = *pattern.count(1).get(0).get<cModel**>(3);

	// Trans, EmMgr's alive list:
	pattern = hook::pattern("8B 35 ? ? ? ? 85 F6 74 ? 8B C6 8B 76 08 3B 05");
	s_emAlive = *pattern.count(1).get(0).get<cModel**>(2);

	// cModel::matUpdate
	pattern = hook::pattern("83 BE F4 00 00 00 00 74 ? 8B CE E8 ? ? ? ? 8B CE E8 ? ? ? ? 56 8D 8E 64 01 00 00");
	ReadCall(pattern.count(1).get(0).get<uint32_t>(18), cModel__partsWorldCalc);
}
