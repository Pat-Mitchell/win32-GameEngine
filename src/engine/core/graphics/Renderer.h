/// @file Renderer.h

#pragma once

#include <GL/glew.h>
#include <vector>
#include "Shader.h"
#include "Mesh.h"
#include "..\math\vector.h"

class Shader;
class Mesh;

class Renderer {
  public:
    Renderer();
    ~Renderer();

    bool initialize();
    void shutdown();

    void clear(const Vec3& color = Vec3(0.0f, 0.0f, 0.0f));
    void render(const Mesh& mesh, const Shader& shader);

    // Set global redering state
    void setViewport(int x, int y, int width, int height);
    void enableDepthTest(bool enable);
    void enableCulling(bool enable);

  private:
    bool m_bInitialized;
};