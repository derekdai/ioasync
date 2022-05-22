#ifndef _CORO_H_
#define _CORO_H_

typedef void (*CoroEntry)();

enum _CoroStatus {
  CORO_STATUS_INIT = 0,
  CORO_STATUS_STARTED,
  CORO_STATUS_DEAD,
};

typedef enum _CoroStatus CoroStatus;

typedef struct _Coro Coro;

void coro_init();

Coro *coro_root();

const char *coro_name();

Coro *coro_new(const char *name, int stackSize);

void coro_free(Coro *self);

Coro *coro_self();

void coro_switch(Coro *target);

void coro_yield();

void coro_start(Coro *self, CoroEntry entry);

#endif /* _CORO_H_ */
