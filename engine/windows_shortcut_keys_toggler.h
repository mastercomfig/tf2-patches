//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Toggles left/right Windows keys.
// See https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/disabling-shortcut-keys-in-games#disable-the-windows-key-with-a-keyboard-hook
//
//========================================================================//

#ifndef ENGINE_WINDOWS_SHORTCUT_KEYS_TOGGLER_H_
#define ENGINE_WINDOWS_SHORTCUT_KEYS_TOGGLER_H_
#ifdef _WIN32
#pragma once
#endif

#include <system_error>

#include "scoped_windows_hook.h"
#include "winlite.h"
#include "tier0/dbg.h"

namespace source::windows {
// Should we enable window shortcut keys or not.
using ShouldEnableWindowsShortcutKeysFunction = bool (*)() noexcept;

// Toggles left/right Windows keys.  See
// https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/disabling-shortcut-keys-in-games#disable-the-windows-key-with-a-keyboard-hook
class WindowsShortcutKeysToggler
{
public:
	// Creates Windows keys toggler by hooking module instance and function to control Windows keys toggle.
	WindowsShortcutKeysToggler( HINSTANCE hook_module, ShouldEnableWindowsShortcutKeysFunction should_enable_windows_shortcut_keys_fn ) noexcept
		: keyboard_hook_{ WH_KEYBOARD_LL, &LowLevelKeyboardProc, hook_module, 0 } {
		Assert(!!should_enable_windows_shortcut_keys_fn);

		should_enable_windows_shortcut_keys_fn_ = should_enable_windows_shortcut_keys_fn;
	}

	WindowsShortcutKeysToggler( WindowsShortcutKeysToggler& t ) = delete;
	WindowsShortcutKeysToggler( WindowsShortcutKeysToggler&& t ) = delete;
	WindowsShortcutKeysToggler& operator=(WindowsShortcutKeysToggler& t ) = delete;
	WindowsShortcutKeysToggler& operator=(WindowsShortcutKeysToggler&& t ) = delete;

	~WindowsShortcutKeysToggler() noexcept = default;

	// Gets toggler initialization errno code.
	[[nodiscard]] std::error_code errno_code() const noexcept {
		return keyboard_hook_.errno_code();
	}

private:
	// Keyboard hook to catch key up/down events.
	const source::windows::ScopedWindowsHook keyboard_hook_;
	// Callback containing needed info to apply keyboard hooks.
	static ShouldEnableWindowsShortcutKeysFunction
		should_enable_windows_shortcut_keys_fn_;

	// Low level keyboard hook procedure.  Performs conditional Windows key filtering.
	static LRESULT CALLBACK LowLevelKeyboardProc( _In_ int hook_code, _In_ WPARAM wide_param, _In_ LPARAM low_param ) {
		if (hook_code < 0 || hook_code != HC_ACTION) {
			// Do not process message.
			return source::windows::ScopedWindowsHook::CallNextHookEx(
				hook_code, wide_param, low_param);
		}

		bool should_skip_key{ false };
		auto* keyboard_dll_hooks = reinterpret_cast<KBDLLHOOKSTRUCT*>(low_param);

		switch (wide_param) {
			case WM_KEYDOWN:
			case WM_KEYUP: {
				should_skip_key = (!should_enable_windows_shortcut_keys_fn_() &&
					((keyboard_dll_hooks->vkCode == VK_LWIN) ||
						(keyboard_dll_hooks->vkCode == VK_RWIN)));
				break;
			}
		}

		// Return 1 to ensure keyboard keys are handled and don't popup further.
		return !should_skip_key
			? source::windows::ScopedWindowsHook::CallNextHookEx(
				hook_code, wide_param, low_param)
			: 1L;
	}
};

ShouldEnableWindowsShortcutKeysFunction
	WindowsShortcutKeysToggler::should_enable_windows_shortcut_keys_fn_ =
	nullptr;
}  // namespace source::windows

#endif  // ENGINE_WINDOWS_SHORTCUT_KEYS_TOGGLER_H_
