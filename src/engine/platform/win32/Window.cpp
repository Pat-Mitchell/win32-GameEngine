/// @file Window.cpp
/// @brief Win32/WGL implementation of the Window platform layer. Lifts the
///   window-class + GL-context + message-pump boilerplate that the test mains
///   hand-rolled, and translates OS messages into IWindowEvents calls.

#include "Window.h"

#include <windowsx.h> // GET_X_LPARAM / GET_Y_LPARAM (signed mouse coords)

namespace {
  const wchar_t* kWindowClass = L"EngineWin32WindowClass";

  // ShowCursor maintains an internal display counter, not a boolean: the cursor
  // is visible while the count is >= 0. Drive it to exactly one step past the
  // boundary so repeated calls don't skew the count.
  void hideCursor() { while(ShowCursor(FALSE) >= 0) {} }
  void showCursor() { while(ShowCursor(TRUE) < 0) {} }
}

Window::~Window() {
  destroy();
}

bool Window::create(const Desc& desc, IWindowEvents* handler) {
  m_pHandler = handler;

  HINSTANCE hInst = GetModuleHandle(nullptr);

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(wc);
  wc.style = CS_OWNDC; // a private DC pairs with a persistent GL context
  wc.lpfnWndProc = staticWndProc;
  wc.hInstance = hInst;
  wc.lpszClassName = kWindowClass;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = nullptr; // GL owns the surface; it clears every frame
  RegisterClassEx(&wc); // harmless ERROR_CLASS_ALREADY_EXISTS on a repeat create

  m_Width = desc.width;
  m_Height = desc.height;

  // Pass 'this' as the creation param so WM_NCCREATE can bind the HWND to us.
  m_Hwnd = CreateWindowEx(
    0, kWindowClass, desc.title, WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, desc.width, desc.height,
    nullptr, nullptr, hInst, this);
  if(m_Hwnd == nullptr) return false;

  if(!createGLContext(desc)) {
    destroy();
    return false;
  }

  ShowWindow(m_Hwnd, SW_SHOW);
  UpdateWindow(m_Hwnd);

  // desc.width/height size the whole window (frame included); report the actual
  // client area, which is what the renderer's viewport and aspect care about.
  RECT rc;
  GetClientRect(m_Hwnd, &rc);
  m_Width = rc.right - rc.left;
  m_Height = rc.bottom - rc.top;
  return true;
}

bool Window::createGLContext(const Desc& desc) {
  m_hDC = GetDC(m_Hwnd);
  if(m_hDC == nullptr) return false;

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                (desc.doubleBuffer ? PFD_DOUBLEBUFFER : 0);
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = (BYTE)desc.depthBits;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(m_hDC, &pfd);
  if(pixelFormat == 0) return false;
  if(!SetPixelFormat(m_hDC, pixelFormat, &pfd)) return false;

  m_hRC = wglCreateContext(m_hDC);
  if(m_hRC == nullptr) return false;
  if(!wglMakeCurrent(m_hDC, m_hRC)) return false;
  return true;
}

void Window::destroy() {
  if(m_RelativeMouse) {
    ClipCursor(nullptr);
    showCursor();
    m_RelativeMouse = false;
  }
  wglMakeCurrent(nullptr, nullptr);
  if(m_hRC != nullptr) { wglDeleteContext(m_hRC); m_hRC = nullptr; }
  if(m_hDC != nullptr && m_Hwnd != nullptr) { ReleaseDC(m_Hwnd, m_hDC); m_hDC = nullptr; }
  if(m_Hwnd != nullptr) { DestroyWindow(m_Hwnd); m_Hwnd = nullptr; }
}

bool Window::pumpEvents() {
  MSG msg;
  while(PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
    if(msg.message == WM_QUIT) { m_ShouldClose = true; break; }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Relative mode: snap the cursor back to center so look rotation is unbounded.
  // Done here -- after this frame's moves have been dispatched and before the
  // next pump -- so the warp echo differences to zero against the re-anchored
  // center (see recenterCursor). Skipped while unfocused so we never yank the
  // cursor away from another app.
  if(m_RelativeMouse && m_HasFocus && !m_ShouldClose) {
    recenterCursor();
  }
  return !m_ShouldClose;
}

void Window::swapBuffers() {
  SwapBuffers(m_hDC);
}

void Window::setRelativeMouseMode(bool enabled) {
  if(enabled == m_RelativeMouse) return;
  m_RelativeMouse = enabled;
  m_HasLast = false; // on a mode flip, re-seed the anchor on the next move (no jump)
  if(enabled) {
    hideCursor(); // pumpEvents does the clip + recenter from here on
  } else {
    ClipCursor(nullptr);
    showCursor();
  }
}

void Window::recenterCursor() {
  RECT rc;
  GetClientRect(m_Hwnd, &rc);
  POINT tl{ rc.left, rc.top }, br{ rc.right, rc.bottom };
  ClientToScreen(m_Hwnd, &tl);
  ClientToScreen(m_Hwnd, &br);

  RECT clip{ tl.x, tl.y, br.x, br.y };
  ClipCursor(&clip);
  SetCursorPos((tl.x + br.x) / 2, (tl.y + br.y) / 2);

  // Anchor to the warp target in CLIENT coords. GetClientRect gives left=top=0,
  // so the client center is exactly the client coordinate WM_MOUSEMOVE reports
  // for that screen point (the integer halving matches on both sides). The warp
  // echo then differences to a zero delta, while real motion after it still
  // measures correctly -- and this survives WM_MOUSEMOVE coalescing because the
  // anchor is a known point, not "whatever the next move happens to be".
  m_LastX = (rc.right - rc.left) / 2;
  m_LastY = (rc.bottom - rc.top) / 2;
  m_HasLast = true;
}

void Window::onButtonDown(int button) {
  if(m_ButtonsDown == 0) SetCapture(m_Hwnd); // first button of a chord grabs capture
  ++m_ButtonsDown;
  if(m_pHandler != nullptr) m_pHandler->onMouseButton(button, true);
}

void Window::onButtonUp(int button) {
  if(m_ButtonsDown > 0 && --m_ButtonsDown == 0) ReleaseCapture(); // last button releases
  if(m_pHandler != nullptr) m_pHandler->onMouseButton(button, false);
}

LRESULT CALLBACK Window::staticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  Window* self = nullptr;
  if(msg == WM_NCCREATE) {
    // CreateWindowEx's last arg arrives here; bind the instance to the HWND.
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
    self = reinterpret_cast<Window*>(cs->lpCreateParams);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    self->m_Hwnd = hwnd;
  } else {
    self = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }
  if(self != nullptr) return self->handleMessage(msg, wParam, lParam);
  return DefWindowProc(hwnd, msg, wParam, lParam); // messages before WM_NCCREATE
}

LRESULT Window::handleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
  switch(msg) {
    case WM_KEYDOWN:
      // Bit 30 of lParam set == the key was already down (auto-repeat); deliver a
      // single press until it is released, per the interface contract.
      if(!(lParam & 0x40000000) && m_pHandler != nullptr) {
        m_pHandler->onKey((int)wParam, true);
      }
      return 0;
    case WM_KEYUP:
      if(m_pHandler != nullptr) m_pHandler->onKey((int)wParam, false);
      return 0;

    // Capture spans the whole chord so drag motion keeps arriving past the window
    // edge: capture on the first button down, release only when the last comes up
    // (relative mode also confines the cursor, but drag-look does not).
    case WM_LBUTTONDOWN: onButtonDown(0); return 0;
    case WM_LBUTTONUP:   onButtonUp(0);   return 0;
    case WM_RBUTTONDOWN: onButtonDown(1); return 0;
    case WM_RBUTTONUP:   onButtonUp(1);   return 0;
    case WM_MBUTTONDOWN: onButtonDown(2); return 0;
    case WM_MBUTTONUP:   onButtonUp(2);   return 0;

    case WM_MOUSEMOVE: {
      int x = GET_X_LPARAM(lParam);
      int y = GET_Y_LPARAM(lParam);
      if(m_pHandler != nullptr) m_pHandler->onMousePosition(x, y);
      if(m_HasLast) {
        float dx = (float)(x - m_LastX);
        float dy = (float)(y - m_LastY);
        if((dx != 0.0f || dy != 0.0f) && m_pHandler != nullptr) {
          m_pHandler->onMouseMotion(dx, dy);
        }
      }
      m_LastX = x;
      m_LastY = y;
      m_HasLast = true;
      return 0;
    }

    case WM_SIZE: {
      int w = LOWORD(lParam);
      int h = HIWORD(lParam);
      m_Width = w;
      m_Height = h;
      if(w > 0 && h > 0 && m_pHandler != nullptr) m_pHandler->onResize(w, h);
      return 0;
    }

    case WM_SETFOCUS:
      m_HasFocus = true;
      if(m_RelativeMouse) hideCursor(); // pumpEvents re-clips + recenters
      if(m_pHandler != nullptr) m_pHandler->onFocus(true);
      return 0;
    case WM_KILLFOCUS:
      // Hand the cursor back so Alt-Tab and other apps work; keep the relative
      // intent so focus regain restores it. Re-seed the anchor on return. A held
      // button's up-event goes to the newly focused window, so drop capture and
      // our chord count here to avoid a stuck button.
      m_HasFocus = false;
      m_HasLast = false;
      m_ButtonsDown = 0;
      ReleaseCapture();
      if(m_RelativeMouse) {
        ClipCursor(nullptr);
        showCursor();
      }
      if(m_pHandler != nullptr) m_pHandler->onFocus(false);
      return 0;

    case WM_CLOSE:
      if(m_pHandler != nullptr) m_pHandler->onClose();
      DestroyWindow(m_Hwnd);
      return 0;
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_PAINT: {
      PAINTSTRUCT ps;
      BeginPaint(m_Hwnd, &ps);
      EndPaint(m_Hwnd, &ps);
      return 0;
    }
  }
  return DefWindowProc(m_Hwnd, msg, wParam, lParam);
}
