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

Coro *coro_new(const char *name, int stackSize);

void coro_free(Coro *self);

Coro *coro_self();

void coro_to(Coro *target);

void coro_to_root();

void coro_start(Coro *self, CoroEntry entry);

#endif /* _CORO_H_ */
