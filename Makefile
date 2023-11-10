CC = clang
CFLAGS=-Wall -Wpedantic -Wextra -std=c99


# Run command
RUN = ./ronto -n

ronto: ronto.c
	$(CC) ronto.c $(CFLAGS) -ggdb -o ronto

release: ronto.c
	$(CC) ronto.c $(CFLAGS) -o ronto


run: ronto
	$(RUN)

clean: ronto
	rm ronto
