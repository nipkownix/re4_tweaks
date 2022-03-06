#pragma once

#include <d3d9.h>

class hook_Direct3D9 final : public IDirect3D9
{
public:
	// Inherited via IDirect3D9
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG WINAPI AddRef(void) override;
	virtual ULONG WINAPI Release(void) override;
	virtual HRESULT WINAPI RegisterSoftwareDevice(void* pInitializeFunction) override;
	virtual UINT WINAPI GetAdapterCount(void) override;
	virtual HRESULT WINAPI GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier) override;
	virtual UINT WINAPI GetAdapterModeCount(UINT Adapter, D3DFORMAT Format) override;
	virtual HRESULT WINAPI EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode) override;
	virtual HRESULT WINAPI GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode) override;
	virtual HRESULT WINAPI CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed) override;
	virtual HRESULT WINAPI CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat) override;
	virtual HRESULT WINAPI CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels) override;
	virtual HRESULT WINAPI CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat) override;
	virtual HRESULT WINAPI CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat) override;
	virtual HRESULT WINAPI GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) override;
	virtual HMONITOR WINAPI GetAdapterMonitor(UINT Adapter) override;
	virtual HRESULT WINAPI CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) override;

public:
	hook_Direct3D9(IDirect3D9* direct3D9)
		: m_direct3D9(direct3D9)
	{
	}

private:
	~hook_Direct3D9()
	{
		m_direct3D9->Release();
	}

	LONG m_refCount = 1;
	IDirect3D9* m_direct3D9;
};

class hook_Direct3DDevice9 final : public IDirect3DDevice9
{
public:
	// Inherited via IDirect3DDevice9
	virtual HRESULT WINAPI QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG WINAPI AddRef(void) override;
	virtual ULONG WINAPI Release(void) override;
	virtual HRESULT WINAPI TestCooperativeLevel(void) override;
	virtual UINT WINAPI GetAvailableTextureMem(void) override;
	virtual HRESULT WINAPI EvictManagedResources(void) override;
	virtual HRESULT WINAPI GetDirect3D(IDirect3D9** ppD3D9) override;
	virtual HRESULT WINAPI GetDeviceCaps(D3DCAPS9* pCaps) override;
	virtual HRESULT WINAPI GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode) override;
	virtual HRESULT WINAPI GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters) override;
	virtual HRESULT WINAPI SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap) override;
	virtual void WINAPI SetCursorPosition(int X, int Y, DWORD Flags) override;
	virtual BOOL WINAPI ShowCursor(BOOL bShow) override;
	virtual HRESULT WINAPI CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain) override;
	virtual HRESULT WINAPI GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain) override;
	virtual UINT WINAPI GetNumberOfSwapChains(void) override;
	virtual HRESULT WINAPI Reset(D3DPRESENT_PARAMETERS* pPresentationParameters) override;
	virtual HRESULT WINAPI Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion) override;
	virtual HRESULT WINAPI GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer) override;
	virtual HRESULT WINAPI GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus) override;
	virtual HRESULT WINAPI SetDialogBoxMode(BOOL bEnableDialogs) override;
	virtual void WINAPI SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP* pRamp) override;
	virtual void WINAPI GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp) override;
	virtual HRESULT WINAPI CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI UpdateSurface(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, const POINT* pDestPoint) override;
	virtual HRESULT WINAPI UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture) override;
	virtual HRESULT WINAPI GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface) override;
	virtual HRESULT WINAPI GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface) override;
	virtual HRESULT WINAPI StretchRect(IDirect3DSurface9* pSourceSurface, const RECT* pSourceRect, IDirect3DSurface9* pDestSurface, const RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter) override;
	virtual HRESULT WINAPI ColorFill(IDirect3DSurface9* pSurface, const RECT* pRect, D3DCOLOR color) override;
	virtual HRESULT WINAPI CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle) override;
	virtual HRESULT WINAPI SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget) override;
	virtual HRESULT WINAPI GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget) override;
	virtual HRESULT WINAPI SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil) override;
	virtual HRESULT WINAPI GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface) override;
	virtual HRESULT WINAPI BeginScene(void) override;
	virtual HRESULT WINAPI EndScene(void) override;
	virtual HRESULT WINAPI Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) override;
	virtual HRESULT WINAPI SetTransform(D3DTRANSFORMSTATETYPE State, const D3DMATRIX* pMatrix) override;
	virtual HRESULT WINAPI GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) override;
	virtual HRESULT WINAPI MultiplyTransform(D3DTRANSFORMSTATETYPE, const D3DMATRIX*) override;
	virtual HRESULT WINAPI SetViewport(const D3DVIEWPORT9* pViewport) override;
	virtual HRESULT WINAPI GetViewport(D3DVIEWPORT9* pViewport) override;
	virtual HRESULT WINAPI SetMaterial(const D3DMATERIAL9* pMaterial) override;
	virtual HRESULT WINAPI GetMaterial(D3DMATERIAL9* pMaterial) override;
	virtual HRESULT WINAPI SetLight(DWORD Index, const D3DLIGHT9*) override;
	virtual HRESULT WINAPI GetLight(DWORD Index, D3DLIGHT9*) override;
	virtual HRESULT WINAPI LightEnable(DWORD Index, BOOL Enable) override;
	virtual HRESULT WINAPI GetLightEnable(DWORD Index, BOOL* pEnable) override;
	virtual HRESULT WINAPI SetClipPlane(DWORD Index, const float* pPlane) override;
	virtual HRESULT WINAPI GetClipPlane(DWORD Index, float* pPlane) override;
	virtual HRESULT WINAPI SetRenderState(D3DRENDERSTATETYPE State, DWORD Value) override;
	virtual HRESULT WINAPI GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue) override;
	virtual HRESULT WINAPI CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB) override;
	virtual HRESULT WINAPI BeginStateBlock(void) override;
	virtual HRESULT WINAPI EndStateBlock(IDirect3DStateBlock9** ppSB) override;
	virtual HRESULT WINAPI SetClipStatus(const D3DCLIPSTATUS9* pClipStatus) override;
	virtual HRESULT WINAPI GetClipStatus(D3DCLIPSTATUS9* pClipStatus) override;
	virtual HRESULT WINAPI GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture) override;
	virtual HRESULT WINAPI SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture) override;
	virtual HRESULT WINAPI GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue) override;
	virtual HRESULT WINAPI SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) override;
	virtual HRESULT WINAPI GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue) override;
	virtual HRESULT WINAPI SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value) override;
	virtual HRESULT WINAPI ValidateDevice(DWORD* pNumPasses) override;
	virtual HRESULT WINAPI SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY* pEntries) override;
	virtual HRESULT WINAPI GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries) override;
	virtual HRESULT WINAPI SetCurrentTexturePalette(UINT PaletteNumber) override;
	virtual HRESULT WINAPI GetCurrentTexturePalette(UINT* PaletteNumber) override;
	virtual HRESULT WINAPI SetScissorRect(const RECT* pRect) override;
	virtual HRESULT WINAPI GetScissorRect(RECT* pRect) override;
	virtual HRESULT WINAPI SetSoftwareVertexProcessing(BOOL bSoftware) override;
	virtual BOOL WINAPI GetSoftwareVertexProcessing(void) override;
	virtual HRESULT WINAPI SetNPatchMode(float nSegments) override;
	virtual float WINAPI GetNPatchMode(void) override;
	virtual HRESULT WINAPI DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount) override;
	virtual HRESULT WINAPI DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) override;
	virtual HRESULT WINAPI DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
	virtual HRESULT WINAPI DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride) override;
	virtual HRESULT WINAPI ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags) override;
	virtual HRESULT WINAPI CreateVertexDeclaration(const D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl) override;
	virtual HRESULT WINAPI SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl) override;
	virtual HRESULT WINAPI GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) override;
	virtual HRESULT WINAPI SetFVF(DWORD FVF) override;
	virtual HRESULT WINAPI GetFVF(DWORD* pFVF) override;
	virtual HRESULT WINAPI CreateVertexShader(const DWORD* pFunction, IDirect3DVertexShader9** ppShader) override;
	virtual HRESULT WINAPI SetVertexShader(IDirect3DVertexShader9* pShader) override;
	virtual HRESULT WINAPI GetVertexShader(IDirect3DVertexShader9** ppShader) override;
	virtual HRESULT WINAPI SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) override;
	virtual HRESULT WINAPI GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
	virtual HRESULT WINAPI SetVertexShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) override;
	virtual HRESULT WINAPI GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
	virtual HRESULT WINAPI SetVertexShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) override;
	virtual HRESULT WINAPI GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
	virtual HRESULT WINAPI SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride) override;
	virtual HRESULT WINAPI GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* pOffsetInBytes, UINT* pStride) override;
	virtual HRESULT WINAPI SetStreamSourceFreq(UINT StreamNumber, UINT Setting) override;
	virtual HRESULT WINAPI GetStreamSourceFreq(UINT StreamNumber, UINT* pSetting) override;
	virtual HRESULT WINAPI SetIndices(IDirect3DIndexBuffer9* pIndexData) override;
	virtual HRESULT WINAPI GetIndices(IDirect3DIndexBuffer9** ppIndexData) override;
	virtual HRESULT WINAPI CreatePixelShader(const DWORD* pFunction, IDirect3DPixelShader9** ppShader) override;
	virtual HRESULT WINAPI SetPixelShader(IDirect3DPixelShader9* pShader) override;
	virtual HRESULT WINAPI GetPixelShader(IDirect3DPixelShader9** ppShader) override;
	virtual HRESULT WINAPI SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) override;
	virtual HRESULT WINAPI GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount) override;
	virtual HRESULT WINAPI SetPixelShaderConstantI(UINT StartRegister, const int* pConstantData, UINT Vector4iCount) override;
	virtual HRESULT WINAPI GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount) override;
	virtual HRESULT WINAPI SetPixelShaderConstantB(UINT StartRegister, const BOOL* pConstantData, UINT BoolCount) override;
	virtual HRESULT WINAPI GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount) override;
	virtual HRESULT WINAPI DrawRectPatch(UINT Handle, const float* pNumSegs, const D3DRECTPATCH_INFO* pRectPatchInfo) override;
	virtual HRESULT WINAPI DrawTriPatch(UINT Handle, const float* pNumSegs, const D3DTRIPATCH_INFO* pTriPatchInfo) override;
	virtual HRESULT WINAPI DeletePatch(UINT Handle) override;
	virtual HRESULT WINAPI CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery) override;

public:
	hook_Direct3DDevice9(IDirect3DDevice9* direct3DDevice9)
		: m_direct3DDevice9(direct3DDevice9)
	{
	}

private:
	~hook_Direct3DDevice9()
	{
		m_direct3DDevice9->Release();
	}

	LONG m_refCount = 1;
	IDirect3DDevice9* m_direct3DDevice9;
};