#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

typedef void (*CoroEntry)();

typedef enum _CoroStatus CoroStatus;

enum _CoroStatus {
  CORO_STATUS_INIT = 0,
  CORO_STATUS_STARTED,
  CORO_STATUS_DEAD,
};

typedef struct _Coro Coro;

struct _Coro {
  ucontext_t ctx;
  char *name;
  CoroStatus status;
  CoroEntry entry;
};

static Coro *root = &(Coro) { };

static Coro *curr = NULL;

#define coro_init() {                       \
  root->name = "main";                      \
  root->status = CORO_STATUS_STARTED;       \
  curr = root;                              \
}

Coro *coro_new(const char *name, int stackSize) {
  Coro *self = calloc(1, sizeof(Coro));
  self->name = strdup(name);
  self->status = CORO_STATUS_INIT;
  self->ctx.uc_stack.ss_sp = malloc(stackSize);
  self->ctx.uc_stack.ss_size = stackSize;

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

void _coro_entry() {
  Coro *self = curr;
  self->entry();
  self->status = CORO_STATUS_DEAD;
}

void coro_to(Coro *target) {
  assert(curr->status == CORO_STATUS_STARTED);

  Coro *self = curr;
  curr = target;

  if(swapcontext((struct ucontext_t *) self, (struct ucontext_t *) target) == -1) {
    printf("failed to swap context: %s\n", strerror(errno));
  } else {
    printf("from %s to %s\n", target->name, self->name);
  }

  curr = self;
}

void coro_start(Coro *self, CoroEntry entry) {
  assert(self);
  assert(entry);
  assert(self->status == CORO_STATUS_INIT);

  self->entry = entry;
  getcontext((struct ucontext_t *) self);
  makecontext((struct ucontext_t *) self, _coro_entry, 0);

  printf("from %s to %s\n", curr->name, self->name);

  self->status = CORO_STATUS_STARTED;
  coro_to(self);
}

void myfunc() {
  printf("myfunc: start\n");
  coro_to(root);
  printf("myfunc: end\n");
}

int main() {
  coro_init();

  Coro *coro1 = coro_new("coro1", 4096);

  printf("main: hello\n");
  coro_start(coro1, myfunc);
  
  printf("main: hello2\n");
  coro_to(coro1);

  printf("main: bye\n");

  return 0;
}
