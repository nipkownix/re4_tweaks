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
	Mtx v25; // [sp+30h] [-80h] BYREF
	MTXConcat(GlobalPtr()->Camera_74.v_mat_30, pSat->mat_60, v25);

	Vec v24[3]; // [sp+8h] [-A8h] BYREF
	for (int i = 0; i < 3; i++)
	{
		int v14 = pSat->poly_p_18[poly_num][i];
		v24[i] = pSat->vert_p_C[v14];
	}

	GXColor v18; // r6
	if ((mode & 3) != 0)
	{
		if ((mode & 3) == 1)
			Draw_poly_local(v24, v25, col, 1);
	}
	else
	{
		*(uint32_t*)&v18 = 0x80808080;
		if ((pSat->poly_p_18[poly_num][4] & 0x2000) == 0)
			v18 = col;
		Draw_line3d_local(v24, &v24[1], v25, v18, 0);
		*(uint32_t*)&v18 = 0x80808080;
		if ((pSat->poly_p_18[poly_num][4] & 0x4000) == 0)
			v18 = col;
		Draw_line3d_local(&v24[1], &v24[2], v25, v18, 0);
		*(uint32_t*)&v18 = 0x80808080;
		if ((pSat->poly_p_18[poly_num][4] & 0x8000) == 0)
			v18 = col;
		Draw_line3d_local(v24, &v24[2], v25, v18, 0);
	}
	VECAdd(v24, &v24[1], v24);

	v18.r = v18.g = v18.b = 0;
	v18.a = 0xFF;

	VECAdd(v24, &v24[2], v24);
	VECScale(v24, v24, 0.33333334f);

	uint16_t v23 = pSat->poly_p_18[poly_num][3];

	Vec v26 = pSat->norm_p_10[v23]; // [sp+60h] [-50h] BYREF
	VECScale(&v26, &v24[1], 100);
	VECAdd(v24, &v24[1], &v24[1]);

	Vec v27; // [sp+70h] [-40h] BYREF
	MTXMultVec(pSat->mat_60, v24, &v27);
	VECSubtract(&v27, &GlobalPtr()->Camera_74.CamPoint_A4.Campos_0, &v27);
	MTXMultVecSR(pSat->mat_60, &v26, &v26);
	if (VECDotProduct(&v26, &v27) > 0.0f)
		*(uint32_t*)&v18 = 0xFFFFFFFF;

	Draw_line3d_local(v24, &v24[1], v25, v18, 0);
}

void cSatMgr__disp(cSatMgr* pMgr, uint32_t mode)
{
	//GXSetLineWidth(6u, 0);
	if (!pMgr->m_nArray_8)
		return;

	for (cSat& pAt : *pMgr)
	{
		if (!pAt.IsValid())
			continue;
		if ((pAt.m_Flag_2A & cSat::FLAG::FLAG_ENABLE_VAL) != cSat::FLAG::FLAG_ENABLE_VAL)
			continue;

		int start_idx = 0;
		int poly_count = 0;

		switch (mode & 0xF)
		{
		case 1:
			start_idx = 0;
			poly_count = pAt.floor_num_20;
			break;
		case 2:
			start_idx = pAt.floor_num_20;
			poly_count = start_idx + pAt.slope_num_22;
			break;
		case 3:
			poly_count = pAt.polygon_num_1E;
			start_idx = poly_count - pAt.wall_num_24;
			break;
		case 0:
			start_idx = 0;
			poly_count = pAt.polygon_num_1E;
		}

		if (start_idx < poly_count)
		{
			int i = start_idx;
			uint32_t v4 = (mode >> 8) & 0xFF0000;

			for (int i = start_idx; i < poly_count; i++)
			{
				uint32_t col = (LOBYTE(pAt.poly_p_18[i][8]) << 0x10) | pAt.poly_p_18[i][9];
				uint32_t disp_mode = 0;
				if (col)
				{
					col = col | 0x40000000;
					if (v4)
					{
						col = 0;
						if ((pAt.poly_p_18[i][9] & 1) != 0)
							col = 0x80808080;
					}
					disp_mode = 1;
				}
				else
				{
					col = 0xA0A0A0A0;
					if (!v4)
						col = 0xFFFFFFFF;
					disp_mode = 0;
				}
				cSat__disp(&pAt, i, *(GXColor*)&col, disp_mode);
			}
		}
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
