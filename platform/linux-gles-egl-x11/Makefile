APPNAME=sxge
CC=clang
CXX=clang++
SXGE_TOPDIR=$(shell pwd)/../../
DEFINES=-DSXGE_USE_GLES -DSXGE_TOPDIR=\"$(SXGE_TOPDIR)\" -I$(SXGE_TOPDIR)/libsxge/include -I$(SXGE_TOPDIR)/apps/include -I$(shell pwd)/include
WOPTS=-Wall -Wextra -Wpedantic
OPTIMIZATION_FLAGS=-pg -O0
LDFLAGS=-lGLESv2 -lEGL -lX11

ifeq ($(WITH_SANITIZER),1)
OPTIMIZATION_FLAGS=-g3 -Os -fno-omit-frame-pointer -fno-inline-functions -fno-inline
WOPTS+=-fsanitize=address
endif

COMMON_FLAGS=$(OPTIMIZATION_FLAGS) $(DEFINES) $(WOPTS)
C_FLAGS=-std=c99 $(COMMON_FLAGS)
CC_FLAGS=-std=c++11 $(COMMON_FLAGS)

CC_FILES = \
	sxge.cc \
	$(SXGE_TOPDIR)/apps/src/sxge/apps/demo1_cube/demo1_cube.cc

C_FILES = \
	$(SXGE_TOPDIR)/libsxge/src/sxge/util/log.c \
	$(SXGE_TOPDIR)/libsxge/src/sxge/util/signals.c

C_OBJ_FILES=$(patsubst %.c,%.o,$(C_FILES))
CC_OBJ_FILES=$(patsubst %.cc,%.o,$(CC_FILES))

all: $(APPNAME)

$(APPNAME): $(CC_OBJ_FILES) $(C_OBJ_FILES)
	$(CXX) $(LDFLAGS) $(CC_FLAGS) -o $@ $(CC_OBJ_FILES) $(C_OBJ_FILES)

$(CC_OBJ_FILES): %.o: %.cc
	$(CXX) $(CC_FLAGS) -c $< -o $@

$(C_OBJ_FILES): %.o: %.c
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	find ../../ -name '*.o' -delete || true
	rm -f $(APPNAME) *.o gmon.out || true
