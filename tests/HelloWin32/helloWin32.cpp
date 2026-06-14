/// @file helloWin32.cpp
/// @brief Simple Win32 application that creates a window and displays "Hello, World!"
///   Coded purely to verify directories are properly set up for Win32

#include <windows.h>
#include <string>

// Window proc function
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hwnd, &ps);

      // background fill
      FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

      // Draw "Hello, World!"
      std::wstring text = L"Hello, World!";
      TextOutW(hdc, 10, 10, text.c_str(), text.length());

      EndPaint(hwnd, &ps);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  // Register window class
  const wchar_t* CLASS_NAME = L"Simple Window Class";

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
    L"Hello, World!",
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