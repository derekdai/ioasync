#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <threads.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <assert.h>
#include "loop.h"
#include "logging.h"
#include "dlist.h"
#include "utils.h"

typedef struct _Handler Handler;

struct _Handler {
  DList list;
  int events;
  Coro *coro;
};

Handler *handler_new(int events, Coro *coro) {
  Handler *self = malloc(sizeof(Handler));
  *self = (Handler) {
    .list = DLIST_INIT,
    .events = events,
    .coro = coro,
  };

  return self;
}

void handler_free(Handler *self) {
  free(self);
}

typedef struct _Loop Loop;

struct _Loop {
  int pollfd;
  Coro *coro;
  bool quit;
  int num_fds;
  Handler *handlers[128];       // 以 fd 作為 index
  int maxevents;
  struct epoll_event events[0];
};

thread_local Loop *loop = NULL;

static void loop_coro_run(Coro *coro) {
  loop_run(coro_data(Loop *, coro));
}

inline Fd loop_fd(Loop *self) {
  assert(self);
  return self->pollfd;
}

Loop *loop_new(int maxevents) {
  Loop *self = malloc(sizeof(Loop) + sizeof(struct epoll_event) * maxevents);
  self->quit = false;
  //self->coro = coro_create("looper", loop_coro_run);
  self->coro = coro_create_full("looper",
                                loop_coro_run,
                                coro_default_stack_size(),
                                sizeof(Loop *),
                                NULL);
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
    dlist_free_all(self->handlers[i], handler_free);
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

static inline Loop *self_or_default(Loop *self) {
  return self ? self : loop_default();
}

Loop *loop_default() {
  assert(loop != NULL);

  return loop;
}

void loop_quit(Loop *self) {
  self_or_default(self)->quit = 1;
}

static inline bool loop_registered(Loop *self, Fd fd) {
  return self->handlers[fd] != NULL;
}

static bool dispatch(Handler *h, struct epoll_event *event) {
  if(!(event->events & (h->events | EPOLLERR))) {
    return true;
  }
  coro_switch(h->coro);

  return false;
}

void loop_dispatch(Loop *self, struct epoll_event *event) {
  assert(loop_registered(self, event->data.fd));

  dlist_foreach(self->handlers[event->data.fd], dispatch, event);
}

void loop_run(Loop *self) {
  self = self_or_default(self);

  while(!self->quit && self->num_fds > 0) {
    int r = epoll_wait(self->pollfd, self->events, self->maxevents, -1);
    if(r == -1) {
      if(errno == EINTR) {
        continue;
      }

      warn("poll failed: %s", strerror(errno));
      break;
    }

    trace("%d events", r);

    int i;
    for(i = 0; i < r; ++ i) {
      loop_dispatch(self, self->events + i);
    }
  }
}

static bool or_events(Handler *h, int *result) {
  *result |= h->events;
  return true;
}

static inline int calc_events(Handler *h) {
  int events = 0;
  dlist_foreach(h, or_events, &events);

  return events;
}

int loop_register(Loop *self, Fd fd, int events, Coro *coro) {
  assert(coro);
  assert(fd_check(fd));
  assert(events != 0);

  self = self_or_default(self);

  int old_events = calc_events(self->handlers[fd]);
  int op = !old_events ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
  if((old_events | events) ^ old_events) {
    ccall(epoll_ctl, self->pollfd, op, fd, &(struct epoll_event) {
      .events = old_events | events,
      .data.fd = fd,
    });
  }

  if(!self->handlers[fd]) {
    ++ self->num_fds;
  }

  dlist_prepend(handler_new(events, coro), self->handlers[fd]);

  return 0;
}

bool comp_handler(Handler *node, void *data) {
  struct Ctx {
    Handler *handler;
    int events;
    Coro *coro;
  } *ctx = data;

  if(ctx->events == node->events && ctx->coro == node->coro) {
    ctx->handler = node;
  }
  return !ctx->handler;
}

int loop_unregister(Loop *self, Fd fd, int events, Coro *coro) {
  assert(coro);
  assert(fd_check(fd));
  assert(events != 0);

  self = self_or_default(self);

  struct Ctx {
    Handler *handler;
    int events;
    Coro *coro;
  } ctx = {
    .handler = NULL,
    .events = events,
    .coro = coro,
  };
  dlist_foreach(self->handlers[fd], comp_handler, &ctx);

  if(!ctx.handler) {
    return 0;
  }

  dlist_remove(ctx.handler);
  if(self->handlers[fd] == ctx.handler) {
    self->handlers[fd] = dlist_next(Handler, ctx.handler);
  }

  int new_events = calc_events(self->handlers[fd]);
  int op = !new_events ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
  if((new_events | events) ^ new_events) {
    ccall(epoll_ctl, self->pollfd, op, fd, &(struct epoll_event) {
      .events = new_events | events,
      .data.fd = fd,
    });
  }

  handler_free(ctx.handler);

  return 0;
}

inline bool loop_is_running(Loop *self) {
  return !self_or_default(self)->quit;
}
