#include <iostream>
#include "dllmain.h"
#include "ConsoleWnd.h"
#include "Settings.h"
#include "imgui\imgui.h"
#include "input.hpp"
#include "Utils.h"

ConsoleOutput con;

bool bConsoleOpen = false;

bool AutoScroll = true;  // Keep scrolling if already at the bottom.

std::string TitleCombo;

std::vector<uint32_t> ConsoleCombo;

bool ParseConsoleKeyCombo(std::string_view in_combo)
{
    if (in_combo.empty())
        return false;

    ConsoleCombo.clear();
    ConsoleCombo = re4t::cfg->ParseKeyCombo(in_combo);

    pInput->register_hotkey({ []() {
        bConsoleOpen = !bConsoleOpen;
    }, &ConsoleCombo });

    return ConsoleCombo.size() > 0;
}

void ConsoleOutput::Render()
{
    // Min/Max window sizes
    const float min_x = 500.0f * esHook._cur_monitor_dpi;
    const float min_y = 400.0f * esHook._cur_monitor_dpi;

    const float max_x = 1920.0f * esHook._cur_monitor_dpi;
    const float max_y = 1080.0f * esHook._cur_monitor_dpi;

    ImGui::SetNextWindowSizeConstraints(ImVec2(min_x, min_y), ImVec2(max_x, max_y));

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Size.x - (550 * esHook._cur_monitor_dpi), viewport->Pos.y + (20 * esHook._cur_monitor_dpi)), ImGuiCond_FirstUseEver);

    std::string name = std::string("Console Output - ") + con.TitleKeyCombo + std::string(" to Show/Hide") + " - " + re4t::cfg->sTrainerFocusUIKeyCombo + " to Focus/Unfocus";

    ImGui::Begin(name.c_str());
    {
        #ifdef VERBOSE
        if (ImGui::CollapsingHeader("Debug"))
        {
            ImGui::SliderFloat("fdbg1", &re4t::cfg->fdbg1, -200.0f, 200.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("fdbg2", &re4t::cfg->fdbg2, -200.0f, 200.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("fdbg3", &re4t::cfg->fdbg3, -200.0f, 200.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("fdbg4", &re4t::cfg->fdbg4, -200.0f, 200.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("fdbg5", &re4t::cfg->fdbg5, -200.0f, 200.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("fdbg6", &re4t::cfg->fdbg6, -200.0f, 200.0f, "%.4f", ImGuiSliderFlags_AlwaysClamp);

            ImGui::Spacing();

            ImGui::Checkbox("bdbg1", &re4t::cfg->bdbg1);
            ImGui::Checkbox("bdbg2", &re4t::cfg->bdbg2);
            ImGui::Checkbox("bdbg3", &re4t::cfg->bdbg3);
        }

        ImGui::Separator();
        #endif

        if (ImGui::BeginTabBar("ConsoleTabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {
            #ifdef VERBOSE
            if (ImGui::BeginTabItem("re4_tweaks log"))
            {
                // Options menu
                if (ImGui::BeginPopup("Options"))
                {
                    ImGui::Checkbox("Auto-scroll", &AutoScroll);
                    ImGui::EndPopup();
                }

                // Main window
                if (ImGui::Button("Options"))
                    ImGui::OpenPopup("Options");
                ImGui::SameLine();
                bool clear = ImGui::Button("Clear");
                ImGui::SameLine();
                bool copy = ImGui::Button("Copy");
                ImGui::SameLine();
                ImGui::Dummy(ImVec2(20 * esHook._cur_monitor_dpi, 0));
                ImGui::SameLine();
                Filter.Draw("Filter", -70.0f);
                ImGui::SameLine();
                if (ImGui::SmallButton("X"))
                    Filter.Clear();

                ImGui::BeginChild("scrolling1", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

                if (clear)
                    con.Clear();
                if (copy)
                    ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                for (unsigned int i = 0; i < lineOffsets.size(); i++)
                {
                    const char* lineStart = textBuffer.begin() + (i > 0 ? lineOffsets[i - 1] : 0);
                    const char* lineEnd = textBuffer.begin() + lineOffsets[i];

                    if (Filter_game.IsActive())
                    {
                        if (Filter_game.PassFilter(lineStart, lineEnd))
                            ImGui::TextUnformatted(lineStart, lineEnd);
                    }
                    else
                    {
                        ImGui::TextUnformatted(lineStart, lineEnd);
                    }
                }
                ImGui::PopStyleVar();

                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                ImGui::EndChild();

                ImGui::EndTabItem();
            }
            #endif
            if (ImGui::BeginTabItem("Game log"))
            {
                // Options menu
                if (ImGui::BeginPopup("Options"))
                {
                    ImGui::Checkbox("Auto-scroll", &AutoScroll);
                    ImGui::EndPopup();
                }

                // Main window
                if (ImGui::Button("Options"))
                    ImGui::OpenPopup("Options");
                ImGui::SameLine();
                bool clear = ImGui::Button("Clear");
                ImGui::SameLine();
                bool copy = ImGui::Button("Copy");
                ImGui::SameLine();
                ImGui::Dummy(ImVec2(20 * esHook._cur_monitor_dpi, 0));
                ImGui::SameLine();
                Filter_game.Draw("Filter", -70.0f);
                ImGui::SameLine();
                if (ImGui::SmallButton("X"))
                    Filter_game.Clear();

                ImGui::BeginChild("scrolling2", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

                if (clear)
                    con.gameClear();
                if (copy)
                    ImGui::LogToClipboard();

                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
                for (unsigned int i = 0; i < lineOffsets_game.size(); i++)
                {
                    const char* lineStart = textBuffer_game.begin() + (i > 0 ? lineOffsets_game[i - 1] : 0);
                    const char* lineEnd = textBuffer_game.begin() + lineOffsets_game[i];

                    if (Filter_game.IsActive())
                    {
                        if (Filter_game.PassFilter(lineStart, lineEnd))
                            ImGui::TextUnformatted(lineStart, lineEnd);
                    }
                    else
                    {
                        ImGui::TextUnformatted(lineStart, lineEnd);
                    }
                }
                ImGui::PopStyleVar();

                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);

                ImGui::EndChild();

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void __cdecl OSReport_hook(const char* msg, ...)
{
    if (!re4t::cfg->bShowGameOutput)
        return;

    va_list args;
    char buffer[1024];

    va_start(args, msg);
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    // Check if the last character in the buffer is a newline
    if (buffer[strlen(buffer) - 1] == '\n')
    {
        // If it is, remove it by overwriting it with a null terminator
        buffer[strlen(buffer) - 1] = '\0';
    }

    con.gameLog("OSReport: %s", buffer);
}

void OSPanic_nullsub_hook(const char* file, int line, const char* message, ...)
{
    if (!re4t::cfg->bShowGameOutput)
        return;

    va_list args;
    char buffer[1024];

    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    // Check if the last character in the buffer is a newline
    if (buffer[strlen(buffer) - 1] == '\n')
    {
        // If it is, remove it by overwriting it with a null terminator
        buffer[strlen(buffer) - 1] = '\0';
    }

    con.gameLog("OSPanic: File: %s, Line: %d, Message: %s", file, line, buffer);
}

void cLog__err_nullsubver_hook(uint32_t flag, uint32_t errId, char* mes, ...)
{
    if (!re4t::cfg->bShowGameOutput)
        return;

    va_list args;
    char buffer[1024];

    va_start(args, mes);
    vsnprintf(buffer, sizeof(buffer), mes, args);
    va_end(args);

    // Check if the last character in the buffer is a newline
    if (buffer[strlen(buffer) - 1] == '\n')
    {
        // If it is, remove it by overwriting it with a null terminator
        buffer[strlen(buffer) - 1] = '\0';
    }

    con.gameLog("cLog::err: Flag: %d, errId: %d, Message: %s", flag, errId, buffer);
}

void cLog__err_1_hook(int a1, int a2, const char* message, ...)
{
    if (!re4t::cfg->bShowGameOutput)
        return;

    va_list args;
    char buffer[1024];

    va_start(args, message);
    vsnprintf(buffer, sizeof(buffer), message, args);
    va_end(args);

    // Check if the last character in the buffer is a newline
    if (buffer[strlen(buffer) - 1] == '\n')
    {
        // If it is, remove it by overwriting it with a null terminator
        buffer[strlen(buffer) - 1] = '\0';
    }

    con.gameLog("cLog::err_1: %s", buffer);
}

void re4t::init::ConsoleWnd()
{
    #ifdef VERBOSE
    bConsoleOpen = true;
    #endif

    // Redirect some unused logging functions to output to our console window instead.
    // May be useful for modders, or just interesting for people like me.
    auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 0C A1 ? ? ? ? F7 40 ? ? ? ? ? 74 2D");
    InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), OSReport_hook, PATCH_JUMP);

    pattern = hook::pattern("E8 ? ? ? ? 83 C4 0C 5F 5E 33 C0 5B 5D C3 68 ? ? ? ? E8");
    InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), OSPanic_nullsub_hook, PATCH_JUMP);

    pattern = hook::pattern("E8 ? ? ? ? 83 C4 0C 5F 5E 5B 5D C2 04 00 B8 ? ? ? ? 66");
    InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cLog__err_nullsubver_hook, PATCH_JUMP);

    pattern = hook::pattern("E8 ? ? ? ? 83 C4 10 8B 75 F8 33 DB 39 5E 48 7E 39 81");
    InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cLog__err_1_hook, PATCH_JUMP);
}