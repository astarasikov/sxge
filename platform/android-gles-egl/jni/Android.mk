LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

APP_OPTIM := debug

LOCAL_MODULE    := libsxge_jni
LOCAL_CFLAGS    := -std=c++11 -g -ggdb -O0 -Wall \
	-DSXGE_TOPDIR=\"/sdcard/\" \
	-Ijni/include \
	-I../../libsxge/include \
	-I../../apps/include

LOCAL_SRC_FILES := gl_code.cpp \
	../../../apps/src/sxge/apps/demo1_cube/demo1_cube.cc

LOCAL_LDLIBS    := -llog -lGLESv2 -lEGL

include $(BUILD_SHARED_LIBRARY)
