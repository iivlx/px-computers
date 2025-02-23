#pragma once

#include <windows.h>
#include <string>

#include "gl/GL.h"

#include "pxOpenGL.h"

#include "pxMainboard.h"

class MainScreen;

class MainWindow {
public:
  MainWindow(int width, int height, const std::string& title);
  ~MainWindow();

  MainScreen* screen = nullptr;

  void tick(std::vector<PxMainboard*> mainboards);
  void redraw(MainScreen& screen, std::vector<PxMainboard*> mainboards);
  void run(MainScreen& screen, std::vector<PxMainboard*> mainboards);

  void PxCPUThread(PxMainboard* mainboard);

  void mouseClickDown(int x, int y);
  void mouseClickUp(int x, int y);
  void mouseMove(int x, int y);

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
  void initalizeOpenGlFunctionPointers();
  void cleanup();

  static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
