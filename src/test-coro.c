#include "coro.h"
#include "logging.h"
#include "loop.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

void on_stdin() {
  info("%s\n", coro_name(coro_self()));

  char buf[1024];
  while(true) {
    int r = read(STDIN_FILENO, buf, sizeof(buf) - 1);
    if(r == -1) {
      if(errno == EINTR) {
        continue;
      } else if(errno == EAGAIN) {
        coro_yield();
        continue;
      }
      warn("failed to read from stdin: %s", strerror(errno));
      loop_quit(NULL);
      break;
    } else if(r == 0) {
      loop_quit(NULL);
      break;
    }

    info("read %d bytes", r);
    buf[r-1] = '\0';
    info("stdin: %s", buf);
  }
}

int main() {
  coro_init();
  loop_init();

  Coro *c = coro_new("stdin-reader", CORO_ENTRY(on_stdin), 0);

  int mode = fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK;
  fcntl(STDIN_FILENO, F_SETFL, mode);
  loop_register(NULL, STDIN_FILENO, EPOLLIN, c);

  loop_run(NULL);

  return 0;
}
