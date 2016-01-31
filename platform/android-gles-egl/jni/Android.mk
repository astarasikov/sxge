LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

APP_OPTIM := debug
SXGE_TOPDIR := ../../../

LOCAL_MODULE    := libsxge_jni
LOCAL_CFLAGS    := -std=c++11 -g -ggdb -O0 -Wall \
	-DSXGE_TOPDIR=\"/sdcard/sxge\" \
	-Ijni/include \
	-I../../libsxge/include \
	-I../../apps/include \
	-I../../3rdparty/include

LOCAL_SRC_FILES := gl_code.cpp \
	$(SXGE_TOPDIR)/apps/src/sxge/apps/demo1_cube/demo1_cube.cc \
	$(SXGE_TOPDIR)/3rdparty/src/tiny_obj_loader/tiny_obj_loader.cc \
	$(SXGE_TOPDIR)/libsxge/src/sxge/opengl/program.cc \
	$(SXGE_TOPDIR)/libsxge/src/sxge/opengl/shader.cc \
	$(SXGE_TOPDIR)/libsxge/src/sxge/opengl/texture.cc \
	$(SXGE_TOPDIR)/libsxge/src/sxge/scene/model.cc

LOCAL_LDLIBS    := -llog -lGLESv2 -lEGL

include $(BUILD_SHARED_LIBRARY)
