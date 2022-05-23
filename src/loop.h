#include "coro.h"
#include <sys/epoll.h>

#ifndef __LOOP_H_
#define __LOOP_H_

typedef struct _Handler Handler;

Handler *handler_new(int events, Coro *coro);

void handler_free(Handler *self);

typedef struct _Loop Loop;

Loop *loop_new(int maxevents);

void loop_free(Loop * self);

void loop_init();

Loop *loop_default();

void loop_quit(Loop *self);

void loop_dispatch(Loop *self, struct epoll_event *event);

void loop_run(Loop *self);

void loop_register(Loop *self, int fd, int events, Coro *coro);

#endif /* __LOOP_H_ */
