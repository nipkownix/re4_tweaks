#pragma once
#define SPDLOG_WCHAR_FILENAMES
#include "spdlog/spdlog.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <filesystem>
#include "../dllmain/Utils.h"
#include "../dllmain/Patches.h"

class spd
{
public:
    static const std::shared_ptr<spdlog::logger>& log()
    {
        static const auto log = spdlog::basic_logger_mt("basic_logger", logPath, true);
        spdlog::set_pattern("[%H:%M:%S] [t %t] %v");
        spdlog::flush_on(spdlog::level::debug);
        spdlog::set_default_logger(log);
        return log;
    }

    static const std::shared_ptr<spdlog::logger>& game_log()
    {
        static const auto log = spdlog::basic_logger_mt("game_output_logger", rootPath + L"game_output.log", true);
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
};