#include <windows.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "gl/GL.h"

#include "pxOpenGL.h"

std::string loadFileText(const std::string& filepath) {
  std::ifstream file(filepath);
  if (!file.is_open()) { throw std::runtime_error("Failed to open file: " + filepath); }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void checkForShaderErrorType(GLuint program, GLenum type) {
  int success;
  char infoLog[512];
  if (glGetShaderiv(program, type, &success); !success) {
    glGetShaderInfoLog(program, 512, nullptr, infoLog);
    throw std::runtime_error("Shader error: " + std::string(infoLog));
  }
}

GLuint compileShader(GLenum type, const char* source) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);

  checkForShaderErrorType(shader, GL_COMPILE_STATUS);

  return shader;
}

void initializeShader(GLuint& program, std::unordered_map<GLenum, std::string> shaders) {
  program = glCreateProgram();

  for (auto pair : shaders) {
    auto shader = compileShader(pair.first, pair.second.c_str());
    glAttachShader(program, shader);
    glDeleteShader(shader);
  }

  glLinkProgram(program);

  checkForShaderErrorType(program, GL_LINK_STATUS);
}
