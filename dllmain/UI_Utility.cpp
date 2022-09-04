#include "dllmain.h"
#include "UI_Utility.h"
#include "input.hpp"

MenuTab Tab = MenuTab::Display;
TrainerTab CurTrainerTab = TrainerTab::Patches;

void ImGui_ItemSeparator()
{
	// ImGui::Separator() doesn't work inside tables?

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();

	drawList->AddLine(ImVec2(p0.x, p0.y), ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y), ImGui::GetColorU32(ImGuiCol_Separator));
}

void ImGui_ItemSeparator2()
{
	// ImGui::Separator() doesn't work inside tables?

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();

	drawList->AddRectFilled(ImVec2(p0.x, p0.y), ImVec2(p0.x + ImGui::GetContentRegionAvail().x, p0.y + 3), ImColor(150, 10, 40));
}

ImColor itmbgColor = ImColor(25, 20, 20, 166);
int itemIdx = -1;
float columnLastY[2] = { 0,0 };
void ImGui_ColumnInit()
{
	itemIdx = -1;
	columnLastY[0] = columnLastY[1] = 0;

	ImGui::TableNextColumn();
}

std::vector<float> bgHeightsPerTab[int(MenuTab::NumTabs)]; // item heights, per-tab
void ImGui_ColumnSwitch()
{
	auto& bgHeights = bgHeightsPerTab[int(Tab)];
	if (itemIdx >= 0)
	{
		// Store bgheight of previous item, so we can use it as a BG height.
		// (ImGui really should have a way to simply get the current row/cell height of a table instead...
		if (bgHeights.size() > size_t(itemIdx))
			bgHeights[itemIdx] = ImGui::GetCursorPos().y;
		else
			bgHeights.push_back(ImGui::GetCursorPos().y);

		ImGui::Dummy(ImVec2(10, 25));
		columnLastY[itemIdx % 2] = ImGui::GetCursorPos().y;
	}

	itemIdx++;
	ImGui::TableSetColumnIndex(itemIdx % 2);

	if (itemIdx > 1) // 0/1 are first item on column, no need to restore lastY
		ImGui::SetCursorPosY(columnLastY[itemIdx % 2]);

	float bgHeight = 0;
	if (bgHeights.size() > size_t(itemIdx))
		bgHeight = bgHeights[itemIdx];
	else
		bgHeight = 0;

	ImGui_ItemBG(bgHeight, itmbgColor);
}

void ImGui_ColumnFinish()
{
	if (itemIdx >= 0)
	{
		auto& bgHeights = bgHeightsPerTab[int(Tab)];

		// Store bgheight of previous item
		if (bgHeights.size() > size_t(itemIdx))
			bgHeights[itemIdx] = ImGui::GetCursorPos().y;
		else
			bgHeights.push_back(ImGui::GetCursorPos().y);
	}

	ImGui::Dummy(ImVec2(10, 25));
}

void ImGui_ItemBG(float RowSize, ImColor bgCol)
{
	// Works best when used inside a table

	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 p0 = ImGui::GetCursorScreenPos();
	float top = ImGui::GetCursorPosY();

	drawList->AddRectFilled(ImVec2(p0.x - 10, p0.y - 10), ImVec2(p0.x + 10 + ImGui::GetContentRegionAvail().x, p0.y + RowSize - top + 7), bgCol, 5.f);
}

// If we don't run these in another thread, we end up locking the rendering
void SetHotkeyComboThread(std::string* cfgHotkey)
{
	bWaitingForHotkey = true;

	pInput->set_hotkey(cfgHotkey, true);

	bWaitingForHotkey = false;
	return;
}

void SetHotkeyThread(std::string* cfgHotkey)
{
	bWaitingForHotkey = true;

	pInput->set_hotkey(cfgHotkey, false);

	bWaitingForHotkey = false;
	return;
}

bool ImGui_ButtonSameLine(const char* label, bool samelinecheck, float offset,
	const ImVec2 size)
{
	bool ret = ImGui::Button(label, size);

	if (samelinecheck)
	{
		ImGuiStyle& style = ImGui::GetStyle();

		float window_visible_x2 = ImGui::GetCursorScreenPos().x + offset + ImGui::GetContentRegionAvail().x;
		float last_button_x2;
		float next_button_x2;

		last_button_x2 = ImGui::GetItemRectMax().x;
		next_button_x2 = last_button_x2 + style.ItemSpacing.x + size.x; // Expected position if next button was on same line
		if (next_button_x2 < window_visible_x2)
			ImGui::SameLine();
	}

	return ret;
}

bool ImGui_TabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, MenuTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::PushStyleColor(ImGuiCol_Button, Tab == tabID ? activeCol : inactiveCol);
	bool ret = ImGui::Button(btnID, ImVec2(172, 31));
	ImGui::PopStyleColor();

	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();

	float textHeight = ImGui::CalcTextSize(text).y;

	float y_text_offset = (size.y - textHeight) * 0.5f;
	float y_icon_offset = y_text_offset + 2.0f;

	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 35.0f, p0.y + y_icon_offset), iconColor, icon, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + 65.0f, p0.y + y_text_offset), IM_COL32_WHITE, text, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));

	if (ret && tabID != MenuTab::NumTabs)
		Tab = tabID;

	return ret;
}

bool ImGui_TrainerTabButton(const char* btnID, const char* text, const ImVec4& activeCol,
	const ImVec4& inactiveCol, TrainerTab tabID, const char* icon, const ImColor iconColor,
	const ImColor textColor, const ImVec2& size, const bool samelinecheck)
{
	ImDrawList* drawList = ImGui::GetWindowDrawList();

	ImGui::PushStyleColor(ImGuiCol_Button, CurTrainerTab == tabID ? activeCol : inactiveCol);
	bool ret = ImGui_ButtonSameLine(btnID, samelinecheck, 0.0f, size);
	ImGui::PopStyleColor();

	auto p0 = ImGui::GetItemRectMin();
	auto p1 = ImGui::GetItemRectMax();

	float textWidth = ImGui::CalcTextSize(text).x;
	float textHeight = ImGui::CalcTextSize(text).y;
	float iconWidth = ImGui::CalcTextSize(icon).x;

	float x_text_offset = iconWidth + ((size.x - textWidth) * 0.5f);
	float y_text_offset = (size.y - textHeight) * 0.5f;

	float x_icon_offset = x_text_offset - iconWidth;
	float y_icon_offset = y_text_offset + 2.0f;

	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + x_icon_offset - 14.0f, p0.y + y_icon_offset), iconColor, icon, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));
	drawList->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(p0.x + x_text_offset - 7.0f, p0.y + y_text_offset), IM_COL32_WHITE, text, NULL, 0.0f, &ImVec4(p0.x, p0.y, p1.x, p1.y));

	if (ret && tabID != TrainerTab::NumTabs)
		CurTrainerTab = tabID;

	return ret;
}
