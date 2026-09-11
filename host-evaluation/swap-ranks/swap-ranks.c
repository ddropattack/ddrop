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

#define START_PFN         (RESERVED_START >> PAGE_SHIFT)
#define END_PFN           (RESERVED_END   >> PAGE_SHIFT)

int main(int argc, char *argv[]) {
  unsigned char* buf = NULL;
  unsigned long long start_pa = 0;
  unsigned long long end_pa = 0;

	printf("Opening driver\n");
	if (open_kmod()) goto error;

  buf = (unsigned char*)malloc(ALLOC_SIZE);
	if (!buf) goto error;

  if (argc != 2) {
    printf("Error: specify physical address to translate\n");
    goto error;
  }

  start_pa = strtoull(argv[1], NULL, 16);
  end_pa = start_pa + 0x1000;

  page_stats_t stats;

  printf("Writing address to cache line, range pa=%llx to pa=%llx.\n", start_pa, end_pa);
  
  int buffer_size = 4096;
  int cl_size = 64;

  memset(buf, 0, ALLOC_SIZE);

  printf("Enable interposer to continue..."); getchar();

  for (unsigned long long cpa = start_pa; cpa < end_pa; cpa += buffer_size) {
    unsigned long long ccpa = cpa;
    for (int j = 0; j < buffer_size; j += cl_size) {
      memcpy(buf + j, &ccpa, sizeof(unsigned long long));
      ccpa += cl_size;
    }

    if( memcpy_topa(cpa, buf, ALLOC_SIZE, &stats, true) ) {
      goto error;
    }
  }

  printf("Disable interposer to continue..."); getchar();
  
  memset(buf, 0, ALLOC_SIZE);

  for (unsigned long long cpa = start_pa; cpa < end_pa; cpa += buffer_size) {
    memset(buf, 0, ALLOC_SIZE);
    if( memcpy_frompa(buf, cpa, ALLOC_SIZE, &stats, true) ) {
      goto error;
    }

    for (int j = 0; j < buffer_size; j += cl_size) {
      unsigned long long ccpa;

      memcpy(&ccpa, buf + j, sizeof(unsigned long long));

      if (ccpa != cpa + j) {
        printf("Found potential alias at %llx: found %llx\n", cpa + j, ccpa);
      }
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

