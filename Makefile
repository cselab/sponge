.POSIX:
.SUFFIX:

CC = cc
BASILISK = $(HOME)/basilisk/src
BASILISK_VIEW_FLAGS = `pkg-config --libs osmesa glu` -lm
MPICC = mpicc
MPICCFLAGS = -O2 -g
CFLAGS = -O2 -g
V_LIBS = $(BASILISK)/gl/libglutils.a $(BASILISK)/gl/libfb_osmesa.a -lm $(BASILISK_VIEW_FLAGS)

QCC = qcc
all: 3 distance stl_mpi stl_single sphere

_3.c: 3.c; CC99=$(MPICC) $(QCC) $(QCCFLAGS) -D_MPI=1 3.c -source
3: _3.c; $(MPICC) -o $@ $(MPICCFLAGS) _3.c -lm

_distance.c: distance.c; CC99=$(MPICC) $(QCC) $(QCCFLAGS) -D_MPI=1 distance.c -source
distance: _distance.c; $(MPICC) -o $@ $(MPICCFLAGS) _distance.c $(V_LIBS)

stl_mpi: stl.c
	CC99=$(MPICC) $(QCC) $(QCCFLAGS) -D_MPI=1 stl.c -source && \
	$(MPICC) -o $@ $(CFLAGS) _stl.c -lm || rm _stl.c

stl_single: stl.c
	$(QCC) $(QCCFLAGS) stl.c -source && \
	$(CC) -o $@ $(CFLAGS) _stl.c -lm || rm _stl.c

tangaroa_mpi: tangaroa.c
	CC99=$(MPICC) $(QCC) $(QCCFLAGS) -D_MPI=1 tangaroa.c -source && \
	$(MPICC) -o $@ $(CFLAGS) _tangaroa.c $(V_LIBS) || rm _tangaroa.c

tangaroa_single: tangaroa.c
	$(QCC) $(QCCFLAGS) tangaroa.c -source && \
	$(CC) -o $@ $(CFLAGS) _tangaroa.c $(V_LIBS) || rm _tangaroa.c


_sphere.c: sphere.c; CC99=$(MPICC) $(QCC) $(QCCFLAGS) -D_MPI=1 sphere.c -source
sphere: _sphere.c; $(MPICC) -o $@ $(MPICCFLAGS) _sphere.c $(V_LIBS)

clean:
	rm -f 3 _3.c distance _distance stl _stl sphere _sphere
