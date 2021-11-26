#include <iostream>
#include "..\includes\stdafx.h"
#include <d3d9.h>
#include "WndProcHook.h"
#include "dllmain.h"
#include "..\external\imgui\imgui.h"
#include "..\external\imgui\imgui_impl_win32.h"
#include "..\external\imgui\imgui_impl_dx9.h"

#define VERBOSE

uintptr_t ptrInputProcess;

// Watch for input to be used in the cfgMenu
void ImGuiInputThread()
{
	while (true)
	{
		if (bCfgMenuOpen)
		{
			ImGuiIO& io = ImGui::GetIO();
			POINT mPos;
			GetCursorPos(&mPos);
			ScreenToClient(hWindow, &mPos);
			io.MousePos.x = mPos.x;
			io.MousePos.y = mPos.y;
			io.MouseDown[0] = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
			io.MouseDown[1] = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
			io.MouseDown[2] = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
		}
		Sleep(1);
	}
}

// Add our new code right before the game calls EndScene
HRESULT APIENTRY EndScene_hook(LPDIRECT3DDEVICE9 pDevice)
{
	static bool didInit = false;

	if (!didInit)
	{
		didInit = true;

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ImGuiInputThread), nullptr, 0, nullptr);

		ImGui_ImplWin32_Init(hWindow);
		ImGui_ImplDX9_Init(pDevice);
	}

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Calls the actual menu function
	if (bCfgMenuOpen)
	{
		ImGui::GetIO().MouseDrawCursor = true;
		ImGui::ShowDemoWindow();
	}
	else
		ImGui::GetIO().MouseDrawCursor = false;

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	return pDevice->EndScene();
}


void Init_cfgMenu()
{
	// EndScene hook
	auto pattern = hook::pattern("8b 08 8b 91 ? ? ? ? 50 ff d2 c6 05 ? ? ? ? ? a1");
	struct EndScene_HookStruct
	{
		void operator()(injector::reg_pack& regs)
		{
			auto pDevice = (LPDIRECT3DDEVICE9)regs.eax;

			EndScene_hook(pDevice);
		}
	}; injector::MakeInline<EndScene_HookStruct>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(11));

	// Input process hook
	pattern = hook::pattern("e8 ? ? ? ? e8 ? ? ? ? 68 ? ? ? ? 6a ? e8 ? ? ? ? 83 c4 ? e8");
	ptrInputProcess = injector::GetBranchDestination(pattern.get_first(0)).as_int();
	struct InputProcess
	{
		void operator()(injector::reg_pack& regs)
		{
			if (!bCfgMenuOpen)
				_asm {call ptrInputProcess}
			else
				_asm {ret}
		}
	}; injector::MakeInline<InputProcess>(pattern.count(1).get(0).get<uint32_t>(0), pattern.count(1).get(0).get<uint32_t>(5));
}