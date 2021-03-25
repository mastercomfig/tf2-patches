//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Toggles accessibility shortcut keys.
// See https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/disabling-shortcut-keys-in-games#disable-the-accessibility-shortcut-keys
//
//========================================================================//

#ifndef ENGINE_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
#define ENGINE_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
#ifdef _WIN32
#pragma once
#endif

#include <optional>
#include <type_traits>
#include <system_error>

#include "winlite.h"
#include "tier0/dbg.h"

namespace source::windows {
// Toggles accessibility shortcut keys.  See
// https://docs.microsoft.com/en-us/windows/desktop/dxtecharts/disabling-shortcut-keys-in-games#disable-the-accessibility-shortcut-keys
class AccessibilityShortcutKeysToggler {
 public:
  // Creates accessibility shortcut keys toggler.
  AccessibilityShortcutKeysToggler() noexcept
      : is_toggled_{},
        startup_sticky_keys_{sizeof(decltype(startup_sticky_keys_)), 0},
        startup_toggle_keys_{sizeof(decltype(startup_toggle_keys_)), 0},
        startup_filter_keys_{sizeof(decltype(startup_filter_keys_)), 0} {
    // Save the current sticky/toggle/filter key settings so they can be
    // restored them later.
    SystemKeysInfo( SPI_GETSTICKYKEYS, startup_sticky_keys_ );
    SystemKeysInfo( SPI_GETTOGGLEKEYS, startup_toggle_keys_ );
    SystemKeysInfo( SPI_GETFILTERKEYS, startup_filter_keys_ );
  }

  AccessibilityShortcutKeysToggler( const AccessibilityShortcutKeysToggler &t ) = delete;
  AccessibilityShortcutKeysToggler( AccessibilityShortcutKeysToggler &&t ) = delete;
  AccessibilityShortcutKeysToggler &operator=( const AccessibilityShortcutKeysToggler &t ) = delete;
  AccessibilityShortcutKeysToggler &operator=( AccessibilityShortcutKeysToggler &&t ) = delete;

  // Restores accessibility shortcut keys to original system state.
  ~AccessibilityShortcutKeysToggler() noexcept { Toggle(true); }

  // Toggle accessibility shortcut keys or not.
  void Toggle( bool toggle ) noexcept;

 private:
  // Small wrapper for better semantics.
  using nullable_bool = std::optional<bool>;

  nullable_bool is_toggled_;

  STICKYKEYS startup_sticky_keys_;
  TOGGLEKEYS startup_toggle_keys_;
  FILTERKEYS startup_filter_keys_;

  template <typename TSystemKey, typename = std::enable_if_t<
                                     std::is_same_v<TSystemKey, STICKYKEYS> ||
                                     std::is_same_v<TSystemKey, TOGGLEKEYS> ||
                                     std::is_same_v<TSystemKey, FILTERKEYS>>>
  void SystemKeysInfo( _In_ UINT action, _In_ TSystemKey &key ) noexcept {
    const BOOL is_succeeded{::SystemParametersInfoW(action, sizeof(key), &key, 0)};
    if (!is_succeeded) {
      const auto lastErrorText = std::system_category().message(::GetLastError());
      Warning("Can't toggle accessibility shortcut keys, please, do not press them in the game: %s\n", lastErrorText.c_str());
    }
  }
};

void AccessibilityShortcutKeysToggler::Toggle( bool toggle ) noexcept {
  if (is_toggled_.has_value() && is_toggled_.value() == toggle) return;

  if (toggle) {
    // Restore StickyKeys/etc to original state and enable Windows key.
    SystemKeysInfo(SPI_SETSTICKYKEYS, startup_sticky_keys_);
    SystemKeysInfo(SPI_SETTOGGLEKEYS, startup_toggle_keys_);
    SystemKeysInfo(SPI_SETFILTERKEYS, startup_filter_keys_);
  } else {
    // Disable StickyKeys/etc shortcuts but if the accessibility feature is
    // on, then leave the settings alone as its probably being usefully used.

    STICKYKEYS sticky_keys_off = startup_sticky_keys_;
    if ((sticky_keys_off.dwFlags & SKF_STICKYKEYSON) == 0) {
      // Disable the hotkey and the confirmation.
      sticky_keys_off.dwFlags &= ~SKF_HOTKEYACTIVE;
      sticky_keys_off.dwFlags &= ~SKF_CONFIRMHOTKEY;

      SystemKeysInfo(SPI_SETSTICKYKEYS, sticky_keys_off);
    }

    TOGGLEKEYS toggle_keys_off = startup_toggle_keys_;
    if ((toggle_keys_off.dwFlags & TKF_TOGGLEKEYSON) == 0) {
      // Disable the hotkey and the confirmation.
      toggle_keys_off.dwFlags &= ~TKF_HOTKEYACTIVE;
      toggle_keys_off.dwFlags &= ~TKF_CONFIRMHOTKEY;

      SystemKeysInfo(SPI_SETTOGGLEKEYS, toggle_keys_off);
    }

    FILTERKEYS filter_keys_off = startup_filter_keys_;
    if ((filter_keys_off.dwFlags & FKF_FILTERKEYSON) == 0) {
      // Disable the hotkey and the confirmation.
      filter_keys_off.dwFlags &= ~FKF_HOTKEYACTIVE;
      filter_keys_off.dwFlags &= ~FKF_CONFIRMHOTKEY;

      SystemKeysInfo(SPI_SETFILTERKEYS, filter_keys_off);
    }
  }

  is_toggled_ = toggle;
}
}  // namespace source::windows

#endif  // ENGINE_ACCESSIBILITY_SHORTCUT_KEYS_TOGGLER_H_
