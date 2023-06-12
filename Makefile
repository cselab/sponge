.POSIX:
.SUFFIX:
.SUFFIX: .c

BASILISK = $(HOME)/basilisk/src
MPICC = mpicc
MPICCFLAGS = -O2 -g
CFLAGS = -O2 -g

QCC = qcc
all: 3
_3.c: 3.c
	$(QCC) $(QCCFLAGS) -D_MPI=1 3.c -source
3: _3.c
	$(MPICC) -o $@ $(MPICCFLAGS) _3.c -lm
clean:
	rm -f 3 _3.c
