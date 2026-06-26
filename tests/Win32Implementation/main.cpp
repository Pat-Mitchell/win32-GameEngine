/// @file main.cpp
/// @brief main entry point 
///   Implementation of the ECS and graphics systems into a win32 app

#include <windows.h>
#include "../../src/engine/core/ecs/World.h"
#include "../../src/engine/core/graphics/Renderer.h"

// Global instances
World* g_pWorld = nullptr;
Renderer* g_pRenderer = nullptr;

// OpenGL context state
HDC g_hDC = nullptr;     // device context for the window
HGLRC g_hRC = nullptr;   // OpenGL rendering context

// Create a double-buffered, depth-enabled OpenGL context and make it current.
// Must run before Renderer::initialize(), since glewInit() needs a live context.
bool createGLContext(HWND hwnd) {
  g_hDC = GetDC(hwnd);
  if(g_hDC == nullptr) {
    return false;
  }

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24; // depth buffer so the cube's faces sort correctly
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
  if(pixelFormat == 0) {
    return false;
  }
  if(!SetPixelFormat(g_hDC, pixelFormat, &pfd)) {
    return false;
  }

  g_hRC = wglCreateContext(g_hDC);
  if(g_hRC == nullptr) {
    return false;
  }
  if(!wglMakeCurrent(g_hDC, g_hRC)) {
    return false;
  }

  return true;
}

// Tear down the OpenGL context. Safe to call even if creation failed partway.
void destroyGLContext(HWND hwnd) {
  wglMakeCurrent(nullptr, nullptr);
  if(g_hRC != nullptr) {
    wglDeleteContext(g_hRC);
    g_hRC = nullptr;
  }
  if(g_hDC != nullptr) {
    ReleaseDC(hwnd, g_hDC);
    g_hDC = nullptr;
  }
}

// Window proc function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
      return 0;
    }
  } 
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // Register window class
  const wchar_t* CLASS_NAME = L"Game Engine Window";

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

  RegisterClassEx(&wc);

  // Create Window
  HWND hwnd = CreateWindowEx(
    0,
    CLASS_NAME,
    L"3D Game Engine",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    1024, 768,
    NULL,
    NULL,
    hInstance,
    NULL
  );

  if(hwnd == NULL) {
    return -1;
  }

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // Bring up the OpenGL context before anything touches GL.
  if(!createGLContext(hwnd)) {
    MessageBox(hwnd, L"Failed to create OpenGL context", L"Error", MB_OK | MB_ICONERROR);
    return -1;
  }

  // Initialize engine components
  g_pWorld = new World();
  g_pRenderer = new Renderer();

  if(!g_pRenderer->initialize()) {
    MessageBox(hwnd, L"Renderer initialization failed", L"Error", MB_OK | MB_ICONERROR);
    destroyGLContext(hwnd);
    return -1;
  }

  // Main message loop
  MSG msg = {};
  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // Cleanup
  delete g_pWorld;
  delete g_pRenderer;
  destroyGLContext(hwnd);

  return 0;
}