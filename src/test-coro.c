#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "coro.h"

int count = 5000000;

void myfunc() {
  while(count) {
    coro_yield();
  }
}

int main() {
  coro_init();

  coro_new("coro1", myfunc, 0);

  while(count) {
    coro_yield();
    -- count;
  }

  return 0;
}
