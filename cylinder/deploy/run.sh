#!/bin/sh

l=8
m=11
(cd ../paraview && make dump2xdmf stl2dump)
#../stl/cylinder.py
 ../stl/center.py $HOME/cfd.stl
../paraview/stl2dump -o -v -- -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump
../paraview/dump2xdmf basilisk.dump basilisk
# exec mpiexec ./cylinder -v -r 220 -l 6 -m 10 -p 100 -e 3000 -d basilisk.dump -o h
