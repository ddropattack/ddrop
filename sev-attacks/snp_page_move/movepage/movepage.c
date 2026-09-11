#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "movepage_ioctls.h"

#define err_log(fmt, ...) fprintf(stderr, "%s:%d : " fmt, __FILE__, __LINE__, ##__VA_ARGS__);

static int kmod_fd = -1;

int do_stroul(char *str, int base, uint64_t *result)
{
    (*result) = strtoul(str, NULL, base);
    // if commented in, we cannot enter zero, as uses zero as an error case. it's just stupid
    /*if ((*result) == 0) {
      printf("line %d: failed to convert %s to uint64_t\n", __LINE__, str);
      return 0;
    }*/
    if ((*result) == ULLONG_MAX && errno == ERANGE)
    {
        err_log( "failed to convert %s to uint64_t. errno was ERANGE\n", str);
        return -1;;
    }
    return 0;
}

int move_page(int pid, uint64_t gpa_src, uint64_t hpa_dst) {
  if (kmod_fd < 0) {
    printf("%s:%d: driver not openened\n", __FILE__, __LINE__);
    return -1;
  }

  struct args args = {
    .pid = pid,
    .gpa_src = gpa_src,
    .hpa_dst = hpa_dst,
  };

  if (ioctl(kmod_fd, PAGE_MOVE, &args)) {
    return -1;
  }

  return 0;
}

int open_kmod() {
  if( kmod_fd == - 1) {
    kmod_fd = open("/dev/movepage_dev", O_RDWR);
  }
  return kmod_fd < 0 ? kmod_fd : 0;
}

void close_kmod() {
  if( kmod_fd != - 1 ) {
    close(kmod_fd);
    kmod_fd = - 1;
  }
}

int main(int argc, char** argv) {
  int ret = 0;

  if(argc != 4) {
    printf("Params: <PID> <GPA to move> <destination HPA>\n");
    return 0;
  }

  int victim_pid = atoi(argv[1]);

  uint64_t victim_gpa;
  if( do_stroul(argv[2], 0, &victim_gpa)) {
    err_log("failed to parse '%s' as victim_gpa\n", argv[1]);
    return -1;
  }

  uint64_t destination_hpa;
  if( do_stroul(argv[3], 0, &destination_hpa)) {
    err_log("failed to parse '%s' as destination_hpa\n", argv[2]);
    return -1;
  }

  printf("Moving GPA 0x%jx to HPA %jx\n", victim_gpa, destination_hpa);

  ret = open_kmod();
  if( ret ) {
    err_log("Failed to open module (running as root?)\n");
    goto cleanup;
  }

  ret = move_page(victim_pid, victim_gpa, destination_hpa);
  if (ret) {
    err_log("Failed to move GPA 0x%jx to HPA %jx\n", victim_gpa, destination_hpa);
    goto cleanup;
  }

cleanup:
  close_kmod();
  return ret;
}
