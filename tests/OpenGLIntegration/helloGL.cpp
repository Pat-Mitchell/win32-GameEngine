/// @file helloGL.cpp
/// @brief A simple Win32 application that integrates OpenGL. 
///   This functions as a "Hello, OpenGL" program.

#include <windows.h>
#include <GL/gl.h>
#include "..\..\libs\GLFW\glfw3.h"
#include <iostream>

// Global variables for OpenGL
HGLRC hRc = NULL;
HDC hDc = NULL;

// Window proc function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_CREATE: {
      hDc = GetDC(hwnd);

      // Set pixel format for OpenGL
      PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,
        8,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
      };

      int pixelFormat = ChoosePixelFormat(hDc, &pfd);
      SetPixelFormat(hDc, pixelFormat, &pfd);

      // Create OpenGL rendering context
      hRc = wglCreateContext(hDc);
      wglMakeCurrent(hDc, hRc);

      return 0;
    }

    case WM_DESTROY:
      // Clean up OpenGL
      if(hRc) {
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(hRc);
        hRc = NULL;
      }
      if(hDc) {
        ReleaseDC(hwnd, hDc);
        hDc = NULL;
      }
      PostQuitMessage(0);
      return 0;

    case WM_PAINT: {
      // Existing paint code
      PAINTSTRUCT ps;
      BeginPaint(hwnd, &ps);

      wglMakeCurrent(hDc, hRc);

      // OpenGL rendering goes here
      glClearColor(0.39f, 0.58f, 0.93f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      // OpenGL drawing commands here

      SwapBuffers(hDc); // Present the frame buffer

      EndPaint(hwnd, &ps);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // Register window class
  const wchar_t* CLASS_NAME = L"Simple OpenGL Window";

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
    L"Hello, OpenGL!",
    WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    800, 600,
    NULL,
    NULL,
    hInstance,
    NULL
  );

  if(hwnd == NULL) {
    return -1;
  }

  // Show and update window
  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  // Message loop
  MSG msg = {};
  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  return 0;
}