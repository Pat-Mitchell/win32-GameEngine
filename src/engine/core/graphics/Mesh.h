/// @file Mesh.h

#pragma once

#include <vector>
#include <GL/glew.h>
#include "..\math\vector.h"

struct Vertex {
  Vec3 position;
  Vec3 normal;
  Vec3 texCoord;
};

class Mesh {
  public:
    Mesh();
    ~Mesh();

    void initialize();
    void cleanup();
    void uploadData();

    void addVertex(const Vertex& vertex);
    void addIndex(GLuint index);

    void bind() const;
    void unbind() const;
    void draw() const;

    size_t getVertexCount() const { return m_Vertices.size(); }
    size_t getIndexConst() const { return m_Indices.size(); }

  private:
    std::vector<Vertex> m_Vertices;
    std::vector<GLuint> m_Indices;

    GLuint m_VAO, m_VBO, m_EBO;
    bool m_Initialized;
};