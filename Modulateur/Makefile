CC=gcc
# grace's version (mac)
#CFLAGS=-Wall -std=c99 -I/opt/homebrew/opt/gsl/include -L/opt/homebrew/opt/gsl/lib -lgsl -lgslcblas
# jetson version
CFLAGS=-Wall -std=c99 -I/usr/include/gsl -lgsl -lgslcblas

all: simulateur

$@: $@.c

clean:
	rm *.o