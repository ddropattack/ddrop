#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

#include <termios.h>

#include "readalias.h"
#include "helpers.h"


#define PAGE_SHIFT 12
#define ALLOC_SIZE 1*PAGE_SIZE
#define CL_SIZE 64

#define START_PFN         (RESERVED_START >> PAGE_SHIFT)
#define END_PFN           (RESERVED_END   >> PAGE_SHIFT)

int main(int argc, char *argv[]) {
  unsigned char* buf = NULL;

	printf("Opening driver\n");
	if (open_kmod()) goto error;

  buf = (unsigned char*)malloc(ALLOC_SIZE);
	if (!buf) goto error;

  for (int i = 0; i < ALLOC_SIZE; ++i) {
    buf[i] = i%256;
  }

  // A random physical address we would like to access
  unsigned long long original_pa = 0x1080008000;
  page_stats_t stats;

  
  if (argc != 2) {
    printf("Error: specify physical address to translate\n");
    goto error;
  }

  original_pa = strtoull(argv[1], NULL, 16);
  printf("Decoding address %llx\n", original_pa);

  struct pamemcpy_cfg memcpy_cfg = {
        .out_stats = {0},
        .access_reserved = false,
        .err_on_access_fail = true,
        .flush_method = FM_NONE,
  };

  memset(buf, 0x0, ALLOC_SIZE);

  memset(buf, 0xaa, CL_SIZE); 
  if( memcpy_topa(original_pa, buf, CL_SIZE, &stats, true) ) {
    goto error;
  }

  // Load the address to ensure it is cached
  if( memcpy_frompa(buf, original_pa, CL_SIZE, &stats, true) ) {
    goto error;
  }

  printf("\n\nSingle CL read (pa=%llx)\n", original_pa);

  printf("Setting victim address to 0xbb\n");
  memset(buf, 0xbb, CL_SIZE);

  // Write something, which will make sure the cache line is flushed
  if( memcpy_topa(original_pa, buf, CL_SIZE, &stats, true) ) {
    goto error;
  }

  printf("Enable interposer to continue..."); getchar();
  
  if( memcpy_frompa_ext(buf, original_pa, CL_SIZE, &memcpy_cfg) ) {
    goto error;
  }
  printf("Disable interposer to continue..."); getchar();
  printf("  Reading when dropped:\n"); hexdump(buf, CL_SIZE);

  goto cleanup;
error:
  printf("Error -- ");
cleanup:
  if (buf) free(buf);
  printf("Closing driver\n");
  close_kmod();
}

