all: test-coro

test-coro: src/test-coro.c src/coro.c
	gcc -Wall -Wextra -ggdb -fsanitize=address -fno-omit-frame-pointer -o $@ src/test-coro.c src/coro.c

clean:
	rm -f test-coro
