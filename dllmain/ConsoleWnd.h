#pragma once
#include <cstdarg>
#include <imgui.h>
#include <string>
#include <vector>
#include <sstream>
#include "UI_Utility.h"

struct ConsoleOutput
{
	std::string TitleKeyCombo;

    // Add a message to the console
    template <typename T>
    void log(const T& value)
    {
        std::stringstream ss;
        ss << value;
        log(ss.str().c_str());
    }

    // Add a message to the console
    void log(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        buffer[sizeof(buffer) - 1] = 0;
        textBuffer.append(buffer);
        textBuffer.append("\n");
        lineOffsets.push_back(textBuffer.size());
    }

    // Add a game log message to the console
    void gameLog(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        char buffer[1024];
        vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        buffer[sizeof(buffer) - 1] = 0;
        textBuffer_game.append(buffer);
        textBuffer_game.append("\n");
        lineOffsets_game.push_back(textBuffer_game.size());
    }

    // Clears the re4t console
    void Clear()
    {
        textBuffer.clear();
        lineOffsets.clear();
    }

    // Clears the game console
    void gameClear()
    {
        textBuffer_game.clear();
        lineOffsets_game.clear();
    }

    // Renders the console
    void Render();

private:
    // re4t messages
    ImGuiTextFilterCustom Filter;
    ImGuiTextBuffer textBuffer;
    std::vector<int> lineOffsets;

    // Orignal RE4 messages
    ImGuiTextFilterCustom Filter_game;
    ImGuiTextBuffer textBuffer_game;
    std::vector<int> lineOffsets_game;
};

extern ConsoleOutput con;
extern bool bConsoleOpen;