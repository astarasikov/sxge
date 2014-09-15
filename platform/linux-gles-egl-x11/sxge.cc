#include <cassert>

#include <sxge/util/signals.h>
#include <sxge/apps/apps.hh>

#include <sxge/platform/opengl/gl_common.hh>
#include <sxge/platform/opengl/window_egl_x11.hh>

//TODO: GLES header and code are duplicated for Android and GNU/Linux

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;

static void sxge_resolve_gles_extensions(void)
{
		assert(glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC)
			eglGetProcAddress("glGenVertexArraysOES"));
		assert(glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC)
			eglGetProcAddress("glBindVertexArrayOES"));
}

int main() {
	sxge_setup_posix_signals();
	sxge_resolve_gles_extensions();

	sxge::Demo1_Cube screen;
	sxge::EGL_X11_Window window(screen);
	return 0;
}
