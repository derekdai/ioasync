#include "coro.h"
#include "types.h"
#include <sys/epoll.h>
#include <stdbool.h>

#ifndef __LOOP_H_
#define __LOOP_H_

typedef struct _Loop Loop;

Fd loop_fd(Loop *self);

Loop *loop_new(int maxevents);

void loop_free(Loop * self);

void loop_init();

Loop *loop_default();

void loop_quit(Loop *self);

bool loop_is_running(Loop *self);

void loop_dispatch(Loop *self, struct epoll_event *event);

void loop_run(Loop *self);

int loop_register(Loop *self, Fd fd, int events, Coro *coro);

int loop_unregister(Loop *self, Fd fd, int events, Coro *coro);

#endif /* __LOOP_H_ */
