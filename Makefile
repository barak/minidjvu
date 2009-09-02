prefix=/usr/local
exec_prefix=$(prefix)
bindir=$(exec_prefix)/bin

.PHONY: all clean install install-strip uninstall

CPPFLAGS += -D__STRICT_ANSI__ -DNDEBUG
CPPFLAGS += -I.

C_ALL_FLAGS = \
	-Wall -Wshadow -pedantic-errors \
	-Wpointer-arith -Waggregate-return \
	-Wlong-long -Wredundant-decls -Wcast-qual -Wcast-align

CFLAGS += -Wmissing-prototypes -Wstrict-prototypes -Wmissing-declarations

C_ALL_FLAGS += -O3

CPPFLAGS += -DHAVE_TIFF
LOADLIBES += -ltiff

CFLAGS += $(C_ALL_FLAGS)
CXXFLAGS += $(C_ALL_FLAGS)

LDFLAGS += -g

SUBDIRS=src \
	minidjvu \
	minidjvu/base \
	minidjvu/alg \
	minidjvu/alg/patterns \
	minidjvu/jb2 \
	minidjvu/formats

CSOURCES=$(wildcard $(addsuffix /*.c,$(SUBDIRS)))
CPPSOURCES=$(wildcard $(addsuffix /*.cpp,$(SUBDIRS)))
OBJECTS=$(CSOURCES:.c=.o) $(CPPSOURCES:.cpp=.o)

ifeq ($(shell uname -o),Cygwin)
    EXEC_SUFFIX=.exe
else
    EXEC_SUFFIX=
endif

TARGET_STEM=src/minidjvu
TARGET=$(TARGET_STEM)$(EXEC_SUFFIX)

all: $(TARGET)

$(TARGET): $(OBJECTS)

# force linkage as C++
$(TARGET): LINK.o=$(LINK.cc)


clean:
	$(RM) $(OBJECTS) $(TARGET)


INSTALL=install
INSTALL_PROGRAM=$(INSTALL)

install: $(TARGET)
	$(INSTALL_PROGRAM) -D $(TARGET) $(DESTDIR)$(bindir)/$(notdir $(TARGET))

install-strip:
	$(MAKE) INSTALL_PROGRAM='$(INSTALL_PROGRAM) -s' install

uninstall:
	$(RM) $(DESTDIR)$(bindir)/$(notdir $(TARGET))

# ensure that touching a .h file rebuilds appropriately
override CFLAGS += -MMD
override CXXFLAGS += -MMD
DFILES = $(OBJECTS:.o=.d)
-include $(DFILES)
