#pragma once
#include "basic_types.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef float f32;

#include "GXEnums.h"

namespace bio4
{
  extern void(__cdecl* GXSetBlendMode)(GXBlendMode type, GXBlendFactor src_factor, GXBlendFactor dst_factor, GXLogicOp op);
  extern void(__cdecl* GXSetCullMode)(GXCullMode mode);
  extern void(__cdecl* GXSetZMode)(bool compare_enable, GXCompare func, bool update_enable);
  extern void(__cdecl* GXSetNumChans)(u8 nChans);
  extern void(__cdecl* GXSetChanCtrl)(GXChannelID chan, bool enable, GXColorSrc amb_src, GXColorSrc mat_src,
    u32 light_mask, GXDiffuseFn diff_fn, GXAttnFn attn_fn);
  extern void(__cdecl* GXSetNumTexGens)(u8 nTexGens);
  extern void(__cdecl* GXSetNumTevStages)(u8 nStages);
  extern void(__cdecl* GXSetTevOp)(GXTevStageID id, GXTevMode mode);
  extern void(__cdecl* GXSetTevOrder)(GXTevStageID stage, GXTexCoordID coord, GXTexMapID map, GXChannelID color);
  extern void(__cdecl* GXClearVtxDesc)();
  extern void(__cdecl* GXSetVtxDesc)(GXAttr attr, GXAttrType type);
  extern void(__cdecl* GXSetVtxAttrFmt)(GXVtxFmt vtxfmt, GXAttr attr, GXCompCnt cnt, GXCompType type, u8 frac);
  extern void(__cdecl* GXLoadPosMtxImm)(const Mtx mtx, u32 id);
  extern void(__cdecl* GXSetCurrentMtx)(u32 id);
  extern void(__cdecl* GXBegin)(GXPrimitive type, GXVtxFmt vtxfmt, u16 nverts);
  extern void(__cdecl* GXEnd)(int a1);

  extern void(__cdecl* GXPosition3f32)(f32 x, f32 y, f32 z);
  extern void(__cdecl* GXColor4u8)(u8 r, u8 g, u8 b, u8 a);
};
