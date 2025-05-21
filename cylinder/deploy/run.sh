#!/bin/sh
set -eu

l=5 m=7
#l=10 m=13
make
(cd ../../dump && make dump2xdmf stl2dump)
../../stl/cylinder.py -n 64 ver.stl
../../stl/center.py ver.stl center.stl
zlim=`../../stl/size.py center.stl`
../../dump/stl2dump -s 26 -o -w z $((m-1)) $zlim -- \
		    -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump
set -- $zlim
z=$2
mpiexec ./cylinder -v -i -Z $z \
    	 -r 2000 -l $l -m $m -p 1 -e 200 -f force.dat -d basilisk.dump -o h -b pp
