#pragma once

#include <string>
#include <unordered_map>

#include "Window.h"

#include "PxDisplay.h"
#include "PxKeyboard.h"
#include "PxMainboard.h"

class Window;

/* opengl "screen" or rendering surface */
// - renders pxdevices
class Screen {
public:
  Screen(int width, int height);
  ~Screen();

  void render(std::vector<PxMainboard*> mainboards);
  void renderDisplay(PxDisplay* display, float x_offset = 1.0f, float y_offset = 1.0f, float scale = 1.0f);
  void renderKeyboard(PxKeyboard* keyboard);

  void addDevice(PxDevice* device);
  void addDisplay(PxDisplay* display);
  void moveToFront(PxDevice* device);

  GLuint renderProgram;
  GLuint computeProgram;

  std::unordered_map<PxDevice*, std::pair<float, float>> devices; // we just keep pair of ints as layout for now...

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
