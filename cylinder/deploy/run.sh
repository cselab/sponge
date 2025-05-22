#!/bin/sh
set -eu

#l=9 m=12
l=7 m=10
make
(cd ../../dump && make dump2xdmf stl2dump)
# ../../stl/cylinder.py -n 64 ver.stl
# ../../stl/center.py ver.stl center.stl
zlim=`../../stl/size.py center.stl`
../../dump/stl2dump -v -s 26 -o -w z $((m-1)) $zlim -- \
		    -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump
set -- $zlim
zlim=$2
mpiexec.openmpi ./cylinder -v -i -Z $zlim \
    	 -r 2000 -l $l -m $m -p 10 -e 200 -f force.dat -d basilisk.dump -o h -b pp
