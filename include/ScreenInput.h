#pragma once

#include "MainScreen.h"

#include "pxDevice.h"
#include "pxDisplay.h"

class MainScreen;

class ScreenInput {
public:
  ScreenInput(MainScreen* screen);
  ~ScreenInput();

  void mouseClickDown(float x, float y);
  void mouseClickUp(float x, float y);
  void mouseMove(float x, float y);
  void mouseClickDownPxDisplay(PxDisplay* display, float x, float y);
  bool mouseOverPxDevice(float x, float y);

  MainScreen* screen;

private:
  PxDevice* clicked;
  bool dragging = false;
  float drag_x;
  float drag_y;

};
