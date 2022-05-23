#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "coro.h"

void myfunc(Coro *coro) {
  printf("%s: 2\n", coro_name(coro));
  coro_yield();
  printf("%s: 4\n", coro_name(coro));
  coro_yield();
}

int main() {
  coro_init();

  coro_new("coro1", myfunc, 0);

  printf("main: 1\n");
  coro_yield();
  printf("main: 3\n");
  coro_yield();
  printf("main: 5\n");

  return 0;
}
