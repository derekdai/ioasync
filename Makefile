all: test-coro

test-coro: src/test-coro.c src/coro.c
	gcc -Wall -Wextra -ggdb -O2 -o $@ src/test-coro.c src/coro.c
