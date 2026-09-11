CFLAGS ?= -O3 -std=gnu11 -Wall -Wextra -Werror

all: ddrop-victim

ddrop-victim: ddrop-victim.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f ddrop-victim

.PHONY: all clean
