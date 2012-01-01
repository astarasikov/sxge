#ifndef __SXGE_OPENGL_WINDOW_EGL_X11_HH__
#define __SXGE_OPENGL_WINDOW_EGL_X11_HH__

#include "opengl/gl_common.hh"
#include "util/log.h"
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

namespace sxge {

void printGLInfo(void) {
	auto renderer = glGetString(GL_RENDERER);
	auto vendor = glGetString(GL_VENDOR);
	auto version = glGetString(GL_VERSION);
	auto glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	printf("OpenGL Vendor %s\n", vendor);
	printf("OpenGL Renderer %s\n", renderer);
	printf("OpenGL Version %s\n", version);
	printf("GLSL Version %s\n", glslVersion);
}

static EGLConfig MakeEGLConfig(EGLDisplay display) {
	EGLConfig config;
	EGLint attributes[32] = {
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_ALPHA_SIZE, EGL_DONT_CARE,
		EGL_STENCIL_SIZE, EGL_DONT_CARE,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE,
	};
	EGLint nconfig;

	if (!eglGetConfigs(display, NULL, 0, &nconfig)) {
		err("failed to get number of EGL configs");
	}

	if (!eglChooseConfig(display, attributes, &config, 1, &nconfig)
		|| !nconfig) {
		err("failed to choose an EGL config");
	}
	return config;
}

class EGL_X11_Window {
public:
	EGL_X11_Window() : window(0) {
		Window root;
		XVisualInfo *visInfo, visTmp;
		int numVisuals;
		EGLint eglVisualId;
		EGLConfig config;
		XSetWindowAttributes xattr;
		unsigned long mask;

		xDisplay = XOpenDisplay(NULL);
		if (!xDisplay) {
			err("failed to open native X11 xDisplay");
		}

		eglDisplay = eglGetDisplay(xDisplay);
		if (eglDisplay == EGL_NO_DISPLAY) {
			err("failed to get EGL display");
		}

		EGLint major, minor;
		if (!eglInitialize(eglDisplay, &major, &minor)) {
			err("failed to initialize display %p", eglDisplay);
		}

		config = MakeEGLConfig(eglDisplay);

		if (!eglGetConfigAttrib(eglDisplay, config, 
			EGL_NATIVE_VISUAL_ID, &eglVisualId))
		{
			err("failed to get EGL visual id");
		}
	
		visTmp.visualid = eglVisualId;
		visInfo = XGetVisualInfo(xDisplay, VisualIDMask, &visTmp, &numVisuals);
		if (!visInfo) {
			err("failed to get visual info");
		}

		root = RootWindow(xDisplay, DefaultScreen(xDisplay));
		if (!root) {
			err("no root window");
		}

		memset(&xattr, 0, sizeof(xattr));
		xattr.colormap = XCreateColormap(xDisplay, root,
			visInfo->visual, AllocNone);
		xattr.event_mask = StructureNotifyMask | ExposureMask | KeyPressMask;
		mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;

		window = XCreateWindow(xDisplay, root, 0, 0, 320, 240,
			0, visInfo->depth, InputOutput, visInfo->visual, mask, &xattr);

		if (!window) {
			err("failed to create X11 window");
		}

		XFree(visInfo);

		XMapWindow(xDisplay, window);
		XSetStandardProperties(xDisplay, window, "SXGE", "SXGE",
			None, 0, 0, 0);

		context = eglCreateContext(eglDisplay, config, 0, 0); 
		if (EGL_NO_CONTEXT == context) {
			err("failed to create EGL context");
		}

		surface = eglCreateWindowSurface(eglDisplay, config,
			window, NULL);

		if (EGL_NO_SURFACE == surface) {
			err("failed to create EGL surface");
		}

		if (!eglMakeCurrent(eglDisplay, surface, surface, context)) {
			err("failed to make surface current");
		}

		printGLInfo();
		sleep(1);
	}
	virtual ~EGL_X11_Window() {
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(eglDisplay, context);
		eglTerminate(eglDisplay);
		if (window) {
			XDestroyWindow(xDisplay, window);
		}
		if (xDisplay) {
			XCloseDisplay(xDisplay);
		}
	}
protected:
	EGLNativeDisplayType xDisplay;
	EGLDisplay eglDisplay;
	EGLContext context;
	EGLSurface surface; 
	Window window;
};

}; //namespace sxge

#endif //__SXGE_OPENGL_WINDOW_EGL_X11_HH__
