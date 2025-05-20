#!/bin/sh

l=6 m=10
make
mpiexec ./cylinder -v -r 2000 -l $l -m $m -p 10 -e 200 -f force.dat -o h -b pp -S sphere -z 12.5
