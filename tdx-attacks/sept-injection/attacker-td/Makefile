
CFLAGS= -O3 -std=gnu11 -Wall -Wextra -Wpedantic -Werror

OBJ_DIR = ./build/obj
BIN_DIR = ./build/binaries

INCS = $(wildcard *.h $(foreach fd, $(SUBDIR), $(fd)/*.h))
SRCS = $(wildcard *.c $(foreach fd, $(SUBDIR), $(fd)/*.c))

BADRAM_DIR ?= $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../../badram)
LIBCOMMON ?= $(BADRAM_DIR)/common-code
LIBKRA ?= $(BADRAM_DIR)/alias-reversing/modules/read_alias

INCLUDES=  -I$(LIBCOMMON)/include -I$(LIBKRA)/include
LIBS = -L$(LIBCOMMON)/build/libs -L$(LIBKRA)


all: setup-dirs $(BIN_DIR)/flip-debug-bit $(BIN_DIR)/change-mrtd $(BIN_DIR)/setup-sept-mapping
.PHONY: clean setup-dirs

#create output directores for build stuff
setup-dirs:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)

$(LIBCOMMON)/build/libs/libcommon.a:
	echo "Building libcommon.a"
	cd $(LIBCOMMON) && make all

$(LIBKRA)/libkmodreadalias.a:
	echo "Building libkmodreadalias.a"
	cd $(LIBKRA) && make libkmodreadalias.a

#build all objects files in this folder
$(OBJ_DIR)/%.o: %.c $(INCS)
	gcc $(CFLAGS) $(INCLUDES) -o $@ -c $<


$(BIN_DIR)/flip-debug-bit : $(OBJ_DIR)/flip-debug-bit.o $(LIBCOMMON)/build/libs/libcommon.a $(LIBKRA)/libkmodreadalias.a
	echo "Building file"
	gcc $(INCLUDES) $(LIBS) $(CFLAGS) -o $(BIN_DIR)/flip-debug-bit $^ -lcommon -lkmodreadalias

$(BIN_DIR)/change-mrtd : $(OBJ_DIR)/change-mrtd.o $(LIBCOMMON)/build/libs/libcommon.a $(LIBKRA)/libkmodreadalias.a
	echo "Building file"
	gcc $(INCLUDES) $(LIBS) $(CFLAGS) -o $(BIN_DIR)/change-mrtd $^ -lcommon -lkmodreadalias

$(BIN_DIR)/setup-sept-mapping : $(OBJ_DIR)/setup-sept-mapping.o $(LIBCOMMON)/build/libs/libcommon.a $(LIBKRA)/libkmodreadalias.a
	echo "Building file"
	gcc $(INCLUDES) $(LIBS) $(CFLAGS) -o $(BIN_DIR)/setup-sept-mapping $^ -lcommon -lkmodreadalias

clean:
	rm -rf ./build
	cd $(LIBCOMMON) && make clean
	cd $(LIBKRA) && make clean
