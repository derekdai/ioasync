#include "coro.h"
#include "logging.h"
#include "loop.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

typedef int Fd;

void fd_close(Fd *fdp) {
  if(fdp && *fdp >= 0) {
    if(close(*fdp)) {
      warn("error while closing fd %d: %s", *fdp, strerror(errno));
    }
  }
}

#define Managed(t, f) __attribute__((cleanup(f))) t

#define FdVar Managed(Fd, fd_close)

int read_async(Fd fd, void *buf, size_t count) {
  int r;
  while(count) {
    r = read(fd, buf, count);
    if(r == -1) {
      switch(errno) {
      case EAGAIN:
        coro_yield();
      case EINTR:
        continue;
      }
      break;
    } else if(r == 0) {
      break;
    }

    buf = ((char *) buf) + r;
    count -= r;
  }

  return r;
}

int write_async(Fd fd, const void *buf, size_t count) {
  int r;
  while(count) {
    r = write(fd, buf, count);
    if(r == -1) {
      switch(errno) {
      case EAGAIN:
        coro_yield();
      case EINTR:
        continue;
      }
      break;
    } else if(r == 0) {
      break;
    }

    buf = ((char *) buf) + r;
    count -= r;
  }

  return r;
}

int main() {
  coro_init();
  loop_init();

  FdVar fd = dup(STDIN_FILENO);
  fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
  loop_register(NULL, fd, EPOLLIN, coro_self());

  char buf[1024];
  while(true) {
    int n = read_async(fd, buf, sizeof(buf) - 1);
    if(n == -1) {
      warn("failed to read: %s", strerror(errno));
      break;
    } else if(n == 0) {
      break;
    }

    buf[n - 1] = '\0';
    printf("%s", buf);
  }

  return 0;
}
