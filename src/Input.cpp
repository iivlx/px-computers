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
  : screen(screen)
  , clicked(nullptr)
  , drag(std::make_pair<float,float>(0.0f, 0.0f))
{
}

Input::~Input() {
}

void Input::mouseClickDown(float x, float y) {
  auto mouse = screen->normalizeMouseCoords(x, y);

  for (auto &device_context : screen->devices) {
    if (isMouseInLayout(mouse, device_context.second)) {

      screen->moveToFront(device_context);
      clicked = &screen->devices.at(0);

      if (!dragging) {
        dragging = true;
        drag = mouse;
      }

      break; // don't click through
    }
  }
}

void Input::mouseClickUp(float x, float y) {
  //auto [ndcX, ndcY] = screen->normalizeMouseCoords(x, y);

  if (dragging) {
    dragging = false;
  }
}

void Input::mouseMove(float x, float y) {
  auto mouse = screen->normalizeMouseCoords(x, y);

  if (dragging && clicked) {
    auto layout = (*clicked).second;

    float dx = (drag.first - mouse.first);
    float dy = (drag.second - mouse.second);
    layout->x -= dx; // opposite direction of mouse drag
    layout->y -= dy;

    drag = mouse;
  }
}

bool Input::mouseOver(float x, float y) {
  auto mouse = screen->normalizeMouseCoords(x, y);

  for (auto device_context : screen->devices) {
    if (isMouseInLayout(mouse, device_context.second)) {
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

bool Input::isMouseInLayout(std::pair<float, float> mouse, Layout* layout) {
  if (mouse.first >= layout->x - layout->scale
    && mouse.first <= layout->x + layout->scale
    && mouse.second >= layout->y - layout->scale
    && mouse.second <= layout->y + layout->scale)
  {
    return true;
  }
  return false;
}
