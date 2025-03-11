#include <windows.h>

#include <stdexcept>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "Window.h"
#include "Input.h"

#include <iostream>


Input::Input(Screen* screen)
  : clicked(nullptr), drag_x(0.0f), drag_y(0.0f)
  , screen(screen)
{
}

Input::~Input() {
}

void Input::mouseClickDown(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // some spongy looking code
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  std::cout << "Coords: " << ndcX << ", " << ndcY << std::endl;

  for (auto device : screen->devices) {

  }
}

void Input::mouseClickUp(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f;
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  if (dragging) {
    dragging = false;
  }
}

void Input::mouseMove(float x, float y) {

  return;

  float ndcX = (x / 512.0f) * 2 - 1.0f; // normal coords
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  if (dragging) {

  }
}

void Input::mouseClickDownPxDisplay(PxDisplay* display, float x, float y) {
  clicked = static_cast<PxDevice*>(display);

  screen->moveToFront(display);

  if (!dragging) {
    dragging = true;
    drag_x = x;
    drag_y = y;
  }
  else {
    //dragging = false; // toggle
  }
}

bool Input::mouseOverPxDevice(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // some spongy looking code
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  for (auto devicecontext : screen->devices) {

    auto device = devicecontext.first;
    auto layout = devicecontext.second;

    float scale = layout->scale;
    float xOffset = layout->x * scale;
    float yOffset = layout->y * scale;

    int width = 64;
    int height = 64;
    //auto [width, height] = display->getSize();

    if ( ndcX >= xOffset - scale
      && ndcX <= xOffset + scale
      && ndcY >= yOffset - scale
      && ndcY <= yOffset + scale) {
      return true;
    }
  }
  return false;
}

void Input::keyDown(int character, bool repeat) {
  if (clicked == nullptr) return; // Need a focused device for text input...

  // Check if `clicked` is a pxKeyboard ?

  // Send correct bytes to pxKeyboard ?


}
