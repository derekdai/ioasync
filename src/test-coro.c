#include "coro.h"
#include "logging.h"
#include "loop.h"
#include "utils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void fd_close(Fd *fdp) {
  if(fdp && *fdp >= 0) {
    if(close(*fdp)) {
      warn("error while closing fd %d: %s", *fdp, strerror(errno));
    }
  }
}

int fd_set_nonblock(Fd fd, bool v) {
  assert(fd != -1);
  int mode = ccall(fcntl, fd, F_GETFL);
  if(v) {
    mode |= O_NONBLOCK;
  } else {
    mode &= ~O_NONBLOCK;
  }
  return ccall(fcntl, fd, F_SETFL, mode);
}

static inline Fd fd_from_native(int fd) {
  assert(fd != -1);
  fd_set_nonblock(fd, true);
  return fd;
}

int fd_read(Fd fd, void *buf, size_t count) {
  int r;
  do {
    r = read(fd, buf, count);
    if(r == -1) {
      switch(errno) {
      case EAGAIN:
        loop_register(NULL, fd, EPOLLIN, coro_self());
        coro_yield();
        loop_unregister(NULL, fd, EPOLLIN, coro_self());
      case EINTR:
        continue;
      }
      break;
    }
  } while(r < 0);

  return r;
}

int fd_write(Fd fd, void *buf, size_t count) {
  int r;
  do {
    r = write(fd, buf, count);
    if(r == -1) {
      switch(errno) {
      case EAGAIN:
        loop_register(NULL, fd, EPOLLOUT, coro_self());
        coro_yield();
        loop_unregister(NULL, fd, EPOLLOUT, coro_self());
      case EINTR:
        continue;
      }
      break;
    }
  } while(r < 0);

  return r;
}

int main() {
  coro_init();
  loop_init();

  FdVar fd0 = ccall(fd_from_native, ccall(dup, STDIN_FILENO));
  FdVar fd1 = ccall(fd_from_native, ccall(dup, STDOUT_FILENO));

  char buf[1024];
  while(true) {
    int n = fd_read(fd0, buf, sizeof(buf) - 1);
    if(n == -1) {
      warn("failed to read: %s", strerror(errno));
      break;
    } else if(n == 0) {
      break;
    }

    fd_write(fd1, ">", 1);
    buf[n - 1] = '\n';
    fd_write(fd1, buf, n);
  }

  return 0;
}
