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

extern PFNGLACTIVETEXTUREPROC glActiveTexture = nullptr;
extern PFNGLCREATESHADERPROC glCreateShader = nullptr;
extern PFNGLSHADERSOURCEPROC glShaderSource = nullptr;
extern PFNGLCOMPILESHADERPROC glCompileShader = nullptr;
extern PFNGLGETSHADERIVPROC glGetShaderiv = nullptr;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = nullptr;
extern PFNGLDELETESHADERPROC glDeleteShader = nullptr;
extern PFNGLCREATEPROGRAMPROC glCreateProgram = nullptr;
extern PFNGLATTACHSHADERPROC glAttachShader = nullptr;
extern PFNGLLINKPROGRAMPROC glLinkProgram = nullptr;
extern PFNGLUSEPROGRAMPROC glUseProgram = nullptr;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv = nullptr;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = nullptr;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = nullptr;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray = nullptr;
extern PFNGLGENBUFFERSPROC glGenBuffers = nullptr;
extern PFNGLBINDBUFFERPROC glBindBuffer = nullptr;
extern PFNGLBUFFERDATAPROC glBufferData = nullptr;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = nullptr;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture = nullptr;
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute = nullptr;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier = nullptr;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
extern PFNGLUNIFORM1IPROC glUniform1i = nullptr;
extern PFNGLUNIFORM1FPROC glUniform1f = nullptr;
extern PFNGLUNIFORM2FPROC glUniform2f = nullptr;

MainWindow::MainWindow(int width, int height, const std::string& title)
  : width(width), height(height), title(title), hWnd(nullptr), hDC(nullptr), hGLRC(nullptr) {
  initializeWindow();
  initializeOpenGL();
}

MainWindow::~MainWindow() {
  cleanup();
}

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

  initializeOpenGLExtensions();
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

  initalizeOpenGlFunctionPointers();
}


void MainWindow::initalizeOpenGlFunctionPointers() {
  glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");

  glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
  glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
  glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
  glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
  glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
  glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");

  glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
  glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
  glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
  glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
  glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
  glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");

  glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)wglGetProcAddress("glGenVertexArrays");
  glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)wglGetProcAddress("glBindVertexArray");

  glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
  glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
  glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
  glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
  glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");

  glBindImageTexture = (PFNGLBINDIMAGETEXTUREPROC)wglGetProcAddress("glBindImageTexture");
  glDispatchCompute = (PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute");
  glMemoryBarrier = (PFNGLMEMORYBARRIERPROC)wglGetProcAddress("glMemoryBarrier");

  glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
  glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
  glUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
  glUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");


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

void MainWindow::mouseClickDown(int x, int y) {
  screen->mouseClickDown(x, y);
}

void MainWindow::mouseClickUp(int x, int y) {
  screen->mouseClickUp(x, y);
}

void MainWindow::mouseMove(int x, int y) {
  screen->mouseMove(x, y);
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
      window->mouseClickDown(x, y);
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
      window->mouseClickUp(x, y);
      return 0;
    }
    case WM_MOUSEMOVE: {
      auto window = reinterpret_cast<MainWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
      int x = LOWORD(lParam);
      int y = HIWORD(lParam);

      if (window->screen->mouseOverPxDevice(x, y)) {
        SetCursor(LoadCursor(nullptr, IDC_HAND));
      } else {
        SetCursor(LoadCursor(nullptr, IDC_ARROW));
      }

      window->mouseMove(x, y);

      return 0;
    }


    case WM_CLOSE: {
      PostQuitMessage(0);
      return 0;
    }
  }

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
