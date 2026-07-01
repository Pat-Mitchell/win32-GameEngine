/// @file main.cpp
/// @brief ECS render-bridge test.
///   Geometry is drawn from entities (Transform + MeshRenderer) by a RenderSystem;
///   a camera entity (Transform + CameraComponent) is driven by an InputSystem that
///   reads the global Keyboard/Mouse. The frame loop is just: clear -> world.update
///   -> SwapBuffers. Contrast with tests/Win32Implementation, which wires the same
///   pipeline by hand with no ECS.

#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM / GET_Y_LPARAM (signed mouse coords)
#include <iostream>
#include <cstdio>

#include "../../src/engine/core/ecs/World.h"
#include "../../src/engine/core/ecs/MeshRenderer.h"
#include "../../src/engine/core/ecs/CameraComponent.h"
#include "../../src/engine/core/ecs/RenderSystem.h"
#include "../../src/engine/core/ecs/InputSystem.h"
#include "../../src/engine/core/graphics/Renderer.h"
#include "../../src/engine/core/loaders/ObjLoader.h"
#include "../../src/engine/core/input/Keyboard.h"
#include "../../src/engine/core/input/Mouse.h"
#include "../../src/engine/core/math/transform.h"

// Engine-global singletons.
World* g_pWorld = nullptr;
Renderer* g_pRenderer = nullptr;
Mesh* g_pCubeMesh = nullptr; // owned here; referenced by MeshRenderer components
Shader* g_pCubeShader = nullptr; // owned here; referenced by MeshRenderer components
EntityID g_CameraEntity = INVALID_ENTITY;

// Input devices: WindowProc feeds them events; the InputSystem polls them.
Keyboard g_keyboard;
Mouse g_mouse;

// OpenGL context state.
HDC g_hDC = nullptr;
HGLRC g_hRC = nullptr;

// Create a double-buffered, depth-enabled OpenGL context and make it current.
// Must run before Renderer::initialize(), since glewInit() needs a live context.
bool createGLContext(HWND hwnd) {
  g_hDC = GetDC(hwnd);
  if(g_hDC == nullptr) return false;

  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 24;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  int pixelFormat = ChoosePixelFormat(g_hDC, &pfd);
  if(pixelFormat == 0) return false;
  if(!SetPixelFormat(g_hDC, pixelFormat, &pfd)) return false;

  g_hRC = wglCreateContext(g_hDC);
  if(g_hRC == nullptr) return false;
  if(!wglMakeCurrent(g_hDC, g_hRC)) return false;

  return true;
}

void destroyGLContext(HWND hwnd) {
  wglMakeCurrent(nullptr, nullptr);
  if(g_hRC != nullptr) { wglDeleteContext(g_hRC); g_hRC = nullptr; }
  if(g_hDC != nullptr) { ReleaseDC(hwnd, g_hDC); g_hDC = nullptr; }
}

// Compile the cube's shader from inline GLSL.
// Vertex shader applies a combined MVP
// Fragment shader does a cheap directional shade so faces read distinctly.
Shader* createCubeShader() {
  const std::string vertexSource = R"glsl(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    layout(location = 1) in vec3 aNormal;
    layout(location = 2) in vec2 aTexCoord;
    uniform mat4 uMVP;
    out vec3 vNormal;
    void main() {
      gl_Position = uMVP * vec4(aPos, 1.0);
      vNormal = aNormal;
    }
  )glsl";

  const std::string fragmentSource = R"glsl(
    #version 330 core
    in vec3 vNormal;
    out vec4 FragColor;
    void main() {
      vec3 n = normalize(vNormal);
      vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
      float diff = max(dot(n, lightDir), 0.0);
      vec3 base = vec3(0.6, 0.7, 0.9);
      FragColor = vec4(base * (0.3 + 0.7 * diff), 1.0);
    }
  )glsl";

  Shader* shader = new Shader();
  if(!shader->loadFromSource(vertexSource, fragmentSource)) {
    delete shader;
    return nullptr;
  }
  return shader;
}

// Enforce the Mouse's cursor-lock intent with the OS calls that need a cursor.
// The ClipCursor confinement and per-frame recentre live in the main loop (so
// they survive window move/resize). Flip cursor visibility.
static void applyCursorLock(bool locked) {
  if(locked) {
    while(ShowCursor(FALSE) >= 0) {} // hide (decrements an internal show-counter)
  } else {
    ClipCursor(nullptr);             // release confinement
    while(ShowCursor(TRUE) < 0) {}   // restore the cursor
  }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch(uMsg) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;
    case WM_KEYDOWN:
      g_keyboard.onKeyDown((int)wParam);
      if(wParam == VK_ESCAPE) PostQuitMessage(0);
      // Tab toggles mouse-look. Ignore auto-repeat (bit 30 = key was already down)
      // so holding Tab doesn't flicker the lock on and off.
      if(wParam == VK_TAB && !(lParam & 0x40000000)) {
        g_mouse.toggleLock();
        applyCursorLock(g_mouse.isLocked());
      }
      return 0;
    case WM_KEYUP:
      g_keyboard.onKeyUp((int)wParam);
      return 0;
    case WM_KILLFOCUS:
      g_keyboard.reset(); // clear held keys so they don't stay stuck
      if(g_mouse.isLocked()) { // release the cursor so Alt-Tab etc. work
        g_mouse.setLocked(false);
        applyCursorLock(false);
      }
      return 0;
    case WM_RBUTTONDOWN:
      g_mouse.onButtonDown(Mouse::Right);
      SetCapture(hwnd);
      return 0;
    case WM_RBUTTONUP:
      g_mouse.onButtonUp(Mouse::Right);
      ReleaseCapture();
      return 0;
    case WM_LBUTTONDOWN:
      g_mouse.onButtonDown(Mouse::Left);
      return 0;
    case WM_LBUTTONUP:
      g_mouse.onButtonUp(Mouse::Left);
      return 0;
    case WM_MOUSEMOVE:
      g_mouse.onMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
      return 0;
    case WM_SIZE: {
      if(g_pRenderer != nullptr) {
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);
        if(width > 0 && height > 0) {
          g_pRenderer->setViewport(0, 0, width, height);
          // Keep the camera entity's projection aspect in sync.
          if(g_pWorld != nullptr && g_pWorld->isValid(g_CameraEntity) &&
             g_pWorld->hasComponent<CameraComponent>(g_CameraEntity)) {
            g_pWorld->getComponent<CameraComponent>(g_CameraEntity).aspect =
              (float)width / (float)height;
          }
        }
      }
      return 0;
    }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      BeginPaint(hwnd, &ps);
      EndPaint(hwnd, &ps);
      return 0;
    }
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Spawn a cube entity at a world position: Transform + a MeshRenderer pointing at
// the shared mesh/shader. Returns the new entity id.
EntityID spawnCube(World& world, const Vec3& position) {
  EntityID e = world.createEntity();
  Transform t;
  t.position = position;
  world.addComponent<Transform>(e, t);

  MeshRenderer mr;
  mr.mesh = g_pCubeMesh;
  mr.shader = g_pCubeShader;
  world.addComponent<MeshRenderer>(e, mr);
  return e;
}

void launchConsole() {
  // 1. Allocate a new console window for the calling process
  if(AllocConsole()) {
    FILE* fp;
        
    // 2. Redirect standard output (stdout) to the console
    freopen_s(&fp, "CONOUT$", "w", stdout);
    // 3. Redirect standard input (stdin) to the console
    freopen_s(&fp, "CONIN$", "r", stdin);
    // 4. Redirect standard error (stderr) to the console
    freopen_s(&fp, "CONOUT$", "w", stderr);

    // 5. Sync the C++ standard streams (cout, cin, cerr)
    std::ios::sync_with_stdio();
       
    // Optional: Give your console window a custom title
    SetConsoleTitle(L"Debug Console");
  }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  const wchar_t* CLASS_NAME = L"ECS Render Test Window";

  launchConsole();

  WNDCLASSEX wc = {};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = hInstance;
  wc.lpszClassName = CLASS_NAME;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = NULL; // OpenGL owns the surface; clears it every frame
  RegisterClassEx(&wc);

  HWND hwnd = CreateWindowEx(
    0, CLASS_NAME, L"ECS Render Bridge", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
    NULL, NULL, hInstance, NULL);
  if(hwnd == NULL) return -1;

  ShowWindow(hwnd, nCmdShow);
  UpdateWindow(hwnd);

  if(!createGLContext(hwnd)) {
    MessageBox(hwnd, L"Failed to create OpenGL context", L"Error", MB_OK | MB_ICONERROR);
    return -1;
  }

  g_pRenderer = new Renderer();
  if(!g_pRenderer->initialize()) {
    MessageBox(hwnd, L"Renderer initialization failed", L"Error", MB_OK | MB_ICONERROR);
    destroyGLContext(hwnd);
    return -1;
  }

  RECT clientRect;
  GetClientRect(hwnd, &clientRect);
  int clientW = clientRect.right - clientRect.left;
  int clientH = clientRect.bottom - clientRect.top;
  g_pRenderer->setViewport(0, 0, clientW, clientH);

  // Load the shared cube mesh from disk.
  g_pCubeMesh = new Mesh();
  if(!loadOBJ("cube.obj", *g_pCubeMesh)) {
    MessageBox(hwnd, L"Failed to load cube.obj", L"Error", MB_OK | MB_ICONERROR);
    delete g_pCubeMesh; delete g_pRenderer;
    destroyGLContext(hwnd);
    return -1;
  }
  g_pCubeMesh->initialize();
  g_pCubeMesh->uploadData();

  // Compile the shared cube shader.
  g_pCubeShader = createCubeShader();
  if(g_pCubeShader == nullptr) {
    MessageBox(hwnd, L"Cube shader failed to compile/link", L"Error", MB_OK | MB_ICONERROR);
    delete g_pCubeMesh; delete g_pRenderer;
    destroyGLContext(hwnd);
    return -1;
  }

  // Build the ECS scene
  g_pWorld = new World();

  // Camera entity: a Transform (placement) + a CameraComponent (lens).
  g_CameraEntity = g_pWorld->createEntity();
  Transform camTransform;
  camTransform.position = Vec3(0.0f, 0.0f, 6.0f); // pulled back to see the cubes
  g_pWorld->addComponent<Transform>(g_CameraEntity, camTransform);
  CameraComponent camLens;
  camLens.aspect = (float)clientW / (float)clientH;
  g_pWorld->addComponent<CameraComponent>(g_CameraEntity, camLens);

  // A few cube entities at different positions, all sharing one mesh + shader.
  // Drawing several from one mesh is the point of the render bridge.
  spawnCube(*g_pWorld, Vec3( 0.0f, 0.0f, 0.0f));
  spawnCube(*g_pWorld, Vec3( 2.0f, 0.5f, 0.0f));
  spawnCube(*g_pWorld, Vec3(-2.0f, 0.0f, 0.0f));
  spawnCube(*g_pWorld, Vec3( 0.0f, 2.0f, 0.0f));
  spawnCube(*g_pWorld, Vec3( 0.0f,-2.0f, 0.0f));

  // Systems
  // InputSystem is registered first so the camera updates before the
  // RenderSystem reads it within the same World::update().
  InputSystem inputSystem(*g_pWorld, g_keyboard, g_mouse, g_CameraEntity);
  RenderSystem renderSystem(*g_pWorld, *g_pRenderer, g_CameraEntity);
  g_pWorld->registerSystem(&inputSystem);
  g_pWorld->registerSystem(&renderSystem);

  // Main loop: drain messages, then clear -> update world -> present
  MSG msg = {};
  bool running = true;
  while(running) {
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) { running = false; break; }
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    static ULONGLONG lastTick = GetTickCount64();
    ULONGLONG nowTick = GetTickCount64();
    float dt = (float)(nowTick - lastTick) / 1000.0f;
    lastTick = nowTick;

            if(g_mouse.getDeltaX() != 0 || g_mouse.getDeltaY() != 0) 
      std::cout << g_mouse.getDeltaX() << ", " << g_mouse.getDeltaY() << std::endl;

    g_pRenderer->clear(Vec3(0.1f, 0.1f, 0.15f));   
    g_pWorld->update(dt); // InputSystem moves the camera, RenderSystem draws entities

    // Cursor lock: confine to the window and snap back to center each frame so
    // look rotation is unbounded. update() already consumed this frame's delta.
    // Recomputed every frame so it survives window move/resize.
    if(g_mouse.isLocked()) {
      RECT rc;
      GetClientRect(hwnd, &rc);
      POINT tl{ rc.left, rc.top }, br{ rc.right, rc.bottom };
      ClientToScreen(hwnd, &tl);
      ClientToScreen(hwnd, &br);
      RECT clip{ tl.x, tl.y, br.x, br.y };
      ClipCursor(&clip);
      SetCursorPos((tl.x + br.x) / 2, (tl.y + br.y) / 2);
      // Anchor the mouse's last position to the warp target (in client coords), so
      // the recenter yields zero delta while real motion after it is still measured.
      // Anchoring to the known center (vs. just dropping the origin) survives
      // WM_MOUSEMOVE coalescing.
      g_mouse.setLastPosition((rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2);
    }

    SwapBuffers(g_hDC);
  }

  // Release the cursor in case we quit while locked (otherwise it stays hidden).
  applyCursorLock(false);

  // Cleanup. Delete GPU objects before the context goes away (their destructors
  // call glDelete*). The systems are stack objects; they outlive this block.
  delete g_pWorld;
  delete g_pCubeShader;
  delete g_pCubeMesh;
  delete g_pRenderer;
  destroyGLContext(hwnd);

  return 0;
}
