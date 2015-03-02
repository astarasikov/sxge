[SXGE](https://github.com/astarasikov/sxge) - Simple Cross-Platform OpenGL ES Engine
====================================================================================

What is it?
-----------
It is my attempt at implementing a simple game engine in OpenGL ES 2.0 without using deprecated fixed-function calls. The primary idea was to keep the engine header-only and make platform-specific code as tiny as possible.

Supported platforms:
 * Android (OpenGL ES 2.0)
 * Mac OS X (OpenGL 3.2 Core Profile)
 * Linux (EGL/X11 - OpenGL ES 2.0)

Implemented features:
 * Vector math library
 * Texturing
 * Lighting
 * Printing backtrace on SIGSEGV

Screenshots

![osx](https://github.com/astarasikov/sxge/blob/master/doc/screenshot_osx.png)

![android](https://github.com/astarasikov/sxge/blob/master/doc/screenshot_android.png)
