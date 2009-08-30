.PHONY: clean rebuild install


CFLAGS:=-pipe -O3 -I. \
        -Wall -Wshadow -pedantic-errors \
        -Wpointer-arith -Waggregate-return \
        -Wlong-long -Winline -Wredundant-decls -Wcast-qual -Wcast-align \
        -D__STRICT_ANSI__ -DNDEBUG -DHAVE_TIFF

LDFLAGS:=-ltiff

C_OPTS:=-Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations


OBJDIR:=obj
SUBDIRS:=src \
         minidjvu \
         minidjvu/base \
         minidjvu/alg \
         minidjvu/alg/patterns \
         minidjvu/jb2 \
         minidjvu/formats

THISFILE:=Makefile

CSOURCES:=$(wildcard $(addsuffix /*.c,$(SUBDIRS)))
CPPSOURCES:=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS)))
HEADERS:=$(wildcard $(addsuffix /*.h,$(SUBDIRS)))
COBJECTS:=$(addprefix $(OBJDIR)/,$(CSOURCES:.c=.o))
CPPOBJECTS:=$(addprefix $(OBJDIR)/,$(CPPSOURCES:.cpp=.o))
OBJECTS:=$(COBJECTS) $(CPPOBJECTS)
CC:=gcc
CXX:=g++


ifeq ($(shell uname -o),Cygwin)
    TARGET:=bin/minidjvu.exe
else
    TARGET:=bin/minidjvu
endif


$(TARGET): $(OBJECTS)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) $^ -o $@


clean:
	rm -f $(OBJECTS)

rebuild: clean $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin

$(OBJDIR)/%.o: %.c $(HEADERS) $(THISFILE)
	@mkdir -p $(dir $@)
	$(CC) $(C_OPTS) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cpp $(HEADERS) $(THISFILE)
	@mkdir -p $(dir $@)
	$(CXX) $(CFLAGS) -c $< -o $@
