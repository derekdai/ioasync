#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include "coro.h"

struct _Coro {
  ucontext_t ctx;
  char *name;
  CoroStatus status;
  CoroEntry entry;
};

static Coro *root = &(Coro) {
  .name = "root",
  .status = CORO_STATUS_INIT
};

static Coro *curr = NULL;

void coro_init() {
  root->status = CORO_STATUS_STARTED;
  curr = root;
}

Coro *coro_root() {
  return root;
}

const char *coro_name() {
  return curr->name;
}

Coro *coro_new(const char *name, int stackSize) {
  assert(curr != NULL);
  assert(name != NULL);

  Coro *self = malloc(sizeof(Coro));
  self->name = strdup(name);
  self->status = CORO_STATUS_INIT;
  self->ctx.uc_stack.ss_sp = malloc(stackSize);
  self->ctx.uc_stack.ss_size = stackSize;

  printf("new coro %s (%p)\n", name, self);

  return self;
}

void coro_free(Coro *self) {
  if(self) {
    free(self->name);
    free(self->ctx.uc_stack.ss_sp);
    free(self);
  }
}

Coro *coro_self() {
  return curr;
}

static void _coro_entry() {
  Coro *self = curr;
  self->entry();
  self->status = CORO_STATUS_DEAD;

  printf("  !%s dead, go %s (%p)\n", self->name, ((Coro *) self->ctx.uc_link)->name, self->ctx.uc_link); 

  curr = (Coro *) self->ctx.uc_link;
  setcontext(self->ctx.uc_link);
}

Coro *coro_prev() {
  return (Coro *) curr->ctx.uc_link;
}

void coro_switch(Coro *self) {
  assert(curr->status == CORO_STATUS_STARTED);

  self->ctx.uc_link = (struct ucontext_t *) curr;
  curr = self;

  if(swapcontext(self->ctx.uc_link, (struct ucontext_t *) self) == -1) {
    printf("failed to swap context: %s\n", strerror(errno));
  } else {
    if(curr->status == CORO_STATUS_DEAD) {
      printf("  !back from %s to %s (%p)\n", self->name, curr->name, curr);
    }
    else {
      printf("  <back from %s to %s (%p)\n", ((Coro *) curr->ctx.uc_link)->name, curr->name, curr);
    }
  }
}

void coro_yield() {
  coro_switch(root);
}

void coro_start(Coro *self, CoroEntry entry) {
  assert(self);
  assert(entry);
  assert(self->status == CORO_STATUS_INIT);

  self->entry = entry;
  getcontext((struct ucontext_t *) self);
  makecontext((struct ucontext_t *) self, _coro_entry, 0);

  printf("  >from %s to %s\n", curr->name, self->name);

  self->status = CORO_STATUS_STARTED;
  coro_switch(self);
}
