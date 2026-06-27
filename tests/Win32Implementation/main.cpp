/// @file main.cpp
/// @brief main entry point 
///   Implementation of the ECS and graphics systems into a win32 app

#include <windows.h>
#include "../../src/engine/core/ecs/World.h"
#include "../../src/engine/core/graphics/Renderer.h"

// Global instances
World* g_pWorld = nullptr;
Renderer* g_pRenderer = nullptr;
Mesh* g_pCubeMesh = nullptr; // Test cube mesh

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
    case WM_SIZE: {
      // Keep the GL viewport (and camera aspect) in sync with the client area.
      if(g_pRenderer != nullptr) {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if(width > 0 && height > 0) {
          g_pRenderer->resize(width, height);
        }
      }
      return 0;
    }
    case WM_PAINT: {
      // GL drives all drawing from the render loop; just validate the region
      // so Windows stops sending WM_PAINT.
      PAINTSTRUCT ps;
      BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
      return 0;
    }
  } 
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Build unit cube centered on the origin with flat shading and upload it to the GPU.
// 24 vertices: 4 per face so each face carries its own flat normal and full 0..1 UVs.
// Each face is wound counter-clockwise as seen from outside, matching the renderer's
// GL_CCW front-face + back-face culling so interior faces are discarded.
// Must be called after the GL context exists (initialize()/uploadData() issue GL calls).
Mesh* createCubeMesh() {
  Mesh* mesh = new Mesh();

  // Per-face data: 4 corner positions (CCW from outside) + the shared face normal.
  // UVs are assigned per-corner below as (0,0)(1,0)(1,1)(0,1).
  struct Face { Vec3 normal; Vec3 corners[4]; };
  const Face faces[6] = {
    // Front (+Z)
    { 
      Vec3( 0,  0,  1), // Normal
      { 
        Vec3(-0.5f,-0.5f, 0.5f), 
        Vec3( 0.5f,-0.5f, 0.5f), 
        Vec3( 0.5f, 0.5f, 0.5f), 
        Vec3(-0.5f, 0.5f, 0.5f)
      } 
    },
    // Back (-Z)
    { 
      Vec3( 0,  0, -1), // Normal
      { 
        Vec3( 0.5f,-0.5f,-0.5f), 
        Vec3(-0.5f,-0.5f,-0.5f), 
        Vec3(-0.5f, 0.5f,-0.5f),
        Vec3( 0.5f, 0.5f,-0.5f) 
      } 
    },
    // Left (-X)
    { 
      Vec3(-1,  0,  0), // Normal
      { 
        Vec3(-0.5f,-0.5f,-0.5f), 
        Vec3(-0.5f,-0.5f, 0.5f), 
        Vec3(-0.5f, 0.5f, 0.5f), 
        Vec3(-0.5f, 0.5f,-0.5f) 
      } 
    },
    // Right (+X)
    { 
      Vec3( 1,  0,  0), // Normal
      { 
        Vec3( 0.5f,-0.5f, 0.5f), 
        Vec3( 0.5f,-0.5f,-0.5f), 
        Vec3( 0.5f, 0.5f,-0.5f), 
        Vec3( 0.5f, 0.5f, 0.5f) 
      } 
    },
    // Top (+Y)
    { 
      Vec3( 0,  1,  0), // Normal
      { Vec3(-0.5f, 0.5f, 0.5f), 
        Vec3( 0.5f, 0.5f, 0.5f), 
        Vec3( 0.5f, 0.5f,-0.5f), 
        Vec3(-0.5f, 0.5f,-0.5f) 
      } 
    },
    // Bottom (-Y)
    { 
      Vec3( 0, -1,  0), // Normal
      { Vec3(-0.5f,-0.5f,-0.5f), 
        Vec3( 0.5f,-0.5f,-0.5f), 
        Vec3( 0.5f,-0.5f, 0.5f), 
        Vec3(-0.5f,-0.5f, 0.5f) 
      } 
    },
  };
  const Vec2 uvs[4] = { Vec2(0, 0), Vec2(1, 0), Vec2(1, 1), Vec2(0, 1) };

  for(int f = 0; f < 6; f++) {
    GLuint base = static_cast<GLuint>(f * 4); // first vertex index of this face
    for(int c = 0; c < 4; c++) {
      Vertex v;
      v.position = faces[f].corners[c];
      v.normal = faces[f].normal;
      v.texCoord = uvs[c];
      mesh->addVertex(v);
    }

    // Two CCW triangles for the quad: (0,1,2) and (0,2,3), offset by this face's base.
    mesh->addIndex(base + 0); 
    mesh->addIndex(base + 1); 
    mesh->addIndex(base + 2);

    mesh->addIndex(base + 0); 
    mesh->addIndex(base + 2); 
    mesh->addIndex(base + 3);
  }

  mesh->initialize();  // create VAO/VBO/EBO
  mesh->uploadData();  // push vertices/indices + set attribute pointers
  return mesh;
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
  // wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1) // PAINTs the window default white. Blocking out so GL has control over PAINT instead of GDI
  // No GDI background brush: OpenGL owns the surface and clears it every frame.
  // Leaving this as COLOR_WINDOW would let GDI erase the client area to white,
  // causing a flash on resize before GL paints over it.
  wc.hbrBackground = NULL;

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

  // Set the initial viewport to match the window's client area.
  RECT clientRect;
  GetClientRect(hwnd, &clientRect);
  g_pRenderer->resize(clientRect.right - clientRect.left,
                      clientRect.bottom - clientRect.top);

  // Build the cube geometry on the GPU. Not drawn yet: drawing needs a shader
  // and MVP uniforms. This just proves the upload path works.
  g_pCubeMesh = createCubeMesh();

  // Main loop: drain pending messages, then render one frame. Unlike GetMessage
  // (which blocks until a message arrives), PeekMessage returns immediately so we
  // keep redrawing every iteration.
  MSG msg = {};
  bool running = true;
  while(running) {
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        running = false;
        break;
      }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    // Render one frame: clear to the background color, then present.
    // The cube draw will slot in between these two calls in the next step.
    g_pRenderer->clear(Vec3(0.1f, 0.1f, 0.15f));
    SwapBuffers(g_hDC);
  }

  // Cleanup. Delete the mesh before the context goes away, since its destructor
  // calls glDeleteVertexArrays/Buffers and needs the context still current.
  delete g_pCubeMesh;
  delete g_pWorld;
  delete g_pRenderer;
  destroyGLContext(hwnd);

  return 0;
}