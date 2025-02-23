#pragma once

#include <windows.h>

#include <gl/GL.h>
#include <gl/GLU.h>

#ifndef GLchar
#define GLchar char
#endif

#ifndef GLsizeiptr
  #include <cstddef>
  #define GLsizeiptr ptrdiff_t
#endif

#ifndef PIXEL_OPENGL

  #define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
  #define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
  #define WGL_CONTEXT_PROFILE_MASK_ARB  0x9126
  #define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
  #define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002

  #define GL_CLAMP_TO_EDGE 0x812F
  #define GL_WRITE_ONLY 0x88B9
  #define GL_ARRAY_BUFFER 0x8892
  #define GL_ELEMENT_ARRAY_BUFFER 0x8893
  #define GL_STATIC_DRAW 0x88E4
  #define GL_LINK_STATUS 0x8B82
  #define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020

  #define GL_SHADING_LANGUAGE_VERSION 0x8B8C
  #define GL_VERTEX_SHADER 0x8B31
  #define GL_FRAGMENT_SHADER 0x8B30
  #define GL_COMPUTE_SHADER 0x91B9
  #define GL_COMPILE_STATUS 0x8B81

  #define GL_TEXTURE0                       0x84C0

  typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum texture);
  typedef GLuint(APIENTRY* PFNGLCREATESHADERPROC)(GLenum type);
  typedef void (APIENTRY* PFNGLSHADERSOURCEPROC)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
  typedef void (APIENTRY* PFNGLCOMPILESHADERPROC)(GLuint shader);
  typedef void (APIENTRY* PFNGLGETSHADERIVPROC)(GLuint shader, GLenum pname, GLint* params);
  typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC)(GLuint shader, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
  typedef void (APIENTRY* PFNGLDELETESHADERPROC)(GLuint shader);
  typedef GLuint(APIENTRY* PFNGLCREATEPROGRAMPROC)(void);
  typedef void (APIENTRY* PFNGLATTACHSHADERPROC)(GLuint program, GLuint shader);
  typedef void (APIENTRY* PFNGLLINKPROGRAMPROC)(GLuint program);
  typedef void (APIENTRY* PFNGLUSEPROGRAMPROC)(GLuint program);
  typedef void (APIENTRY* PFNGLGETPROGRAMIVPROC)(GLuint program, GLenum pname, GLint* params);
  typedef void (APIENTRY* PFNGLGETPROGRAMINFOLOGPROC)(GLuint program, GLsizei maxLength, GLsizei* length, GLchar* infoLog);
  typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC)(GLsizei n, GLuint* arrays);
  typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC)(GLuint array);
  typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
  typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
  typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
  typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC)(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
  typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC)(GLuint index);
  typedef void (APIENTRY* PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
  typedef void (APIENTRY* PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
  typedef void (APIENTRY* PFNGLMEMORYBARRIERPROC)(GLbitfield barriers);
  typedef GLint(APIENTRY* PFNGLGETUNIFORMLOCATIONPROC)(GLuint program, const GLchar* name);
  typedef void(APIENTRY* PFNGLUNIFORM1IPROC)(GLint location, GLint v0);
  typedef void(APIENTRY* PFNGLUNIFORM1FPROC)(GLint location, GLfloat v0);
  typedef void(APIENTRY* PFNGLUNIFORM2FPROC)(GLint location, GLfloat v0, GLfloat v1);

  extern PFNGLACTIVETEXTUREPROC glActiveTexture;
  extern PFNGLCREATESHADERPROC glCreateShader;
  extern PFNGLSHADERSOURCEPROC glShaderSource;
  extern PFNGLCOMPILESHADERPROC glCompileShader;
  extern PFNGLGETSHADERIVPROC glGetShaderiv;
  extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
  extern PFNGLDELETESHADERPROC glDeleteShader;
  extern PFNGLCREATEPROGRAMPROC glCreateProgram;
  extern PFNGLATTACHSHADERPROC glAttachShader;
  extern PFNGLLINKPROGRAMPROC glLinkProgram;
  extern PFNGLUSEPROGRAMPROC glUseProgram;
  extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
  extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
  extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
  extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
  extern PFNGLGENBUFFERSPROC glGenBuffers;
  extern PFNGLBINDBUFFERPROC glBindBuffer;
  extern PFNGLBUFFERDATAPROC glBufferData;
  extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
  extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
  extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
  extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
  extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;
  extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
  extern PFNGLUNIFORM1IPROC glUniform1i;
  extern PFNGLUNIFORM1FPROC glUniform1f;
  extern PFNGLUNIFORM2FPROC glUniform2f;

#endif
