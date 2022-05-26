CC=gcc
CFLAGS=-Wall -Wextra -D_GNU_SOURCE -DLOG_LEVEL=5 -ggdb -fno-omit-frame-pointer

all: test-coro

test-coro: src/test-coro.c src/coro.c
	$(CC) $(CFLAGS) \
		-o $@ \
		src/test-coro.c \
		src/coro.c \
		src/dlist.c \
		src/loop.c

test-dlist: tests/test-dlist.c src/dlist.c
	$(CC) $(CFLAGS) -o $@ tests/test-dlist.c src/dlist.c

tests: test-dlist

run-tests: tests
	./test-dlist

run-tests-valgrind: tests
	valgrind --leak-check=full ./test-dlist

clean:
	rm -f \
		test-coro \
		test-dlist

