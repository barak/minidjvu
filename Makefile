# This is a Makefile that works on my machine.
# It uses libtool, and installs to /usr/local/lib and /usr/local/bin.

CFLAGS:=-pipe -g -I. \
        -Wall -Werror -Wshadow -pedantic-errors \
        -Wpointer-arith -Waggregate-return \
        -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations \
        -Wlong-long -Winline -Wredundant-decls -Wcast-qual -Wcast-align \
        -D__STRICT_ANSI__ # -fPIC
LDFLAGS:=-rpath /usr/local/lib -version-info 1:0:0 #-shared
OBJDIR:=obj
SUBDIRS:=minidjvu \
         minidjvu/base/0porting \
         minidjvu/base/1error \
         minidjvu/base/2io \
         minidjvu/base/3graymap \
         minidjvu/base/4bitmap \
         minidjvu/base/5image \
         minidjvu/alg/smooth \
         minidjvu/alg/split \
         minidjvu/alg/blitsort \
         minidjvu/alg/jb2 \
         minidjvu/formats/pbm \
         minidjvu/formats/bmp \
         minidjvu/formats/djvu
THISFILE:=Makefile

CSOURCES:=$(wildcard $(addsuffix /*.c,$(SUBDIRS)))
CPPSOURCES:=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS)))
HEADERS:=$(wildcard $(addsuffix /*.h,$(SUBDIRS)))
COBJECTS:=$(addprefix $(OBJDIR)/,$(CSOURCES:.c=_c.o))
CPPOBJECTS:=$(addprefix $(OBJDIR)/,$(CPPSOURCES:.cpp=_cpp.o))
OBJECTS:=$(COBJECTS) $(CPPOBJECTS)
CC:=libtool gcc
CXX:=libtool g++

.PHONY: all program install

all: bin/libminidjvu.la program

program:
	cd src && make

install: all
	libtool install -c bin/libminidjvu.la /usr/local/lib/libminidjvu.la
	libtool install -c bin/minidjvu /usr/local/bin/minidjvu
        
bin/libminidjvu.la: $(OBJECTS)
	mkdir -p $(dir $@)        
	$(CXX) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%_c.o: %.c $(HEADERS) $(THISFILE)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%_cpp.o: %.cpp $(HEADERS) $(THISFILE)
	mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@
