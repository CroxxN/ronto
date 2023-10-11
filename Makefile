CC = gcc
CFLAGS=-Wall -Wpedantic -std=c99 

# Use lld instead of ld
LDFLAGS=-fuse-ld=lld

# Run command
RUN = ./ronto -n

main: ronto.c
	$(CC) ronto.c $(CFLAGS) $(LDFLAGS) -o ronto

debug: ronto.c
	$(CC) ronto.c $(CFLAGS) $(LDFLAGS) -ggdb -o ronto


run: ronto
	$(RUN)

clean: ronto
	rm ronto