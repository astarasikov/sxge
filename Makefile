APPNAME=sxge
CXX ?= g++
SXGE_TOPDIR=$(shell pwd)
CXFLAGS=-std=c++0x -I$(SXGE_TOPDIR) -O0 -Wall -Werror -Wextra -pg -lGLESv2

CXFILES = main.cc
OBJFILES = $(patsubst %.cc,%.o,$(CXFILES))

all: $(APPNAME)

$(APPNAME): $(OBJFILES)
	$(CXX) $(CXFLAGS) -o $@ $(OBJFILES)

$(OBJFILES): %.o: %.cc
	$(CXX) $(CXFLAGS) -c $< -o $@

clean:
	rm -f $(APPNAME)
	rm -f *.o
	rm -f gmon.out
