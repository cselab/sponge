.POSIX:
.SUFFIXES:
.SUFFIXES: .c

MPICC = mpicc
MPICCFLAGS = -O2 -g
M = \
cylinder\

all: $M
.c:
	$(MPICC) $< -o $@ $(MPICCFLAGS) $(LDFLAGS) -lm
clean:; rm -f $M
install: $M
	mkdir -p $(HOME)/.local/bin && \
	cp cylinder $(HOME)/.local/bin/
