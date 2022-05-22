#ifndef _CORO_H_
#define _CORO_H_

#define CORO_ENTRY(p) ((CoroEntry *)(p))

typedef struct _CoroSche CoroSche;

typedef struct _Coro Coro;

typedef void (*CoroEntry)(Coro *coro);

enum _CoroStatus {
  CORO_STATUS_INIT = 0,
  CORO_STATUS_STARTED,
  CORO_STATUS_DEAD,
};

typedef enum _CoroStatus CoroStatus;

void coro_init();

Coro *coro_root();

const char *coro_name();

Coro *coro_new(const char *name, CoroEntry entry, int data_size);

void coro_free(Coro *self);

Coro *coro_self();

int coro_switch(Coro *target);

int coro_switch_name(const char *name);

void coro_yield();

void coro_start(Coro *self, CoroEntry entry);

#endif /* _CORO_H_ */
