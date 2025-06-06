CC = clang
CFLAGS=-Wall -Wpedantic -Wextra -std=c99


# Run command
RUN = ./ronto -n

ronto: ronto.c token.c
	$(CC) ronto.c token.c $(CFLAGS) -ggdb -o ronto

release: ronto.c token.c
	$(CC) ronto.c token.c $(CFLAGS) -o ronto


run: ronto
	$(RUN)

rm: log
	rm log Untitled-*

clean: ronto
	rm ronto
