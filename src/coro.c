#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <threads.h>
#include "coro.h"

#define log(fmt, ...) fprintf(stderr, "\e[2;34m" fmt "\e[0m\n", __VA_ARGS__)
#define debug(fmt, ...) log(" D" fmt, __VA_ARGS__)

#define STACK_SIZE (4096)
#define CORO(p) ((Coro *)(p))
#define UCTX(p) ((struct ucontext_t *)(p))

enum CoroPrioroty {
  CORO_PRIO_HIGHEST = -64,
  CORO_PRIO_NORMAL = 0,
  CORO_PRIO_LOWEST = 63,
};

/**
 * 設計為
 * - doubld-linked list
 * - 單向 traverse
 * - 盡可能源少 pointer 的操作量
 *
 * - 因為有 root, `num_coros` 至少為 1, 所以 `head` 不可能是 NULL
 * - curr 指向 head
 *   - curr 為目前運行中 coro
 * - head 的 `prev` 為 tail
 * - tail 的 `next` 為 head
 *
 * - 下個要執行的 coro 為 `curr->next`
 *   - 所以 add 時串在 `curr->next` 位置
 *
 * - curr 執行後, 移到 `curr->next`, 也就是目前的 curr 成為了 tail
 * 
 * - 如果用 switch 切換至指定 coro, 要先將它從 list 中移除, 然後取代
 *   curr, 這樣執行完後就自動成為 tail
 */
struct _CoroSche {
  int stack_size;
  Coro *root;
  int num_coros;
  Coro *curr;
};

struct _Coro {
  ucontext_t ctx;
  char *name;
  CoroStatus status;
  Coro *prev;
  Coro *next;
};

thread_local CoroSche *sche = &(CoroSche) {
  .stack_size = STACK_SIZE,
  .root = &(Coro) {
    .name = "root",
    .status = CORO_STATUS_INIT,
  },
  .num_coros = 0,
  .curr = NULL,
};

void coro_set_default_stack_size(int stack_size) {
  assert(stack_size >= STACK_SIZE);

  sche->stack_size = stack_size;
}

static inline Coro *coro_head() {
  return sche->curr;
}

static inline Coro *coro_tail() {
  return coro_head()->prev;
}

static inline Coro *coro_root() {
  return sche->root;
}

inline const char *coro_name(Coro *coro) {
  return coro->name;
}

inline Coro *coro_self() {
  return coro_head();
}

inline CoroStatus coro_status(Coro *coro) {
  return coro->status;
}

/**
 * 此時 list 為 empty, 特殊處理, 在 `coro_add()` 中就不需要判斷 `head` 為 NULL 的情況了
 */
void coro_init() {
  sche->num_coros = 1;
  sche->curr = coro_root();
  coro_head()->prev = coro_head();
  coro_head()->next = coro_head();
}

Coro *coro_find(const char *name) {
  if(!strcmp("root", name)) {
    return coro_root();
  }

  Coro *c = coro_head();
  while(c != coro_tail()) {
    if(!strcmp(name, c->name)) {
      return c;
    }
    c = c->next;
  }

  return NULL;
}

static void coro_add(Coro *coro) {
  coro->next = coro_head()->next;
  coro_head()->next->prev = coro;

  coro->prev = coro_head();
  coro_head()->next = coro;

  ++ sche->num_coros;
}

/**
 * `curr` 的操作由 caller 完成
 */
static void coro_remove(Coro *coro) {
  if(coro->prev) {
    coro->prev->next = coro->next;
  }

  if(coro->next) {
    coro->next->prev = coro->prev;
  }
}

static void coro_set_head(Coro *coro) {
  if(coro == coro_head()) {
    return;
  } else  if(coro != coro_head()->next) {
    coro_remove(coro);
    coro_add(coro);
  }

  sche->curr = sche->curr->next;
}

Coro *coro_new(const char *name, CoroEntry entry, int data_size) {
  assert(coro_root() != NULL);
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
  assert(self != coro_head());
  assert(self->status == CORO_STATUS_INIT || self->status == CORO_STATUS_DEAD);

  free(self->name);
  free(self->ctx.uc_stack.ss_sp);
  free(self);
}

int coro_switch_with_name(const char *name) {
  Coro *c;
  if(!strcmp("root", name)) {
    c = sche->root;
  } else {
    c = coro_find(name);
    if(!c) {
      errno = ENOENT;
      return -1;
    }
  }

  coro_switch(c);

  return 0;
}

void coro_switch(Coro *target) {
  assert(target);
  assert(target->status != CORO_STATUS_DEAD);

  Coro *self = coro_head();

  //debug("  %s>%s\n", self->name, target->name);

  coro_set_head(target);
  target->status = CORO_STATUS_STARTED;
  swapcontext(UCTX(self), UCTX(target));

  //debug("  %s<\n", self->name);
  coro_set_head(self);
}

void coro_sche() {
  if(sche->num_coros == 1) {
    return;
  }

  coro_switch(sche->curr->next);
}

void coro_yield() {
  coro_sche();
}
