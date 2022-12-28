#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "../dllmain/ConsoleWnd.h"
#include "log.h"
#include <shellapi.h>

/*
*  Simple .ini reader with command-line override support
*/

class iniReader
{
public:
    iniReader(const std::string& filename)
    {
        // Store the filename
        filename_ = filename;

        // Open the file for reading
        std::ifstream file(filename);

        // Read the file line by line
        std::string line;
        std::string section;
        while (std::getline(file, line))
        {
            // Skip comments
            if (line[0] == '#' || line[0] == ';')
                continue;

            // Check if this is a section header
            size_t separator_pos = line.find('=');
            if (line[0] == '[')
            {
                // Get section name
                size_t closing_bracket_pos = line.find(']');
                if (closing_bracket_pos != std::string::npos)
                    section = line.substr(1, closing_bracket_pos - 1);
            }
            else if (separator_pos != std::string::npos) // Should be key-value pair then
            {
                // Extract the key and value strings
                std::string key = line.substr(0, separator_pos);
                std::string value = line.substr(separator_pos + 1);

                // Remove leading and trailing white spaces from key and value
                while (!key.empty() && key.back() == ' ')
                    key.pop_back();
                while (!value.empty() && (value.back() == ' ' || value.back() == '\t'))
                    value.pop_back();
                while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
                    value.erase(0, 1);

                // Store the pair in our values map
                values_[section + "." + key] = value;
            }
        }
    }

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

        // Read from values map
        std::string full_key = section + "." + key;
        if (values_.count(full_key) > 0)
            return values_[full_key];
        else
            return default_value;
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

        // Read from values map
        std::string full_key = section + "." + key;
        if (values_.count(full_key) > 0)
        {
            std::string value = values_[full_key];
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);

            if (value == "true" || value == "1") // Treat "1" as true
                return true;
            else if (value == "false" || value == "0") // Also treat "0" as false
                return false;
            else
                return default_value; // If everything fails, just return the default value
        }
        else
            return default_value;
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

        // Read from values map
        std::string full_key = section + "." + key;
        if (values_.count(full_key) > 0)
        {
            std::istringstream iss(values_[full_key]);
            int value;
            iss >> value;
            return value;
        }
        else
            return default_value;
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

        std::string full_key = section + "." + key;
        if (values_.count(full_key) > 0)
        {
            std::istringstream iss(values_[full_key]);
            float value;
            iss >> value;
            return value;
        }
        else
            return default_value;
    }

    // Write
    void writeString(const std::string& section, const std::string& key, const std::string& value)
    {
        std::string value_str = " " + value;
        WritePrivateProfileStringA(section.c_str(), key.c_str(), value_str.c_str(), filename_.c_str());
    }
    
    void writeBool(const std::string& section, const std::string& key, bool value)
    {
        std::string value_str = " " + std::string(value ? "True" : "False");
        WritePrivateProfileStringA(section.c_str(), key.c_str(), value_str.c_str(), filename_.c_str());
    }

    void writeInt(const std::string& section, const std::string& key, int value)
    {
        char value_char[255];
        _snprintf_s(value_char, 255, "%s%d", " ", value);
        WritePrivateProfileStringA(section.c_str(), key.c_str(), value_char, filename_.c_str());
    }

    void writeFloat(const std::string& section, const std::string& key, float value)
    {
        char value_char[255];
        _snprintf_s(value_char, 255, "%s%.6f", " ", value);
        WritePrivateProfileStringA(section.c_str(), key.c_str(), value_char, filename_.c_str());
    }

private:
    std::string filename_;
    std::map<std::string, std::string> values_;

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

