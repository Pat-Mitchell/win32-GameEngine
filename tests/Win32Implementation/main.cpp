/// @file main.cpp
/// @brief main entry point 
///   Implementation of the ECS and graphics systems into a win32 app

#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM / GET_Y_LPARAM (signed mouse coords)
#include "../../src/engine/core/ecs/World.h"
#include "../../src/engine/core/graphics/Renderer.h"
#include "../../src/engine/core/loaders/ObjLoader.h"
#include "../../src/engine/core/input/Keyboard.h"
#include "../../src/engine/core/input/Mouse.h"

// Global instances
World* g_pWorld = nullptr;
Renderer* g_pRenderer = nullptr;
Mesh* g_pCubeMesh = nullptr;     // Test cube mesh
Shader* g_pCubeShader = nullptr; // Shader for the test cube

// Input devices: WindowProc feeds them events; the render loop polls them.
Keyboard g_keyboard;
Mouse g_mouse;

// Fly-camera state driven by the input devices. g_cameraPos mirrors the Camera's
// position. g_yaw/g_pitch accumulate from mouse-look.
// Pitch=yaw=0 looks down -Z, matching the Camera's defaults.
Vec3 g_cameraPos(0.0f, 0.0f, 3.0f);
float g_yaw = 0.0f;
float g_pitch = 0.0f;

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
    case WM_KEYDOWN:
      g_keyboard.onKeyDown((int)wParam);
      if(wParam == VK_ESCAPE) {
        PostQuitMessage(0);
      }
      return 0;
    case WM_KEYUP:
      g_keyboard.onKeyUp((int)wParam);
      return 0;
    case WM_KILLFOCUS:
      // Lost focus: clear held keys so they don't stay "stuck" down.
      g_keyboard.reset();
      return 0;
    case WM_RBUTTONDOWN:
      g_mouse.onButtonDown(Mouse::Right);
      SetCapture(hwnd); // keep receiving moves if the cursor leaves the window
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
      { 
        Vec3(-0.5f, 0.5f, 0.5f), 
        Vec3( 0.5f, 0.5f, 0.5f), 
        Vec3( 0.5f, 0.5f,-0.5f), 
        Vec3(-0.5f, 0.5f,-0.5f) 
      } 
    },
    // Bottom (-Y)
    { 
      Vec3( 0, -1,  0), // Normal
      { 
        Vec3(-0.5f,-0.5f,-0.5f), 
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

// Compile and link the cube's shader from inline GLSL source. Returns nullptr on
// failure. Must be called after the GL context exists (compilation issues GL calls).
//
// Vertex shader: transforms each vertex by a single combined MVP matrix (uMVP) and
// passes the normal through. The attribute layout locations (0/1/2) match the order
// Mesh::uploadData() sets up (position/normal/texCoord).
//
// Fragment shader: a cheap directional shade so adjacent faces differ in brightness,
// otherwise a solid-color cube reads as a flat silhouette once it's on screen.
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
      // Ambient term keeps unlit faces visible; diffuse separates the faces.
      vec3 color = base * (0.3 + 0.7 * diff);
      FragColor = vec4(color, 1.0);
    }
  )glsl";

  Shader* shader = new Shader();
  if(!shader->loadFromSource(vertexSource, fragmentSource)) {
    delete shader;
    return nullptr;
  }
  return shader;
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

  // Load the test model from disk. Fall back to the procedural cube if the file
  // is missing so the app still runs.
  g_pCubeMesh = new Mesh();
  if(loadOBJ("cube.obj", *g_pCubeMesh)) {
    g_pCubeMesh->initialize();  // create VAO/VBO/EBO
    g_pCubeMesh->uploadData();  // push vertices/indices + set attribute pointers
  } else {
    delete g_pCubeMesh;
    g_pCubeMesh = createCubeMesh();
  }

  // Compile the cube's shader. Not used in the draw yet (Step 5 feeds it the MVP);
  // this proves the inline GLSL compiles and links against the live context.
  g_pCubeShader = createCubeShader();
  if(g_pCubeShader == nullptr) {
    MessageBox(hwnd, L"Cube shader failed to compile/link", L"Error", MB_OK | MB_ICONERROR);
    delete g_pCubeMesh;
    destroyGLContext(hwnd);
    return -1;
  }

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

    // Per-frame timing
    // Movement is frame-rate independent
    static ULONGLONG s_lastTick = GetTickCount64();
    ULONGLONG nowTick = GetTickCount64();
    float dt = (float)(nowTick - s_lastTick) / 1000.0f;
    s_lastTick = nowTick;

    Camera& camera = g_pRenderer->getCamera();

    // Mouse-look: 
    // turn the camera while the right button is held
    if(g_mouse.isButtonDown(Mouse::Right)) {
      const float sensitivity = 0.15f;
      g_yaw += g_mouse.getDeltaX() * sensitivity;
      g_pitch -= g_mouse.getDeltaY() * sensitivity; // screen-down -> look down
      if(g_pitch > 89.0f) g_pitch = 89.0f;
      if(g_pitch < -89.0f) g_pitch = -89.0f;
      camera.setRotation(g_pitch, g_yaw);
    }
    g_mouse.consumeDelta(); // reset accumulated motion for the next frame

    // Keyboard fly movement
    // (WASD + Q/E up/down), relative to where we look
    Vec3 front = camera.getFront();
    Vec3 worldUp(0.0f, 1.0f, 0.0f);
    Vec3 rightDir = front.cross(worldUp).normalized();
    float moveSpeed = 3.0f * dt; // units per second
    if(g_keyboard.isKeyDown('W')) g_cameraPos += front * moveSpeed;
    if(g_keyboard.isKeyDown('S')) g_cameraPos += front * (-moveSpeed);
    if(g_keyboard.isKeyDown('D')) g_cameraPos += rightDir * moveSpeed;
    if(g_keyboard.isKeyDown('A')) g_cameraPos += rightDir * (-moveSpeed);
    if(g_keyboard.isKeyDown('E')) g_cameraPos += worldUp * moveSpeed;
    if(g_keyboard.isKeyDown('Q')) g_cameraPos += worldUp * (-moveSpeed);
    camera.setPosition(g_cameraPos);

    // Render one frame: clear, draw the spinning model, then present
    g_pRenderer->clear(Vec3(0.1f, 0.1f, 0.15f));

    // Model: spin on two axes over time so every face comes into view.
    float t = (float)GetTickCount64() / 1000.0f; // seconds since boot
    Mat4 model = Mat4::rotateY(t * 2.5f) * Mat4::rotateX(t * 1.15f);
    Mat4 mvp = camera.getProjectionMatrix() * camera.getViewMatrix() * model;

    // The program must be active before glUniform* writes to it, so use() first,
    // then upload the MVP. setUniformMatrix4fv passes GL_TRUE internally to handle
    // Mat4's row-major storage. render() calls use() again (harmless) and draws.
    g_pCubeShader->use();
    g_pCubeShader->setUniformMatrix4fv("uMVP", &mvp.m[0][0]);
    g_pRenderer->render(*g_pCubeMesh, *g_pCubeShader);

    SwapBuffers(g_hDC);
  }

  // Cleanup. Delete GPU objects before the context goes away, since their
  // destructors call glDelete* and need the context still current.
  delete g_pCubeShader;
  delete g_pCubeMesh;
  delete g_pWorld;
  delete g_pRenderer;
  destroyGLContext(hwnd);

  return 0;
}