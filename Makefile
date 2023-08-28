CC = gcc
CFLAGS=-Wall -Wpedantic -std=c99

main: main.c
	$(CC) main.c $(CFLAGS) -g -o main

nodebug: main.c
	$(CC) main.c $(CFLAGS) -o main

run: main
	./main

clean: main
	rm main