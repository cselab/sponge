.POSIX:
.SUFFIXES:
.SUFFIXES: .c
CC = gcc
CFLAGS = -O3 -g
CFLAGS_OPENMP = -fopenmp

all: stl2dump dump2xdmf
stl2dump: stl2dump.c
	$(CC) $< $(CFLAGS_OPENMP) $(CFLAGS) -o $@ -lm
.c:; $(CC) $< $(CFLAGS) -o $@
clean:
	-rm stl2dump dump2xdmf
