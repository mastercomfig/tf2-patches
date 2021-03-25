//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Scoped windows hook.
//
//========================================================================//

#ifndef ENGINE_SCOPED_WINDOWS_HOOK_H_
#define ENGINE_SCOPED_WINDOWS_HOOK_H_
#ifdef _WIN32
#pragma once
#endif

#include <system_error>

#include "winlite.h"
#include "tier0/dbg.h"

namespace source::windows {
// Scoped windows hook.
class ScopedWindowsHook
{
public:
	// Creates scoped windows hook by hook type, hook procedure, instance and thread id.
	ScopedWindowsHook( int hook_type, HOOKPROC hook_proc, HINSTANCE instance, DWORD thread_id ) noexcept
		: hook_{ ::SetWindowsHookExW( hook_type, hook_proc, instance, thread_id ) },
		errno_code_{ std::error_code{ static_cast<int>(::GetLastError()), std::system_category() } } {}
	// Destroys windows hook.
	~ScopedWindowsHook() noexcept {
		if ( !errno_code_ ) {
			if ( !::UnhookWindowsHookEx(hook_) ) {
				const auto lastErrorText = std::system_category().message( ::GetLastError() );
				Warning( "Can't remove hook %p: %s\n", hook_, lastErrorText.c_str() );
			}
		}
	}

	ScopedWindowsHook( ScopedWindowsHook& h ) = delete;
	ScopedWindowsHook( ScopedWindowsHook&& h ) = delete;
	ScopedWindowsHook& operator=( ScopedWindowsHook& h ) = delete;
	ScopedWindowsHook& operator=( ScopedWindowsHook&& h ) = delete;

	// Hook initialization errno code.
	[[nodiscard]] std::error_code errno_code() const noexcept {
		return errno_code_;
	}

	// Calls next hook with hook code, wide_param and low_param.
	[[nodiscard]] static LRESULT CallNextHookEx( _In_ int hook_code, _In_ WPARAM wide_param, _In_ LPARAM low_param ) {
		// Hook parameter is ignored, see
		// https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-callnexthookex
		return ::CallNextHookEx( nullptr, hook_code, wide_param, low_param );
	}

private:
	// Hook.
	HHOOK hook_;
	// Hook initialization error code.
	const std::error_code errno_code_;
};
}  // namespace source::windows

#endif  // ENGINE_SCOPED_WINDOWS_HOOK_H_
