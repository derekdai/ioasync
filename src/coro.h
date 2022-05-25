#include <stdbool.h>

#ifndef _CORO_H_
#define _CORO_H_

#define DEFAULT_STACK_SIZE (4096)

#define coro_data(t, c) (*(t *) _coro_data(c))

#define CORO_ENTRY(p) ((CoroEntry)(p))

#define CORO_DESTROY(p) ((CoroDestroy)(p))

typedef struct _CoroSche CoroSche;

typedef struct _Coro Coro;

typedef void (*CoroDestroy)(Coro *coro);

typedef void (*CoroEntry)(Coro *coro);

enum _CoroStatus {
  CORO_STATUS_INIT = 0,
  CORO_STATUS_STARTED,
  CORO_STATUS_DEAD,
};

typedef enum _CoroStatus CoroStatus;

void coro_init();

const char *coro_name(Coro *coro);

CoroStatus coro_status(Coro *coro);

Coro *coro_create(const char *name, CoroEntry entry);

Coro *coro_create_full(const char *name,
                       CoroEntry entry,
                       int stack_size,
                       int data_size,
                       CoroDestroy destroy);

void coro_free(Coro *self);

Coro *coro_self();

void coro_switch(Coro *target);

int coro_switch_with_name(const char *name);

bool coro_yield();

void coro_start(Coro *self, CoroEntry entry);

void coro_set_default_stack_size(int stack_size);

int coro_default_stack_size();

void *_coro_data(Coro *self);

void coro_join(Coro *coro);

void coro_join_all();

#endif /* _CORO_H_ */
