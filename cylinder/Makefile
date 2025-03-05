.POSIX:
.SUFFIXES:
.SUFFIXES: .c
CFLAGS = -O2 -g
QCC = qcc
QCC_DISABLE_DIMENSIONS = -disable-dimensions
MPICC = mpicc
BASILISK = $(HOME)/basilisk/src
deploy/cylinder.c: _cylinder.c
	mv _cylinder.c deploy/cylinder.c
_cylinder.c: cylinder.c
	CC99=$(MPICC) $(QCC) $(QCC_DISABLE_DIMENSIONS) -source $(QCCFLAGS) -D_MPI=1 cylinder.c
clean:
	rm -f _cylinder.c cylinder
_cylinder.c: output_xdmf.h
