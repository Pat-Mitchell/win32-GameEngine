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

    // Anchor the "last known position" to a specific point, in the same client
    // coordinate space onMove() receives. Use after a recenter warp: set this to
    // the warp target (window center) so the recenter itself yields zero delta
    // while real motion after it is still measured. Anchoring to a known point
    // (rather than just dropping the origin) survives WM_MOUSEMOVE coalescing --
    // Windows keeps only the latest move in the queue, so the surviving move may
    // be the user's real motion rather than the warp echo; measured against the
    // center anchor, either one yields the correct delta.
    void setLastPosition(int x, int y);

  private:
    int m_X = 0;
    int m_Y = 0;
    float m_DeltaX = 0.0f;
    float m_DeltaY = 0.0f;
    bool m_Buttons[BUTTON_COUNT] = {};
    bool m_Locked = false;  // window layer enforces; this is just the intent
    bool m_HasLast = false; // false until the first move, to avoid a huge delta
};
