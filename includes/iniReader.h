#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "../dllmain/ConsoleWnd.h"
#include <simpleini/SimpleIni.h>
#include "log.h"
#include <shellapi.h>

/*
*  Simple .ini reader with command-line override support
*/

class iniReader
{
public:
    // Use .ini path/filename from parameter
    iniReader(const std::wstring& filename) : inipath(filename)
    {
        // Read and parse the INI file
        m_ini.LoadFile(filename.c_str());
    }

    // Find the .ini path/filename using the module path
    iniReader()
    {
        // Get module handle
        HMODULE hModule = nullptr;
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&StrToUpper), &hModule);

        // Get the full module path
        wchar_t pathBuf[MAX_PATH];
        GetModuleFileNameW(hModule, pathBuf, MAX_PATH);

        // wstring
        std::wstring modulePath = pathBuf;

        // Store the file path
        inipath = modulePath.substr(0, modulePath.find_last_of('.')) + L".ini";

        // Read and parse the INI file
        m_ini.LoadFile(inipath.c_str());
    }

    // Destructor
    ~iniReader()
    {
        if (argv)
            LocalFree(argv);
        argv = nullptr;
        argc = 0;
    }

    // Read
    std::string getString(const std::string& section, const std::string& key, const std::string& default_value = "")
    {
        // Search for command-line overrides
        std::string cmd_value = SearchCommandLineParamValue(key);
        if (!cmd_value.empty())
        {
            #ifdef VERBOSE
            con.log("iniReader: Overriding key \"%s\" from section \"%s\" with value: %s (command-line overrides)", key.c_str(), section.c_str(), cmd_value.c_str());
            #endif

            spd::log()->info("iniReader: Overriding key \"{}\" from section \"{}\" with value: {} (command-line overrides)", key, section, cmd_value);

            return cmd_value;
        }

        // Read from simpleIni
        return m_ini.GetValue(section.c_str(), key.c_str(), default_value.c_str());
    }

    bool getBool(const std::string& section, const std::string& key, bool default_value = false)
    {
        // Search for command-line overrides
        std::string cmd_value = SearchCommandLineParamValue(key, true);
        if (!cmd_value.empty())
        {
            #ifdef VERBOSE
            con.log("iniReader: Overriding key \"%s\" from section \"%s\" with value: %s (command-line overrides)", key.c_str(), section.c_str(), cmd_value.c_str());
            #endif

            spd::log()->info("iniReader: Overriding key \"{}\" from section \"{}\" with value: {} (command-line overrides)", key, section, cmd_value);

            return !(_stricmp(cmd_value.c_str(), "false") == 0 || _stricmp(cmd_value.c_str(), "0") == 0);
        }

        // Read from simpleIni
        return m_ini.GetBoolValue(section.c_str(), key.c_str(), default_value);
    }

    int getInt(const std::string& section, const std::string& key, int default_value = 0)
    {
        // Search for command-line overrides
        std::string cmd_value = SearchCommandLineParamValue(key);
        if (!cmd_value.empty())
        {
            try
            {
                #ifdef VERBOSE
                con.log("iniReader: Overriding key \"%s\" from section \"%s\" with value: %s (command-line overrides)", key.c_str(), section.c_str(), cmd_value.c_str());
                #endif

                spd::log()->info("iniReader: Overriding key \"{}\" from section \"{}\" with value: {} (command-line overrides)", key, section, cmd_value);

                return std::stoi(cmd_value, nullptr, 0);
            }
            catch (std::exception&) {}
        }

        // Read from simpleIni
        return int(m_ini.GetLongValue(section.c_str(), key.c_str(), default_value));
    }

    float getFloat(const std::string& section, const std::string& key, float default_value = 0.0f)
    {
        // Search for command-line overrides
        std::string cmd_value = SearchCommandLineParamValue(key);
        if (!cmd_value.empty())
        {
            try
            {
                #ifdef VERBOSE
                con.log("iniReader: Overriding key \"%s\" from section \"%s\" with value: %s (command-line overrides)", key.c_str(), section.c_str(), cmd_value.c_str());
                #endif

                spd::log()->info("iniReader: Overriding key \"{}\" from section \"{}\" with value: {} (command-line overrides)", key, section, cmd_value);

                return (float)std::stod(cmd_value);
            }
            catch (std::exception&) {}
        }

        // Read from simpleIni
        return float(m_ini.GetDoubleValue(section.c_str(), key.c_str(), default_value));
    }

    // Write
    void setString(const std::string& section, const std::string& key, const std::string& value)
    {
        m_ini.SetValue(section.c_str(), key.c_str(), value.c_str());
    }
    
    void setBool(const std::string& section, const std::string& key, bool value)
    {
        m_ini.SetBoolValue(section.c_str(), key.c_str(), value);
    }

    void setInt(const std::string& section, const std::string& key, int value)
    {
        m_ini.SetLongValue(section.c_str(), key.c_str(), value);
    }

    void setFloat(const std::string& section, const std::string& key, float value)
    {
        m_ini.SetDoubleValue(section.c_str(), key.c_str(), value);
    }

    void writeIni() {
        m_ini.SaveFile(inipath.c_str());
    }

private:
    CSimpleIniA m_ini;

    std::wstring inipath;

    // CommandLine overrides
    LPWSTR* argv = nullptr;
    int argc = 0;

    std::string SearchCommandLineParamValue(std::string_view param_name, bool is_boolean = false)
    {
        if (!argv)
            argv = CommandLineToArgvW(GetCommandLineW(), &argc);

        // Convert param name to wide char to make comparing against wide args easier
        WCHAR param_buf[256];
        if (!SUCCEEDED(MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, param_name.data(), -1, param_buf, 256)))
            return "";

        BOOL lpUsedDefaultChar = FALSE;
        char arg_buf[256];
        for (int i = 1; i < argc; i++)
        {
            // Make sure the arg is at least large enough for a switch char (- or /) along with 1 extra char
            if (wcslen(argv[i]) <= 1)
                continue;

            // Is this arg the param we're looking for?
            if (_wcsicmp(param_buf, argv[i] + 1) != 0) // argv[i] + 1 to skip the switch character
                continue;

            // Found the param, grab the value
            if (i + 1 < argc)
            {
                if (SUCCEEDED(WideCharToMultiByte(CP_ACP, 0, argv[i + 1], -1, arg_buf, 256, NULL, &lpUsedDefaultChar)))
                    return arg_buf;
            }
            else
            {
                // Param was specified but no value given, if it's a bool then we'll treat specifying it as true
                if (is_boolean)
                    return "true";
            }
        }

        return "";
    }
};

