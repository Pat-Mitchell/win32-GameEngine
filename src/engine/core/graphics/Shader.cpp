/// @file Shader.cpp

#include "Shader.h"
#include <GL/glew.h>
#include <fstream>
#include <sstream>

Shader::Shader() : m_ShaderProgram(0) {}

Shader::~Shader() {
  if(m_ShaderProgram != 0) {
    glDeleteProgram(m_ShaderProgram);
  }
}

bool Shader::loadFromFile(const std::string& vertexPath, const std::string& fragmentPath) {
  // Read shader source files
  std::string vertexCode;
  std::string fragmentCode;

  try {
    std::ifstream vertexFile(vertexPath);
    std::ifstream fragmentFile(fragmentPath);

    if(!vertexFile.is_open() || !fragmentFile.is_open()) {
      // Failed to open shader files
      return false;
    }

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vertexFile.rdbuf();
    fShaderStream << fragmentFile.rdbuf();

    vertexFile.close();
    fragmentFile.close();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();
  } catch(const std::exception& e) {
    // Error reading shader files
    return false;
  }

  return loadFromSource(vertexCode, fragmentCode);
}

bool Shader::loadFromSource(const std::string& vertexSource, const std::string& fragmentSource) {
  m_VertexSource = vertexSource;
  m_FragmentSource = fragmentSource;

  // Drop any previously linked program so reloading doesn't leak it on the GPU.
  if(m_ShaderProgram != 0) {
    glDeleteProgram(m_ShaderProgram);
    m_ShaderProgram = 0;
  }

  // Compile shaders
  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

  // Compile vertex shader
  const char* vShaderCode = vertexSource.c_str();
  glShaderSource(vertexShader, 1, &vShaderCode, NULL);
  glCompileShader(vertexShader);

  // Check for compilation errors
  GLint success;
  GLchar infoLog[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    // Vertex compilation failed
    glDeleteShader(vertexShader);
    return false;
  }

  // Compile fragment shader
  const char* fShaderCode = fragmentSource.c_str();
  glShaderSource(fragmentShader, 1, &fShaderCode, NULL);
  glCompileShader(fragmentShader);
  
  // Check for compilation errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if(!success) {
    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    // Fragment shader compilation failed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return false;
  }

  // Create shader program
  m_ShaderProgram = glCreateProgram();
  glAttachShader(m_ShaderProgram, vertexShader);
  glAttachShader(m_ShaderProgram, fragmentShader);
  glLinkProgram(m_ShaderProgram);

  // Check for linking errors
  glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
  if(!success) {
    glGetProgramInfoLog(m_ShaderProgram, 512, NULL, infoLog);
    // Shader program linking failed
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // Discard the failed program so use() can't bind an unlinked program.
    glDeleteProgram(m_ShaderProgram);
    m_ShaderProgram = 0;
    return false;
  }

  // Clean up shaders
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  return true;
}

void Shader::use() const {
  if(m_ShaderProgram != 0) {
    glUseProgram(m_ShaderProgram);
  }
}

void Shader::setUniform1f(const std::string& name, float value) const {
  GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
  if(location != -1) {
    glUniform1f(location, value);
  }
}

void Shader::setUniform3f(const std::string& name, const float* value) const {
  GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
  if(location != -1) {
    glUniform3fv(location, 1, value);
  }
}

void Shader::setUniformMatrix4fv(const std::string& name, const float* matrix) const {
  GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
  if(location != -1) {
    glUniformMatrix4fv(location, 1, GL_TRUE, matrix); // Mat4 scripts are in row-major
  }
}