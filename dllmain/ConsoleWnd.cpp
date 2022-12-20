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
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Size.x - (550 * esHook._cur_monitor_dpi), viewport->Pos.y + (20 * esHook._cur_monitor_dpi)), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500 * esHook._cur_monitor_dpi, 400 * esHook._cur_monitor_dpi), ImGuiCond_FirstUseEver);

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
                if (Filter.IsActive())
                {
                    for (unsigned int i = 0; i < lineOffsets.size(); i++)
                    {
                        const char* lineStart = textBuffer.begin() + (i > 0 ? lineOffsets[i - 1] : 0);
                        const char* lineEnd = textBuffer.begin() + lineOffsets[i];
                        if (Filter.PassFilter(lineStart, lineEnd))
                            ImGui::TextUnformatted(lineStart, lineEnd);
                    }
                }
                else
                {
                    ImGuiListClipper clipper(lineOffsets.size());
                    while (clipper.Step())
                    {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                        {
                            const char* lineStart = textBuffer.begin() + (i > 0 ? lineOffsets[i - 1] : 0);
                            const char* lineEnd = textBuffer.begin() + lineOffsets[i];
                            ImGui::TextUnformatted(lineStart, lineEnd);
                        }
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
                if (Filter_game.IsActive())
                {
                    for (unsigned int i = 0; i < lineOffsets_game.size(); i++)
                    {
                        const char* lineStart = textBuffer_game.begin() + (i > 0 ? lineOffsets_game[i - 1] : 0);
                        const char* lineEnd = textBuffer_game.begin() + lineOffsets_game[i];
                        if (Filter_game.PassFilter(lineStart, lineEnd))
                            ImGui::TextUnformatted(lineStart, lineEnd);
                    }
                }
                else
                {
                    ImGuiListClipper clipper(lineOffsets_game.size());
                    while (clipper.Step())
                    {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
                        {
                            const char* lineStart = textBuffer_game.begin() + (i > 0 ? lineOffsets_game[i - 1] : 0);
                            const char* lineEnd = textBuffer_game.begin() + lineOffsets_game[i];
                            ImGui::TextUnformatted(lineStart, lineEnd);
                        }
                    }
                }
                ImGui::PopStyleVar();

                if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(0.90f);

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
    va_list args;
    va_start(args, msg);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), msg, args);
    va_end(args);

    // Append "cLog: " at the start of the buffer
    char OSReportBuffer[1024];
    strcpy(OSReportBuffer, "OSReport: ");
    strcat(OSReportBuffer, buffer);

    // Check if the last character in the buffer is a newline
    if (OSReportBuffer[strlen(OSReportBuffer) - 1] == '\n')
    {
        // If it is, remove it by overwriting it with a null terminator
        OSReportBuffer[strlen(OSReportBuffer) - 1] = '\0';
    }

    con.gameLog(OSReportBuffer);
}

void cLog__err_nullsubver_hook(uint32_t flag, uint32_t errId, char* mes, ...)
{
    va_list args;
    va_start(args, mes);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), mes, args);
    va_end(args);

    // Append "cLog: " at the start of the buffer
    char cLogBuffer[1024];
    strcpy(cLogBuffer, "cLog: ");
    strcat(cLogBuffer, buffer);

    // Check if the last character in the buffer is a newline
    if (cLogBuffer[strlen(cLogBuffer) - 1] == '\n')
    {
        // If it is, remove it by overwriting it with a null terminator
        cLogBuffer[strlen(cLogBuffer) - 1] = '\0';
    }

    con.gameLog(cLogBuffer);
}

void re4t::init::ConsoleWnd()
{
    #ifdef VERBOSE
    bConsoleOpen = true;
    #endif

    // Redirect some unused logging functions to output to our console window instead.
    // May be useful for modders, or just interesting for people like me.
    if (re4t::cfg->bShowGameOutput)
    {
        auto pattern = hook::pattern("E8 ? ? ? ? 83 C4 0C A1 ? ? ? ? F7 40 ? ? ? ? ? 74 2D");
        InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), OSReport_hook, PATCH_JUMP);

        pattern = hook::pattern("E8 ? ? ? ? 83 C4 0C 5F 5E 5B 5D C2 04 00 B8 ? ? ? ? 66");
        InjectHook(injector::GetBranchDestination(pattern.count(1).get(0).get<uint32_t>(0)).as_int(), cLog__err_nullsubver_hook, PATCH_JUMP);
    }
}