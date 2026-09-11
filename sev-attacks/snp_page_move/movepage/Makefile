obj-m := kmod_movepage.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all: module movepage

module: 
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f movepage

.PHONY: all module clean movepage

movepage: movepage.c
	gcc $(CFLAGS) -o movepage movepage.c

