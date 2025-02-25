#include <Windows.h>

#include <stdexcept>
#include <thread>
#include <unordered_map>

#include "gl/GL.h"

#include "MainWindow.h"
#include "MainScreen.h"

#include "pxOpenGL.h" 
#include "pxCPU.h"

#include <iostream>

/* Creates a Window (hWnd) and attaches an OpenGL Rendering Context (hGLRC) referenced by its device context (hDC) */
MainWindow::MainWindow(int width, int height, const std::string& title)
  : width(width), height(height), title(title)
  , hWnd(nullptr), hDC(nullptr), hGLRC(nullptr)
{
  initializeWindow();                  // Create the Window (hWnd) and get its device context (hDc)
  initializeOpenGL();                  // Create a temporary OpenGL 1.1 context (hGLRC) to load extensions
  initializeOpenGLExtensions();        // Create a new OpenGL 4.3 context to replace the old one
  initializeOpenGlFunctionPointers();  // Load upgraded OpenGL function pointers (for shaders, VAOs, etc)
}

MainWindow::~MainWindow() {
  cleanup();
}

/* Windows and OpenGL */

void MainWindow::initializeWindow() {
  WNDCLASS wc = {};
  wc.lpfnWndProc = MainWindow::WindowProc;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.lpszClassName = "MainWindowClass";

  if (!RegisterClass(&wc)) {
    throw std::runtime_error("Failed to register window class");
  }

  RECT clientRect = { 0, 0, width, height };
  AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);

  hWnd = CreateWindowEx(
    0, "MainWindowClass", title.c_str(),
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT, CW_USEDEFAULT,
    clientRect.right - clientRect.left,
    clientRect.bottom - clientRect.top,
    nullptr, nullptr, wc.hInstance, nullptr);

  if (!hWnd) {
    throw std::runtime_error("Failed to create window");
  }

  SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this)); // 'this' ptr

  hDC = GetDC(hWnd);
}

void MainWindow::initializeOpenGL() {
  PIXELFORMATDESCRIPTOR pfd = {};
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;

  int pixelFormat = ChoosePixelFormat(hDC, &pfd);
  if (!pixelFormat || !SetPixelFormat(hDC, pixelFormat, &pfd)) {
    throw std::runtime_error("Failed to set pixel format for OpenGL");
  }

  hGLRC = wglCreateContext(hDC);
  if (!hGLRC || !wglMakeCurrent(hDC, hGLRC)) {
    throw std::runtime_error("Failed to create or activate OpenGL context");
  }
}

void MainWindow::initializeOpenGLExtensions() {
  HGLRC tempContext = wglCreateContext(hDC);
  if (!tempContext || !wglMakeCurrent(hDC, tempContext)) {
    throw std::runtime_error("Failed to create temporary OpenGL context");
  }

  auto wglCreateContextAttribsARB = reinterpret_cast<HGLRC(WINAPI*)(HDC, HGLRC, const int*)>(
    wglGetProcAddress("wglCreateContextAttribsARB"));

  if (!wglCreateContextAttribsARB) {
    throw std::runtime_error("Failed to load wglCreateContextAttribsARB");
  }

  const int attribs[] = {
      WGL_CONTEXT_MAJOR_VERSION_ARB, 4, // OpenGL 4.3
      WGL_CONTEXT_MINOR_VERSION_ARB, 3,
      WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
      0
  };

  hGLRC = wglCreateContextAttribsARB(hDC, nullptr, attribs);
  if (!hGLRC || !wglMakeCurrent(hDC, hGLRC)) {
    throw std::runtime_error("Failed to create modern OpenGL context");
  }

  wglDeleteContext(tempContext);
}

void MainWindow::cleanup() {
  if (hGLRC) {
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(hGLRC);
  }
  if (hWnd && hDC) {
    ReleaseDC(hWnd, hDC);
  }
  if (hWnd) {
    DestroyWindow(hWnd);
  }
}

/* Window callback and event handling */

LRESULT CALLBACK MainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

  switch (uMsg) {
  case WM_NCCREATE: {
    auto createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams)); // store 'this' ptr
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
  }
  case WM_LBUTTONDOWN: {
    auto window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    window->screen->mouseClickDown(x, y);
    return 0;
  }
  case WM_RBUTTONDOWN: {
    ; // randomly capture right clicks
    return 0;
  }
  case WM_LBUTTONUP: {
    auto window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);
    window->screen->mouseClickUp(x, y);
    return 0;
  }
  case WM_MOUSEMOVE: {
    auto window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    int x = LOWORD(lParam);
    int y = HIWORD(lParam);

    if (window->screen->mouseOverPxDevice(x, y)) {
      SetCursor(LoadCursor(nullptr, IDC_HAND));
    }
    else {
      SetCursor(LoadCursor(nullptr, IDC_ARROW));
    }

    window->screen->mouseMove(x, y);

    return 0;
  }
  case WM_CLOSE: {
    PostQuitMessage(0);
    return 0;
  }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void MainWindow::tick(std::vector<PxMainboard*> mainboards) {
  for (const auto& mainboard : mainboards) {
    mainboard->tick();
  }
}

void MainWindow::redraw(MainScreen& screen, std::vector<PxMainboard*> mainboards) {
  static float x = 0.0f;
  static float y = 0.0f;
  x += 0.0001f;
  y += 0.0001f;

  glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  for (const auto mainboard : mainboards) {
    //if (mainboard->main_display->redrawRequested) {
      //mainboard->main_display->redrawRequested = false;
      screen.render(mainboard->main_display, x, y);
    //}
  } 
  SwapBuffers(hDC);
}

void MainWindow::PxCPUThread(PxMainboard* mainboard) {
  while (running) {
    mainboard->tick();
  }

  PxCPU* cpu = mainboard->cpus.at(0);
  int cycles = cpu->getCycles();
  float runningTime = static_cast<float>(cpu->getRunningTime() / 1000.0f);

  std::cout << "Cycles executed: " << cycles << " in " << runningTime << std::endl;
}

void MainWindow::run(MainScreen& screen, std::vector<PxMainboard*> mainboards) {
  bool multi = true;
  std::vector<std::thread> executionThreads;
  running = true;

  if (multi) {
    for (PxMainboard* mainboard : mainboards) {
      std::thread mainboardThread(&MainWindow::PxCPUThread, this, mainboard);
      executionThreads.push_back(std::move(mainboardThread));
    }
  }

  MSG msg = {};
  while (msg.message != WM_QUIT) {

    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    else {
      if (!multi) tick(mainboards);
      redraw(screen, mainboards); // this is uhhh.. somewhat questionable.
    }

  }

  running = false;
  if (multi) {
    for (auto& thread : executionThreads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }
  executionThreads.clear();
}
