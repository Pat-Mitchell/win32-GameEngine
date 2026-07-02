/// @file Texture.h

#pragma once

#include <string>
#include <GL/glew.h>

class Texture {
  public:
    Texture();
    ~Texture();

    void bind() const;
    void unbind() const;

    GLuint getId() const { return m_TextureID; }

  private:
    GLuint m_TextureID;
};