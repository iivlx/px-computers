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
  , drag_x(0.0f), drag_y(0.0f)
{
}

Input::~Input() {
}

void Input::mouseClickDown(float x, float y) {
  auto [ndcX, ndcY] = screen->normalizeMouseCoords(x, y);

  std::cout << "Coords: " << ndcX << ", " << ndcY << std::endl;

  for (auto &device_context : screen->devices) {
    auto [device, layout] = device_context;

    std::pair<float, float> mouse = { ndcX, ndcY };

    if (screen->isMouseInLayout(mouse, layout)) {

      screen->moveToFront(device_context);
      clicked = &device_context;

      if (!dragging) {
        dragging = true;
        drag_x = ndcX;
        drag_y = ndcY;
      }
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
  auto [ndcX, ndcY] = screen->normalizeMouseCoords(x, y);

  if (dragging && clicked) {
    auto [device, layout] = *clicked;

    float dx = (drag_x - ndcX) / layout->scale;
    float dy = (drag_y - ndcY) / layout->scale;

    drag_x = ndcX;
    drag_y = ndcY;

    layout->x -= dx * layout->scale; // opposite direction of mouse drag
    layout->y -= dy * layout->scale;
  }
}

bool Input::mouseOver(float x, float y) {
  auto [ndcX, ndcY] = screen->normalizeMouseCoords(x, y);

  for (auto device_context : screen->devices) {
    auto [device, layout] = device_context;

    if (screen->isMouseInLayout({ ndcX, ndcY }, layout)) {
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
