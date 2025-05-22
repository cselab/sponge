#!/bin/sh

l=6 m=8
make
mpiexec.openmpi ./cylinder -v -r 100 -l $l -m $m -p 1 -e 200 -f force.dat -o h -b pp -S sphere -z 12.5
