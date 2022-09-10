// Recreates certain debug-related displays missing from UHD build
// Currently recreates effect of:
// - DBG_SAT_DISP
// - DBG_EAT_DISP

#include "dllmain.h"
#include "Game.h"

namespace bio4
{
	void(__cdecl* GXSetBlendMode)(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor, GXLogicOp op);
	void(__cdecl* GXSetCullMode)(GXCullMode mode);
	void(__cdecl* GXSetZMode)(bool compare_enable, GXCompare func, bool update_enable);
	void(__cdecl* GXSetNumChans)(u8 nChans);
	void(__cdecl* GXSetChanCtrl)(GXChannelID chan, bool enable, GXColorSrc amb_src, GXColorSrc mat_src,
		u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn);
	void(__cdecl* GXSetNumTexGens)(u8 nTexGens);
	void(__cdecl* GXSetNumTevStages)(u8 nStages);
	void(__cdecl* GXSetTevOp)(GXTevStageID id, GXTevMode mode);
	void(__cdecl* GXSetTevOrder)(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color);
	void(__cdecl* GXClearVtxDesc)();
	void(__cdecl* GXSetVtxDesc)(GXAttr attr, GXAttrType type);
	void(__cdecl* GXSetVtxAttrFmt)(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac);
	void(__cdecl* GXLoadPosMtxImm)(const Mtx mtx, u32 id);
	void(__cdecl* GXSetCurrentMtx)(u32 id);
	void(__cdecl* GXBegin)(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
	void(__cdecl* GXEnd)(int a1);

	void(__cdecl* GXPosition3f32)(f32 x, f32 y, f32 z);
	void(__cdecl* GXColor4u8)(u8 r, u8 g, u8 b, u8 a);

	void(__cdecl* CameraCurrentProjection)();
};

void Draw_line3d_local(Vec* p0, Vec* p1, Mtx mat, GXColor col, uint32_t blend)
{
	if (blend)
		bio4::GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
	else
		bio4::GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);

	bio4::CameraCurrentProjection();
	bio4::GXSetCullMode(GX_CULL_NONE);
	if (col.a == 0xFE)
		bio4::GXSetZMode(false, GX_LEQUAL, true);
	else
		bio4::GXSetZMode(true, GX_LEQUAL, true);
	bio4::GXSetNumChans(1);
	bio4::GXSetChanCtrl(GX_COLOR0A0, false, GX_SRC_REG, GX_SRC_VTX, 0,
		GX_DF_NONE, GX_AF_NONE);

	bio4::GXSetNumTexGens(0);
	bio4::GXSetNumTevStages(1);
	bio4::GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	bio4::GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	bio4::GXClearVtxDesc();
	bio4::GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	bio4::GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	bio4::GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_RGBA6, 0);
	bio4::GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_POS_XYZ, GX_RGBA8, 0);
	bio4::GXLoadPosMtxImm(mat, 0);
	bio4::GXSetCurrentMtx(0);
	bio4::GXBegin(GX_LINESTRIP, GX_VTXFMT0, 2);

	//MEMORY[0xCC008000] = p0->z;
	//MEMORY[0xCC008000] = 0xFF;
	//MEMORY[0xCC008000] = p1->z;
	//MEMORY[0xCC008000] = 0xFF;
	bio4::GXPosition3f32(p0->x, p0->y, p0->z);
	bio4::GXColor4u8(col.r, col.g, col.b, 0xFF);

	bio4::GXPosition3f32(p1->x, p1->y, p1->z);
	bio4::GXColor4u8(col.r, col.g, col.b, 0xFF);

	bio4::GXEnd(0);
}

void Draw_poly_local(Vec* dpos, Mtx mat, GXColor col, bool zmode)
{
	bio4::CameraCurrentProjection();
	bio4::GXSetCullMode(GX_CULL_NONE);
	if (zmode)
		bio4::GXSetZMode(1, GX_LEQUAL, 1);
	else
		bio4::GXSetZMode(1, GX_LEQUAL, 0);
	bio4::GXSetNumChans(1);
	bio4::GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_VTX, GX_SRC_VTX, 0, GX_DF_NONE, GX_AF_NONE);
	bio4::GXSetNumTexGens(0);
	bio4::GXSetNumTevStages(1u);
	bio4::GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	bio4::GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	bio4::GXClearVtxDesc();
	bio4::GXSetVtxDesc(GX_VA_PNMTXIDX, GX_DIRECT);
	bio4::GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
	bio4::GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	bio4::GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	bio4::GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_POS_XYZ, GX_RGBA8, 0);
	bio4::GXLoadPosMtxImm(mat, 0);
	bio4::GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	bio4::GXBegin(GX_TRIANGLES, GX_VTXFMT0, 3u);

	//MEMORY[0xCC008000] = 0;
	//MEMORY[0xCC008000] = dpos->z;
	//MEMORY[0xCC008000] = 0;
	//v9 = dpos + 1;
	//MEMORY[0xCC008000] = v9->z;
	//MEMORY[0xCC008000] = 0;
	//MEMORY[0xCC008000] = v9[1].z;
	//MEMORY[0xCC008000] = v7;

	bio4::GXPosition3f32(dpos[0].x, dpos[0].y, dpos[0].z);
	bio4::GXColor4u8(col.r, col.g, col.b, col.a);

	bio4::GXPosition3f32(dpos[1].x, dpos[1].y, dpos[1].z);
	bio4::GXColor4u8(col.r, col.g, col.b, col.a);

	bio4::GXPosition3f32(dpos[2].x, dpos[2].y, dpos[2].z);
	bio4::GXColor4u8(col.r, col.g, col.b, col.a);

	bio4::GXEnd(0);
}

void cSat__disp(cSat* pSat, uint32_t poly_num, GXColor col, uint32_t mode)
{
	uint16_t(*v7)[10]; // r22
	float(*v9)[3]; // r31
	unsigned int v10; // r8
	uint32_t v11; // r20
	uint16_t* v12; // r30
	unsigned int v13; // r10
	int v14; // r9
	float* v16; // r9
	uint16_t* v17; // r30
	GXColor v18; // r6
	float(*v22)[3]; // r8
	uint16_t* v23; // r11
	Vec v24[3]; // [sp+8h] [-A8h] BYREF
	Mtx v25; // [sp+30h] [-80h] BYREF
	Vec v26; // [sp+60h] [-50h] BYREF
	Vec v27; // [sp+70h] [-40h] BYREF

	GLOBAL_WK* pG = GlobalPtr();
	v7 = pSat->poly_p_18;
	v9 = pSat->vert_p_C;
	MTXConcat(pG->Camera_74.v_mat_30, pSat->mat_60, v25);
	v10 = 0;
	v11 = poly_num;
	v12 = v7[poly_num];
	do
	{
		v13 = v10;
		v14 = v12[v10];
		v10 = (unsigned __int16)(v10 + 1);
		v14 *= 0xC;
		v16 = (float*)((char*)v9 + v14);
		v24[v13].x = v16[0];
		v24[v13].y = v16[1];
		v24[v13].z = v16[2];
	} while (v10 <= 2);
	if ((mode & 3) != 0)
	{
		if ((mode & 3) == 1)
			Draw_poly_local(v24, v25, col, 1);
	}
	else
	{
		v17 = v7[v11];
		*(uint32_t*)&v18 = 0x80808080;
		if ((v7[v11][4] & 0x2000) == 0)
			v18 = col;
		Draw_line3d_local(v24, &v24[1], v25, v18, 0);
		*(uint32_t*)&v18 = 0x80808080;
		if ((v17[4] & 0x4000) == 0)
			v18 = col;
		Draw_line3d_local(&v24[1], &v24[2], v25, v18, 0);
		*(uint32_t*)&v18 = 0x80808080;
		if ((v17[4] & 0x8000) == 0)
			v18 = col;
		Draw_line3d_local(v24, &v24[2], v25, v18, 0);
	}
	VECAdd(v24, &v24[1], v24);

	v18.r = v18.g = v18.b = 0;
	v18.a = 0xFF;

	VECAdd(v24, &v24[2], v24);
	VECScale(v24, v24, 0.33333334f);
	v22 = pSat->norm_p_10;
	v23 = pSat->poly_p_18[v11];
	v26.x = v22[v23[3]][0];
	v26.y = v22[v23[3]][1];
	v26.z = v22[v23[3]][2];
	VECScale(&v26, &v24[1], 100);
	VECAdd(v24, &v24[1], &v24[1]);
	MTXMultVec(pSat->mat_60, v24, &v27);
	VECSubtract(&v27, &pG->Camera_74.CamPoint_A4.Campos_0, &v27);
	MTXMultVecSR(pSat->mat_60, &v26, &v26);
	if (VECDotProduct(&v26, &v27) > 0.0f)
		*(uint32_t*)&v18 = 0xFFFFFFFF;
	Draw_line3d_local(v24, &v24[1], v25, v18, 0);
}

void cSatMgr__disp(cSatMgr* pMgr, uint32_t mode)
{
	uint32_t v4; // r24
	uint32_t v5; // r10
	uint32_t v6; // r28
	uint32_t v7; // r9
	cSat* v8; // r30
	int v9; // r0
	uint32_t v11; // r0
	signed int v12; // r4
	signed int v13; // r27
	signed int v14; // r31
	int v15; // r29
	uint32_t v16; // r11
	uint32_t v17; // r5
	uint32_t v18; // r6

	//GXSetLineWidth(6u, 0);
	v5 = 0;
	if (pMgr->m_nArray_8)
	{
		do
		{
			v6 = v5 + 1;
			v7 = pMgr->m_blockSize_C * v5;
			v8 = (cSat*)((char*)pMgr->m_Array_4 + v7);
			//if ((int)v8 < 0 && (unsigned int)v8 <= 0x82FFFFFF)
			{
				v9 = *(uint32_t*)((char*)&pMgr->m_Array_4->be_flag_4 + v7);
				v6 = v5 + 1;
				if ((v9 & 0x201) == 1 && (v8->m_Flag_2A & 4) != 0)
				{
					v11 = mode & 0xF;
					v12 = 0;
					v13 = 0;
					if (v11 == 1)
					{
						v12 = 0;
						v13 = v8->floor_num_20;
					}
					else if ((mode & 0xF) != 0)
					{
						if (v11 == 2)
						{
							v12 = v8->floor_num_20;
							v13 = v12 + v8->slope_num_22;
						}
						else if (v11 == 3)
						{
							v13 = v8->polygon_num_1E;
							v12 = v13 - v8->wall_num_24;
						}
					}
					else
					{
						v12 = 0;
						v13 = v8->polygon_num_1E;
					}
					v14 = v12;
					v6 = v5 + 1;
					if (v12 < v13)
					{
						v15 = v12;
						v4 = (mode >> 8) & 0xFF0000;
						do
						{
							v16 = (LOBYTE(v8->poly_p_18[v15][8]) << 0x10) | v8->poly_p_18[v15][9];
							if (v16)
							{
								v17 = v16 | 0x40000000;
								if (v4)
								{
									v17 = 0;
									if ((v8->poly_p_18[v15][9] & 1) != 0)
										v17 = 0x80808080;
								}
								v18 = 1;
							}
							else
							{
								v17 = 0xA0A0A0A0;
								if (!v4)
									v17 = 0xFFFFFFFF;
								v18 = 0;
							}
							cSat__disp(v8, v14++, *(GXColor*)&v17, v18);
							++v15;
						} while (v14 < v13);
					}
				}
			}
			v5 = v6;
		} while (v6 < pMgr->m_nArray_8);
	}
}

void(__cdecl* SubScreenCall)();
void SubScreenCall_Hook()
{
	// TODO: this allows SATs to be displayed, but doesn't show the current polygon touching player in red like GC debug does
	// Maybe that's handled by another debug-display func, or maybe code that would update the cSat for the polygon being touched was removed...

	if (FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_SAT_DISP)))
		cSatMgr__disp(SatMgr, 0);
	if (FlagIsSet(GlobalPtr()->flags_DEBUG_0_60, uint32_t(Flags_DEBUG::DBG_EAT_DISP)))
		cSatMgr__disp(EatMgr, 0);

	SubScreenCall();
}

void Init_DebugDisplay()
{
	auto pattern = hook::pattern("56 53 6A 05 6A 04 6A 01 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x8), bio4::GXSetBlendMode); // 0x956E60

	pattern = hook::pattern("53 E8 ? ? ? ? 6A 01 6A 07 53 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x1), bio4::GXSetCullMode); // 0x957130
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xB), bio4::GXSetZMode); // 0x957FE0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x11), bio4::GXSetNumTexGens); // 0x9575F0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x18), bio4::GXSetNumTevStages); // 0x957580
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x20), bio4::GXSetTevOp); // 0x957AC0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x32), bio4::GXSetTevOrder); // 0x957B30

	pattern = hook::pattern("83 C4 48 6A 01 E8 ? ? ? ? 6A 02 53");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x5), bio4::GXSetNumChans); // 0x9574A0

	pattern = hook::pattern("6A 01 6A 01 53 53 6A 04 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x8), bio4::GXSetChanCtrl); // 0x956F50
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xD), bio4::GXClearVtxDesc); // 0x9558C0
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x16), bio4::GXSetVtxDesc); // 0x957F70
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x2C), bio4::GXSetVtxAttrFmt); // 0x957EF0

	pattern = hook::pattern("52 E8 ? ? ? ? 8D 45 ? 53 50 E8 ? ? ? ? 53 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0xB), bio4::GXLoadPosMtxImm); // 0x956310
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x11), bio4::GXSetCurrentMtx); // 0x9571A0

	pattern = hook::pattern("6A 0C 53 68 90 00 00 00 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x8), bio4::GXBegin); // 0x955840

	pattern = hook::pattern("53 53 53 E8 ? ? ? ? 53 E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x3), bio4::GXColor4u8); // 0x955B60
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x9), bio4::GXEnd); // 0x955E50

	pattern = hook::pattern("D9 04 08 D9 1C ? E8");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x6), bio4::GXPosition3f32); // 0x956A70

	pattern = hook::pattern("E8 ? ? ? ? 8A 46 30 3C FD");
	ReadCall(pattern.count(1).get(0).get<uint8_t>(0x0), bio4::CameraCurrentProjection); // 0x59F3A0

	// dbg version includes some extra code after SubScreenCall that we need to change pattern for...
	if (GameVersionIsDebug())
		pattern = hook::pattern("E8 ? ? ? ? 8B 0D ? ? ? ? BE 00 00 20 00");
	else
		pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 25 00 20 00 00 33 C9");

	ReadCall(injector::GetBranchDestination(pattern.count(1).get(0).get<uint8_t>(0x0)).as_int(), SubScreenCall);
	InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0x0)).as_int(), SubScreenCall_Hook);
}
