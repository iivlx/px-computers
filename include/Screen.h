#pragma once

#include <string>
#include <unordered_map>

#include "ScreenInput.h"

#include "pxDisplay.h"

class Window;
class ScreenInput;

class Screen {
public:
  Screen(Window* window, int width, int height);
  ~Screen();

  void render(PxDisplay* display, float x_offset = 1.0f, float y_offset = 1.0f);

  void addDisplay(PxDisplay* display);
  void moveToFront(PxDisplay* display);

  ScreenInput* input;

  GLuint renderProgram;
  GLuint computeProgram;

  std::vector<PxDisplay*> displays;
  //std::unordered_map<PxDisplay*, std::pair<GLuint, GLuint>> displays; // textures, vaos

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
