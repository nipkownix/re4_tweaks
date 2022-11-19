#include "dllmain.h"
#include "D3D9Hook.h"
#include <iostream>
#include <imgui_impl_dx9.h>
#include "Patches.h"
#include "Settings.h"
#include "../dxvk/src/d3d9/d3d9_main.h"
#include "../dxvk/src/config.h"

static IDirect3D9* (WINAPI* orgDirect3DCreate9)(UINT SDKVersion);
static IDirect3D9* WINAPI hook_Direct3DCreate9(UINT SDKVersion)
{
	spd::log()->info("{} -> Creating IDirect3D9 object...", __FUNCTION__);

	if (re4t::dxvk::cfg->bUseVulkanRenderer)
	{
		spd::log()->info("{} -> UseVulkanRenderer is enabled, using DX9 -> VK translation layer...", __FUNCTION__);

		IDirect3D9Ex* pDirect3D = nullptr;
		dxvk::CreateD3D9(false, &pDirect3D);

		return new hook_Direct3D9(pDirect3D);
	}
	else
	{
		IDirect3D9* d3dInterface = orgDirect3DCreate9(SDKVersion);
		return new hook_Direct3D9(d3dInterface);
	}
}

void re4t::init::D3D9Hook()
{
	auto pattern = hook::pattern("E8 ? ? ? ? A3 ? ? ? ? 85 C0 75 17");

	ReadCall(pattern.count(1).get(0).get<uint32_t>(0), orgDirect3DCreate9);
	InjectHook(pattern.count(1).get(0).get<uint32_t>(0), hook_Direct3DCreate9);
}

// ========= hook_Direct3D9 =========
HRESULT hook_Direct3D9::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr) return E_POINTER;

	if (riid == __uuidof(IUnknown) ||
		riid == __uuidof(IDirect3D9))
	{
		*ppvObj = static_cast<IDirect3D9*>(this);
		AddRef();
		return S_OK;
	}

	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

ULONG hook_Direct3D9::AddRef(void)
{
	return _InterlockedIncrement(&m_refCount);
}

ULONG hook_Direct3D9::Release(void)
{
	const LONG ref = _InterlockedDecrement(&m_refCount);
	if (ref == 0)
	{
		delete this;
	}
	return ref;
}

HRESULT hook_Direct3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
	return m_direct3D9->RegisterSoftwareDevice(pInitializeFunction);
}

UINT hook_Direct3D9::GetAdapterCount(void)
{
	return m_direct3D9->GetAdapterCount();
}

HRESULT hook_Direct3D9::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	return m_direct3D9->GetAdapterIdentifier(Adapter, Flags, pIdentifier);
}

UINT hook_Direct3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
	return m_direct3D9->GetAdapterModeCount(Adapter, Format);
}

HRESULT hook_Direct3D9::EnumAdapterModes(UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode)
{
	return m_direct3D9->EnumAdapterModes(Adapter, Format, Mode, pMode);
}

HRESULT hook_Direct3D9::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
	return m_direct3D9->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT hook_Direct3D9::CheckDeviceType(UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	return m_direct3D9->CheckDeviceType(Adapter, DevType, AdapterFormat, BackBufferFormat, bWindowed);
}

HRESULT hook_Direct3D9::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
	return m_direct3D9->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT hook_Direct3D9::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels)
{
	return m_direct3D9->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels);
}

HRESULT hook_Direct3D9::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	return m_direct3D9->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT hook_Direct3D9::CheckDeviceFormatConversion(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat)
{
	return m_direct3D9->CheckDeviceFormatConversion(Adapter, DeviceType, SourceFormat, TargetFormat);
}

HRESULT hook_Direct3D9::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps)
{
	return m_direct3D9->GetDeviceCaps(Adapter, DeviceType, pCaps);
}

HMONITOR hook_Direct3D9::GetAdapterMonitor(UINT Adapter)
{
	return m_direct3D9->GetAdapterMonitor(Adapter);
}

HRESULT hook_Direct3D9::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
	// Force v-sync off
	if (re4t::cfg->bDisableVsync)
		pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	IDirect3DDevice9* device = nullptr;

	HRESULT result = m_direct3D9->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &device);
	if (FAILED(result))
	{
		*ppReturnedDeviceInterface = nullptr;
		return result;
	}

	*ppReturnedDeviceInterface = new hook_Direct3DDevice9(device);
	return result;
}

// ========= hook_Direct3DDevice9 =========

HRESULT hook_Direct3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr) return E_POINTER;

	if (riid == __uuidof(IUnknown) ||
		riid == __uuidof(IDirect3DDevice9))
	{
		*ppvObj = static_cast<IDirect3DDevice9*>(this);
		AddRef();
		return S_OK;
	}

	*ppvObj = nullptr;
	return E_NOINTERFACE;
}

ULONG hook_Direct3DDevice9::AddRef(void)
{
	return _InterlockedIncrement(&m_refCount);
}

ULONG hook_Direct3DDevice9::Release(void)
{
	const LONG ref = _InterlockedDecrement(&m_refCount);
	if (ref == 0)
	{
		delete this;
	}
	return ref;
}

HRESULT hook_Direct3DDevice9::TestCooperativeLevel(void)
{
	return m_direct3DDevice9->TestCooperativeLevel();
}

UINT hook_Direct3DDevice9::GetAvailableTextureMem(void)
{
	return m_direct3DDevice9->GetAvailableTextureMem();
}

HRESULT hook_Direct3DDevice9::EvictManagedResources(void)
{
	return m_direct3DDevice9->EvictManagedResources();;
}

HRESULT hook_Direct3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
	return m_direct3DDevice9->GetDirect3D(ppD3D9);
}

HRESULT hook_Direct3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
	return m_direct3DDevice9->GetDeviceCaps(pCaps);
}

HRESULT hook_Direct3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	return m_direct3DDevice9->GetDisplayMode(iSwapChain, pMode);
}

HRESULT hook_Direct3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
	return m_direct3DDevice9->GetCreationParameters(pParameters);
}

HRESULT hook_Direct3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	return m_direct3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

void hook_Direct3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	m_direct3DDevice9->SetCursorPosition(X, Y, Flags);
}

BOOL hook_Direct3DDevice9::ShowCursor(BOOL bShow)
{
	return m_direct3DDevice9->ShowCursor(bShow);
}

HRESULT hook_Direct3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	return m_direct3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain);
}

HRESULT hook_Direct3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	return m_direct3DDevice9->GetSwapChain(iSwapChain, pSwapChain);
}

UINT hook_Direct3DDevice9::GetNumberOfSwapChains(void)
{
	return m_direct3DDevice9->GetNumberOfSwapChains();
}

HRESULT hook_Direct3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	// Force v-sync off
	if (re4t::cfg->bDisableVsync)
		pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	ImGui_ImplDX9_InvalidateDeviceObjects(); // Reset ImGui objects to prevent freezing

	HRESULT hr = m_direct3DDevice9->Reset(pPresentationParameters);

	ImGui_ImplDX9_CreateDeviceObjects();

	return hr;
}

HRESULT hook_Direct3DDevice9::Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	static bool restorePickupTransparency = false;
	HRESULT res = m_direct3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	if (res == D3DERR_DEVICELOST)
	{
		if (re4t::cfg->bRestorePickupTransparency)
		{
			// Avoid alt-tab crash when on item-pickup screen with bRestorePickupTransparency active
			re4t::cfg->bRestorePickupTransparency = false;
			restorePickupTransparency = true;
		}
	}
	else if (res == D3D_OK)
	{
		// Restore bRestorePickupTransparency if needed
		if (restorePickupTransparency)
		{
			re4t::cfg->bRestorePickupTransparency = true;
			restorePickupTransparency = false;
		}
	}
	return res;
}

HRESULT hook_Direct3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	return m_direct3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);
}

HRESULT hook_Direct3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	return m_direct3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus);
}

HRESULT hook_Direct3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	return m_direct3DDevice9->SetDialogBoxMode(bEnableDialogs);
}

void hook_Direct3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, const D3DGAMMARAMP* pRamp)
{
	m_direct3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp);
}

void hook_Direct3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	m_direct3DDevice9->GetGammaRamp(iSwapChain, pRamp);
}

HRESULT hook_Direct3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::BeginStateBlock()
{
	return m_direct3DDevice9->BeginStateBlock();
}

HRESULT hook_Direct3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	return m_direct3DDevice9->CreateStateBlock(Type, ppSB);
}

HRESULT hook_Direct3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	return m_direct3DDevice9->EndStateBlock(ppSB);
}

HRESULT hook_Direct3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	return m_direct3DDevice9->GetClipStatus(pClipStatus);
}

HRESULT hook_Direct3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	return m_direct3DDevice9->GetRenderState(State, pValue);
}

HRESULT hook_Direct3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	return m_direct3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget);
}

HRESULT hook_Direct3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	return m_direct3DDevice9->GetTransform(State, pMatrix);
}

HRESULT hook_Direct3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	return m_direct3DDevice9->SetClipStatus(pClipStatus);
}

HRESULT hook_Direct3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	return m_direct3DDevice9->SetRenderState(State, Value);;
}

HRESULT hook_Direct3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	return m_direct3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

HRESULT hook_Direct3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	return m_direct3DDevice9->SetTransform(State, pMatrix);
}

HRESULT hook_Direct3DDevice9::DeletePatch(UINT Handle)
{
	return m_direct3DDevice9->DeletePatch(Handle);
}

HRESULT hook_Direct3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	return m_direct3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo);
}

HRESULT hook_Direct3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	return m_direct3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo);
}

HRESULT hook_Direct3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return m_direct3DDevice9->GetIndices(ppIndexData);
}

HRESULT hook_Direct3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return m_direct3DDevice9->SetIndices(pIndexData);
}

HRESULT hook_Direct3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	return m_direct3DDevice9->GetLight(Index, pLight);
}

HRESULT hook_Direct3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	return m_direct3DDevice9->GetLightEnable(Index, pEnable);
}

HRESULT hook_Direct3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
	return m_direct3DDevice9->GetMaterial(pMaterial);
}

HRESULT hook_Direct3DDevice9::LightEnable(DWORD LightIndex, BOOL bEnable)
{
	return m_direct3DDevice9->LightEnable(LightIndex, bEnable);
}

HRESULT hook_Direct3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{

	return m_direct3DDevice9->SetLight(Index, pLight);
}

HRESULT hook_Direct3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	return m_direct3DDevice9->SetMaterial(pMaterial);
}

HRESULT hook_Direct3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	return m_direct3DDevice9->MultiplyTransform(State, pMatrix);
}

HRESULT hook_Direct3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	return m_direct3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags);
}

HRESULT hook_Direct3DDevice9::GetCurrentTexturePalette(UINT* pPaletteNumber)
{
	return m_direct3DDevice9->GetCurrentTexturePalette(pPaletteNumber);
}

HRESULT hook_Direct3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	return m_direct3DDevice9->GetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT hook_Direct3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return m_direct3DDevice9->SetCurrentTexturePalette(PaletteNumber);
}

HRESULT hook_Direct3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	return m_direct3DDevice9->SetPaletteEntries(PaletteNumber, pEntries);
}

HRESULT hook_Direct3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	return m_direct3DDevice9->CreatePixelShader(pFunction, ppShader);
}

HRESULT hook_Direct3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return m_direct3DDevice9->GetPixelShader(ppShader);
}

HRESULT hook_Direct3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return m_direct3DDevice9->SetPixelShader(pShader);
}

HRESULT hook_Direct3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	return m_direct3DDevice9->DrawIndexedPrimitive(Type, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT hook_Direct3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return m_direct3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT hook_Direct3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	return m_direct3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT hook_Direct3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	return m_direct3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride);
}

HRESULT hook_Direct3DDevice9::BeginScene()
{
	return m_direct3DDevice9->BeginScene();
}

HRESULT hook_Direct3DDevice9::EndScene()
{
	// Used to render our ImGui interface
	esHook.EndScene_hook(m_direct3DDevice9);

	return m_direct3DDevice9->EndScene();
}

HRESULT hook_Direct3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	return m_direct3DDevice9->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride);
}

HRESULT hook_Direct3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	return m_direct3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

HRESULT hook_Direct3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	return m_direct3DDevice9->GetDepthStencilSurface(ppZStencilSurface);
}

HRESULT hook_Direct3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	return m_direct3DDevice9->GetTexture(Stage, ppTexture);
}

HRESULT hook_Direct3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	return m_direct3DDevice9->GetTextureStageState(Stage, Type, pValue);
}

HRESULT hook_Direct3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	return m_direct3DDevice9->SetTexture(Stage, pTexture);
}

HRESULT hook_Direct3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	return m_direct3DDevice9->SetTextureStageState(Stage, Type, Value);
}

HRESULT hook_Direct3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	return m_direct3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture);
}

HRESULT hook_Direct3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
	return m_direct3DDevice9->ValidateDevice(pNumPasses);
}

HRESULT hook_Direct3DDevice9::GetClipPlane(DWORD Index, float* pPlane)
{
	return m_direct3DDevice9->GetClipPlane(Index, pPlane);
}

HRESULT hook_Direct3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	return m_direct3DDevice9->SetClipPlane(Index, pPlane);
}

HRESULT hook_Direct3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	return m_direct3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT hook_Direct3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
	return m_direct3DDevice9->GetViewport(pViewport);
}

HRESULT hook_Direct3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	return m_direct3DDevice9->SetViewport(pViewport);
}

HRESULT hook_Direct3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	return m_direct3DDevice9->CreateVertexShader(pFunction, ppShader);
}

HRESULT hook_Direct3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return m_direct3DDevice9->GetVertexShader(ppShader);
}

HRESULT hook_Direct3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return m_direct3DDevice9->SetVertexShader(pShader);
}

HRESULT hook_Direct3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	return m_direct3DDevice9->CreateQuery(Type, ppQuery);
}

HRESULT hook_Direct3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return m_direct3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT hook_Direct3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return m_direct3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT hook_Direct3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT hook_Direct3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT hook_Direct3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT hook_Direct3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT hook_Direct3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	return m_direct3DDevice9->SetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT hook_Direct3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
{
	return m_direct3DDevice9->GetStreamSourceFreq(StreamNumber, Divider);
}

HRESULT hook_Direct3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	return m_direct3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT hook_Direct3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	return m_direct3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount);
}

HRESULT hook_Direct3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT hook_Direct3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	return m_direct3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

HRESULT hook_Direct3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT hook_Direct3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	return m_direct3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount);
}

HRESULT hook_Direct3DDevice9::SetFVF(DWORD FVF)
{
	return m_direct3DDevice9->SetFVF(FVF);
}

HRESULT hook_Direct3DDevice9::GetFVF(DWORD* pFVF)
{
	return m_direct3DDevice9->GetFVF(pFVF);
}

HRESULT hook_Direct3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	return m_direct3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl);
}

HRESULT hook_Direct3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return m_direct3DDevice9->SetVertexDeclaration(pDecl);
}

HRESULT hook_Direct3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return m_direct3DDevice9->GetVertexDeclaration(ppDecl);
}

HRESULT hook_Direct3DDevice9::SetNPatchMode(float nSegments)
{
	return m_direct3DDevice9->SetNPatchMode(nSegments);
}

float hook_Direct3DDevice9::GetNPatchMode(THIS)
{
	return m_direct3DDevice9->GetNPatchMode();
}

int hook_Direct3DDevice9::GetSoftwareVertexProcessing(THIS)
{
	return m_direct3DDevice9->GetSoftwareVertexProcessing();
}

HRESULT hook_Direct3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return m_direct3DDevice9->SetSoftwareVertexProcessing(bSoftware);
}

HRESULT hook_Direct3DDevice9::SetScissorRect(CONST RECT* pRect)
{
	return m_direct3DDevice9->SetScissorRect(pRect);
}

HRESULT hook_Direct3DDevice9::GetScissorRect(RECT* pRect)
{
	return m_direct3DDevice9->GetScissorRect(pRect);
}

HRESULT hook_Direct3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	return m_direct3DDevice9->GetSamplerState(Sampler, Type, pValue);
}

HRESULT hook_Direct3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	return m_direct3DDevice9->SetSamplerState(Sampler, Type, Value);
}

HRESULT hook_Direct3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	return m_direct3DDevice9->SetDepthStencilSurface(pNewZStencil);
}

HRESULT hook_Direct3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	return m_direct3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);
}

HRESULT hook_Direct3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	return m_direct3DDevice9->ColorFill(pSurface, pRect, color);
}

HRESULT hook_Direct3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	return m_direct3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

HRESULT hook_Direct3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	return m_direct3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface);
}

HRESULT hook_Direct3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	return m_direct3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface);
}

HRESULT hook_Direct3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	return m_direct3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}