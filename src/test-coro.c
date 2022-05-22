#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "coro.h"

void myfunc() {
  printf("myfunc: 1\n");
}

int main() {
  coro_init();

  Coro *coro1 = coro_new("coro1", myfunc, 0);

  printf("main: 1\n");
  coro_switch(coro1);
  
  printf("main: 3\n");

  return 0;
}
