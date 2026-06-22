/// @file Mesh.cpp

#include "Mesh.h"
#include <GL/glew.h>

Mesh::Mesh() : m_VAO(0), m_VBO(0), m_EBO(0), m_Initialized(false) {}

Mesh::~Mesh() {
  cleanup();
}

void Mesh::initialize() {
  if(m_Initialized) return;

  // Generate and bind VAO
  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  // Generate and bind VBO
  glGenBuffers(1, &m_VBO);

  // Generate and bind EBO
  glGenBuffers(1, &m_EBO);

  m_Initialized = true;
}

void Mesh::cleanup() {
  if(!m_Initialized) return;

  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_EBO);

  m_VAO = 0;
  m_VBO = 0;
  m_EBO = 0;
  m_Initialized = false;
}

void Mesh::uploadData() {
  if(!m_Initialized || m_Vertices.empty()) return;

  // Bind VAO
  glBindVertexArray(m_VAO);

  // Upload vertex data to VBO
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(Vertex), m_Vertices.data(), GL_STATIC_DRAW);

  // Set up vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);

  // Normal attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  glEnableVertexAttribArray(1);

  // Texture coordinate attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
  glEnableVertexAttribArray(2);

  // Upload index data if indives exist
  if(!m_Indices.empty()) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_Indices.size() * sizeof(GLuint), m_Indices.data(), GL_STATIC_DRAW);
  }

  // Unbid VAO
  glBindVertexArray(0);
}

void Mesh::addVertex(const Vertex& vertex) {
  m_Vertices.push_back(vertex);
}

void Mesh::addIndex(GLuint index) {
  m_Indices.push_back(index);
}

void Mesh::bind() const {
  if(!m_Initialized) return;
  glBindVertexArray(m_VAO);
}

void Mesh::unbind() const {
  glBindVertexArray(0);
}

void Mesh::draw() const {
  if(!m_Initialized) return;

  if(!m_Indices.empty()) {
    // draw with indices
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_Indices.size()), GL_UNSIGNED_INT, 0);
  } else {
    // draw without indices
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(m_Vertices.size()));
  }
}