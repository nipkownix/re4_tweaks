#include <iostream>
#include "dllmain.h"
#include "Patches.h"
#include <d3d9.h>
#include <shellapi.h>
#include "Settings.h"
#include "imgui\imgui.h"
#include "imgui\imgui_impl_win32.h"
#include "imgui\imgui_impl_dx9.h"

LAApatch laa;

bool LAApatch::GameIsLargeAddressAware()
{
	static PBYTE module_base = reinterpret_cast<PBYTE>(GetModuleHandle(nullptr));

	PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);
	PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + dos_header->e_lfanew);

	return (nt_headers->FileHeader.Characteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) == IMAGE_FILE_LARGE_ADDRESS_AWARE;
}

//https://programmerall.com/article/8899839695/
uint32_t calc_checksum(uint32_t checksum, void* data, int length) {
	if (length && data != nullptr) {
		uint32_t sum = 0;
		do {
			sum = *(uint16_t*)data + checksum;
			checksum = (uint16_t)sum + (sum >> 16);
			data = (char*)data + 2;
		} while (--length);
	}

	return checksum + (checksum >> 16);
}

LAADialogState LAA_State = LAADialogState::NotShowing;
int LAA_ErrorNum = 0;
void LAApatch::LAARender()
{
	ImGuiIO& io = ImGui::GetIO();

	if (GameIsLargeAddressAware())
	{
		// Exit out in case we needlessly ended up here somehow
		LAA_State = LAADialogState::NotShowing;
		return;
	}

	if (LAA_State == LAADialogState::Showing)
	{
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(400, 175), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(400, 175), ImVec2(400, 175));
		ImGui::Begin("4GB / Large Address Aware patch missing!");
		ImGui::TextWrapped("Your game executable is missing the 4GB/LAA patch, this will likely cause issues with mods that require increased memory.\n\nDo you want re4_tweaks to patch the game EXE for you? (requires relaunch!)");

		ImGui::Spacing();
		ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 225, ImGui::GetWindowHeight() - 45));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.51f, 0.00f, 0.14f, 1.00f));

		if (ImGui::Button("Yes", ImVec2(104, 35)))
		{
			char module_path_array[4096];
			GetModuleFileNameA(GetModuleHandle(nullptr), module_path_array, 4096);

			std::string module_path = module_path_array;
			std::string module_path_new = module_path + ".new";
			std::string module_path_bak = module_path + ".bak";

			if (GetFileAttributesA(module_path_new.c_str()) != 0xFFFFFFFF)
				DeleteFileA(module_path_new.c_str());

			if (GetFileAttributesA(module_path_bak.c_str()) != 0xFFFFFFFF)
				DeleteFileA(module_path_bak.c_str());

			BOOL result_CopyFileA = CopyFileA(module_path.c_str(), module_path_new.c_str(), false);

			FILE* file;
			LAA_ErrorNum = fopen_s(&file, module_path_new.c_str(), "rb+");
			if (LAA_ErrorNum == 0)
			{
				fseek(file, 0, SEEK_END);
				std::vector<uint8_t> exe_data(ftell(file));
				fseek(file, 0, SEEK_SET);

				if (fread(exe_data.data(), 1, exe_data.size(), file) != exe_data.size())
				{
					fclose(file);
					LAA_ErrorNum = 1;
				}
				else
				{
					PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(exe_data.data());
					PIMAGE_NT_HEADERS nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS>(exe_data.data() + dos_header->e_lfanew);

					// Set LAA flag in PE headers
					nt_headers->FileHeader.Characteristics |= IMAGE_FILE_LARGE_ADDRESS_AWARE;

					// Fix up PE checksum - required for some locales to work properly?
					uint32_t header_size = (uintptr_t)nt_headers - (uintptr_t)exe_data.data() +
						((uintptr_t)&nt_headers->OptionalHeader.CheckSum - (uintptr_t)nt_headers);

					// Skip over CheckSum field
					uint32_t remain_size = (exe_data.size() - header_size - 4) / sizeof(uint16_t);
					void* remain = &nt_headers->OptionalHeader.Subsystem;

					uint32_t header_checksum = calc_checksum(0, exe_data.data(), header_size / sizeof(uint16_t));
					uint32_t file_checksum = calc_checksum(header_checksum, remain, remain_size);
					if (exe_data.size() & 1)
						file_checksum += *((char*)exe_data.data() + exe_data.size() - 1);

					nt_headers->OptionalHeader.CheckSum = file_checksum + exe_data.size();

					fseek(file, dos_header->e_lfanew, SEEK_SET);
					auto wrote = fwrite(nt_headers, sizeof(IMAGE_NT_HEADERS), 1, file);
					fclose(file);

					if (wrote != 1)
					{
						LAA_ErrorNum = 2;
					}
					else
					{
						BOOL result_moveFile1 = MoveFileExA(module_path.c_str(), module_path_bak.c_str(), MOVEFILE_REPLACE_EXISTING);
						BOOL result_moveFile2 = MoveFileA(module_path_new.c_str(), module_path.c_str());
						if (!result_moveFile1)
							LAA_ErrorNum = 3;
						else if (!result_moveFile2)
							LAA_ErrorNum = 4;

						if (result_moveFile1 && !result_moveFile2)
						{
							// Users original EXE was moved, but we couldn't move replacement for it for some reason
							// Try restoring the users original EXE so they aren't left with a broken install...
							MoveFileA(module_path_bak.c_str(), module_path.c_str());
						}
					}
				}
			}
			LAA_State = LAADialogState::Finished;
		}

		ImGui::SameLine();

		if (ImGui::Button("No", ImVec2(104, 35)))
		{
			LAA_State = LAADialogState::NotShowing;
		}

		ImGui::PopStyleColor();

		ImGui::End();
	}
	else if (LAA_State == LAADialogState::Finished)
	{
		// Prompted the user & finished performing patches, report the results back to them
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(400, 175), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(400, 175), ImVec2(400, 175));
		if (LAA_ErrorNum == 0)
		{
			ImGui::Begin("Game 4GB patched successfully!");
			ImGui::TextWrapped("re4_tweaks has successfully patched your game EXE (a backup has also been made)\n\nPress OK to relaunch the game for the patch to take effect!");
		}
		else
		{
			ImGui::Begin("Game 4GB patch failed...");
			ImGui::TextWrapped("re4_tweaks failed to patch the game EXE (error %d)\n\nYou can manually patch it yourself by using the \"NTCore 4GB Patch\" tool - an internet search should help find it!", LAA_ErrorNum);
		}

		ImGui::Dummy(ImVec2(0.0f, 15.0f));

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 113);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.51f, 0.00f, 0.14f, 1.00f));

		if (ImGui::Button("OK", ImVec2(104, 35)))
		{
			LAA_State = LAADialogState::NotShowing;

			// Relaunch the game
			std::string bio4path = rootPath + "bio4.exe";
			if ((int)ShellExecuteA(nullptr, "open", bio4path.c_str(), nullptr, nullptr, SW_SHOWDEFAULT) > 32)
			{
				exit(0);
				return;
			}
		}

		ImGui::PopStyleColor();

		ImGui::End();
	}
}