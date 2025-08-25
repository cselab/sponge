#!/bin/sh

l=6 m=$((l+2))
make
mpiexec.openmpi ./cylinder -v -r 2000 -l $l -m $m -p 1 -e 200 -f force.dat -o h -b sp -S cylinder -z 12.5
