#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <threads.h>
#include "coro.h"

#define STACK_SIZE (4096)
#define CORO(p) ((Coro *)(p))
#define UCTX(p) ((struct ucontext_t *)(p))

enum CoroPrioroty {
  CORO_PRIO_HIGHEST = -64,
  CORO_PRIO_NORMAL = 0,
  CORO_PRIO_LOWEST = 63,
};

struct _CoroSche {
  int stack_size;
  Coro *root;
  Coro *curr;
  int num_child;
  Coro *children;
};

struct _Coro {
  ucontext_t ctx;
  char *name;
  CoroStatus status;
  CoroEntry entry;
  Coro *next;
};

thread_local CoroSche *sche = &(CoroSche) {
  .stack_size = STACK_SIZE,
  .root = &(Coro) {
    .name = "root",
    .status = CORO_STATUS_INIT,
  },
  .curr = NULL,
  .num_child = 0,
  .children = NULL,
};

Coro *coro_find(const char *name) {
  if(!strcmp("root", name)) {
    return coro_root();
  }

  Coro *c = sche->children;
  while(c != NULL) {
    if(!strcmp(name, c->name)) {
      return c;
    }
    c = c->next;
  }

  return NULL;
}

void coro_add(Coro *coro) {
  coro->next = sche->children;
  sche->children = coro;
  ++ sche->num_child;
}

void coro_del(Coro *coro) {
  Coro *p = sche->children;
  if(p == coro) {
    sche->children = coro->next;
    -- sche->num_child;
    return;
  }
  while(p) {
    if(p->next == coro) {
      p->next = coro->next;
      -- sche->num_child;
      break;
    }
    p = p->next;
  }
}

Coro *coro_root() {
  return sche->root;
}

const char *coro_name() {
  return coro_self()->name;
}

void coro_set_default_stack_size(int stack_size) {
  assert(stack_size >= STACK_SIZE);

  sche->stack_size = stack_size;
}

void coro_init() {
  sche->curr = sche->root;
  sche->curr->status = CORO_STATUS_STARTED;
  getcontext(UCTX(sche->root));
}

Coro *coro_new(const char *name, CoroEntry entry, int data_size) {
  assert(coro_self() != NULL);
  assert(name != NULL);

  Coro *self = malloc(sizeof(Coro) + data_size);
  self->name = strdup(name);
  self->status = CORO_STATUS_INIT;
  UCTX(self)->uc_stack.ss_sp = malloc(sche->stack_size);
  UCTX(self)->uc_stack.ss_size = sche->stack_size;
  UCTX(self)->uc_link = UCTX(coro_root());
  getcontext(UCTX(self));
  makecontext(UCTX(self), (void (*)()) entry, 1, self);

  coro_add(self);

  return self;
}

void coro_free(Coro *self) {
  assert(self);
  assert(self->status == CORO_STATUS_INIT || self->status == CORO_STATUS_DEAD);

  free(self->name);
  free(self->ctx.uc_stack.ss_sp);
  free(self);
}

Coro *coro_self() {
  return sche->curr;
}

int coro_switch_name(const char *name) {
  Coro *c = coro_find(name);
  return coro_switch(c);
}

void coro_yield() {
  coro_switch(coro_root());
}

int coro_switch(Coro *target) {
  assert(target);
  assert(target->status == CORO_STATUS_INIT);

  Coro *self = sche->curr;
  sche->curr = target;
  target->status = CORO_STATUS_STARTED;
  if(swapcontext(UCTX(self), UCTX(target)) == -1) {
    target->status = CORO_STATUS_INIT;
    sche->curr = self;
    return -1;
  }

  printf("  to %s\n", self->name);    
  sche->curr = self;

  return 0;
}
