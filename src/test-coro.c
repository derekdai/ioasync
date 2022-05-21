#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "coro.h"

void myfunc() {
  printf("myfunc: start\n");
  coro_to_root();
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
