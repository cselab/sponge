#!/bin/sh

l=7 m=11
make
mpiexec ./cylinder -v -r 100 -l $l -m $m -p 10 -e 200 -f force.dat -o h -b pp -S sphere -z 12.5
