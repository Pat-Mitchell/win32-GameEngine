/// @file Mouse.h
/// @brief Tracks mouse button state and relative motion, fed by the message loop.

#pragma once

// Records mouse button state and accumulates relative motion between frames.
// The platform layer (WindowProc) calls onMove/onButtonDown/onButtonUp as the
// WM_MOUSEMOVE / WM_*BUTTON* messages arrive; game code reads the delta once per
// frame and then calls consumeDelta() to reset it.
//
// Motion is tracked on every move (not just while a button is held), so the
// delta stays small frame-to-frame and there is no jump when a button is first
// pressed. OS-level concerns like SetCapture() stay in the window layer.
class Mouse {
  public:
    enum Button { Left = 0, Right = 1, Middle = 2, BUTTON_COUNT = 3 };

    void onMove(int x, int y);
    void onButtonDown(Button button);
    void onButtonUp(Button button);

    bool isButtonDown(Button button) const;

    // Relative motion accumulated since the last consumeDelta(), in pixels.
    float getDeltaX() const { return m_DeltaX; }
    float getDeltaY() const { return m_DeltaY; }
    void consumeDelta(); // zero the accumulated delta; call once per frame

    int getX() const { return m_X; }
    int getY() const { return m_Y; }

    // Cursor-lock intent for mouse-look. Mouse only records the desired state.
    // the window layer reads isLocked() and performs the OS-level work
    // (ClipCursor/ShowCursor/recenter), the same split already used for
    // SetCapture(). Toggling clears motion tracking so neither the toggle nor a
    // recenter warp surfaces a spurious jump on the next frame.
    void setLocked(bool locked);
    void toggleLock();
    bool isLocked() const { return m_Locked; }

    // Treat the next onMove() as a teleport (e.g. a recenter warp): re-seed the
    // origin from it instead of producing motion, so the warp adds no false delta.
    void ignoreNextMove() { m_HasLast = false; }

  private:
    int m_X = 0;
    int m_Y = 0;
    float m_DeltaX = 0.0f;
    float m_DeltaY = 0.0f;
    bool m_Buttons[BUTTON_COUNT] = {};
    bool m_HasLast = false; // false until the first move, to avoid a huge delta
    bool m_Locked = false;  // window layer enforces; this is just the intent
};
