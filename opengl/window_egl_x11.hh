#ifndef __SXGE_OPENGL_WINDOW_EGL_X11_HH__
#define __SXGE_OPENGL_WINDOW_EGL_X11_HH__

#include "opengl/gl_common.hh"
#include "util/log.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

namespace sxge {

class EGL_X11_Window {
public:
	EGL_X11_Window() {
		err("%s", __func__);		
	}
	virtual ~EGL_X11_Window() {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(display, context);
	}
protected:
	EGLNativeDisplayType display;
	EGLContext context;
};

}; //namespace sxge

#endif //__SXGE_OPENGL_WINDOW_EGL_X11_HH__
