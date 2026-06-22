/// @file graphics_gl_smoke_test.cpp
/// @brief GL integration smoke test for Renderer / Shader / Mesh.
///
/// Brings up a *hidden* GLFW window purely to get a 3.3 core OpenGL context,
/// then exercises the real GPU paths:
///   - Renderer::initialize() (which runs glewInit + state setup)
///   - Shader::loadFromSource() for both valid and intentionally-broken GLSL
///     (the broken case exercises the failure path that resets m_ShaderProgram)
///   - Mesh initialize / uploadData / bind / draw, checking glGetError() stays
///     clean across the sequence
///
/// Requires GLEW + GLFW. No window is shown (GLFW_VISIBLE = false), so it runs
/// from a console, but it is NOT headless — a GPU/driver context is still created.

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../../src/engine/core/graphics/Renderer.h"
#include "../../src/engine/core/graphics/Shader.h"
#include "../../src/engine/core/graphics/Mesh.h"

// ────── ⋆⋅☆⋅⋆ ────────
//      CHECK harness
// ────── ⋆⋅☆⋅⋆ ────────

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                         \
  do {                                                                      \
    ++g_checks;                                                             \
    if(!(cond)) {                                                           \
      ++g_failures;                                                         \
      std::cout << "  [FAIL] " << #cond << "  (line " << __LINE__ << ")\n"; \
    }                                                                       \
  } while(0)

static void section(const char* name) {
  std::cout << "[ " << name << " ]\n";
}

// Drain and report any pending GL error. Returns true if the queue was clean.
static bool glClean(const char* where) {
  bool clean = true;
  for(GLenum e = glGetError(); e != GL_NO_ERROR; e = glGetError()) {
    clean = false;
    std::cout << "  [GL ERROR] 0x" << std::hex << e << std::dec
              << " at " << where << "\n";
  }
  return clean;
}

// ────── ⋆⋅☆⋅⋆ ────────
//      Shader sources
// ────── ⋆⋅☆⋅⋆ ────────

static const char* kValidVertex =
  "#version 330 core\n"
  "layout(location = 0) in vec3 aPos;\n"
  "void main() { gl_Position = vec4(aPos, 1.0); }\n";

static const char* kValidFragment =
  "#version 330 core\n"
  "out vec4 FragColor;\n"
  "void main() { FragColor = vec4(1.0); }\n";

// Deliberate syntax error ('vec3 nope' is never closed / no main).
static const char* kBrokenFragment =
  "#version 330 core\n"
  "out vec4 FragColor;\n"
  "void main() { this is not glsl }\n";

// ────── ⋆⋅☆⋅⋆ ────────
//          Tests
// ────── ⋆⋅☆⋅⋆ ────────

static void testRenderer(Renderer& renderer) {
  section("Renderer::initialize");
  CHECK(renderer.initialize());
  CHECK(glClean("after Renderer::initialize"));

  renderer.setViewport(0, 0, 320, 240);
  renderer.clear(Vec3(0.1f, 0.2f, 0.3f));
  CHECK(glClean("after clear/setViewport"));
}

static void testShader() {
  section("Shader compile / link");

  Shader good;
  CHECK(good.loadFromSource(kValidVertex, kValidFragment));
  good.use(); // should bind without error
  CHECK(glClean("after valid shader use"));

  // Broken fragment must fail cleanly (and, per the fix, leave the shader
  // unusable rather than binding a half-built program).
  Shader bad;
  CHECK(!bad.loadFromSource(kValidVertex, kBrokenFragment));
  bad.use(); // no valid program -> must be a safe no-op, no GL error
  CHECK(glClean("after failed shader use"));
}

static void testMesh() {
  section("Mesh upload / draw");

  Mesh mesh;
  mesh.initialize();
  CHECK(glClean("after Mesh::initialize"));

  // One front-facing triangle.
  Vertex a{}, b{}, c{};
  a.position = Vec3(-0.5f, -0.5f, 0.0f);
  b.position = Vec3( 0.5f, -0.5f, 0.0f);
  c.position = Vec3( 0.0f,  0.5f, 0.0f);
  mesh.addVertex(a);
  mesh.addVertex(b);
  mesh.addVertex(c);
  mesh.addIndex(0);
  mesh.addIndex(1);
  mesh.addIndex(2);

  mesh.uploadData();
  CHECK(glClean("after Mesh::uploadData"));

  mesh.bind();
  mesh.draw();
  mesh.unbind();
  CHECK(glClean("after Mesh::draw"));
}

// ────── ⋆⋅☆⋅⋆ ────────
//       Entry point
// ────── ⋆⋅☆⋅⋆ ────────

int main() {
  std::cout << "Graphics GL smoke test\n======================\n";

  if(!glfwInit()) {
    std::cout << "FATAL: glfwInit failed\n";
    return 1;
  }

  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);            // hidden: console-friendly
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(320, 240, "gfx-smoke", nullptr, nullptr);
  if(!window) {
    std::cout << "FATAL: could not create a GL 3.3 context (no GPU/driver?)\n";
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  {
    Renderer renderer;
    testRenderer(renderer);
    testShader();
    testMesh();
    // renderer/mesh/shader destructors run here, before context teardown.
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  std::cout << "\n--------------------------------\n";
  std::cout << "checks: " << g_checks
            << "   failures: " << g_failures << "\n";

  if(g_failures == 0) {
    std::cout << "RESULT: PASS\n";
    return 0;
  }
  std::cout << "RESULT: FAIL\n";
  return 1;
}
