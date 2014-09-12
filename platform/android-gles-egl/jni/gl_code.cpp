#include <jni.h>
#include <android/log.h>

#define GL_GLEXT_PROTOTYPES 1

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <sxge/apps/apps.hh>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define LOG_TAG "libsxge"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static sxge::Screen *screen = NULL;

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArrays;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArray;

void
sxge_log(enum sxge_log_level level, const char *format, ...)
{
	enum {
		SXGE_LOG_SIZE = 512,
	};
	char buf[SXGE_LOG_SIZE];
	va_list args;
	va_start(args, format);
	int count = vsnprintf(buf, SXGE_LOG_SIZE, format, args);
	va_end(args);

	if (count >= 0 && count < SXGE_LOG_SIZE) {
		__android_log_print(ANDROID_LOG_INFO, LOG_TAG, "%s", buf);
	}
}

bool setupGraphics(int w, int h)
{
	if (!screen) {
		glGenVertexArrays = (PFNGLGENVERTEXARRAYSOESPROC) eglGetProcAddress("glGenVertexArraysOES");
		glBindVertexArray = (PFNGLBINDVERTEXARRAYOESPROC) eglGetProcAddress("glBindVertexArrayOES");
		screen = new sxge::Demo1_Cube();
		screen->init();
	}
	screen->reshape(w, h);
    return true;
}

void renderFrame()
{
	if (screen) {
		screen->display();
	}
}

extern "C" {
JNIEXPORT void JNICALL
    Java_io_github_astarasikov_sxge_SxgeLib_init(JNIEnv* env, jobject obj, jint width, jint height);

JNIEXPORT void JNICALL
    Java_io_github_astarasikov_sxge_SxgeLib_step(JNIEnv* env, jobject obj);
};

JNIEXPORT void JNICALL
    Java_io_github_astarasikov_sxge_SxgeLib_init(JNIEnv* env, jobject obj, jint width, jint height)
{
    setupGraphics(width, height);
}

JNIEXPORT void JNICALL
    Java_io_github_astarasikov_sxge_SxgeLib_step(JNIEnv* env, jobject obj)
{
    renderFrame();
}
