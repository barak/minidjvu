# Running this makefile will compile the "base" into `libmdjvubase.so'.

CFLAGS:=-fPIC -pipe -Wall -g -I.
LDFLAGS:=-shared
OBJDIR:=obj
SUBDIRS:=minidjvu \
         minidjvu/base/0porting \
         minidjvu/base/1io \
         minidjvu/base/2graymap \
         minidjvu/base/3bitmap \
         minidjvu/base/4image \
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
CC:=g++

.PHONY: all program

all: bin/libminidjvu.so program

program:
	cd src && make

bin/libminidjvu.so: $(OBJECTS)
	mkdir -p $(dir $@)        
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJDIR)/%_c.o: %.c $(HEADERS) $(THISFILE)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%_cpp.o: %.cpp $(HEADERS) $(THISFILE)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@
