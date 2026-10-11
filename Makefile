CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -g

all: main

main: main.o escalonador.o
	$(CC) $(CFLAGS) -o main main.o escalonador.o

main.o: main.c escalonador.h
	$(CC) $(CFLAGS) -c main.c

escalonador.o: escalonador.c escalonador.h
	$(CC) $(CFLAGS) -c escalonador.c

clean:
	rm -f *.o main