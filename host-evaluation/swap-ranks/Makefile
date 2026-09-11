
CFLAGS= -O3 -std=gnu11 -Wall -Wextra -Wpedantic -Werror

OBJ_DIR = ./build/obj
BIN_DIR = ./build/binaries

INCS = $(wildcard *.h $(foreach fd, $(SUBDIR), $(fd)/*.h))
SRCS = $(wildcard *.c $(foreach fd, $(SUBDIR), $(fd)/*.c))

BADRAM_DIR ?= $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/../../badram)
LIBCOMMON ?= $(BADRAM_DIR)/common-code
LIBKRA ?= $(BADRAM_DIR)/alias-reversing/modules/read_alias

INCLUDES=  -I$(LIBCOMMON)/include -I$(LIBKRA)/include
LIBS = -L$(LIBCOMMON)/build/libs -L$(LIBKRA)


all: setup-dirs $(BIN_DIR)/swap-ranks
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


$(BIN_DIR)/swap-ranks : $(OBJ_DIR)/swap-ranks.o $(LIBCOMMON)/build/libs/libcommon.a $(LIBKRA)/libkmodreadalias.a
	echo "Building file"
	gcc $(INCLUDES) $(LIBS) $(CFLAGS) -o $(BIN_DIR)/swap-ranks $^ -lcommon -lkmodreadalias


clean:
	rm -rf ./build
	cd $(LIBCOMMON) && make clean
	cd $(LIBKRA) && make clean
