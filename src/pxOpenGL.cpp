#include "pxOpenGL.h"

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

void initalizeOpenGlFunctionPointers() {
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