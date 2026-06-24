/// @file main.cpp
/// @brief main entry point 
///   Implementation of the ECS and graphics systems into a win32 app

#include <windows.h>
#include "../../src/engine/core/ecs/World.h"
#include "../../src/engine/core/graphics/Renderer.h"

// Global instances
World* g_pWorld = nullptr;
Renderer* g_pRenderer = nullptr;

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

  // Initialize engine components
  g_pWorld = new World();
  g_pRenderer = new Renderer();

  if(!g_pRenderer->initialize()) {
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

  return 0;
}