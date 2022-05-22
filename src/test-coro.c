#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "coro.h"

void myfunc() {
  printf("myfunc: 2\n");
  coro_switch_with_name("root");
  printf("myfunc: 4\n");
}

int main() {
  coro_init();

  coro_new("coro1", myfunc, 0);

  printf("main: 1\n");
  coro_switch_with_name("coro1");
  printf("main: 3\n");
  coro_switch_with_name("coro1");
  printf("main: 5\n");

  return 0;
}
