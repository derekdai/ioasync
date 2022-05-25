#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <threads.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>
#include "loop.h"
#include "logging.h"

#define self_or_default(self) ({  \
  typeof(self) v = (self);        \
  v ? v : coro_default()          \
})

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
  Coro *coro;
  int pollfd;
  int num_fds;
  Handler *handlers[128];       // 以 fd 作為 index
  int maxevents;
  struct epoll_event events[0];
};

thread_local Loop *loop = NULL;

static void loop_coro_run(Coro *coro) {
  loop_run(coro_data(Loop *, coro));
}

Loop *loop_new(int maxevents) {
  Loop *self = malloc(sizeof(Loop) + sizeof(struct epoll_event) * maxevents);
  self->quit = false;
  //self->coro = coro_create("looper", loop_coro_run);
  self->coro = coro_create_full("looper", loop_coro_run, sizeof(Loop *), NULL);
  coro_data(Loop *, self->coro) = self;
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
  assert(coro);
  assert(fd >= 0);
  assert(events);

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

inline bool loop_is_running(Loop *self) {
  return !(self ? self : loop_default())->quit;
}
