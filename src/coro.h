#ifndef _CORO_H_
#define _CORO_H_

#define CORO_ENTRY(p) ((CoroEntry)(p))

typedef struct _CoroSche CoroSche;

typedef struct _Coro Coro;

typedef void (*CoroEntry)();

enum _CoroStatus {
  CORO_STATUS_INIT = 0,
  CORO_STATUS_STARTED,
  CORO_STATUS_DEAD,
};

typedef enum _CoroStatus CoroStatus;

void coro_init();

const char *coro_name(Coro *coro);

CoroStatus coro_status(Coro *coro);

Coro *coro_new(const char *name, CoroEntry entry, int data_size);

void coro_free(Coro *self);

Coro *coro_self();

void coro_switch(Coro *target);

int coro_switch_with_name(const char *name);

void coro_yield();

void coro_start(Coro *self, CoroEntry entry);

void coro_set_default_stack_size(int stack_size);

#endif /* _CORO_H_ */
