CC=gcc
CFLAGS=-Wall -std=c99 -O3 -march=native
LDFLAGS= -I/usr/include/gsl -lgsl -lgslcblas -lm

all: simulator.x

simulator.x: simulator.c
	$(CC) $(CFLAGS) simulator.c -o simulator.x $(LDFLAGS)

clean:
	rm *.x
