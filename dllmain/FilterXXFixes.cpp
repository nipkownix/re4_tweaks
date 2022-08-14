#include <iostream>
#include "dllmain.h"
#include "Settings.h"

struct Filter01Params
{
	void* OtHandle; // OT = render pass or something
	int Distance;
	float AlphaLevel;
	uint8_t FocusMode;
	uint8_t UnkD; // if set then it uses code-path with constant 4-pass loop, else uses pass with loop based on AlphaLevel
};

float* ptr_InternalWidth;
float* ptr_InternalHeight;
float* ptr_InternalHeightScale;
int* ptr_RenderHeight;
uint32_t* ptr_filter01_buff;

void(__cdecl* GXBegin)(int a1, int a2, short a3);
void(__cdecl* GXPosition3f32)(float a1, float a2, float a3);
void(__cdecl* GXColor4u8)(uint8_t a1, uint8_t a2, uint8_t a3, uint8_t a4);
void(__cdecl* GXTexCoord2f32)(float a1, float a2);
void(__cdecl* GXShaderCall_Maybe)(int a1);

void(__cdecl* GXSetTexCopySrc)(short a1, short a2, short a3, short a4);
void(__cdecl* GXSetTexCopyDst)(short a1, short a2, int a3, char a4);
void(__cdecl* GXCopyTex)(uint32_t a1, char a2, int a3);

void(__cdecl* Filter0aGXDraw_Orig)(
	float PositionX, float PositionY, float PositionZ,
	float TexOffsetX, float TexOffsetY,
	float ColorA,
	float ScreenSizeDivisor, int IsMaskTex);

uint32_t* ptr_Filter0aGXDraw_call1;
uint32_t* ptr_Filter0aGXDraw_call2;
uint32_t* ptr_Filter0aGXDraw_call3;
uint8_t* ptr_Filter0aDrawBuffer_GetEFBCall;
int32_t* ptr_filter0a_shader_num;

int filter0a_ecx_value = 0;
// Filter0aGXDraw_Orig makes use of ECX for part of screen-size division code
// So we need a trampoline to set that first before calling it...
void __declspec(naked) Filter0aGXDraw_Orig_Tramp(float PositionX, float PositionY, float PositionZ,
	float TexOffsetX, float TexOffsetY,
	float ColorA,
	float ScreenSizeDivisor, int IsMaskTex)
{
	__asm
	{
		mov ecx, [filter0a_ecx_value]
		jmp Filter0aGXDraw_Orig
	}
}

void __cdecl Filter0aGXDraw_Hook(
	float PositionX, float PositionY, float PositionZ,
	float TexOffsetX, float TexOffsetY,
	float ColorA,
	float ScreenSizeDivisor, int IsMaskTex)
{
	// We lost the ECX parameter since VC doesn't have any caller-saved calling conventions that use it, game probably had LTO or something to create a custom one
	// Luckily the game only uses a couple different values for it, so we can work out what it was pretty easily
	filter0a_ecx_value = (int)ScreenSizeDivisor;
	if (ColorA == 255 && ScreenSizeDivisor == 4 && IsMaskTex == 0)
		filter0a_ecx_value = 1; // special case for first GXDraw call inside Filter0aDrawBuffer

	// Run original Filter0aGXDraw for it to handle the GXInitTexObj etc things
	// (needs to be patched to make it skip over the things we reimpl. here first though!)
	Filter0aGXDraw_Orig_Tramp(PositionX, PositionY, PositionZ, TexOffsetX, TexOffsetY, ColorA, ScreenSizeDivisor, IsMaskTex);

	float GameWidth = *ptr_InternalWidth;
	float GameHeight = *ptr_InternalHeight;

	// Internally the game is scaled at a seperate aspect ratio, causing some invisible black-bars around the image, which GX drawing code has to skip over
	// With this code we first work out the difference between that internal height & the actual render height, which gives us the size of both black bars
	// Then work out the proportion of 1 black bar by dividing by "heightInternalScaled + heightInternalScaled" (which is just a more optimized way of dividing by heightInternalScaled and then again dividing result by 2)
	// Finally multiply that proportion by the games original GC height to get the size of black-bar in 'GC-space'
	float heightInternalScaled = *ptr_InternalHeightScale * 448.0f;
	float blackBarSize = 448.0f * (heightInternalScaled - *ptr_RenderHeight) / (heightInternalScaled + heightInternalScaled);

	// GameHeight seems to contain both black bars inside it already
	// So to scale that properly we first need to remove both black bars, scale it by ScreenSizeDivisor, then add 1 black-bar back to skip over the top black-bar area
	float pos_bottom = PositionY + ((GameHeight - (blackBarSize * 2)) / ScreenSizeDivisor) + blackBarSize;
	float pos_top = PositionY + blackBarSize;
	float pos_left = PositionX;
	float pos_right = PositionX + GameWidth / ScreenSizeDivisor;

	// Prevent bleed-thru of certain effects into the transparency-drawing area
	pos_left += -0.25f;

	// Roughly center the blur-mask-effect inside the scope
	// TODO: GC version seems to have no blur in the inner-scope at all, any way we can fix this to make it the same here?
	if (IsMaskTex)
	{
		pos_top += -1.0f;
		pos_bottom += 4.0f;
		pos_left += -4.0f;
		pos_right += 4.0f;
	}

	GXBegin(0x80, 0, 4);

	GXPosition3f32(pos_left, pos_top, PositionZ);
	GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)ColorA);
	GXTexCoord2f32(0.0f + TexOffsetX, 0.0f + TexOffsetY);

	GXPosition3f32(pos_right, pos_top, PositionZ);
	GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)ColorA);
	GXTexCoord2f32(1.0f + TexOffsetX, 0.0f + TexOffsetY);

	GXPosition3f32(pos_right, pos_bottom, PositionZ);
	GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)ColorA);
	GXTexCoord2f32(1.0f + TexOffsetX, 1.0f + TexOffsetY);

	GXPosition3f32(pos_left, pos_bottom, PositionZ);
	GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)ColorA);
	GXTexCoord2f32(0.0f + TexOffsetX, 1.0f + TexOffsetY);

	int filter0a_shader_num = *ptr_filter0a_shader_num;
	GXShaderCall_Maybe(filter0a_shader_num);
}

void __cdecl Filter01Render_Hook1(Filter01Params* params)
{
	float blurPositionOffset = params->AlphaLevel * 0.33f;

	float GameWidth = *ptr_InternalWidth;
	float GameHeight = *ptr_InternalHeight;

	// Code that was in original Filter01Render
	// Seems to usually be 1/8th of 448, might change depending on ptr_InternalHeightScale / ptr_RenderHeight though
	// Offset is used to fix Y coords of the blur effect, otherwise the blur ends up offset from the image for some reason
	float heightScaled = *ptr_InternalHeightScale * 448.0f;
	float offsetY = 448.0f * (heightScaled - *ptr_RenderHeight) / (heightScaled + heightScaled);

	float deltaX = 0;
	float deltaY = 0;
	float posZ = (float)params->Distance;
	for (int loopIdx = 0; loopIdx < 4; loopIdx++)
	{
		// Main reason for this hook is to add the deltaX/deltaY vars below, needed for the DoF effect to work properly
		// GC debug had this, but was removed in X360 port onwards for some reason
		// (yet even though this was removed, the X360 devs did add the code for offsetY above to fix the blur...)

		if (loopIdx == 0) {
			deltaX = 0.0f - blurPositionOffset;
			deltaY = 0.0f - blurPositionOffset;
		}
		else if (loopIdx == 1) {
			deltaX = 0.0f + blurPositionOffset;
			deltaY = 0.0f + blurPositionOffset;
		}
		else if (loopIdx == 2) {
			deltaX = 0.0f + blurPositionOffset;
			deltaY = 0.0f - blurPositionOffset;
		}
		else if (loopIdx == 3) {
			deltaX = 0.0f - blurPositionOffset;
			deltaY = 0.0f + blurPositionOffset;
		}

		posZ = posZ + 100.0f;
		if (posZ > 65536)
			posZ = 65536;

		GXBegin(0x80, 0, 4);

		GXPosition3f32(0.0f + deltaX, 0.0f + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(0.0f, 0.0f);

		GXPosition3f32(0.0f + deltaX + GameWidth, 0.0f + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(1.0f, 0.0f);

		GXPosition3f32(0.0f + deltaX + GameWidth, 0.0f + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(1.0f, 1.0f);

		GXPosition3f32(0.0f + deltaX, 0.0f + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(0.0f, 1.0f);

		GXShaderCall_Maybe(0xE);

		if (pConfig->bUseEnhancedGCBlur)
		{
			// 2nd blur step - Not present on the original code. We do this to blur the image a bit more, which makes the effect
			// look better on modern high-definition displays.
			float blurAlpha = 225.0f;

			GXSetTexCopySrc(0, 0, (short)GameWidth, (short)GameHeight);
			GXSetTexCopyDst(((short)GameWidth) >> 1, ((short)GameHeight) >> 1, 6, 1);
			GXCopyTex(*ptr_filter01_buff, 0, 0);

			GXBegin(0x80, 0, 4);

			float Xnew = -0.6472f;
			float Ynew = -0.6472f;

			GXPosition3f32(Xnew, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 1.0f);

			GXPosition3f32(Xnew, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 1.0f);

			GXShaderCall_Maybe(0xE);

			// 3rd blur step - Also not present on the original code.
			GXBegin(0x80, 0, 4);

			Xnew = -0.1f;
			Ynew = -0.1f;

			GXPosition3f32(Xnew, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 1.0f);

			GXPosition3f32(Xnew, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 1.0f);

			GXShaderCall_Maybe(0xE);
		}

	}

	if (params->AlphaLevel > 1)
	{
		float blurAlpha = ((float)params->AlphaLevel - 1.0f) * 25.0f;
		if (blurAlpha > 255.0f)
			blurAlpha = 255.0f;

		if (blurAlpha > 0.0f)
		{
			GXSetTexCopySrc(0, 0, (short)GameWidth, (short)GameHeight);
			GXSetTexCopyDst(((short)GameWidth) >> 1, ((short)GameHeight) >> 1, 6, 1);
			GXCopyTex(*ptr_filter01_buff, 0, 0);

			GXBegin(0x80, 0, 4);

			GXPosition3f32(0.25f + deltaX, 0.25f + deltaY + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 0.0f);

			GXPosition3f32(0.25f + deltaX + GameWidth, 0.25f + deltaY + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 0.0f);

			GXPosition3f32(0.25f + deltaX + GameWidth, 0.25f + deltaY + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 1.0f);

			GXPosition3f32(0.25f + deltaX, 0.25f + deltaY + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 1.0f);

			GXShaderCall_Maybe(0xE);
		}
	}
}

void __cdecl Filter01Render_Hook2(Filter01Params* params)
{
	float GameWidth = *ptr_InternalWidth;
	float GameHeight = *ptr_InternalHeight;

	std::vector<float> AlphaLvlTable = { // level_tbl1 in GC debug
		0.0f,
		0.60f, // 0.60000002
		1.5f,
		2.60f, // 2.5999999
		1.5f,
		1.6f,
		1.6f,
		2.5f,
		2.60f, // 2.5999999
		4.5f,
		6.60f // 6.5999999
	};

	// Replace some values to reduce ghosting a bit
	if (pConfig->bUseEnhancedGCBlur)
	{
		AlphaLvlTable = {
			0.0f,
			0.6f,
			1.5f,
			1.7f,
			1.5f,
			1.6f,
			1.6f,
			1.8f,
			1.9f,
			2.5f,
			3.60f
		};
	}

	int AlphaLevel_floor = (int)floorf(params->AlphaLevel); // TODO: should this be ceil instead?
	float blurPositionOffset = AlphaLvlTable[AlphaLevel_floor];

	int numBlurPasses = 4;
	switch (AlphaLevel_floor)
	{
	case 0:
		numBlurPasses = 0;
		break;
	case 1:
	case 2:
	case 3:
		numBlurPasses = 1;
		break;
	case 4:
		numBlurPasses = 2;
		break;
	case 5:
		numBlurPasses = 3;
		break;
	}

	// Code that was in original Filter01Render
	// Seems to usually be 1/8th of 448, might change depending on ptr_InternalHeightScale / ptr_RenderHeight though
	// Offset is used to fix Y coords of the blur effect, otherwise the blur ends up offset from the image for some reason
	float heightScaled = *ptr_InternalHeightScale * 448.0f;
	float offsetY = 448.0f * (heightScaled - *ptr_RenderHeight) / (heightScaled + heightScaled);

	float deltaX = 0.0f;
	float deltaY = 0.0f;
	float posZ = (float)params->Distance;
	for (int loopIdx = 0; loopIdx < numBlurPasses; loopIdx++)
	{
		// Main reason for this hook is to add the deltaX/deltaY vars below, needed for the DoF effect to work properly
		// GC debug had this, but was removed in X360 port onwards for some reason
		// (yet even though this was removed, the X360 devs did add the code for offsetY above to fix the blur...)

		if (loopIdx == 0) {
			deltaX = 0.0f - blurPositionOffset;
			deltaY = 0.0f - blurPositionOffset;
		}
		else if (loopIdx == 1) {
			deltaX = 0.0f + blurPositionOffset;
			deltaY = 0.0f + blurPositionOffset;
		}
		else if (loopIdx == 2) {
			deltaX = 0.0f + blurPositionOffset;
			deltaY = 0.0f - blurPositionOffset;
		}
		else if (loopIdx == 3) {
			deltaX = 0.0f - blurPositionOffset;
			deltaY = 0.0f + blurPositionOffset;
		}

		posZ = posZ + 100.f;
		if (posZ > 65536.0f)
			posZ = 65536.0f;

		GXBegin(0xA0, 0, 4); // TODO: 0xA0 (GX_TRIANGLEFAN) in PC, 0x80 (GX_QUADS) in GC - should this be changed?

		GXPosition3f32(0.0f + deltaX, 0.0f + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(0.0f, 0.0f);

		GXPosition3f32(0.0f + deltaX + GameWidth, 0.0f + deltaY + offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(1.0f, 0.0f);

		GXPosition3f32(0.0f + deltaX + GameWidth, 0.0f + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(1.0f, 1.0f);

		GXPosition3f32(0.0f + deltaX, 0.0f + deltaY + GameHeight - offsetY, posZ);
		GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)0x80);
		GXTexCoord2f32(0.0f, 1.0f);

		GXShaderCall_Maybe(0xE);

		if (pConfig->bUseEnhancedGCBlur)
		{
			// 2nd blur step - Not present on the original code. We do this to blur the image a bit more, which makes the effect
			// look better on modern high-definition displays.
			float blurAlpha = 225.0f;

			GXSetTexCopySrc(0, 0, (short)GameWidth, (short)GameHeight);
			GXSetTexCopyDst(((short)GameWidth) >> 1, ((short)GameHeight) >> 1, 6, 1);
			GXCopyTex(*ptr_filter01_buff, 0, 0);

			GXBegin(0x80, 0, 4);

			float Xnew = -0.6472f;
			float Ynew = -0.6472f;

			GXPosition3f32(Xnew, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 1.0f);

			GXPosition3f32(Xnew, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 1.0f);

			GXShaderCall_Maybe(0xE);

			// 3rd blur step - Also not present on the original code.
			GXBegin(0x80, 0, 4);

			Xnew = -0.1f;
			Ynew = -0.1f;

			GXPosition3f32(Xnew, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 0.0f);

			GXPosition3f32(Xnew + GameWidth, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(1.0f, 1.0f);

			GXPosition3f32(Xnew, Ynew + GameHeight - offsetY, posZ);
			GXColor4u8(0xFF, 0xFF, 0xFF, (uint8_t)blurAlpha);
			GXTexCoord2f32(0.0f, 1.0f);

			GXShaderCall_Maybe(0xE);
		}
	}
}

void GetFilterPointers()
{
	// GC blur fix
	auto pattern = hook::pattern("E8 ? ? ? ? 03 5D ? 6A ? 53 57 E8 ? ? ? ? 0F B6 56");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXBegin);

	pattern = hook::pattern("E8 ? ? ? ? D9 45 ? 8B 75 ? 8B 7D ? D9 7D");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXPosition3f32);

	pattern = hook::pattern("E8 ? ? ? ? 8B 5D ? 8B 55 ? 83 C4 ? 6A ? 53");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXColor4u8);

	pattern = hook::pattern("E8 ? ? ? ? 8B 4D ? 51 E8 ? ? ? ? 8B 4D ? 83 C4 ? 33 CD");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXTexCoord2f32);

	pattern = hook::pattern("E8 ? ? ? ? 47 83 C4 ? 83 C6 ? 3B 3D ? ? ? ? 0F 82");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXShaderCall_Maybe);

	pattern = hook::pattern("E8 ? ? ? ? 0F B7 4E ? 0F B7 56 ? 6A ? 6A ? 51");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXSetTexCopySrc);

	pattern = hook::pattern("E8 ? ? ? ? 6A 01 6A 06 D9");
	uint32_t* varPtr = pattern.count(1).get(0).get<uint32_t>(11);
	ptr_InternalHeight = (float*)*varPtr;

	pattern = hook::pattern("D1 E8 50 D9 AD ? ? ? ? D9 05 ? ? ? ?");
	varPtr = pattern.count(1).get(0).get<uint32_t>(11);
	ptr_InternalWidth = (float*)*varPtr;

	pattern = hook::pattern("E8 ? ? ? ? A1 ? ? ? ? 6A 04 6A 00 50 E8");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXSetTexCopyDst);

	pattern = hook::pattern("D1 E9 51 D9 AD ? ? ? ? E8 ? ? ? ? 8B 15");
	varPtr = pattern.count(1).get(0).get<uint32_t>(16);
	ptr_filter01_buff = (uint32_t*)*varPtr;

	pattern = hook::pattern("E8 ? ? ? ? D9 05 ? ? ? ? D9 7D ? 6A ? 0F B7 45");
	ReadCall(injector::GetBranchDestination(pattern.get_first()).as_int(), GXCopyTex);

	pattern = hook::pattern("83 C4 ? D9 ? ? ? ? ? D9 05 ? ? ? ? 8B ? ? ? ? ? DD");
	varPtr = pattern.count(1).get(0).get<uint32_t>(11);
	ptr_InternalHeightScale = (float*)*varPtr;

	varPtr = pattern.count(1).get(0).get<uint32_t>(17);
	ptr_RenderHeight = (int*)*varPtr;

	pattern = hook::pattern("E8 ? ? ? ? 33 DB 83 C4 20 39 1D ? ? ? ? 0F 86 ? ? ? ? A1 ? ? ? ? 56");
	ptr_Filter0aGXDraw_call1 = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("E8 ? ? ? ? 46 83 C4 20 3B F7 0F 8C ? ? ? ? A1 ? ? ? ? 43 3B 1D ? ? ? ? 0F 82 ? ? ? ? 5F");
	ptr_Filter0aGXDraw_call2 = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("E8 ? ? ? ? 6A 01 6A 03 6A 01 E8 ? ? ? ? 6A 01 E8 ? ? ? ? 8B 4D FC 83 C4 30 33 CD 5B E8 ? ? ? ? 8B E5 5D C3 90");
	ptr_Filter0aGXDraw_call3 = pattern.count(1).get(0).get<uint32_t>(0);

	pattern = hook::pattern("5F 5E 6A 01 6A 01 E8 ? ? ? ? 83 C4 ? E8");
	ptr_Filter0aDrawBuffer_GetEFBCall = pattern.count(1).get(0).get<uint8_t>(3);

	pattern = hook::pattern("56 6A 00 50 C7 05 ? ? ? ? 1F 00 00 00");
	varPtr = pattern.count(1).get(0).get<uint32_t>(6);
	ptr_filter0a_shader_num = (int32_t*)*varPtr;
}

void Init_FilterXXFixes()
{
	GetFilterPointers();

	if (pConfig->bEnableGCBlur)
	{
		// Hook Filter01Render, first block (loop that's ran 4 times + 0.25 pass)
		auto pattern = hook::pattern("3C 01 0F 85 ? ? ? ? D9 85");
		uint8_t* filter01_end = pattern.count(1).get(0).get<uint8_t>(4);
		filter01_end += sizeof(int32_t) + *(int32_t*)filter01_end;

		injector::WriteMemory(pattern.get_first(8), uint8_t(0x56), true); // PUSH ESI (esi = Filter01Params*)
		injector::MakeCALL(pattern.get_first(9), Filter01Render_Hook1, true);
		injector::WriteMemory(pattern.get_first(14), uint8_t(0x58), true); // POP EAX (fixes esp)
		injector::MakeJMP(pattern.get_first(15), filter01_end, true); // JMP over code that was reimplemented

		// Hook Filter01Render second block (loop ran N times depending on AlphaLevel)
		pattern = hook::pattern("D9 46 ? B9 04 00 00 00 D9");

		injector::WriteMemory(pattern.get_first(0), uint8_t(0x56), true); // PUSH ESI (esi = Filter01Params*)
		injector::MakeCALL(pattern.get_first(1), Filter01Render_Hook2, true);
		injector::WriteMemory(pattern.get_first(6), uint8_t(0x58), true); // POP EAX (fixes esp)
		injector::MakeJMP(pattern.get_first(7), filter01_end, true); // JMP over code that was reimplemented

		spd::log()->info("{} -> EnableGCBlur applied", __FUNCTION__);
	}

	if (pConfig->bEnableGCScopeBlur)
	{
		// Short-circuit Filter0aGXDraw to skip over the GXPosition etc things that we reimplement ourselves
		auto pattern = hook::pattern("D9 45 A4 DC 15 ? ? ? ? DF E0 F6 C4 41 75 ? DC 1D ? ? ? ? DF E0 F6 C4 05 0F 8B ? ? ? ? EB");
		injector::WriteMemory(pattern.get_first(27), uint16_t(0xE990), true);

		// Change GetEFB(1,1) to GetEFB(4,4)
		injector::WriteMemory(ptr_Filter0aDrawBuffer_GetEFBCall, uint8_t(0x4), true);
		injector::WriteMemory(ptr_Filter0aDrawBuffer_GetEFBCall + 2, uint8_t(0x4), true);

		ReadCall(ptr_Filter0aGXDraw_call1, Filter0aGXDraw_Orig);
		InjectHook(ptr_Filter0aGXDraw_call1, Filter0aGXDraw_Hook);
		InjectHook(ptr_Filter0aGXDraw_call2, Filter0aGXDraw_Hook);
		InjectHook(ptr_Filter0aGXDraw_call3, Filter0aGXDraw_Hook);

		spd::log()->info("{} -> EnableGCScopeBlur applied", __FUNCTION__);
	}
}