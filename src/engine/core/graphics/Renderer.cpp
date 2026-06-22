/// @file Renderer.cpp

#include "Renderer.h"

Renderer::Renderer() : m_bInitialized(false) {}

Renderer::~Renderer() {
  shutdown();
}

bool Renderer::initialize() {
  // Initialize GLEW
  GLenum err = glewInit();
  if(err != GLEW_OK) {
    return false;
  }

  // Check for OpenGL 3.3 support
  if(!GLEW_VERSION_3_3) {
    return false;
  }

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Enable backface culling
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  m_bInitialized = true;
  return true;
}

void Renderer::shutdown() {
  if(m_bInitialized) {
    m_bInitialized = false;
  }
}

void Renderer::clear(const Vec3& color) {
  if(!m_bInitialized) return;

  glClearColor(color.x, color.y, color.z, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::render(const Mesh& mesh, const Shader& shader) {
  if(!m_bInitialized) return;

  shader.use();
  mesh.bind();
  mesh.draw();
  mesh.unbind();
}

void Renderer::setViewport(int x, int y, int width, int height) {
  if(!m_bInitialized) return;

  glViewport(x, y, width, height);
}

void Renderer::enableDepthTest(bool enable) {
  if(!m_bInitialized) return;

  if(enable) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void Renderer::enableCulling(bool enable) {
  if(!m_bInitialized) return;

  if(enable) {
    glEnable(GL_CULL_FACE);
  } else {
    glDisable(GL_CULL_FACE);
  }
}