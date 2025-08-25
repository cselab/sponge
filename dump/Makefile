.POSIX:
.SUFFIXES:
.SUFFIXES: .c
.PHONY: install clean all
MPICC = mpicc
MPICCFLAGS = -O2 -g
LDFLAGS = -lm
QCC = qcc
QCCFLAGS = -events -debug -progress
QCC_DISABLE_DIMENSIONS = -disable-dimensions
PREFIX = $(HOME)/.local
CFLAGS = -O3 -g
M = \
dump_2iso \
dump2xdmf \
dump_info \
dump_coarse \
dump_move \
dump_select \
morton \
stl2dump \
tile \

all: $M
.c:; $(CC) $< $(CFLAGS) -o $@
stl2dump: stl2dump.c
	$(CC) $< -fopenmp $(CFLAGS) -o $@ -lm
tree_check: tree_check.c
	CC99=$(MPICC) $(QCC) $(QCCFLAGS) $(QCC_DISABLE_DIMENSIONS) -D_MPI=1 $(MPICCFLAGS) $< -o $@ $(LDFLAGS)
clean:; rm -r $M tree_check
install: stl2dump dump2xdmf
	mkdir -p $(HOME)/.local/bin
	cp stl2dump dump2xdmf $(HOME)/.local/bin
stl2dump: predicate_c.inc predicate.inc
