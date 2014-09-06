#ifndef __SXGE_PLATFORM_OPENGL_GL__COMMON_HH__
#define __SXGE_PLATFORM_OPENGL_GL__COMMON_HH__

#ifndef SXGE_USE_OPENGL
	#include <GLES2/gl2.h>
#else
	#define GL_GLEXT_PROTOTYPES
	#include <GL/gl.h>
	#include <GL/glext.h>
#endif
#include <EGL/egl.h>
#include <EGL/eglplatform.h>

#endif
