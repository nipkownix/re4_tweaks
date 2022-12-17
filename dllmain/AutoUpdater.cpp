#include "dllmain.h"
#include "Patches.h"
#include "Settings.h"
#include <shellapi.h>
#include <atlbase.h>
#include <nlohmann/json.hpp>
#include "AutoUpdater.h"
#include "resource.h"
#include <miniz/miniz.h>
#include "UI_Utility.h"
#pragma comment(lib, "urlmon.lib")
using json = nlohmann::json;

AutoUpdate updt;

float download_progress = 0.0f;

std::string fail_msg;

void updateCheck()
{
	// Delete old dll file if it is present
	if (std::filesystem::remove(rootPath + L"dinput8.dll.deleteonnextlaunch"))
	{
		spd::log()->info("{} -> Old .dll found and deleted", __FUNCTION__);
	}

	if (re4t::cfg->bNeverCheckForUpdates)
	{
		updt.UpdateStatus = UpdateStatus::Finished;
		return;
	}

	// Check for update using GH API
	CComPtr<IStream> pStream;
	HRESULT hr = URLOpenBlockingStreamW(nullptr, L"https://api.github.com/repos/nipkownix/re4_tweaks/releases?per_page=1", &pStream, 0, nullptr);

	if (FAILED(hr))
	{
		spd::log()->info("{} -> Failed to connect to GitHub!", __FUNCTION__);

		#ifdef VERBOSE
		con.AddLogChar("AutoUpdate: Failed to connect to GitHub!");
		#endif 

		updt.UpdateStatus = UpdateStatus::Finished;

		return;
	}

	// Download json from API
	char buffer[4096];
	std::string json_str;
	json json_obj;

	do
	{
		DWORD bytesRead = 0;
		hr = pStream->Read(buffer, sizeof(buffer), &bytesRead);
		buffer[bytesRead] = 0;

		if (bytesRead > 0)
		{
			json_str.append(buffer, bytesRead);
		}
	} while (SUCCEEDED(hr) && hr != S_FALSE);

	if (FAILED(hr))
	{
		spd::log()->info("{} -> Failed to get json from GitHub!", __FUNCTION__);

		#ifdef VERBOSE
		con.AddLogChar("AutoUpdate: Failed to download json from GitHub!");
		#endif 

		updt.UpdateStatus = UpdateStatus::Finished;

		return;
	}

	json_obj = json::parse(json_str);

	if (json_obj.empty())
	{
		spd::log()->info("{} -> Invalid json!", __FUNCTION__);

		#ifdef VERBOSE
		con.AddLogChar("AutoUpdate: Invalid json!");
		#endif 

		updt.UpdateStatus = UpdateStatus::Finished;

		return;
	}

	// Get info from json
	if (json_obj.is_structured() && json_obj.is_array())
	{
		for (auto& json : json_obj)
		{
			if (json.is_structured() && json.is_object())
			{
				json.at("tag_name").get_to(updt.version);
				json.at("assets").at(0).at("browser_download_url").get_to(updt.url);
				json.at("body").get_to(updt.description);
			}
		}
	}

	if (APP_VERSION < updt.version)
	{
		spd::log()->info("{} -> New version found: {}", __FUNCTION__, updt.version);
		updt.UpdateStatus = UpdateStatus::Available;
	}
	else 
		updt.UpdateStatus = UpdateStatus::Finished;

	return;
}

class CCallback : public IBindStatusCallback
{
public:
	CCallback() {}
	~CCallback() {}
	STDMETHOD(OnStartBinding)(DWORD dwReserved, IBinding __RPC_FAR* pib) { return E_NOTIMPL; }
	STDMETHOD(GetPriority)(LONG __RPC_FAR* pnPriority) { return E_NOTIMPL; }
	STDMETHOD(OnLowResource)(DWORD reserved) { return E_NOTIMPL; }
	STDMETHOD(OnProgress)(ULONG ulProgress, ULONG ulProgressMax, ULONG ulStatusCode, LPCWSTR wszStatusText)
	{
		if (!(ulProgressMax | ulProgress))
			return E_NOTIMPL;

		download_progress = (float)((double)ulProgress / (double)ulProgressMax * 100.0);

		return S_OK;
	}
	STDMETHOD(OnStopBinding)(HRESULT hresult, LPCWSTR szError) { return E_NOTIMPL; }
	STDMETHOD(GetBindInfo)(DWORD __RPC_FAR* grfBINDF, BINDINFO __RPC_FAR* pbindinfo) { return E_NOTIMPL; }
	STDMETHOD(OnDataAvailable)(DWORD grfBSCF, DWORD dwSize, FORMATETC __RPC_FAR* pformatetc, STGMEDIUM __RPC_FAR* pstgmed) { return E_NOTIMPL; }
	STDMETHOD(OnObjectAvailable)(REFIID riid, IUnknown __RPC_FAR* punk) { return E_NOTIMPL; }
	STDMETHOD_(ULONG, AddRef)() { return 0; }
	STDMETHOD_(ULONG, Release)() { return 0; }
	STDMETHOD(QueryInterface)(REFIID riid, void __RPC_FAR* __RPC_FAR* ppvObject) { return E_NOTIMPL; }
};

CCallback callback;


void updateDownloadApply()
{
	mz_zip_archive zip;
	ZeroMemory(&zip, sizeof(zip));

	std::wstring zip_path = rootPath + L"re4_tweaks\\tmp_updt\\updt.zip";

	// Clear tmp dir
	std::filesystem::remove_all(rootPath + L"re4_tweaks\\tmp_updt");

	// Create directory if it doesn't exist
	std::filesystem::create_directories(rootPath + L"re4_tweaks\\tmp_updt");

	// Download update
	HRESULT result = URLDownloadToFileW(NULL, StrToWstr(updt.url).c_str(), zip_path.c_str(), 0, &callback);

	if (!SUCCEEDED(result)) {
		spd::log()->info("{} -> Update download failed!", __FUNCTION__);

		#ifdef VERBOSE
		con.AddLogChar("AutoUpdate: Update download failed!");
		#endif 

		fail_msg = "Error when downloading new files.\n\nPlease try to manually update on:\nhttps://github.com/nipkownix/re4_tweaks";
		updt.UpdateStatus = UpdateStatus::Failed;
	}
	else
	{
		if (!std::filesystem::exists(zip_path))
		{
			spd::log()->info("{} -> Zip file missing!", __FUNCTION__);

			#ifdef VERBOSE
			con.AddLogChar("AutoUpdate: zip file missing!");
			#endif 

			fail_msg = "Updated files are missing.\n\nPlease try to manually update on:\nhttps://github.com/nipkownix/re4_tweaks";
			updt.UpdateStatus = UpdateStatus::Failed;
		}
		else
		{
			mz_zip_reader_init_file(&zip, WstrToStr(zip_path).c_str(), 0);

			std::wstring target = rootPath + L"re4_tweaks\\tmp_updt\\extracted\\";

			mz_uint numfiles = mz_zip_reader_get_num_files(&zip);
			bool successful = false;

			// Extract all files
			for (mz_uint i = 0; i < numfiles; i++)
			{
				mz_zip_archive_file_stat zstat;
				mz_zip_reader_file_stat(&zip, i, &zstat);

				auto file_path = std::filesystem::path(target + StrToWstr(zstat.m_filename));

				// Create directory if it doesn't exist
				std::filesystem::create_directories(file_path.parent_path());

				// Extract
				FILE* target_file = NULL;
				_wfopen_s(&target_file, file_path.wstring().c_str(), L"wb");
				if (target_file)
				{
					mz_zip_reader_extract_to_cfile(&zip, i, target_file, 0);
					fclose(target_file);
				}
			}

			// Rename the dll to something that will be deleted on the next launch
			std::wstring module_path = rootPath + L"dinput8.dll";
			std::wstring module_path_del = rootPath + L"dinput8.dll.deleteonnextlaunch";

			BOOL result_moveFile = MoveFileExW(module_path.c_str(), module_path_del.c_str(), MOVEFILE_REPLACE_EXISTING);

			// Apply the updated files
			if (result_moveFile)
			{
				const auto copyOptions = std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive;

				try
				{
					std::filesystem::copy(target, rootPath, copyOptions);

					// Save current settings on new .ini files
					re4t::cfg->WriteSettings(false);

					// Done
					updt.UpdateStatus = UpdateStatus::Success;
				}
				catch (...)
				{
					spd::log()->info("{} -> Failed to copy new files!", __FUNCTION__);

					#ifdef VERBOSE
					con.AddLogChar("AutoUpdate: Failed to copy new files!");
					#endif 

					fail_msg = "Error when copying updated files to the game's directory.\n\nPlease try to manually update on:\nhttps://github.com/nipkownix/re4_tweaks";
					updt.UpdateStatus = UpdateStatus::Failed;

					// Try to restore the dll
					MoveFileExW(module_path_del.c_str(), module_path.c_str(), MOVEFILE_REPLACE_EXISTING);
				}
			}
			else
			{
				// Couldn't rename the dll for some reason?
				fail_msg = "A problem occurred when trying to move the updated files.\n\nPlease try to manually update on:\nhttps://github.com/nipkownix/re4_tweaks";
				updt.UpdateStatus = UpdateStatus::Failed;
			}

			mz_zip_reader_end(&zip);
		}
	}

	// Clear tmp dir
	std::filesystem::remove_all(rootPath + L"re4_tweaks\\tmp_updt");
}

void AutoUpdate::RenderUI()
{
	ImGuiIO& io = ImGui::GetIO();

	// Min/Max window sizes
	const float max_x = 415.0f * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale;
	const float max_y = 360.0f * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale;

	if (updt.UpdateStatus == UpdateStatus::Available)
	{
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(max_x, max_y), ImGuiCond_Appearing);
		ImGui::SetNextWindowSizeConstraints(ImVec2(max_x, max_y), ImVec2(max_x, max_y));

		if (ImGui::Begin("re4_tweaks Update Available", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::Text("An update for re4_tweaks is available.");

			ImGui::Bullet(); ImGui::Text("Version available: %s", updt.version.c_str());
			ImGui::Bullet(); ImGui::Text("Version installed: %s", std::string(APP_VERSION).c_str());

			ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale));

			ImGui::Text("Changelog:");

			ImGui::Separator();
			ImGui::BeginChild("chnglog", ImVec2(0, 130 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale));
			ImGui::TextWrapped(updt.description.c_str());
			ImGui::EndChild();
			ImGui::Separator();
			
			ImGui::Dummy(ImVec2(10, 10 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale));

			ImGui::Text("Do you want to update now?");
			ImGui::Spacing();

			ImVec2 btn_size = ImVec2(104 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale, 35 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale);

			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - btn_size.y - 10);

			if (ImGui::Button("Yes", btn_size))
			{
				CreateThreadAutoClose(0, 0, (LPTHREAD_START_ROUTINE)&updateDownloadApply, NULL, 0, NULL);
				updt.UpdateStatus = UpdateStatus::InProgress;
			}

			ImGui::SameLine();

			if (ImGui::Button("No", btn_size))
			{
				updt.UpdateStatus = UpdateStatus::Finished;
			}

			ImGui::SameLine();

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - btn_size.x);

			if (ImGui::Button("Never ask again", btn_size))
			{
				re4t::cfg->bNeverCheckForUpdates = true;
				re4t::cfg->WriteSettings(false);
				updt.UpdateStatus = UpdateStatus::Finished;
			}

			ImGui::End();
		}
	}

	if (updt.UpdateStatus == UpdateStatus::InProgress)
	{
		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("Downloading update...", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar))
		{
			const ImU32 col = ImColor(255, 25, 40, 255);
			const ImU32 bg = ImColor(143, 143, 143, 255);

			ImVec2 bar_size = ImVec2(400 * re4t::cfg->fdbg2, 25 * re4t::cfg->fdbg2);
			
			ImGui_BufferingBar("##buffer_bar", (float)(download_progress / 100.0f), bar_size, bg, col);

			ImGui::End();
		}
	}
	
	if (updt.UpdateStatus == UpdateStatus::Success)
	{
		// Min/Max window sizes
		const float max_x = 400.0f * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale;
		const float max_y = 160.0f * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale;

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(max_x, max_y), ImGuiCond_Always);
		ImGui::SetNextWindowSizeConstraints(ImVec2(max_x, max_y), ImVec2(max_x, max_y));

		if (ImGui::Begin("Updated successfully!", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped("re4_tweaks has successfully been updated to version %s.\n\nPress OK to relaunch the game for the update to take effect!", updt.version.c_str());

			ImVec2 btn_size = ImVec2(104 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale, 35 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale);
			
			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - btn_size.y - 10);

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - btn_size.x);

			if (ImGui::Button("OK", btn_size))
			{
				updt.UpdateStatus = UpdateStatus::Finished;

				// Relaunch the game
				std::wstring bio4path = rootPath + L"bio4.exe";
				if ((int)ShellExecuteW(nullptr, L"open", bio4path.c_str(), nullptr, nullptr, SW_SHOWDEFAULT) > 32)
				{
					exit(0);
					return;
				}
			}

			ImGui::End();
		}
	}

	if (updt.UpdateStatus == UpdateStatus::Failed)
	{
		// Min/Max window sizes
		const float max_x = 400.0f * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale;
		const float max_y = 200.0f * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale;

		ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(max_x, max_y), ImGuiCond_Always);
		ImGui::SetNextWindowSizeConstraints(ImVec2(max_x, max_y), ImVec2(max_x, max_y));

		if (ImGui::Begin("Updated failed", nullptr, ImGuiWindowFlags_NoCollapse))
		{
			ImGui::TextWrapped("Failure to update re4_tweaks:");

			ImGui::TextWrapped(fail_msg.c_str());

			ImVec2 btn_size = ImVec2(104 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale, 35 * esHook._cur_monitor_dpi * re4t::cfg->fFontSizeScale);

			ImGui::SetCursorPosY(ImGui::GetWindowSize().y - btn_size.y - 10);

			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - btn_size.x);

			if (ImGui::Button("OK", btn_size))
			{
				updt.UpdateStatus = UpdateStatus::Finished;
			}

			ImGui::End();
		}
	}
}