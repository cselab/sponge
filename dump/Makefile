.POSIX:
.SUFFIXES:
.SUFFIXES: .c
MPICC = mpicc
MPICCFLAGS = -O2 -g
QCC = qcc
QCC_DISABLE_DIMENSIONS = -disable-dimensions
PREFIX = $(HOME)/.local
CFLAGS = -O3 -g
M = \
dump_2iso \
dump2xdmf \
dump_info \
dump_select \
morton \
stl2dump \
tree_check\

all: $M
.c:; $(CC) $< $(CFLAGS) -o $@
stl2dump: stl2dump.c
	$(CC) $< -fopenmp $(CFLAGS) -o $@ -lm
tree_check: tree_check.c
	$(QCC) $(QCC_DISABLE_DIMENSIONS) -source -D_MPI=1 $<
	$(MPICC) $(MPICCFLAGS) _$< -o $@ -lm

clean:; -rm $M _tree_check.c

install: all
	mkdir -p -- '$(PREFIX)/bin' && cp -- sdf2lmp split '$(PREFIX)/bin'
