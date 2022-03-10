########################
#
# Makefile
# (for GNU Make 3.81)
#
########################

# Byte Order
#   0 = undefined (use endian-safe code, might be slightly slower)
#   1 = Little Endian (Intel) [default]
#   2 = Big Endian (Motorola)
ifndef BYTE_ORDER
BYTE_ORDER = 1
endif

ifndef DEBUG
DEBUG = 0
endif

CC = gcc
ifndef PREFIX
PREFIX = /usr/local
#PREFIX = $(HOME)/.local
endif

# -- VGMPlay specific Compile Flags --
MAINFLAGS := -DCONSOLE_MODE -DADDITIONAL_FORMATS -DSET_CONSOLE_TITLE

# -- Byte Order Optimizations --
ifeq ($(BYTE_ORDER), 1)
# Intel Byte Order
MAINFLAGS += -DVGM_LITTLE_ENDIAN
EMUFLAGS += -DVGM_LITTLE_ENDIAN
else
ifeq ($(BYTE_ORDER), 2)
# Motorola Byte Order
MAINFLAGS += -DVGM_BIG_ENDIAN
EMUFLAGS += -DVGM_BIG_ENDIAN
else
# undefined byte order
endif
endif



ifeq ($(DEBUG), 0)
# -- General Compile Flags --
CFLAGS := -O3 -g0 -Wno-unused-variable -Wno-unused-value $(CFLAGS)
else
CFLAGS := -g -Wall $(CFLAGS)
endif
LDFLAGS := -lm -lz -lSDL2 -lSDL2_ttf -lpthread $(LDFLAGS)


EMUOBJS = \
	obj/src/chips/fm2612.o \
	obj/src/chips/2612intf.o

MAINOBJS = \
	obj/src/waffle.o \
	obj/src/waffle_ui.o \
	obj/src/dmf.o \
	obj/src/datareader.o
 

all:	waffle

waffle:	$(EMUOBJS) $(MAINOBJS)
	@echo Linking waffle ...
	@$(CXX) $(MAINOBJS) $(EMUOBJS) $(LDFLAGS) -o waffle
	@echo Done.


# compile the chip-emulator c-files
obj/src/chips/%.o:	src/chips/%.c
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(EMUFLAGS) -c $< -o $@

# compile the main c-files
obj/src/%.o:	src/%.cpp
	@echo Compiling $< ...
	@mkdir -p $(@D)
	@$(CXX) $(CXXFLAGS) $(MAINFLAGS) -c $< -o $@

clean:
	@echo Deleting object files ...
	@rm -f $(MAINOBJS) $(EMUOBJS)
	@echo Deleting executable files ...
	@rm -f waffle
	@echo Done.


.PHONY: all clean
