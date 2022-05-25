all: test-coro

test-coro: src/test-coro.c src/coro.c
#	$(CC) -Wall -Wextra -D_GNU_SOURCE -DLOG_LEVEL=5 -ggdb -fsanitize=address -fno-omit-frame-pointer -o $@
	$(CC) -Wall -Wextra -D_GNU_SOURCE -DLOG_LEVEL=5 -ggdb -fno-omit-frame-pointer -o $@ \
		src/test-coro.c \
		src/coro.c \
		src/loop.c

clean:
	rm -f test-coro
