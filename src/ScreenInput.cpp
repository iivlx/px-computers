#include <windows.h>

#include <stdexcept>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "ScreenInput.h"

#include <iostream>


ScreenInput::ScreenInput(Screen* screen)
  : screen(screen), clicked(nullptr), drag_x(0.0f), drag_y(0.0f)
{
}

ScreenInput::~ScreenInput() {
}

void ScreenInput::mouseClickDown(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // some spongy looking code
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  std::cout << "Coords: " << ndcX << ", " << ndcY << std::endl;

  // some horrible factoring here...
  for (auto it = screen->displays.rbegin(); it != screen->displays.rend(); ++it) {

    auto display = *it;

    float xOffset = display->x_offset;
    float yOffset = display->y_offset;
    float scale = display->scale;
    auto [width, height] = display->getSize();

    if (ndcX >= xOffset - (scale) && ndcX <= xOffset + scale
      && ndcY >= yOffset - scale && ndcY <= yOffset + scale) {

      //std::cout << "Hit: " << display->textureID << " : " << ndcX << ", " << ndcY << std::endl;

      mouseClickDownPxDisplay(display, ndcX, ndcY);

      return;
    }
  }
}

void ScreenInput::mouseClickUp(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f;
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  if (dragging) {
    dragging = false;
  }
}

void ScreenInput::mouseMove(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // normal coords
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

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

void ScreenInput::mouseClickDownPxDisplay(PxDisplay* display, float x, float y) {
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

bool ScreenInput::mouseOverPxDevice(float x, float y) {
  float ndcX = (x / 512.0f) * 2 - 1.0f; // some spongy looking code
  float ndcY = (1.0f - (y / 512.0f)) * 2 - 1.0f;

  // some horrible factoring here...
  for (auto it = screen->displays.rbegin(); it != screen->displays.rend(); ++it) {

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
