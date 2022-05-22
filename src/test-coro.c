#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "coro.h"

void myfunc2() {
  printf("myfunc2: start\n");
  coro_yield();
  printf("myfunc2: end\n");
}

void myfunc() {
  printf("myfunc: start\n");
  Coro *coro2 = coro_new("coro2", 4096);
  coro_start(coro2, myfunc2);
  printf("myfunc: go myfunc2 again\n");
  coro_switch(coro2);
  printf("myfunc: end\n");
  coro_free(coro2);
}

int main() {
  coro_init();

  Coro *coro1 = coro_new("coro1", 4096);

  printf("main: hello\n");
  coro_start(coro1, myfunc);
  
  printf("main: hello2\n");
  coro_switch(coro1);

  printf("main: bye\n");

  coro_free(coro1);

  return 0;
}
