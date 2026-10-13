CC=gcc-16
CFLAGS=-Wall -Wextra -std=c11 -g
CPPFLAGS=-D_XOPEN_SOURCE=700

all: main

main: main.o scheduler.o
	$(CC) $(CFLAGS) -o $@ $^

main.o: main.c scheduler.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

scheduler.o: scheduler.c scheduler.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

clean:
	rm -f *.o main
