#ifndef __SXGE_OPENGL_WINDOW_EGL_X11_HH__
#define __SXGE_OPENGL_WINDOW_EGL_X11_HH__

#include "opengl/gl_common.hh"
#include "opengl/screen.hh"
#include "util/log.h"
#include <cstring>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

namespace sxge {

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
	#ifndef SXGE_USE_OPENGL
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	#else
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	#endif
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
	EGL_X11_Window(sxge::Screen &screen) : window(0), sxgeScreen(screen) {
		Window root;
		XVisualInfo *visInfo, visTmp;
		int numVisuals;
		EGLint eglVisualId;
		EGLConfig config;
		XSetWindowAttributes xattr;
		unsigned long mask;

		static const EGLint ctx_attrs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE,
		};

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
			None, NULL, 0, NULL);

		context = eglCreateContext(eglDisplay, config, EGL_NO_CONTEXT,
			ctx_attrs); 
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
		
		sxgeScreen.init();
		eventLoop();
	}
	virtual ~EGL_X11_Window() {
		eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(eglDisplay, context);
		eglDestroySurface(eglDisplay, surface);
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
	sxge::Screen &sxgeScreen;
	bool appRunning;

	//Not a single lookup table because X keycodes are sparse
	bool translateSpecial(unsigned keysym, Screen::SpecialKey &sk) {
		switch(keysym) {
			case XK_BackSpace:
				sk = Screen::SpecialKey::SK_BackSpace;
				return true;
			case XK_Tab:
				sk = Screen::SpecialKey::SK_Tab;
				return true;
			case XK_Return:
				sk = Screen::SpecialKey::SK_Return;
				return true;
			case XK_Delete:
				sk = Screen::SpecialKey::SK_Delete;
				return true;
			case XK_Escape:
				sk = Screen::SpecialKey::SK_Escape;
				return true;
			case XK_Left:
				sk = Screen::SpecialKey::SK_Left;
				return true;
			case XK_Right:
				sk = Screen::SpecialKey::SK_Right;
				return true;
			case XK_Down:
				sk = Screen::SpecialKey::SK_Down;
				return true;
			case XK_Up:
				sk = Screen::SpecialKey::SK_Up;
				return true;
			case XK_KP_Space:
				sk = Screen::SpecialKey::SK_Space;
				return true;
		}
		
		return false;
	}

	void translateKeyState(XKeyEvent *ev, Screen::KeyStatus &ks, bool isDown) {
		if (isDown) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Down));
		}

		if (ev->state & ControlMask) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Ctrl));
		}

		if (ev->state & ShiftMask) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Shift));
		}

		if (ev->state & Mod1Mask) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Mod1));
		}

		if (ev->state & Mod2Mask) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Mod2));
		}
		
		if (ev->state & Mod3Mask) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Mod3));
		}
		
		if (ev->state & Mod4Mask) {
			ks = (Screen::KeyStatus)(ks | (Screen::KS_Mod4));
		}
	}

	void keyPressEvent(XKeyEvent *ev, bool isDown) {
		Screen::SpecialKey sk;
		unsigned keycode, keysym;
		keycode = ev->keycode;
		keysym = XKeycodeToKeysym(xDisplay, keycode, 0);

		//XXX: add notification
		if (keysym == XK_Escape) {
			appRunning = false;
			return;
		}

		Screen::KeyStatus ks = Screen::KeyStatus::KS_Up;
		translateKeyState(ev, ks, isDown);

		if (translateSpecial(keysym, sk)) {
			sxgeScreen.keyEvent(0, sk, ks);
			return;
		}

		char keyChar;
		if (XLookupString(ev, &keyChar, sizeof(char), NULL, NULL)) {
			sxgeScreen.keyEvent(keyChar, Screen::SK_None, ks);
		}
	}

	void motionEvent(XMotionEvent *ev) {
		sxgeScreen.mouseEvent(ev->x, ev->y, sxge::Screen::MouseButton::NONE);
	}

	void eventLoop(void) {
		appRunning = true;
		while(appRunning) {
			while(XPending(xDisplay)) {
				XEvent xev;
				XNextEvent(xDisplay, &xev);
				switch (xev.type) {
					case KeyPress:
						keyPressEvent((XKeyEvent*)&xev, true);
						break;
					case KeyRelease:
						keyPressEvent((XKeyEvent*)&xev, false);
						break;
					case ConfigureNotify:
						sxgeScreen.reshape(xev.xconfigure.width,
							xev.xconfigure.height);
						break;
					case MotionNotify:
						motionEvent((XMotionEvent*)&xev);
						break;
				}
			}
			sxgeScreen.display();
			eglSwapBuffers(eglDisplay, surface);
		}
	}
};

}; //namespace sxge

#endif //__SXGE_OPENGL_WINDOW_EGL_X11_HH__
