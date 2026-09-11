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
  unsigned long long original_pa = 0;
  page_stats_t stats;

	printf("Opening driver\n");
	if (open_kmod()) goto error;

  buf = (unsigned char*)malloc(ALLOC_SIZE);
	if (!buf) goto error;
  
  if (argc != 2) {
    printf("Error: specify physical address to translate\n");
    goto error;
  }

  original_pa = strtoull(argv[1], NULL, 16);
  printf("Decoding address %llx\n", original_pa);

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

  printf("Enable interposer to continue..."); getchar();
  
  if( memcpy_topa(original_pa, buf, CL_SIZE, &stats, true) ) {
    goto error;
  }

  printf("Disable interposer to continue..."); getchar();
  
  if( memcpy_frompa(buf, original_pa, CL_SIZE, &stats, true) ) {
    goto error;
  }
  printf("  Reading directly:\n"); hexdump(buf, CL_SIZE);

  goto cleanup;
error:
  printf("Error -- ");
cleanup:
  if (buf) free(buf);
  printf("Closing driver\n");
  close_kmod();
}

