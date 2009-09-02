prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin

.PHONY: all clean install


CPPFLAGS += -D__STRICT_ANSI__ -DNDEBUG
CPPFLAGS += -I.

C_ALL_FLAGS = \
	-Wall -Wshadow -pedantic-errors \
        -Wpointer-arith -Waggregate-return \
        -Wlong-long -Winline -Wredundant-decls -Wcast-qual -Wcast-align

CFLAGS += -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations

C_ALL_FLAGS += -O3

CPPFLAGS += -DHAVE_TIFF
LOADLIBES += -ltiff

CFLAGS += $(C_ALL_FLAGS)
CXXFLAGS += $(C_ALL_FLAGS)

LDFLAGS += -g

SUBDIRS =src \
         minidjvu \
         minidjvu/base \
         minidjvu/alg \
         minidjvu/alg/patterns \
         minidjvu/jb2 \
         minidjvu/formats

CSOURCES=$(wildcard $(addsuffix /*.c,$(SUBDIRS)))
CPPSOURCES=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS)))
# HEADERS=$(wildcard $(addsuffix /*.h,$(SUBDIRS)))
COBJECTS=$(CSOURCES:.c=.o)
CPPOBJECTS=$(CPPSOURCES:.cpp=.o)
OBJECTS=$(COBJECTS) $(CPPOBJECTS)


ifeq ($(shell uname -o),Cygwin)
    EXEC_SUFFIX=.exe
else
    EXEC_SUFFIX=
endif

TARGET_STEM=src/minidjvu
TARGET=$(TARGET_STEM)$(EXEC_SUFFIX)

all: $(TARGET)

$(TARGET): $(OBJECTS)


# We are using the below explicit rule for $(TARGET) instead of
# the following:

 # force use of C++ compiler as linker even though immediate source is C
 # $(TARGET): CC=$(CXX)

# because the $(CC) would be dynamically scoped, thereby messing up the
# compiler used for C files in the dependencies.

$(TARGET): $(TARGET_STEM).o
	$(CXX) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $@


clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	mkdir --parents $(DESTDIR)$(bindir)
	cp $(TARGET) $(DESTDIR)$(bindir)/

# ensure the touching a .h file will rebuild appropriately
override CFLAGS += -MMD
override CXXFLAGS += -MMD
DFILES = $(OBJECTS:.o=.d)
-include $(DFILES)
