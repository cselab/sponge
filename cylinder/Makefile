.POSIX:
.SUFFIXES:
QCC = qcc
QCC_DISABLE_DIMENSIONS = -disable-dimensions
MPICC = mpicc
MPICCFLAGS = -O2 -g
LDFLAGS = -lm

deploy/cylinder.c: _cylinder.c
	mv _cylinder.c deploy/cylinder.c
_cylinder.c: cylinder.c
	CC99=$(MPICC) $(QCC) $(QCC_DISABLE_DIMENSIONS) -source $(QCCFLAGS) -D_MPI=1 cylinder.c
cylinder: cylinder.c centered.h output_xdmf.h poisson.h viscosity-embed.h
	CC99=$(MPICC) $(QCC) $(QCC_DISABLE_DIMENSIONS) $(QCCFLAGS) -D_MPI=1 $(MPICCFLAGS) $< -o $@ $(LDFLAGS)
_cylinder.c: centered.h output_xdmf.h

clean:
	rm -f _cylinder.c cylinder _cylinder0.c cylinder0
