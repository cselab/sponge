.POSIX:
.SUFFIXES:
.SUFFIXES: .c

MPICC = mpicc
MPICCFLAGS = -O2 -g
BASILISK = $(HOME)/basilisk/src
M = \
cylinder\
cylinder0\

all: $M
.c:
	$(MPICC) $< -o $@ $(MPICCFLAGS) $(LDFLAGS) -I$(BASILISK) -lm
clean:; rm -f $M
install: $M
	mkdir -p $(HOME)/.local/bin && \
	cp cylinder cylinder0 $(HOME)/.local/bin/
