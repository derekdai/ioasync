all: test-coro

test-coro: src/test-coro.c src/coro.c
	gcc -Wall -Wextra -ggdb -o $@ src/test-coro.c src/coro.c
