/// @file Keyboard.h
/// @brief Tracks keyboard key state, fed by the window's message loop.

#pragma once

// Records which keys are currently held. The platform layer (WindowProc) calls
// onKeyDown/onKeyUp as WM_KEYDOWN/WM_KEYUP arrive; game code polls isKeyDown().
//
// Keys are identified by their Win32 virtual-key code (e.g. 'W', VK_SPACE).
// Because state is event-driven rather than polled from the OS, keys only
// register while the window has focus. Call reset() on focus loss to clear
// any keys left "stuck" down.
class Keyboard {
  public:
    void onKeyDown(int virtualKey);
    void onKeyUp(int virtualKey);
    void reset(); // clear all keys (e.g. on WM_KILLFOCUS)

    bool isKeyDown(int virtualKey) const;

  private:
    static constexpr int KEY_COUNT = 256; // Win32 virtual-key codes are 0..254
    bool m_Keys[KEY_COUNT] = {};
};
