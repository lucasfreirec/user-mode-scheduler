CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -g

all: main

main: main.o scheduler.o
	$(CC) $(CFLAGS) -o main main.o scheduler.o

main.o: main.c scheduler.h
	$(CC) $(CFLAGS) -c main.c

escalonador.o: scheduler.c scheduler.h
	$(CC) $(CFLAGS) -c scheduler.c

clean:
	rm -f *.o main