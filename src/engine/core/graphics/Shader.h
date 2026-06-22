/// @file Shader.h

#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>

class Shader {
  public:
    Shader();
    ~Shader();

    bool loadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
    void use() const;

    // Uniform setting methods
    void setUniform1f(const std::string& name, float value) const;
    void setUniform3f(const std::string&name, const float* value) const;
    void setUniformMatrix4fv(const std::string& name, const float* matrix) const;

  private:
    GLuint m_ShaderProgram;
    std::string m_VertexSource;
    std::string m_FragmentSource;

    bool compileShader(GLuint shader, const std::string& source);
    bool linkProgram();
};