CC = gcc
CFLAGS=-Wall -Wpedantic -std=c99 
LDFLAGS=-fuse-ld=lld

main: main.c
	$(CC) main.c $(CFLAGS) $(LDFLAGS) -o main

debug: main.c
	$(CC) main.c $(CFLAGS) $(LDFLAGS) -ggdb -o main


run: main
	./main

clean: main
	rm main