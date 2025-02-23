#pragma once

#include <string>
#include <unordered_map>

#include "pxDisplay.h"

class MainScreen {
public:
  MainScreen(MainWindow* window, int width, int height);
  ~MainScreen();

  void render(PxDisplay* display, float x_offset = 1.0f, float y_offset = 1.0f);
  void mouseClickDown(float x, float y);
  void mouseClickUp(float x, float y);
  void mouseMove(float x, float y);
  void mouseClickDownPxDisplay(PxDisplay* display, float x, float y);
  bool mouseOverPxDevice(float x, float y);

  void addDisplay(PxDisplay* display);
  void moveToFront(PxDisplay* display);

  GLuint renderProgram;
  GLuint computeProgram;

  std::vector<PxDisplay*> displays;
  //std::unordered_map<PxDisplay*, std::pair<GLuint, GLuint>> displays; // textures, vaos


  PxDevice* clicked;
  bool dragging = false;
  float drag_x;
  float drag_y;

private:
  int width, height;
  static inline int nextDisplayID = 0;

  void initializeShaderPrograms();
  void initializeShaderUniforms();
  void initializeQuadVAO(GLuint& VAO);
  void initializeTexture(GLuint& textureID);

  // shader uniforms
  GLint xOffsetLoc;
  GLint yOffsetLoc;
  GLint scaleLoc;

};
