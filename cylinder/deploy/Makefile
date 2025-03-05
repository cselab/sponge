.POSIX:
.SUFFIXES:

MPICC = mpicc
MPICCFLAGS = -O2 -g
BASILISK = $(HOME)/basilisk/src

all: cylinder
cylinder: cylinder.c
	$(MPICC) $< -o $@ $(MPICCFLAGS) $(LDFLAGS) -I$(BASILISK) -lm
clean:; rm -f -- cylinder
install: cylinder
	mkdir -p -- '$(HOME)/.local/bin' && \
	cp -- cylinder '$(HOME)/.local/bin/'
