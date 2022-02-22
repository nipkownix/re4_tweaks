#pragma once
#include "..\includes\stdafx.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include <hashes.h>
#include <sffont.hpp>

struct EndSceneHook
{
	std::chrono::high_resolution_clock::duration _last_frame_duration;
	std::chrono::high_resolution_clock::time_point _start_time;
	std::chrono::high_resolution_clock::time_point _last_present_time;

	ImGuiContext* _imgui_context = nullptr;
};

extern EndSceneHook esHook;

void Init_EndSceneHook();