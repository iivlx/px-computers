#pragma once

#include <string>
#include <unordered_map>

#include "Window.h"
#include "Layout.h"

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
  void renderDisplay(PxDisplay* display, Layout* layer);
  void renderKeyboard(PxKeyboard* keyboard);

  void addDevice(PxDevice* device);
  void addDisplay(PxDisplay* display);

  void moveToFront(std::pair<PxDevice*, Layout*> device_context);
  std::pair<float, float> normalizeMouseCoords(float x, float y);

  GLuint renderProgram;
  GLuint computeProgram;

  std::vector<std::pair<PxDevice*, Layout*>> devices;

private:
  int width, height;
  static inline int nextDisplayID = 0;

  void initializeShaderPrograms();
  void initializeShaderUniforms();
  void initializeQuadVAO(GLuint& VAO);
  void initializeTexture(GLuint& textureID, std::pair<int, int>);

  // shader uniforms
  GLint xOffsetLoc;
  GLint yOffsetLoc;
  GLint scaleLoc;

};
