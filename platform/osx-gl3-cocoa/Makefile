APPNAME=sxge
CC=clang
CXX=clang++
SXGE_TOPDIR=$(shell pwd)/../..
DEFINES=-DSXGE_USE_OPENGL -DSXGE_TOPDIR=\"$(SXGE_TOPDIR)\" -I$(SXGE_TOPDIR)/libsxge/include -I$(SXGE_TOPDIR)/apps/include -I$(shell pwd)/include
WOPTS=-Wall -Wpedantic
OPTIMIZATION_FLAGS=-pg -O0

ifeq ($(WITH_SANITIZER),1)
OPTIMIZATION_FLAGS=-g3 -Os -fno-omit-frame-pointer -fno-inline-functions -fno-inline
LOCAL_LLVM_BUILD?=please_set_the_path_to_llvm
CC=$(LOCAL_LLVM_BUILD)/bin/clang
CXX=$(LOCAL_LLVM_BUILD)/bin/clang++
DEFINES+=-I/Applications/Xcode.app/Contents//Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/c++/v1
DEFINES+=-lLLVMInstrumentation -L$(LOCAL_LLVM_BUILD)/lib/
DEFINES+=-L$(LOCAL_LLVM_BUILD)/lib/clang/3.6.0/lib/darwin/ -lclang_rt.asan_osx_dynamic -lc++ -lc++abi
DEFINES+=-Xlinker -rpath -Xlinker $(LOCAL_LLVM_BUILD)/lib
WOPTS+=-fsanitize=address
endif

COMMON_FLAGS=$(OPTIMIZATION_FLAGS) $(DEFINES) $(WOPTS)
C_FLAGS=-std=c99 $(COMMON_FLAGS)
MM_FLAGS=-std=c++11 $(COMMON_FLAGS)

LDFLAGS=-framework Cocoa \
	-framework CoreVideo \
	-framework OpenGL

MM_FILES = \
	sxge.mm \
	sxge_gl_controller.mm \
	opengl_view.mm

CC_FILES = \
	$(SXGE_TOPDIR)/apps/src/sxge/apps/demo1_cube/demo1_cube.cc

C_FILES = \
	$(SXGE_TOPDIR)/libsxge/src/sxge/util/log.c \
	$(SXGE_TOPDIR)/libsxge/src/sxge/util/signals.c

C_OBJ_FILES=$(patsubst %.c,%.o,$(C_FILES))
CC_OBJ_FILES=$(patsubst %.cc,%.o,$(CC_FILES))
MM_OBJ_FILES=$(patsubst %.mm,%.o,$(MM_FILES))

all: $(APPNAME)

$(APPNAME): $(MM_OBJ_FILES) $(CC_OBJ_FILES) $(C_OBJ_FILES)
	$(CXX) $(LDFLAGS) $(MM_FLAGS) -o $@ $(MM_OBJ_FILES) $(CC_OBJ_FILES) $(C_OBJ_FILES)

$(MM_OBJ_FILES): %.o: %.mm
	$(CXX) $(MM_FLAGS) -c $< -o $@

$(CC_OBJ_FILES): %.o: %.cc
	$(CXX) $(MM_FLAGS) -c $< -o $@

$(C_OBJ_FILES): %.o: %.c
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	find ../../ -name '*.o' -delete || true
	rm *.o $(APPNAME) || true

reindent:
	clang-format -i -style=WebKit $(C_FILES) $(MM_FILES)

run:
	make clean
	make all
	dsymutil $(APPNAME)
	cp $(APPNAME) $(APPNAME).app/Contents/MacOS/$(APPNAME) || true
	./$(APPNAME).app/Contents/MacOS/$(APPNAME)