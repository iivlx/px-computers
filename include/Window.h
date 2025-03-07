#pragma once

#include <windows.h>
#include <string>

#include "gl/GL.h"

#include "pxOpenGL.h"

#include "PxMainboard.h"

class Screen;

class Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window();

  Screen* screen = nullptr;

  void tick(std::vector<PxMainboard*> mainboards);
  void redraw(Screen& screen, std::vector<PxMainboard*> mainboards);
  void run(Screen& screen, std::vector<PxMainboard*> mainboards);

  void PxCPUThread(PxMainboard* mainboard);

  HDC hDC;
private:
  int width, height;
  std::string title;
  HWND hWnd;
  HGLRC hGLRC;

  bool running;

  std::pair<float, float> getMousePosition();

  void initializeWindow();
  void initializeOpenGL();
  void initializeOpenGLExtensions();
  void cleanup();

  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
