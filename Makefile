.POSIX:
.SUFFIX:
.SUFFIX: .c

BASILISK = $(HOME)/basilisk/src
BASILISK_VIEW_FLAGS = `pkg-config --libs osmesa glu` -lm
MPICC = mpicc
MPICCFLAGS = -O2 -g

QCC = qcc
all: 3 distance stl sphere
_3.c: 3.c; $(QCC) $(QCCFLAGS) -D_MPI=1 3.c -source
3: _3.c; $(MPICC) -o $@ $(MPICCFLAGS) _3.c -lm
_distance.c: distance.c; $(QCC) $(QCCFLAGS) -D_MPI=1 distance.c -source
distance: _distance.c; $(MPICC) -o $@ $(MPICCFLAGS) _distance.c $(BASILISK)/gl/libglutils.a $(BASILISK)/gl/libfb_osmesa.a -lm $(BASILISK_VIEW_FLAGS)

_stl.c: stl.c; $(QCC) $(QCCFLAGS) -D_MPI=1 stl.c -source
stl: _stl.c; $(MPICC) -o $@ $(MPICCFLAGS) _stl.c $(BASILISK)/gl/libglutils.a $(BASILISK)/gl/libfb_osmesa.a -lm $(BASILISK_VIEW_FLAGS)

_sphere.c: sphere.c; $(QCC) $(QCCFLAGS) -D_MPI=1 sphere.c -source
sphere: _sphere.c; $(MPICC) -o $@ $(MPICCFLAGS) _sphere.c $(BASILISK)/gl/libglutils.a $(BASILISK)/gl/libfb_osmesa.a -lm $(BASILISK_VIEW_FLAGS)

clean:
	rm -f 3 _3.c distance _distance stl _stl sphere _sphere
