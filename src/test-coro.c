#include "coro.h"
#include "logging.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <threads.h>
#include <unistd.h>
#include <sys/epoll.h>

typedef struct _Handler Handler;

struct _Handler {
  int events;
  Coro *coro;
  Handler *prev;
  Handler *next;
};

Handler *handler_new(int events, Coro *coro) {
  Handler *self = malloc(sizeof(Handler));
  *self = (Handler) {
    .events = events,
    .coro = coro,
    .prev = NULL,
    .next = NULL,
  };

  return self;
}

void handler_free(Handler *self) {
  free(self);
}

typedef struct _Loop Loop;

struct _Loop {
  bool quit;
  int pollfd;
  int num_fds;
  Handler *handlers[128];
  int maxevents;
  struct epoll_event events[0];
  // 以 fd 作為 index
};

thread_local Loop *loop = NULL;

Loop *loop_new(int maxevents) {
  Loop *self = malloc(sizeof(Loop) + sizeof(struct epoll_event) * maxevents);
  self->quit = false;
  self->pollfd = epoll_create1(EPOLL_CLOEXEC);
  self->num_fds = 0;
  self->maxevents = maxevents;
  memset(self->handlers, 0, sizeof(self->handlers));

  return self;
}

void loop_free(Loop * self) {
  if(!self) {
    return;
  }

  int i;
  for(i = 0; i < 128; ++ i) {
    Handler *h = self->handlers[i];
    while(h) {
      Handler *n = h->next;
      handler_free(h);
      h = n;
    }
  }

  close(self->pollfd);
  free(self);
}

static void loop_deinit() {
  if(loop) {
    loop_free(loop);
  }
}

void loop_init() {
  assert(loop == NULL);

  loop = loop_new(100);
  atexit(loop_deinit);
}

Loop *loop_default() {
  assert(loop != NULL);

  return loop;
}

void loop_quit(Loop *self) {
  self = self ? self : loop_default();
  self->quit = 1;
}

void loop_dispatch(Loop *self, struct epoll_event *event) {
  Handler *h = self->handlers[event->data.fd];
  while(h) {
    if(!(event->events & (h->events | EPOLLERR))) {
      continue;
    }
    coro_switch(h->coro);

    h = h->next;
  }
}

void loop_run(Loop *self) {
  self = self ? self : loop_default();

  while(!self->quit && self->num_fds > 0) {
    int r = epoll_wait(self->pollfd, self->events, self->maxevents, -1);
    if(r == -1) {
      if(errno == EINTR) {
        continue;
      }

      warn("poll failed: %s", strerror(errno));
      break;
    }

    debug("%d events", r);

    int i;
    for(i = 0; i < r; ++ i) {
      loop_dispatch(self, self->events + i);
    }
  }
}

void loop_register(Loop *self, int fd, int events, Coro *coro) {
  self = self ? self : loop_default();

  struct epoll_event e = {
    .events = events,
    .data.fd = fd,
  };

  int r = epoll_ctl(self->pollfd, EPOLL_CTL_ADD, fd, &e);
  if(r == -1) {
    warn("failed to register fd (%d): %s", fd, strerror(errno));
    return;
  }

  Handler *h = handler_new(events, coro);
  h->next = self->handlers[fd];
  if(self->handlers[fd]) {
    self->handlers[fd]->prev = h;
  }
  self->handlers[fd] = h;

  ++ self->num_fds;
}

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
