#pragma once

#include "Screen.h"

#include "PxDevice.h"
#include "PxDisplay.h"

class Screen;

class ScreenInput {
public:
  ScreenInput(Screen* screen);
  ~ScreenInput();

  void mouseClickDown(float x, float y);
  void mouseClickUp(float x, float y);
  void mouseMove(float x, float y);
  void mouseClickDownPxDisplay(PxDisplay* display, float x, float y);
  bool mouseOverPxDevice(float x, float y);

  void keyDown(int keycode, bool repeat);

  Screen* screen;

private:
  PxDevice* clicked;
  bool dragging = false;
  float drag_x;
  float drag_y;

};
