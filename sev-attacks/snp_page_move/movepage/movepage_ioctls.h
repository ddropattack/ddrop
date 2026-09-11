#ifndef MOVEPAGE_IOCTLS_H
#define MOVEPAGE_IOCTLS_H

#ifdef __KERNEL__
#include <linux/types.h>
#else
#include <stdint.h>
#endif

struct args {
  int        pid;
  uint64_t   gpa_src;
  uint64_t   hpa_dst;
};

#define PAGE_MOVE      _IOW('f', 0x20, struct args*)

#endif
