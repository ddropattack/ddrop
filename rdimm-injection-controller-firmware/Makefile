# Teensyduino Core Library
# http://www.pjrc.com/teensy/
# Copyright (c) 2019 PJRC.COM, LLC.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# 1. The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# 2. If the Software is incorporated into a build system that allows
# selection among a list of target devices, then similar target
# devices manufactured by PJRC.COM must be included in the list of
# target devices and selectable in the same manner.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Use these lines for Teensy 4.0
#MCU = IMXRT1062
#MCU_LD = imxrt1062.ld
#MCU_DEF = ARDUINO_TEENSY40

# Use these lines for Teensy 4.1
MCU = IMXRT1062
MCU_LD = imxrt1062_t41.ld
MCU_DEF = ARDUINO_TEENSY41

# The name of your project (used to name the compiled .hex file)
TARGET = main

# configurable options
OPTIONS = -DF_CPU=600000000 -DUSB_RAWHID -DLAYOUT_US_ENGLISH -DUSING_MAKEFILE
#
# USB Type configuration:
#   -DUSB_SERIAL
#   -DUSB_DUAL_SERIAL
#   -DUSB_TRIPLE_SERIAL
#   -DUSB_KEYBOARDONLY
#   -DUSB_TOUCHSCREEN
#   -DUSB_HID_TOUCHSCREEN
#   -DUSB_HID
#   -DUSB_SERIAL_HID
#   -DUSB_MIDI
#   -DUSB_MIDI4
#   -DUSB_MIDI16
#   -DUSB_MIDI_SERIAL
#   -DUSB_MIDI4_SERIAL
#   -DUSB_MIDI16_SERIAL
#   -DUSB_AUDIO
#   -DUSB_MIDI_AUDIO_SERIAL
#   -DUSB_MIDI16_AUDIO_SERIAL
#   -DUSB_MTPDISK
#   -DUSB_RAWHID
#   -DUSB_FLIGHTSIM
#   -DUSB_FLIGHTSIM_JOYSTICK

# options needed by many Arduino libraries to configure for Teensy model
OPTIONS += -D__$(MCU)__ -DARDUINO=10607 -DTEENSYDUINO=157 -D$(MCU_DEF)

# for Cortex M7 with single & double precision FPU
CPUOPTIONS = -mcpu=cortex-m7 -mfloat-abi=hard -mfpu=fpv5-d16 -mthumb

# use this for a smaller, no-float printf
#SPECS = --specs=nano.specs

# Other Makefiles and project templates for Teensy
#
# https://forum.pjrc.com/threads/57251?p=213332&viewfull=1#post213332
# https://github.com/apmorton/teensy-template
# https://github.com/xxxajk/Arduino_Makefile_master
# https://github.com/JonHylands/uCee


#************************************************************************
# Location of Teensyduino utilities, Toolchain, and Arduino Libraries.
# To use this makefile without Arduino, copy the resources from these
# locations and edit the pathnames.  The rest of Arduino is not needed.
#************************************************************************

# Those that specify a NO_ARDUINO environment variable will
# be able to use this Makefile with no Arduino dependency.
# Please note that if ARDUINOPATH was set, it will override
# the NO_ARDUINO behaviour.
ifndef NO_ARDUINO
# Path to your arduino installation
ARDUINOPATH ?= $(HOME)/.arduino15
endif

ifndef TOOLS_VERSION
	TOOLS_VERSION=$(shell ls $(ARDUINOPATH)/packages/teensy/tools/teensy-compile/ 2>/dev/null)
endif

ifndef COMPILER_VERSION
	COMPILER_VERSION=$(shell ls $(ARDUINOPATH)/packages/teensy/tools/teensy-compile/ 2>/dev/null)
endif

ifdef ARDUINOPATH

# path location for Teensy Loader, teensy_post_compile and teensy_reboot (on Linux)
TOOLSPATH = $(abspath $(ARDUINOPATH)/packages/teensy/tools/teensy-tools/$(TOOLS_VERSION))

# path location for Arduino libraries (currently not used)
LIBRARYPATH = $(abspath $(ARDUINOPATH)/libraries)

# path location for the arm-none-eabi compiler
COMPILERPATH = $(abspath $(ARDUINOPATH)/packages/teensy/tools/teensy-compile/$(COMPILER_VERSION)/arm/bin)

else
# Default to the normal GNU/Linux compiler path if NO_ARDUINO
# and ARDUINOPATH was not set.
COMPILERPATH ?= /usr/bin
TOOLSPATH ?= /usr/bin

endif

#************************************************************************
# Settings below this point usually do not need to be edited
#************************************************************************

# CPPFLAGS = compiler options for C and C++
CPPFLAGS = -Wall -g -O2 $(CPUOPTIONS) -MMD $(OPTIONS) -I. -Icores \
	   -Icores/libraries/NativeEthernet/src\
	   -Icores/libraries/FNET/src\
	   -Icores/libraries/SPI\
	   -Icores/libraries/TeensyThreads\
	   -ffunction-sections -fdata-sections\
		 $(FLAGS)

# compiler options for C++ only
CXXFLAGS = -std=gnu++14 -felide-constructors -fno-exceptions \
	   -fpermissive -fno-rtti -fno-threadsafe-statics -Wno-error=narrowing

# compiler options for C only
CFLAGS = -std=gnu11

# linker options
LDFLAGS = -Os -Wl,--gc-sections,--relax $(SPECS) $(CPUOPTIONS) -T$(MCU_LD)

# additional libraries to link
LIBS = -larm_cortexM7lfsp_math -lm -lstdc++

# names for the compiler programs
CC = $(COMPILERPATH)/arm-none-eabi-gcc
CXX = $(COMPILERPATH)/arm-none-eabi-g++
AR = $(COMPILERPATH)/arm-none-eabi-gcc-ar
OBJCOPY = $(COMPILERPATH)/arm-none-eabi-objcopy
SIZE = $(COMPILERPATH)/arm-none-eabi-size

# automatically create lists of the sources and objects
# TODO: this does not handle Arduino libraries yet...
C_FILES := $(wildcard src/*.c)
CPP_FILES := $(wildcard src/*.cpp)
OBJS := $(C_FILES:.c=.c.o) $(CPP_FILES:.cpp=.cpp.o)

C_FILES_CORE := $(wildcard cores/*.c)
CPP_FILES_CORE := $(wildcard cores/*.cpp)
ASM_FILES_CORE := $(wildcard cores/*.S)
OBJS_CORE := $(C_FILES_CORE:.c=.c.o) $(CPP_FILES_CORE:.cpp=.cpp.o) $(ASM_FILES_CORE:.S=.S.o)

BUILD_PATH := build

# the actual makefile rules (all .o files built by GNU make's default implicit rules)

C_FILES_LIBS := $(wildcard cores/libraries/NativeEthernet/src/*.c)
CPP_FILES_LIBS := $(wildcard cores/libraries/NativeEthernet/src/*.cpp)

C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/port/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/port/cpu/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/port/cpu/mimxrt/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/port/netif/fec/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/autoip/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/azure/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/bench/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/dhcp/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/dns/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/flash/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/fs/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/http/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/link/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/llmnr/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/mdns/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/ping/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/serial/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/shell/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/sntp/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/telnet/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/tftp/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/autoip/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/service/tls/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/stack/*.c)
C_FILES_LIBS += $(wildcard cores/libraries/FNET/src/third_party/mbedtls-2.12.0/src/*.c)

ASM_FILES_LIBS := $(wildcard cores/libraries/TeensyThreads/*.S)

CPP_FILES_LIBS += $(wildcard cores/libraries/FNET/src/*.cpp)
CPP_FILES_LIBS += $(wildcard cores/libraries/TeensyThreads/*.cpp)

OBJS_LIBS := cores/libraries/SPI/SPI.o
OBJS_LIBS += $(C_FILES_LIBS:.c=.c.o) $(CPP_FILES_LIBS:.cpp=.cpp.o) $(ASM_FILES_LIBS:.S=.S.o)

.PHONY: clean clean_libs clean_hex

all: $(BUILD_PATH) $(BUILD_PATH)/$(TARGET).hex

upload: clean_hex all


$(BUILD_PATH): 
	mkdir -p $(BUILD_PATH)/src
	mkdir -p $(BUILD_PATH)/cores/libraries/SPI
	mkdir -p $(BUILD_PATH)/cores/libraries/NativeEthernet/src
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/port/cpu/mimxrt
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/port/netif/fec
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/autoip
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/azure
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/bench
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/dhcp
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/dns
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/flash
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/fs
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/http
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/link
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/llmnr
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/mdns
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/ping
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/serial
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/shell
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/sntp
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/telnet
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/tftp
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/tls
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/service/shell
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/stack
	mkdir -p $(BUILD_PATH)/cores/libraries/FNET/src/third_party/mbedtls-2.12.0/src/mbedtls
	mkdir -p $(BUILD_PATH)/cores/libraries/TeensyThreads

$(BUILD_PATH)/cores/libraries/SPI/SPI.o: \
	cores/libraries/SPI/SPI.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) cores/libraries/SPI/SPI.cpp\
		-o $(BUILD_PATH)/cores/libraries/SPI/SPI.o

$(BUILD_PATH)/cores/libraries/NativeEthernet/src/%.cpp.o: \
	cores/libraries/NativeEthernet/src/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_PATH)/cores/libraries/NativeEthernet/src/%.c.o: \
	cores/libraries/NativeEthernet/src/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_PATH)/cores/libraries/FNET/src/port/%.c.o: \
	cores/libraries/FNET/src/port/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_PATH)/libs.a: $(patsubst %, build/%, $(OBJS_LIBS))
	$(AR) rcs $(BUILD_PATH)/libs.a $(patsubst %, build/%, $(OBJS_LIBS))

$(BUILD_PATH)/cores/%.cpp.o: cores/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_PATH)/cores/%.c.o: cores/%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

$(BUILD_PATH)/cores/%.S.o: cores/%.S
	$(CC) -c $(CFLAGS) -x assembler-with-cpp $(CPPFLAGS) $< \
		-o $@

$(BUILD_PATH)/core.a: $(patsubst %, $(BUILD_PATH)/%, $(OBJS_CORE))
	$(AR) rcs $(BUILD_PATH)/core.a $(patsubst %, $(BUILD_PATH)/%, $(OBJS_CORE))

$(BUILD_PATH)/src/%.cpp.o: src/%.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

$(BUILD_PATH)/$(TARGET).elf: $(BUILD_PATH)/libs.a $(BUILD_PATH)/core.a \
	$(patsubst %, $(BUILD_PATH)/%, $(OBJS)) $(MCU_LD)
	$(CXX) $(LDFLAGS) -o $@ \
		$(patsubst %, $(BUILD_PATH)/%, $(OBJS))\
		$(BUILD_PATH)/libs.a \
		$(BUILD_PATH)/core.a $(LIBS)

$(BUILD_PATH)/%.hex: $(BUILD_PATH)/%.elf
	$(SIZE) $<
	$(OBJCOPY) -O ihex -R .eeprom $< $@
ifneq (,$(wildcard $(TOOLSPATH)/teensy_post_compile*))
	-$(TOOLSPATH)/teensy_post_compile -file=$(basename $@)\
		-path=$(shell pwd) -tools=$(TOOLSPATH)
	-$(TOOLSPATH)/teensy_reboot
endif

# compiler generated dependency info
-include $(OBJS:.o=.d)

clean_libs:
	rm -rdf $(BUILD_PATH)

clean_hex:
	rm $(BUILD_PATH)/$(TARGET).hex

clean: clean_libs
	rm -f *.o core.a cores/*.o *.d $(TARGET).elf $(TARGET).hex




