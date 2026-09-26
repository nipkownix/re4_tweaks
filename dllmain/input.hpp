/*
 * Originally part of "ReShade", Copyright (C) 2014 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/reshade#license
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <shared_mutex>
#include <unordered_map>

typedef void (*HotkeyFunc)();

struct Hotkey {
	HotkeyFunc func;
	std::vector<uint32_t>* keyComboVector;
};

namespace re4t
{
	class input
	{
	public:
		enum button
		{
			button_left,
			button_right,
			button_middle,
			button_xbutton1,
			button_xbutton2,
		};

		// Inject hooks and populates the keymap
		void init();

		/// <summary>
		/// A window handle (HWND).
		/// </summary>
		using window_handle = void*;

		explicit input(window_handle window);

		/// <summary>
		/// Registers a window using raw input with the input manager.
		/// </summary>
		/// <param name="window">Window handle of the target window.</param>
		/// <param name="usage">HID usage of the registered device (0x02 = mouse, 0x06 = keyboard).</param>
		/// <param name="flags">Raw input mode flags that were used to register the device.</param>
		static void register_window_with_raw_input(window_handle window, unsigned short usage, unsigned int flags);
		/// <summary>
		/// Registers a window using normal input window messages with the input manager.
		/// </summary>
		/// <param name="window">Window handle of the target window.</param>
		/// <returns>Pointer to the input manager registered for this <paramref name="window"/>.</returns>
		static std::shared_ptr<input> register_window(window_handle window);

		/// <summary>
		/// Registers the mouse for raw input on <paramref name="window"/> (legacy messages kept, foreground only),
		/// unless something already has the mouse registered. Call once, right after the game window is created.
		/// </summary>
		static void ensure_raw_mouse_registration(window_handle window);

		/// <summary>
		/// Call from the window procedure on WM_ACTIVATEAPP. Logs focus changes (alt-tab, minimize) and starts a new focus session.
		/// </summary>
		static void notify_focus_changed(bool focused);
		/// <summary>
		/// Increments every time the game window regains focus. Lets the game thread tell "first frames after alt-tab" apart.
		/// </summary>
		static uint32_t focus_session();

		window_handle get_window_handle() const { return _window; }

		// Before accessing input data with any of the member functions below, first call "lock()" and keep the returned object alive while accessing it.

		bool is_key_down(unsigned int keycode) const;
		bool is_key_pressed(unsigned int keycode) const;
		bool is_key_pressed(unsigned int keycode, bool ctrl, bool shift, bool alt, bool force_modifiers = false) const;
		bool is_key_pressed(const unsigned int key[4], bool force_modifiers = false) const { return is_key_pressed(key[0], key[1] != 0, key[2] != 0, key[3] != 0, force_modifiers); }
		bool is_key_released(unsigned int keycode) const;
		bool is_combo_pressed(std::vector<uint32_t>* KeyVector) const;
		bool is_combo_down(std::vector<uint32_t>* KeyVector) const;
		bool is_any_key_down() const;
		bool is_any_key_pressed() const;
		bool is_any_key_released() const;
		unsigned int last_key_pressed() const;
		unsigned int last_key_released() const;
		bool is_mouse_button_down(unsigned int button) const;
		bool is_mouse_button_pressed(unsigned int button) const;
		bool is_mouse_button_released(unsigned int button) const;
		bool is_any_mouse_button_down() const;
		bool is_any_mouse_button_pressed() const;
		bool is_any_mouse_button_released() const;
		auto mouse_wheel_delta() const { return _mouse_wheel_delta; }
		auto mouse_movement_delta_x() const { return static_cast<int>(_mouse_position[0] - _last_mouse_position[0]); }
		auto mouse_movement_delta_y() const { return static_cast<int>(_mouse_position[1] - _last_mouse_position[1]); }
		unsigned int mouse_position_x() const { return _mouse_position[0]; }
		unsigned int mouse_position_y() const { return _mouse_position[1]; }
		void max_mouse_position(unsigned int position[2]) const;

		/// <summary>
		/// Raw mouse movement for the current game frame, as latched by latch_raw_mouse_delta.
		/// The value is stable for the whole frame and can be read any number of times, by any number of consumers.
		/// Game thread only.
		/// </summary>
		int raw_mouse_delta_x() const { return _raw_mouse_frame_delta[0]; }
		int raw_mouse_delta_y() const { return _raw_mouse_frame_delta[1]; }

		/// <summary>
		/// Checks if any of this frame's raw mouse movement came from an absolute-position device
		/// (Remote Desktop, streaming, VMs, tablets). Such deltas are not in mouse counts and should not be used for aiming.
		/// </summary>
		bool raw_mouse_is_absolute() const { return _raw_mouse_frame_absolute; }

		/// <summary>
		/// Moves everything accumulated from WM_INPUT since the previous call into the per-frame snapshot, and zeroes the accumulator.
		/// Call exactly once per game tick, at the start of PadRead, from the game thread.
		/// </summary>
		void latch_raw_mouse_delta();

		/// <summary>
		/// Discards any raw mouse movement that has not been latched yet (e.g. on focus loss). Thread-safe.
		/// </summary>
		void clear_raw_mouse_delta();

		void clear_mouse_wheel_delta()
		{
			_mouse_wheel_delta = 0;
		}

		/// <summary>
		/// Gets the character input as captured by 'WM_CHAR' for the current frame.
		/// </summary>
		const std::wstring& text_input() const { return _text_input; }

		/// <summary>
		/// Set to <see langword="true"/> to prevent mouse input window messages from reaching the application.
		/// </summary>
		void block_mouse_input(bool enable, bool releaseClipCursor);
		bool is_blocking_mouse_input() const { return _block_mouse; }
		/// <summary>
		/// Set to <see langword="true"/> to prevent keyboard input window messages from reaching the application.
		/// </summary>
		void block_keyboard_input(bool enable);
		bool is_blocking_keyboard_input() const { return _block_keyboard; }

		/// <summary>
		/// Locks access to the input data so it cannot be modified in another thread.
		/// </summary>
		/// <returns>RAII object holding the lock, which releases it after going out of scope.</returns>
		auto lock() { return std::shared_lock<std::shared_mutex>(_mutex); }

		/// <summary>
		/// Notifies the input manager to advance a frame.
		/// This updates input state to e.g. track whether a key was pressed this frame or before.
		/// </summary>
		void next_frame();
		void imgui_next_frame();

		/// <summary>
		/// Generates a human-friendly text representation of the specified <paramref name="keycode"/>.
		/// </summary>
		/// <param name="keycode">Virtual key code to use.</param>
		static std::string key_name_from_vk(unsigned int keycode);

		/// <summary>
		/// Generates a VK keycode from the key name in text form.
		/// </summary>
		/// <param name="key_name">Key name to use.</param>
		unsigned int vk_from_key_name(std::string key_name);

		/// <summary>
		/// Generates a DIK keycode from the key name in text form.
		/// </summary>
		/// <param name="key_name">Key name to use.</param>
		unsigned int dik_from_key_name(std::string key_name);

		/// <summary>
		/// Registers new hotkey or keycombo.
		/// </summary>
		void register_hotkey(Hotkey hotkey);

		/// <summary>
		/// Clear all hotkeys.
		/// </summary>
		void clear_hotkeys();

		/// <summary>
		/// Internal window message procedure. This looks for input messages and updates state for the corresponding windows accordingly.
		/// </summary>
		/// <param name="message_data">Pointer to a <see cref="MSG"/> with the message data.</param>
		/// <returns><see langword="true"/> if the called should ignore this message, or <see langword="false"/> if it should pass it on to the application.</returns>
		static bool handle_window_message(const void* message_data);

	private:
		std::shared_mutex _mutex;
		window_handle _window;
		bool _block_mouse = false;
		bool _block_keyboard = false;
		uint8_t _keys[256] = {};
		unsigned int _keys_time[256] = {};
		short _mouse_wheel_delta = 0;
		unsigned int _mouse_position[2] = {};
		unsigned int _last_mouse_position[2] = {};
		uint64_t _frame_count = 0; // Keep track of frame count to identify windows with a lot of rendering
		std::wstring _text_input;

		// Written by the window thread (GX render thread in RE4) under _mutex
		int _raw_mouse_accum[2] = {};
		bool _raw_mouse_absolute_seen = false;
		int _raw_mouse_prev_absolute[2] = {};
		bool _raw_mouse_prev_absolute_valid = false;

		// Per-frame snapshot. Written and read only by the game thread
		int _raw_mouse_frame_delta[2] = {};
		bool _raw_mouse_frame_absolute = false;
	};
}

extern std::shared_ptr<class re4t::input> pInput;