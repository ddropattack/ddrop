#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include <sys/mman.h>
#include <unistd.h>

#include "parse_pagemap.h"

typedef struct
{
    char *name;
    uint64_t vaddr;
} code_gadget_t;

static void hexdump(uint8_t* a, const size_t n)
{
    for(size_t i = 0; i < n; i++) {
    if (a[i]) printf("\x1b[31m%02x \x1b[0m", a[i]);
    else printf("%02x ", a[i]);
    if (i % 32 == 31) printf("\n");
  }
    printf("\n");
}

static void flush(void *p)
{
      asm volatile("clflush 0(%0)\n" : : "c"(p) : "rax");
      asm volatile("mfence\n");
}


void flush_buffer(uint8_t *b, const size_t n) {
  for (size_t i = 0; i < n; i += 64)
      flush(b + i);
}



int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    uint64_t *mem_buffer = (uint64_t *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    if (mem_buffer == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    memset(mem_buffer, 0, 4096);

    uint64_t *mem_buffer2 = (uint64_t *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    if (mem_buffer == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    memset(mem_buffer2, 0xaa, 4096);

    code_gadget_t gadgets[] = {
        {
            .name = "mem_buffer",
            .vaddr = (uint64_t)mem_buffer,
        },
    };

    pid_t pid = getpid();
    for (size_t i = 0; i < sizeof(gadgets) / sizeof(gadgets[0]); i++) {
        code_gadget_t *g = gadgets + i;
        uint64_t paddr;
        if (virt_to_phys_user(&paddr, pid, g->vaddr)) {
            printf("Failed to translate vaddr 0x%jx of gadget %s to paddr\n", g->vaddr, g->name);
            return -1;
        }
        printf("Allocated memory buffer of 4096 bytes\n");
        printf(" --> Guest virtual address:  0x%jx\n", g->vaddr);
        printf(" --> Guest physical address: 0x%jx\n", paddr);
    }

    printf("\nBuffer initialized to zero:\n");
    hexdump((uint8_t*)mem_buffer, 128);

    gadgets[0].name = "mem_buffer2";
    gadgets[0].vaddr = (uint64_t)mem_buffer2;

    for (size_t i = 0; i < sizeof(gadgets) / sizeof(gadgets[0]); i++) {
        code_gadget_t *g = gadgets + i;
        uint64_t paddr;
        if (virt_to_phys_user(&paddr, pid, g->vaddr)) {
            printf("Failed to translate vaddr 0x%jx of gadget %s to paddr\n", g->vaddr, g->name);
            return -1;
        }
        printf("Allocated data buffer of 4096 bytes, initialized with 0xaa\n");
        printf(" --> Guest virtual address:  0x%jx\n", g->vaddr);
        printf(" --> Guest physical address: 0x%jx\n", paddr);
    }

    printf("\nMove pages, then press enter\n");
    getchar();

    printf("Printing memory buffer\n");
    hexdump((uint8_t*)mem_buffer, 128);
}
