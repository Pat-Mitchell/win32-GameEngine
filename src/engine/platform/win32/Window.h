/// @file Window.h
/// @brief Win32/WGL window + OpenGL context and message pump.
///   The app implements IWindowEvents. Window translates
///   OS messages into those calls.

#pragma once

#include <windows.h>

// Event sink implemented by the application. The Win32 Window translates OS
// messages (WM_KEYDOWN, WM_MOUSEMOVE, WM_INPUT, WM_SIZE, ...) into these calls.
// All handlers have empty defaults so an app overrides only what it needs.
class IWindowEvents {
  public:
    virtual ~IWindowEvents() = default;

    // Keyboard: Win32 virtual-key code + pressed/released. Auto-repeat is
    // filtered out by Window, so a held key yields one onKey(down) until release.
    virtual void onKey(int vkey, bool down) {}

    // Mouse button index (0 = left, 1 = right, 2 = middle) + pressed/released.
    virtual void onMouseButton(int button, bool down) {}

    // Relative pointer motion in pixels since the last event. This is the channel
    // that becomes raw-input-backed. look-code accumulates it frame to frame.
    virtual void onMouseMotion(float dx, float dy) {}

    // Absolute pointer position in client pixels, for UI / picking. Not
    // meaningful while in relative-mouse mode; the fly-cam ignores it.
    virtual void onMousePosition(int x, int y) {}

    // Client-area size after a resize, in pixels.
    virtual void onResize(int width, int height) {}

    // The window was asked to close (WM_CLOSE / WM_DESTROY).
    virtual void onClose() {}
};

// Owns the native window, the WGL OpenGL context, and the message pump. One-way
// dependencies: Window depends only on IWindowEvents (declared above), never on
// core/. The app supplies an IWindowEvents adapter that forwards into the
// platform-free Keyboard/Mouse in core/input.
class Window {
  public:
    struct Desc {
      const wchar_t* title = L"Engine";
      int width = 1024;
      int height = 768;
      int depthBits = 24;
      bool doubleBuffer = true;
    };

    Window() = default;
    ~Window();

    // Create the window + GL context and make the context current. Returns false
    // on any failure (class registration, pixel format, wglCreateContext, ...).
    bool create(const Desc& desc, IWindowEvents* handler);
    void destroy();

    // Drain all pending OS messages, dispatching each to the handler. Returns
    // false once the window has been asked to close (WM_QUIT observed), so the
    // frame loop is: while(window.pumpEvents()) { ...draw...; window.swapBuffers(); }
    bool pumpEvents();

    void swapBuffers();

    // "Cursor lock" as a mode, not a per-frame ritual. TODAY: hide the cursor,
    // ClipCursor to the client rect, and recenter + re-anchor each pump so motion
    // is unbounded. 
    // LATER WITH RAW INPUT: RegisterRawInputDevices + RIDEV_NOLEGACY and
    // hide the cursor. The recenter/ClipCursor/anchor work will be removed. 
    // Either way the app just toggles this. The mechanism is Window's private business.
    void setRelativeMouseMode(bool enabled);
    bool isRelativeMouseMode() const { return m_RelativeMouse; }

    HWND nativeHandle() const { return m_Hwnd; } // for the GL-context bootstrap
    int width() const { return m_Width; }
    int height() const { return m_Height; }

  private:
    IWindowEvents* m_pHandler = nullptr;
    HWND m_Hwnd = nullptr;
    HDC m_hDC = nullptr;
    HGLRC m_hRC = nullptr;
    int m_Width = 0;
    int m_Height = 0;
    bool m_RelativeMouse = false;
    bool m_ShouldClose = false;
};
