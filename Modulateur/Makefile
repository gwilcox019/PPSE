CC=gcc
CFLAGS=-Wall -std=gnu99 -O3 -march=native -I/opt/homebrew/opt/gsl/include -DENABLE_STATS
# mac version
#LDFLAGS=-L/opt/homebrew/opt/gsl/lib -DENABLE_STATS
#LDLIBS=-lgsl -lgslcblas -lm -lpthread #-lrt
# jetson version
LDLIBS= -I/usr/include/gsl -lgsl -lgslcblas -lm

all: simulator

simulator: simulator.o generate.o encoder.o modulate.o decode.o monitor.o

simulator.o: simulator.c debug_func.h generate.h encoder.h modulate.h decode.h monitor.h

debug_func: debug_func.o generate.o encoder.o modulate.o decode.o 

debug_func.o: debug_func.c debug_func.h

generate.o: generate.c generate.h

encoder.o: encoder.c encoder.h

modulate.o: modulate.c modulate.h

decode.o: decode.c decode.h

monitor.o: monitor.c monitor.h

clean:
	rm *.x *.o simulator debug_func
