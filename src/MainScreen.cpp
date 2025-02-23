#include <windows.h>

#include <stdexcept>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "gl/GL.h"

#include "MainWindow.h"
#include "MainScreen.h"

#include "pxOpenGL.h"
#include "pxOpenGLUtil.h"

#include <iostream>

MainScreen::MainScreen(MainWindow* window, int width, int height)
  : width(width), height(height)
{
  window->screen = this;
  initializeShaderPrograms();
  initializeShaderUniforms();
}

MainScreen::~MainScreen() {
}

void MainScreen::addDisplay(PxDisplay* pd) {
  initializeTexture(pd->textureID);
  initializeQuadVAO(pd->VAO);

  displays.emplace_back(pd);
}

void MainScreen::initializeShaderPrograms() {
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

void MainScreen::initializeShaderUniforms() {
  xOffsetLoc = glGetUniformLocation(renderProgram, "xOffset");
  yOffsetLoc = glGetUniformLocation(renderProgram, "yOffset");
  scaleLoc = glGetUniformLocation(renderProgram, "scale");
}

void MainScreen::initializeTexture(GLuint& textureID) {
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void MainScreen::initializeQuadVAO(GLuint& quadVAO) {
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

/* Render a display's buffer as a texture to the screen... */
void MainScreen::render(PxDisplay* display, float x_offset, float y_offset) {

  // update the display's texture from its buffer data
  glBindTexture(GL_TEXTURE_2D, display->textureID);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, display->currentBuffer->data());

  // update shader with display's position
  glUseProgram(renderProgram);
  glUniform1f(xOffsetLoc, display->x_offset);
  glUniform1f(yOffsetLoc, display->y_offset);
  glUniform1f(scaleLoc, display->scale);

  // draw
  glBindVertexArray(display->VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void MainScreen::moveToFront(PxDisplay* display) {
  auto it = std::find(displays.begin(), displays.end(), display);
  if (it != displays.end()) {
    displays.erase(it);
    displays.emplace_back(display);
  }
}

void MainScreen::mouseClickDown(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // some spongy looking code
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  std::cout << "Coords: " << ndcX << ", " << ndcY << std::endl;

  // some horrible factoring here...
  for (auto it = displays.rbegin(); it != displays.rend(); ++it) {

    auto display = *it;

    float xOffset = display->x_offset;
    float yOffset = display->y_offset;
    float scale = display->scale;
    auto [width, height] = display->getSize();

    if (ndcX >= xOffset - (scale) && ndcX <= xOffset + scale
      && ndcY >= yOffset-scale && ndcY <= yOffset +scale) {

      //std::cout << "Hit: " << display->textureID << " : " << ndcX << ", " << ndcY << std::endl;

      mouseClickDownPxDisplay(display, ndcX, ndcY);

      return;
    }
  }
}

void MainScreen::mouseClickUp(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f;
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  if (dragging) {
    dragging = false;
  }
}

void MainScreen::mouseMove(float x, float y) {
  float ndcX = (x / 512.0f)*2 - 1.0f; // normal coords
  float ndcY = (1.0f - (y / 512.0f))*2 - 1.0f;

  if (dragging) {
    auto display = static_cast<PxDisplay*>(clicked);

    float dx = (drag_x - ndcX);
    float dy = (drag_y - ndcY);

    drag_x = ndcX;
    drag_y = ndcY;

    display->x_offset -= dx; // opposite direction of mouse drag
    display->y_offset -= dy;

    //std::cout << "Move: " << display->textureID << " : " << display->x_offset << ", " << display->y_offset << std::endl;
  }
}

void MainScreen::mouseClickDownPxDisplay(PxDisplay* display, float x, float y) {
  clicked = static_cast<PxDevice*>(display);

  moveToFront(display);

  if (!dragging) {
    dragging = true;
    drag_x = x;
    drag_y = y;
  }
  else {
    //dragging = false; // toggle
  }
}

bool MainScreen::mouseOverPxDevice(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // some spongy looking code
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  // some horrible factoring here...
  for (auto it = displays.rbegin(); it != displays.rend(); ++it) {

    auto display = *it;

    float xOffset = display->x_offset;
    float yOffset = display->y_offset;
    float scale = display->scale;
    auto [width, height] = display->getSize();

    if (ndcX >= xOffset - (scale) && ndcX <= xOffset + scale
      && ndcY >= yOffset - scale && ndcY <= yOffset + scale) {
      return true;
    }
  }
  return false;
}
