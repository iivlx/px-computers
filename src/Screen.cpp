#include <windows.h>

#include <stdexcept>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "gl/GL.h"

#include "Window.h"
#include "Screen.h"
#include "Input.h"

#include "pxOpenGL.h"
#include "pxOpenGLUtil.h"

#include <iostream>

Screen::Screen(int width, int height)
  : width(width), height(height)
{
  initializeShaderPrograms();
  initializeShaderUniforms();
}

Screen::~Screen() {
}

void Screen::addDevice(PxDevice* pd) {

  if (auto* display = dynamic_cast<PxDisplay*>(pd)) {
    addDisplay(display);
  }

  //else if (auto* keyboard = dynamic_cast<PxKeyboard*>(pd)) {
  //  addKeyboard(keyboard);
  //}

}

void Screen::addDisplay(PxDisplay* pd) {
  initializeTexture(pd->textureID);
  initializeQuadVAO(pd->VAO);

  std::pair<PxDisplay*, Layout*> device_context = { pd, new Layout(0.0f, 0.0f, 0.5f) };
  devices.emplace_back(device_context);
}

void Screen::initializeShaderPrograms() {
  std::unordered_map<GLenum, std::string> renderShaders = {
    { GL_VERTEX_SHADER, loadFileText("shaders/px.vert") },
    { GL_FRAGMENT_SHADER, loadFileText("shaders/px.frag") }
  };
  std::unordered_map<GLenum, std::string> computeShaders = {
    {GL_COMPUTE_SHADER, loadFileText("shaders/px.comp")}
  };

  initializeShader(renderProgram, renderShaders);
  initializeShader(computeProgram, computeShaders);
}


void Screen::initializeShaderUniforms() {
  xOffsetLoc = glGetUniformLocation(renderProgram, "xOffset");
  yOffsetLoc = glGetUniformLocation(renderProgram, "yOffset");
  scaleLoc = glGetUniformLocation(renderProgram, "scale");
}

void Screen::initializeTexture(GLuint& textureID) {
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void Screen::initializeQuadVAO(GLuint& quadVAO) {
  float vertices[] = {
    // x,y - u,v
    -1.0f, -1.0f,    0.0f, 0.0f, // bl
     1.0f, -1.0f,    1.0f, 0.0f, // br
     1.0f,  1.0f,    1.0f, 1.0f, // tr
    -1.0f,  1.0f,    0.0f, 1.0f  // tl
  };

  unsigned int indices[] = {
      0, 1, 2, // 1st half
      2, 3, 0  // 2nd half
  };

  GLuint VBO, EBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(quadVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}


void Screen::render(std::vector<PxMainboard*> mainboards) {
  for (auto device_context : devices) {
    auto layout = device_context.second;

    if (auto* device = dynamic_cast<PxDisplay*>(device_context.first)) {
      renderDisplay(device, layout);
    }
    if (auto* device = dynamic_cast<PxKeyboard*>(device_context.first)) {
      //renderKeyboard(device, layout->x, layout->y, layout->scale);
    }
  }
}


/* Render a display's buffer as a texture to the screen... */
void Screen::renderDisplay(PxDisplay* display, Layout* layout) {
  // update the display's texture from its buffer data
  glBindTexture(GL_TEXTURE_2D, display->textureID);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, display->currentBuffer->data());

  // update shader with display's position
  glUseProgram(renderProgram);
  glUniform1f(xOffsetLoc, layout->x);
  glUniform1f(yOffsetLoc, layout->y);
  glUniform1f(scaleLoc, layout->scale);

  // draw
  glBindVertexArray(display->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void Screen::moveToFront(PxDevice* pd) {
}
