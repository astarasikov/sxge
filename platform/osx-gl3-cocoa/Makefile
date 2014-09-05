APPNAME=gl3
CC=clang++
SXGE_TOPDIR=$(shell pwd)/../../
DEFINES=-DSXGE_USE_OPENGL -I$(SXGE_TOPDIR)/libsxge
WOPTS=-Wall -Wpedantic
CFLAGS=-std=c++11 -pg -O0 $(DEFINES) $(WOPTS)
LDFLAGS=-framework Cocoa \
	-framework CoreVideo \
	-framework OpenGL

CFILES = \
	sxge.mm \
	sxge_gl_controller.mm \
	opengl_view.mm

OBJFILES=$(patsubst %.mm,%.o,$(CFILES))

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(OBJFILES)

$(OBJFILES): %.o: %.mm
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(APPNAME)
	rm *.o

run:
	make clean
	make all
	cp $(APPNAME) $(APPNAME).app/Contents/MacOS/$(APPNAME)
	#open $(APPNAME).app
	./$(APPNAME)
