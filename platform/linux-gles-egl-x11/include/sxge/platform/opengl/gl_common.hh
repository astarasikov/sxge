#ifndef __SXGE_PLATFORM_OPENGL_GL__COMMON_HH__
#define __SXGE_PLATFORM_OPENGL_GL__COMMON_HH__

#define GL_GLEXT_PROTOTYPES 1

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#define EGL_EGLEXT_PROTOTYPES 1
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#ifndef GL_INVALID_INDEX
#define GL_INVALID_INDEX ((GLuint)-1)
#endif

extern PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;

#endif //__SXGE_PLATFORM_OPENGL_GL__COMMON_HH__
