CC = clang
CFLAGS=-Wall -Wpedantic -Wextra -std=c99 


# Run command
RUN = ./ronto -n

main: ronto.c
	$(CC) ronto.c $(CFLAGS) -o ronto

debug: ronto.c
	$(CC) ronto.c $(CFLAGS) -ggdb -o ronto


run: ronto
	$(RUN)

clean: ronto
	rm ronto
