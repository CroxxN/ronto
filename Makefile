CC = gcc
CFLAGS=-Wall -Wpedantic -std=c99 

main: main.c
	$(CC) main.c $(CFLAGS) -o main

debug: main.c
	$(CC) main.c $(CFLAGS) -ggdb -o main


run: main
	./main

clean: main
	rm main