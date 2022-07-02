#pragma once

#define SPDLOG_WCHAR_FILENAMES
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <filesystem>

class spd
{
public:
    static const std::shared_ptr<spdlog::logger>& log()
    {
        static const auto log = spdlog::basic_logger_mt("basic_logger", spd::GetLogName(), true);
        spdlog::set_pattern("[%H:%M:%S] [t %t] %v");
        spdlog::flush_on(spdlog::level::debug);
        spdlog::set_default_logger(log);
        return log;
    }

    // Logs the process name and PID
    static void LogProcessNameAndPID()
    {
        // Get process path
        char exepath[MAX_PATH] = { 0 };
        GetModuleFileNameA(nullptr, exepath, MAX_PATH);

        // Get exe name
        std::string exename = std::filesystem::path(exepath).filename().string();

        // Log process name and ID
        log()->info("{0} (PID: {1})", exename, GetCurrentProcessId());
    }

private:
    inline static const std::wstring GetLogName()
    {
        HMODULE hm = NULL;
        GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&GetLogName, &hm);
        std::wstring ret;
        ret.resize(MAX_PATH);
        GetModuleFileNameW(hm, &ret[0], ret.size());
        ret = ret.substr(0, ret.find_last_of('.')) + L".log";
        return ret;
    }
};