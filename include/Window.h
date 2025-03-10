#pragma once

#include <windows.h>
#include <string>

#include "gl/GL.h"

#include "pxOpenGL.h"

#include "PxMainboard.h"

class Screen;
class Input;

/* win32 window */
// -loads win32 opengl functions and extensions
// -creates and acts as a win32 opengl context
// -handles win32 mouse/keyboard interactions
class Window {
public:
  Window(int width, int height, const std::string& title);
  ~Window();

  Screen* screen = nullptr;
  Input* input = nullptr;

  void tick(std::vector<PxMainboard*> mainboards); 
  void redraw(std::vector<PxMainboard*> mainboards);
  void run(std::vector<PxMainboard*> mainboards);

  void PxCPUThread(PxMainboard* mainboard);

  HDC hDC;
private:
  int width, height;
  std::string title;
  HWND hWnd;
  HGLRC hGLRC;

  bool running;

  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  std::pair<float, float> getMousePosition();

  void initializeWindow();
  void initializeOpenGL();
  void initializeOpenGLExtensions();
  void cleanup();
};
