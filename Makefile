all: test-coro

test-coro: src/test-coro.c src/coro.c
	$(CC) -Wall -Wextra -ggdb -fsanitize=address -fno-omit-frame-pointer -o $@ \
		src/test-coro.c \
		src/coro.c \
		src/loop.c

clean:
	rm -f test-coro
