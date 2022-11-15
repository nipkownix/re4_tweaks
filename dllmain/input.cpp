/*
 * Originally part of "ReShade", Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#include "input.hpp"
#include "dllmain.h"
#include "Patches.h"
#include <algorithm>
#include <unordered_map>
#include <Windows.h>
#include <cassert>
#include "../external/eHooking/Hook.h"
#include "Settings.h"
#include <iomanip>
#include <array>
#include "spdlog/fmt/bin_to_hex.h"
#include "Utils.h"

std::shared_ptr<class input> pInput;

static std::shared_mutex s_windows_mutex;
static std::unordered_map<HWND, unsigned int> s_raw_input_windows;
static std::unordered_map<HWND, std::weak_ptr<input>> s_windows;

static std::unordered_map<unsigned int, std::string> _keyboardStringMap;

static std::unordered_map<std::string, unsigned int> _keyboardVKMap;
static std::unordered_map<std::string, unsigned int> _keyboardDIKMap;

std::vector<Hotkey> m_Hotkeys;

input::input(window_handle window)
	: _window(window)
{
}

void input::RegisterHotkey(Hotkey hotkey)
{ 
	m_Hotkeys.push_back(hotkey); 
}

void input::ClearHotkeys()
{ 
	m_Hotkeys.clear();
}

void input::register_window_with_raw_input(window_handle window, bool no_legacy_keyboard, bool no_legacy_mouse)
{
	assert(window != nullptr);

	const std::unique_lock<std::shared_mutex> lock(s_windows_mutex);

	const auto flags = (no_legacy_keyboard ? 0x1u : 0u) | (no_legacy_mouse ? 0x2u : 0u);
	const auto insert = s_raw_input_windows.emplace(static_cast<HWND>(window), flags);

	if (!insert.second) insert.first->second |= flags;
}
std::shared_ptr<input> input::register_window(window_handle window)
{
	assert(window != nullptr);

	const std::unique_lock<std::shared_mutex> lock(s_windows_mutex);

	const auto insert = s_windows.emplace(static_cast<HWND>(window), std::weak_ptr<input>());

	if (insert.second || insert.first->second.expired())
	{
		spd::log()->info("{0} -> Starting input capture for window {1}", __FUNCTION__, window);

		const auto instance = std::make_shared<input>(window);

		insert.first->second = instance;

		return instance;
	}
	else
	{
		return insert.first->second.lock();
	}
}

bool input::handle_window_message(const void *message_data)
{
	assert(message_data != nullptr);

	MSG details = *static_cast<const MSG *>(message_data);

	bool is_mouse_message = details.message >= WM_MOUSEFIRST && details.message <= WM_MOUSELAST;
	bool is_keyboard_message = details.message >= WM_KEYFIRST && details.message <= WM_KEYLAST;

	// Ignore messages that are not related to mouse or keyboard input
	if (details.message != WM_INPUT && !is_mouse_message && !is_keyboard_message)
		return false;

	// Guard access to windows list against race conditions
	std::unique_lock<std::shared_mutex> lock(s_windows_mutex);

	// Remove any expired entry from the list
	for (auto it = s_windows.begin(); it != s_windows.end();)
		if (it->second.expired())
			it = s_windows.erase(it);
		else
			++it;

	// Look up the window in the list of known input windows
	auto input_window = s_windows.find(details.hwnd);
	const auto raw_input_window = s_raw_input_windows.find(details.hwnd);

	if (input_window == s_windows.end())
	{
		// Walk through the window chain and until an known window is found
		EnumChildWindows(details.hwnd, [](HWND hwnd, LPARAM lparam) -> BOOL {
			auto &input_window = *reinterpret_cast<decltype(s_windows)::iterator *>(lparam);
			// Return true to continue enumeration
			return (input_window = s_windows.find(hwnd)) == s_windows.end();
		}, reinterpret_cast<LPARAM>(&input_window));
	}
	if (input_window == s_windows.end())
	{
		// Some applications handle input in a child window to the main render window
		if (const HWND parent = GetParent(details.hwnd); parent != NULL)
			input_window = s_windows.find(parent);
	}

	if (input_window == s_windows.end() && raw_input_window != s_raw_input_windows.end())
	{
		// Reroute this raw input message to the window with the most rendering
		input_window = std::max_element(s_windows.begin(), s_windows.end(),
			[](auto lhs, auto rhs) { return lhs.second.lock()->_frame_count < rhs.second.lock()->_frame_count; });
	}

	if (input_window == s_windows.end())
		return false;

	const std::shared_ptr<input> input = input_window->second.lock();
	// It may happen that the input was destroyed between the removal of expired entries above and here, so need to abort in this case
	if (input == nullptr)
		return false;

	// At this point we have a shared pointer to the input object and no longer reference any memory from the windows list, so can release the lock
	lock.unlock();

	// Calculate window client mouse position
	ScreenToClient(static_cast<HWND>(input->_window), &details.pt);

	// Prevent input threads from modifying input while it is accessed elsewhere
	const std::unique_lock<std::shared_mutex> input_lock = input->lock();

	input->_mouse_position[0] = details.pt.x;
	input->_mouse_position[1] = details.pt.y;

	switch (details.message)
	{
	case WM_INPUT:
		RAWINPUT raw_data;
		if (UINT raw_data_size = sizeof(raw_data);
			GET_RAWINPUT_CODE_WPARAM(details.wParam) != RIM_INPUT || // Ignore all input sink messages (when window is not focused)
			GetRawInputData(reinterpret_cast<HRAWINPUT>(details.lParam), RID_INPUT, &raw_data, &raw_data_size, sizeof(raw_data.header)) == UINT(-1))
			break;
		switch (raw_data.header.dwType)
		{
		case RIM_TYPEMOUSE:
			is_mouse_message = true;

			if (raw_input_window == s_raw_input_windows.end() || (raw_input_window->second & 0x2) == 0)
				break; // Input is already handled (since legacy mouse messages are enabled), so nothing to do here

			// Update raw mouse delta values
			if (raw_data.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
			{
				bool isVirtualDesktop = (raw_data.data.mouse.usFlags & MOUSE_VIRTUAL_DESKTOP) == MOUSE_VIRTUAL_DESKTOP;
				float xChange = (raw_data.data.mouse.lLastX / 65535.0f) * GetSystemMetrics(isVirtualDesktop ? SM_CXVIRTUALSCREEN : SM_CXSCREEN);
				float yChange = (raw_data.data.mouse.lLastY / 65535.0f) * GetSystemMetrics(isVirtualDesktop ? SM_CYVIRTUALSCREEN : SM_CYSCREEN);

				input->_raw_mouse_delta[0] = input->_raw_mouse_delta[0] + raw_data.data.mouse.lLastX - input->position.first;
				input->_raw_mouse_delta[1] = input->_raw_mouse_delta[1] + raw_data.data.mouse.lLastY - input->position.second;

				input->position.first += xChange;
				input->position.second += yChange;
			}
			else
			{
				input->_raw_mouse_delta[0] = input->_raw_mouse_delta[0] + raw_data.data.mouse.lLastX;
				input->_raw_mouse_delta[1] = input->_raw_mouse_delta[1] + raw_data.data.mouse.lLastY;

				input->position.first += raw_data.data.mouse.lLastX;
				input->position.second += raw_data.data.mouse.lLastY;
			}

			if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN)
				input->_keys[VK_LBUTTON] = 0x88;
			else if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP)
				input->_keys[VK_LBUTTON] = 0x08;
			if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)
				input->_keys[VK_RBUTTON] = 0x88;
			else if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_RIGHT_BUTTON_UP)
				input->_keys[VK_RBUTTON] = 0x08;
			if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN)
				input->_keys[VK_MBUTTON] = 0x88;
			else if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_MIDDLE_BUTTON_UP)
				input->_keys[VK_MBUTTON] = 0x08;

			if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_DOWN)
				input->_keys[VK_XBUTTON1] = 0x88;
			else if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_4_UP)
				input->_keys[VK_XBUTTON1] = 0x08;

			if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_DOWN)
				input->_keys[VK_XBUTTON2] = 0x88;
			else if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_BUTTON_5_UP)
				input->_keys[VK_XBUTTON2] = 0x08;

			if (raw_data.data.mouse.usButtonFlags & RI_MOUSE_WHEEL)
				input->_mouse_wheel_delta += static_cast<short>(raw_data.data.mouse.usButtonData) / WHEEL_DELTA;
			break;
		case RIM_TYPEKEYBOARD:
			if (raw_data.data.keyboard.VKey == 0)
				break; // Ignore messages without a valid key code

			is_keyboard_message = true;
			// Do not block key up messages if the key down one was not blocked previously
			if (input->_block_keyboard && (raw_data.data.keyboard.Flags & RI_KEY_BREAK) != 0 && raw_data.data.keyboard.VKey < 0xFF && (input->_keys[raw_data.data.keyboard.VKey] & 0x04) == 0)
				is_keyboard_message = false;

			if (raw_input_window == s_raw_input_windows.end() || (raw_input_window->second & 0x1) == 0)
				break; // Input is already handled by 'WM_KEYDOWN' and friends (since legacy keyboard messages are enabled), so nothing to do here

			// Filter out prefix messages without a key code
			if (raw_data.data.keyboard.VKey < 0xFF)
				input->_keys[raw_data.data.keyboard.VKey] = (raw_data.data.keyboard.Flags & RI_KEY_BREAK) == 0 ? 0x88 : 0x08,
				input->_keys_time[raw_data.data.keyboard.VKey] = details.time;

			// No 'WM_CHAR' messages are sent if legacy keyboard messages are disabled, so need to generate text input manually here
			// Cannot use the ToUnicode function always as it seems to reset dead key state and thus calling it can break subsequent application input, should be fine here though since the application is already explicitly using raw input
			// Since Windows 10 version 1607 this supports the 0x2 flag, which prevents the keyboard state from being changed, so it is not a problem there anymore either way
			if (WCHAR ch[3] = {}; (raw_data.data.keyboard.Flags & RI_KEY_BREAK) == 0 && ToUnicode(raw_data.data.keyboard.VKey, raw_data.data.keyboard.MakeCode, input->_keys, ch, 2, 0x2))
				input->_text_input += ch;
			break;
		}
		break;
	case WM_CHAR:
		input->_text_input += static_cast<wchar_t>(details.wParam);
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		assert(details.wParam > 0 && details.wParam < ARRAYSIZE(input->_keys));
		input->_keys[details.wParam] = 0x88;
		input->_keys_time[details.wParam] = details.time;
		if (input->_block_keyboard)
			input->_keys[details.wParam] |= 0x04;
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		assert(details.wParam > 0 && details.wParam < ARRAYSIZE(input->_keys));
		// Do not block key up messages if the key down one was not blocked previously (so key does not get stuck for the application)
		if (input->_block_keyboard && (input->_keys[details.wParam] & 0x04) == 0)
			is_keyboard_message = false;
		input->_keys[details.wParam] = 0x08;
		input->_keys_time[details.wParam] = details.time;
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK: // Double clicking generates this sequence: WM_LBUTTONDOWN -> WM_LBUTTONUP -> WM_LBUTTONDBLCLK -> WM_LBUTTONUP, so handle it like a normal down
		input->_keys[VK_LBUTTON] = 0x88;
		break;
	case WM_LBUTTONUP:
		input->_keys[VK_LBUTTON] = 0x08;
		break;
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		input->_keys[VK_RBUTTON] = 0x88;
		break;
	case WM_RBUTTONUP:
		input->_keys[VK_RBUTTON] = 0x08;
		break;
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		input->_keys[VK_MBUTTON] = 0x88;
		break;
	case WM_MBUTTONUP:
		input->_keys[VK_MBUTTON] = 0x08;
		break;
	case WM_MOUSEWHEEL:
		input->_mouse_wheel_delta += GET_WHEEL_DELTA_WPARAM(details.wParam) / WHEEL_DELTA;
		break;
	case WM_XBUTTONDOWN:
		assert(HIWORD(details.wParam) == XBUTTON1 || HIWORD(details.wParam) == XBUTTON2);
		input->_keys[VK_XBUTTON1 + (HIWORD(details.wParam) - XBUTTON1)] = 0x88;
		break;
	case WM_XBUTTONUP:
		assert(HIWORD(details.wParam) == XBUTTON1 || HIWORD(details.wParam) == XBUTTON2);
		input->_keys[VK_XBUTTON1 + (HIWORD(details.wParam) - XBUTTON1)] = 0x08;
		break;
	}

	return (is_mouse_message && input->_block_mouse) || (is_keyboard_message && input->_block_keyboard);
}

bool input::is_key_down(unsigned int keycode) const
{
	assert(keycode < ARRAYSIZE(_keys));
	return keycode < ARRAYSIZE(_keys) && (_keys[keycode] & 0x80) == 0x80;
}

bool input::is_key_pressed(unsigned int keycode) const
{
	assert(keycode < ARRAYSIZE(_keys));

	return keycode > 0 && keycode < ARRAYSIZE(_keys) && (_keys[keycode] & 0x88) == 0x88;
}

bool input::is_key_pressed(unsigned int keycode, bool ctrl, bool shift, bool alt, bool force_modifiers) const
{
	if (keycode == 0)
		return false;

	const bool key_down = is_key_pressed(keycode), ctrl_down = is_key_down(VK_CONTROL), shift_down = is_key_down(VK_SHIFT), alt_down = is_key_down(VK_MENU);
	if (force_modifiers) // Modifier state is required to match
		return key_down && (ctrl == ctrl_down && shift == shift_down && alt == alt_down);
	else // Modifier state is optional and only has to match when down
		return key_down && (!ctrl || ctrl_down) && (!shift || shift_down) && (!alt || alt_down);
}

bool input::is_key_released(unsigned int keycode) const
{
	assert(keycode < ARRAYSIZE(_keys));
	return keycode > 0 && keycode < ARRAYSIZE(_keys) && (_keys[keycode] & 0x88) == 0x08;
}

bool bStateChanged;
bool input::is_combo_pressed(std::vector<uint32_t>* KeyVector) const
{
	if (KeyVector->size() < 1)
		return false;

	if (KeyVector->size() == 1)
		return is_key_pressed(KeyVector->at(0));

	bool isComboPressed = KeyVector->size() > 0;
	for (auto& key : *KeyVector)
	{
		if (!is_key_down(key) || bStateChanged)
			isComboPressed = false;

		if (is_key_released(key) && bStateChanged)
			bStateChanged = false;
	}

	if (isComboPressed)
		bStateChanged = true;

	return isComboPressed;
}

bool input::is_combo_down(std::vector<uint32_t>* KeyVector) const
{
	if (KeyVector->size() < 1)
		return false;

	if (KeyVector->size() == 1)
		return is_key_down(KeyVector->at(0));

	bool isComboDown = KeyVector->size() > 0;
	for (auto& key : *KeyVector)
	{
		if (!pInput->is_key_down(key) || bStateChanged)
			isComboDown = false;
	}

	return isComboDown;
}

bool input::is_any_key_down() const
{
	// Skip mouse buttons
	for (unsigned int i = VK_XBUTTON2 + 1; i < ARRAYSIZE(_keys); i++)
		if (is_key_down(i))
			return true;
	return false;
}
bool input::is_any_key_pressed() const
{
	return last_key_pressed() != 0;
}
bool input::is_any_key_released() const
{
	return last_key_released() != 0;
}

unsigned int input::last_key_pressed() const
{
	for (unsigned int i = VK_XBUTTON2 + 1; i < ARRAYSIZE(_keys); i++)
		if (is_key_pressed(i))
			return i;
	return 0;
}
unsigned int input::last_key_released() const
{
	for (unsigned int i = VK_XBUTTON2 + 1; i < ARRAYSIZE(_keys); i++)
		if (is_key_released(i))
			return i;
	return 0;
}

bool input::is_mouse_button_down(unsigned int button) const
{
	assert(button < 5);
	return is_key_down(VK_LBUTTON + button + (button < 2 ? 0 : 1)); // VK_CANCEL is being ignored by runtime
}
bool input::is_mouse_button_pressed(unsigned int button) const
{
	assert(button < 5);
	return is_key_pressed(VK_LBUTTON + button + (button < 2 ? 0 : 1)); // VK_CANCEL is being ignored by runtime
}
bool input::is_mouse_button_released(unsigned int button) const
{
	assert(button < 5);
	return is_key_released(VK_LBUTTON + button + (button < 2 ? 0 : 1)); // VK_CANCEL is being ignored by runtime
}

bool input::is_any_mouse_button_down() const
{
	for (unsigned int i = 0; i < 5; i++)
		if (is_mouse_button_down(i))
			return true;
	return false;
}
bool input::is_any_mouse_button_pressed() const
{
	for (unsigned int i = 0; i < 5; i++)
		if (is_mouse_button_pressed(i))
			return true;
	return false;
}
bool input::is_any_mouse_button_released() const
{
	for (unsigned int i = 0; i < 5; i++)
		if (is_mouse_button_released(i))
			return true;
	return false;
}

std::string input::KeyMap_getSTR(int keyINT)
{
	assert(!_keyboardStringMap.empty());

	if (!_keyboardStringMap.count(keyINT))
		return std::string();

	return _keyboardStringMap[keyINT];
}

int input::KeyMap_getVK(std::string keySTR)
{
	assert(!_keyboardVKMap.empty());

	if (!_keyboardVKMap.count(keySTR))
		return 0;

	return _keyboardVKMap[keySTR];
}

int input::KeyMap_getDIK(std::string keySTR)
{
	assert(!_keyboardDIKMap.empty());

	if (!_keyboardDIKMap.count(keySTR))
		return 0;

	return _keyboardDIKMap[keySTR];
}

void ClearKeyboardBuffer(int key_code, BYTE* keyboard_state)
{
	unsigned int scan_code = MapVirtualKeyW(key_code, MAPVK_VK_TO_VSC);

	wchar_t chars[5];
	int code = 0;
	do {
		code = ToUnicode(key_code, scan_code, keyboard_state, chars, 4, 0);
	} while (code < 0);
}

std::string get_str_from_VK(int key_code) {

	unsigned int scan_code = MapVirtualKeyW(key_code, MAPVK_VK_TO_VSC);

	uint8_t keyStates[256];
	memset(keyStates, 1, 256); // all pressed

	wchar_t chars[5];
	int code = ToUnicode(key_code, scan_code, keyStates, chars, 4, 0);

	if (code == -1) {
		// dead key
		if (chars[0] == 0 || iswcntrl(chars[0])) {
			return std::string();
		}
		code = 1;
	}

	// Avoid stuff like ´´ or ~~
	ClearKeyboardBuffer(key_code, keyStates);

	if (code <= 0 || (code == 1 && iswcntrl(chars[0]))) {
		return std::string();
	}

	return WstrToStr(chars);
}

void input::set_hotkey(std::string* cfgHotkey, bool supportsCombo)
{
	int HotkeyVK1 = 0;
	int HotkeyVK2 = 0;

	std::string OrigHotkeyCombo = *cfgHotkey;
	std::string FinalHotkeyCombo;

	// Hacky way to change ImGui's current button label
	*cfgHotkey = "Press any key...";

	bool waitingforkey = true;

	while (waitingforkey) {
		for (unsigned int Key1 = 0; Key1 < ARRAYSIZE(_keys); Key1++)
		{
			while (pInput->is_key_down(Key1)) {
				HotkeyVK1 = Key1;

				if (supportsCombo)
				{
					for (unsigned int Key2 = 0; Key2 < ARRAYSIZE(_keys); Key2++)
					{
						if (pInput->is_key_down(Key2) && (Key2 != Key1))
						{
							HotkeyVK2 = Key2;
						}
					}
				}
				waitingforkey = false;
			}
		}
	}

	// Clear the second hotkey if they're the same.
	// I've had that happen before due to deadkey weirdness.
	if (HotkeyVK1 == HotkeyVK2)
		HotkeyVK2 = 0;

	#ifdef VERBOSE
	con.AddConcatLog("HotkeyVK1 = ", HotkeyVK1);
	con.AddConcatLog("HotkeyVK2 = ", HotkeyVK2);
	#endif

	// Check if our KeyMap function is able to identify the key names, restore the original combo if not.
	// As of now, there's no popup informing the user that the key is unsupported. TODO: Add popup?
	if (HotkeyVK2 > 0)
	{
		if (KeyMap_getSTR(HotkeyVK1).empty() || KeyMap_getSTR(HotkeyVK2).empty())
		{
			*cfgHotkey = OrigHotkeyCombo;
			return;
		}
		else
			FinalHotkeyCombo = KeyMap_getSTR(HotkeyVK1) + "+" + KeyMap_getSTR(HotkeyVK2);
	}
	else
	{
		if (KeyMap_getSTR(HotkeyVK1).empty())
		{
			*cfgHotkey = OrigHotkeyCombo;
			return;
		}
		else
			FinalHotkeyCombo = KeyMap_getSTR(HotkeyVK1);
	}

	#ifdef VERBOSE
	con.AddConcatLog("FinalHotkeyCombo = ", FinalHotkeyCombo.c_str());
	#endif

	*cfgHotkey = FinalHotkeyCombo;
}

void input::next_frame()
{
	// Check hotkey status
	for (const Hotkey& hotkey : m_Hotkeys) {
		if (is_combo_pressed(hotkey.keyComboVector))
		{
			hotkey.func();
		}
	}

	_frame_count++;

	for (auto &state : _keys)
		state &= ~0x08;

	// Reset any pressed down key states (apart from mouse buttons) that have not been updated for more than 5 seconds
	// Do not check mouse buttons here, since 'GetAsyncKeyState' always returns the state of the physical mouse buttons, not the logical ones in case they were remapped
	// See https://docs.microsoft.com/windows/win32/api/winuser/nf-winuser-getasynckeystate
	// And time is not tracked for mouse buttons anyway
	const DWORD time = GetTickCount();
	for (unsigned int i = 8; i < 256; ++i)
		if ((_keys[i] & 0x80) != 0 &&
			(time - _keys_time[i]) > 5000 &&
			(GetAsyncKeyState(i) & 0x8000) == 0)
			(_keys[i] = 0x08);

	_text_input.clear();
	_mouse_wheel_delta = 0;
	_last_mouse_position[0] = _mouse_position[0];
	_last_mouse_position[1] = _mouse_position[1];
	_last_raw_mouse_delta[0] = _raw_mouse_delta[0];
	_last_raw_mouse_delta[1] = _raw_mouse_delta[1];

	// Update caps lock state
	_keys[VK_CAPITAL] |= GetKeyState(VK_CAPITAL) & 0x1;

	// Update modifier key state
	if ((_keys[VK_MENU] & 0x88) != 0 &&
		(GetKeyState(VK_MENU) & 0x8000) == 0)
		(_keys[VK_MENU] = 0x08);

	// Update print screen state (there is no key down message, but the key up one is received via the message queue)
	if ((_keys[VK_SNAPSHOT] & 0x80) == 0 &&
		(GetAsyncKeyState(VK_SNAPSHOT) & 0x8000) != 0)
		(_keys[VK_SNAPSHOT] = 0x88),
		(_keys_time[VK_SNAPSHOT] = time);
}

static inline bool is_blocking_mouse_input()
{
	const std::shared_lock<std::shared_mutex> lock(s_windows_mutex);

	const auto predicate = [](const auto &input_window) {
		return !input_window.second.expired() && input_window.second.lock()->is_blocking_mouse_input();
	};
	return std::any_of(s_windows.cbegin(), s_windows.cend(), predicate);
}
static inline bool is_blocking_keyboard_input()
{
	const std::shared_lock<std::shared_mutex> lock(s_windows_mutex);

	const auto predicate = [](const auto &input_window) {
		return !input_window.second.expired() && input_window.second.lock()->is_blocking_keyboard_input();
	};
	return std::any_of(s_windows.cbegin(), s_windows.cend(), predicate);
}

BOOL(WINAPI* GetMessageA_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL WINAPI HookGetMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
#if 1
	// Implement 'GetMessage' with a timeout (see also DLL_PROCESS_DETACH in dllmain.cpp for more explanation)
	while (!PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE) && g_module_handle != nullptr)
		MsgWaitForMultipleObjects(0, nullptr, FALSE, 500, QS_ALLINPUT);

	if (g_module_handle == nullptr && lpMsg->message != WM_QUIT)
		std::memset(lpMsg, 0, sizeof(MSG)); // Clear message structure, so application does not process it
#else
	const BOOL result = GetMessageA_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
	if (result < 0) // If there is an error, the return value is negative (https://docs.microsoft.com/windows/win32/api/winuser/nf-winuser-getmessage)
		return result;

	assert(lpMsg != nullptr);

	if (lpMsg->hwnd != nullptr && input::handle_window_message(lpMsg))
	{
		// We still want 'WM_CHAR' messages, so translate message
		TranslateMessage(lpMsg);

		// Change message so it is ignored by the recipient window
		lpMsg->message = WM_NULL;
	}
#endif

	return lpMsg->message != WM_QUIT;
}

FARPROC p_GetMessageA = nullptr;
void InstallGetMessageA_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking GetMessageA...");
		
	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_GetMessageA, Hook::HotPatch(Hook::GetProcAddress(h_user32, "GetMessageA"), "GetMessageA", HookGetMessageA));

	GetMessageA_orig = (decltype(GetMessageA_orig))InterlockedCompareExchangePointer((PVOID*)&p_GetMessageA, nullptr, nullptr);
}

BOOL(WINAPI* GetMessageW_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
BOOL WINAPI HookGetMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax)
{
#if 1
	while (!PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE) && g_module_handle != nullptr)
		MsgWaitForMultipleObjects(0, nullptr, FALSE, 500, QS_ALLINPUT);

	if (g_module_handle == nullptr && lpMsg->message != WM_QUIT)
		std::memset(lpMsg, 0, sizeof(MSG));
#else
	const BOOL result = GetMessageW_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax);
	if (result < 0)
		return result;

	assert(lpMsg != nullptr);

	if (lpMsg->hwnd != nullptr && input::handle_window_message(lpMsg))
	{
		// We still want 'WM_CHAR' messages, so translate message
		TranslateMessage(lpMsg);

		// Change message so it is ignored by the recipient window
		lpMsg->message = WM_NULL;
	}
#endif

	return lpMsg->message != WM_QUIT;
}

FARPROC p_GetMessageW = nullptr;
void InstallGetMessageW_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking GetMessageW...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_GetMessageW, Hook::HotPatch(Hook::GetProcAddress(h_user32, "GetMessageW"), "GetMessageW", HookGetMessageW));

	GetMessageW_orig = (decltype(GetMessageW_orig))InterlockedCompareExchangePointer((PVOID*)&p_GetMessageW, nullptr, nullptr);
}

BOOL(WINAPI* PeekMessageA_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL WINAPI HookPeekMessageA(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	if (!PeekMessageA_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
		return FALSE;

	assert(lpMsg != nullptr);

	if (lpMsg->hwnd != nullptr && (wRemoveMsg & PM_REMOVE) != 0 && input::handle_window_message(lpMsg))
	{
		// We still want 'WM_CHAR' messages, so translate message
		TranslateMessage(lpMsg);

		// Change message so it is ignored by the recipient window
		lpMsg->message = WM_NULL;
	}

	return TRUE;
}

FARPROC p_PeekMessageA = nullptr;
void InstallPeekMessageA_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking PeekMessageA...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_PeekMessageA, Hook::HotPatch(Hook::GetProcAddress(h_user32, "PeekMessageA"), "PeekMessageA", HookPeekMessageA));

	PeekMessageA_orig = (decltype(PeekMessageA_orig))InterlockedCompareExchangePointer((PVOID*)&p_PeekMessageA, nullptr, nullptr);
}

BOOL(WINAPI* PeekMessageW_orig)(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg);
BOOL WINAPI HookPeekMessageW(LPMSG lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg)
{
	if (!PeekMessageW_orig(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
		return FALSE;

	assert(lpMsg != nullptr);

	if (lpMsg->hwnd != nullptr && (wRemoveMsg & PM_REMOVE) != 0 && input::handle_window_message(lpMsg))
	{
		// We still want 'WM_CHAR' messages, so translate message
		TranslateMessage(lpMsg);

		// Change message so it is ignored by the recipient window
		lpMsg->message = WM_NULL;
	}

	return TRUE;
}

FARPROC p_PeekMessageW = nullptr;
void InstallPeekMessageW_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking PeekMessageW...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_PeekMessageW, Hook::HotPatch(Hook::GetProcAddress(h_user32, "PeekMessageW"), "PeekMessageW", HookPeekMessageW));

	PeekMessageW_orig = (decltype(PeekMessageW_orig))InterlockedCompareExchangePointer((PVOID*)&p_PeekMessageW, nullptr, nullptr);
}

BOOL(WINAPI* PostMessageA_orig)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI HookPostMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// Do not allow mouse movement simulation while we block input
	if (is_blocking_mouse_input() && Msg == WM_MOUSEMOVE)
		return TRUE;

	return PostMessageA_orig(hWnd, Msg, wParam, lParam);
}

FARPROC p_PostMessageA = nullptr;
void InstallPostMessageA_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking PostMessageA...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_PostMessageA, Hook::HotPatch(Hook::GetProcAddress(h_user32, "PostMessageA"), "PostMessageA", HookPostMessageA));

	PostMessageA_orig = (decltype(PostMessageA_orig))InterlockedCompareExchangePointer((PVOID*)&p_PostMessageA, nullptr, nullptr);
}

BOOL(WINAPI* PostMessageW_orig)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI HookPostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (is_blocking_mouse_input() && Msg == WM_MOUSEMOVE)
		return TRUE;

	return PostMessageW_orig(hWnd, Msg, wParam, lParam);
}

FARPROC p_PostMessageW = nullptr;
void InstallPostMessageW_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking PostMessageW...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_PostMessageW, Hook::HotPatch(Hook::GetProcAddress(h_user32, "PostMessageW"), "PostMessageW", HookPostMessageW));

	PostMessageW_orig = (decltype(PostMessageW_orig))InterlockedCompareExchangePointer((PVOID*)&p_PostMessageW, nullptr, nullptr);
}

BOOL(WINAPI* RegisterRawInputDevices_orig)(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize);
BOOL WINAPI HookRegisterRawInputDevices(PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize)
{
	if (pConfig->bVerboseLog)
		spd::log()->info("{0} -> Redirecting RegisterRawInputDevices (pRawInputDevices = {1}, uiNumDevices = {2}, cbSize = {3})", __FUNCTION__, IntToHexStr(pRawInputDevices), uiNumDevices, cbSize);

	for (UINT i = 0; i < uiNumDevices; ++i)
	{
		const auto &device = pRawInputDevices[i];

		if (pConfig->bVerboseLog)
		{
			spd::log()->info("{0} -> Dumping device registration at index {1}:", __FUNCTION__, i);
			spd::log()->info("+-----------------------------------------+-----------------------------------------+");
			spd::log()->info("| Parameter                               | Value                                   |");
			spd::log()->info("+-----------------------------------------+-----------------------------------------+");
			spd::log()->info("| UsagePage                               | {:>39} |", IntToHexStr(device.usUsagePage));
			spd::log()->info("| Usage                                   | {:>39} |", IntToHexStr(device.usUsage));
			spd::log()->info("| Flags                                   | {:>39} |", IntToHexStr(device.dwFlags));
			spd::log()->info("| TargetWindow                            | {:>39} |", IntToHexStr(device.hwndTarget));
			spd::log()->info("+-----------------------------------------+-----------------------------------------+");
		}

		if (device.usUsagePage != 1 || device.hwndTarget == nullptr)
			continue;

		input::register_window_with_raw_input(device.hwndTarget, device.usUsage == 0x06 && (device.dwFlags & RIDEV_NOLEGACY) != 0, device.usUsage == 0x02 && (device.dwFlags & RIDEV_NOLEGACY) != 0);
	}

	if (!RegisterRawInputDevices_orig(pRawInputDevices, uiNumDevices, cbSize))
	{
		spd::log()->info("{0} -> Failed with error code {1}", __FUNCTION__, GetLastError());
		return FALSE;
	}

	return TRUE;
}

FARPROC p_RegisterRawInputDevices = nullptr;
void InstallRegisterRawInputDevices_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking RegisterRawInputDevices...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_RegisterRawInputDevices, Hook::HotPatch(Hook::GetProcAddress(h_user32, "RegisterRawInputDevices"), "RegisterRawInputDevices", HookRegisterRawInputDevices));

	RegisterRawInputDevices_orig = (decltype(RegisterRawInputDevices_orig))InterlockedCompareExchangePointer((PVOID*)&p_RegisterRawInputDevices, nullptr, nullptr);
}

BOOL(WINAPI* ClipCursor_orig)(const RECT* lpRect);
BOOL WINAPI HookClipCursor(const RECT *lpRect)
{
	if (is_blocking_mouse_input())
		// Some applications clip the mouse cursor, so disable that while we want full control over mouse input
		lpRect = nullptr;

	return ClipCursor_orig(lpRect);
}

FARPROC p_ClipCursor = nullptr;
void InstallClipCursor_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking ClipCursor...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_ClipCursor, Hook::HotPatch(Hook::GetProcAddress(h_user32, "ClipCursor"), "ClipCursor", HookClipCursor));

	ClipCursor_orig = (decltype(ClipCursor_orig))InterlockedCompareExchangePointer((PVOID*)&p_ClipCursor, nullptr, nullptr);
}

static POINT s_last_cursor_position = {};

BOOL(WINAPI* SetCursorPos_orig)(int X, int Y);
BOOL WINAPI HookSetCursorPos(int X, int Y)
{
	s_last_cursor_position.x = X;
	s_last_cursor_position.y = Y;

	if (is_blocking_mouse_input())
		return TRUE;

	return SetCursorPos_orig(X, Y);
}

FARPROC p_SetCursorPos = nullptr;
void InstallSetCursorPos_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking SetCursorPos...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_SetCursorPos, Hook::HotPatch(Hook::GetProcAddress(h_user32, "SetCursorPos"), "SetCursorPos", HookSetCursorPos));

	SetCursorPos_orig = (decltype(SetCursorPos_orig))InterlockedCompareExchangePointer((PVOID*)&p_SetCursorPos, nullptr, nullptr);
}

BOOL(WINAPI* GetCursorPos_orig)(LPPOINT lpPoint);
BOOL WINAPI HookGetCursorPos(LPPOINT lpPoint)
{
	if (is_blocking_mouse_input())
	{
		assert(lpPoint != nullptr);

		// Just return the last cursor position before we started to block mouse input, to stop it from moving
		*lpPoint = s_last_cursor_position;

		return TRUE;
	}
	return GetCursorPos_orig(lpPoint);
}

FARPROC p_GetCursorPos = nullptr;
void InstallGetCursorPos_Hook()
{
	if (pConfig->bVerboseLog)
		spd::log()->info("Hooking GetCursorPos...");

	HMODULE h_user32 = GetModuleHandle(L"user32.dll");
	InterlockedExchangePointer((PVOID*)&p_GetCursorPos, Hook::HotPatch(Hook::GetProcAddress(h_user32, "GetCursorPos"), "GetCursorPos", HookGetCursorPos));

	GetCursorPos_orig = (decltype(GetCursorPos_orig))InterlockedCompareExchangePointer((PVOID*)&p_GetCursorPos, nullptr, nullptr);
}

void input::PopulateKeymap()
{
	// Populate string map

	spd::log()->info("Populating keymap");

	std::string keystring;
	for (int i = 0; i < 256; ++i)
	{
		keystring = get_str_from_VK(i);

		_keyboardStringMap.insert(_keyboardStringMap.end(), std::pair<unsigned int, std::string>(i, keystring));
		keystring = "";
	}

	_keyboardStringMap[0x1B] = "ESCAPE";
	_keyboardStringMap[0x70] = "F1";
	_keyboardStringMap[0x71] = "F2";
	_keyboardStringMap[0x72] = "F3";
	_keyboardStringMap[0x73] = "F4";
	_keyboardStringMap[0x74] = "F5";
	_keyboardStringMap[0x75] = "F6";
	_keyboardStringMap[0x76] = "F7";
	_keyboardStringMap[0x77] = "F8";
	_keyboardStringMap[0x78] = "F9";
	_keyboardStringMap[0x79] = "F10";
	_keyboardStringMap[0x7A] = "F11";
	_keyboardStringMap[0x7B] = "F12";
	_keyboardStringMap[0x7C] = "F13";
	_keyboardStringMap[0x7D] = "F14";
	_keyboardStringMap[0x7E] = "F15";
	_keyboardStringMap[0x08] = "BACKSPACE";
	_keyboardStringMap[0x20] = "SPACE";
	_keyboardStringMap[0x2D] = "INSERT";
	_keyboardStringMap[0x23] = "END";
	_keyboardStringMap[0x24] = "HOME";
	_keyboardStringMap[0x22] = "PAGEDOWN";
	_keyboardStringMap[0x21] = "PAGEUP";
	_keyboardStringMap[0x2E] = "DELETE";
	_keyboardStringMap[0x0D] = "ENTER";
	_keyboardStringMap[0x09] = "TAB";
	_keyboardStringMap[0x14] = "CAPSLOCK";
	_keyboardStringMap[0x5D] = "APPS";
	_keyboardStringMap[0x15] = "KANA";
	_keyboardStringMap[0x19] = "KANJI";
	_keyboardStringMap[0x1C] = "CONVERT";
	_keyboardStringMap[0x1D] = "NONCONVERT";
	_keyboardStringMap[0x2C] = "PRINTSCR";
	_keyboardStringMap[0x91] = "SCROLL";
	_keyboardStringMap[0x13] = "PAUSE";

	// Numpad
	_keyboardStringMap[0x90] = "NUMLOCK";
	_keyboardStringMap[0x6F] = "NUMPAD_/";
	_keyboardStringMap[0x6C] = "NUMPAD_ENTER";
	_keyboardStringMap[0x6A] = "NUMPAD_*";
	_keyboardStringMap[0x6D] = "NUMPAD_-";
	_keyboardStringMap[0x6B] = "NUMPAD_+";
	_keyboardStringMap[0x6E] = "NUMPAD_.";
	_keyboardStringMap[0x60] = "NUMPAD_0";
	_keyboardStringMap[0x61] = "NUMPAD_1";
	_keyboardStringMap[0x62] = "NUMPAD_2";
	_keyboardStringMap[0x63] = "NUMPAD_3";
	_keyboardStringMap[0x64] = "NUMPAD_4";
	_keyboardStringMap[0x65] = "NUMPAD_5";
	_keyboardStringMap[0x66] = "NUMPAD_6";
	_keyboardStringMap[0x67] = "NUMPAD_7";
	_keyboardStringMap[0x68] = "NUMPAD_8";
	_keyboardStringMap[0x68] = "NUMPAD_9";

	// Arrow keys
	_keyboardStringMap[0x26] = "UP";
	_keyboardStringMap[0x28] = "DOWN";
	_keyboardStringMap[0x25] = "LEFT";
	_keyboardStringMap[0x27] = "RIGHT";

	// Shift
	_keyboardStringMap[0x10] = "SHIFT";
	_keyboardStringMap[0xA0] = "LSHIFT";
	_keyboardStringMap[0xA1] = "RSHIFT";

	// Control
	_keyboardStringMap[0x11] = "CTRL";
	_keyboardStringMap[0xA2] = "LCTRL";
	_keyboardStringMap[0xA3] = "RCTRL";

	// Alt
	_keyboardStringMap[0x12] = "ALT";
	_keyboardStringMap[0xA4] = "LALT";
	_keyboardStringMap[0xA5] = "RALT";

	// Winkey
	_keyboardStringMap[0x5B] = "LWIN";
	_keyboardStringMap[0x5C] = "RWIN";

	// Mouse
	_keyboardStringMap[VK_LBUTTON] = "LMOUSE";
	_keyboardStringMap[VK_RBUTTON] = "RMOUSE";
	_keyboardStringMap[VK_MBUTTON] = "MMOUSE";
	_keyboardStringMap[VK_XBUTTON1] = "MOUSE4";
	_keyboardStringMap[VK_XBUTTON2] = "MOUSE5";

	// Copy string map to vk map, inverting the items, so we can get VK from str
	for (std::unordered_map<unsigned int, std::string>::iterator i = _keyboardStringMap.begin(); i != _keyboardStringMap.end(); ++i)
		_keyboardVKMap[i->second] = i->first;

	// Populate DIK map, so we can get DIK from str
	for (int i = 0; i < 256; ++i)
	{
		unsigned int DIK = MapVirtualKeyEx(i, /*MAPVK_VK_TO_VSC*/0, GetKeyboardLayout(0)) & 0xFF;

		_keyboardDIKMap.insert(_keyboardDIKMap.end(), std::pair<std::string, unsigned int>(_keyboardStringMap[i], DIK));
	}
}

void input::InstallHooks()
{
	spd::log()->info("Hooking input-related APIs...");

	InstallGetMessageA_Hook();
	InstallGetMessageW_Hook();
	InstallPeekMessageA_Hook();
	InstallPeekMessageW_Hook();
	InstallPostMessageA_Hook();
	InstallPostMessageW_Hook();
	InstallRegisterRawInputDevices_Hook();
	InstallClipCursor_Hook();
	InstallSetCursorPos_Hook();
	InstallGetCursorPos_Hook();
}