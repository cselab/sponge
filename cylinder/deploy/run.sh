#!/bin/sh
set -eu

#l=9 m=12
: ${l=7}
: ${m=10}
: ${mpiexec=mpiexec}

make cylinder
(cd ../../dump && make dump2xdmf stl2dump)
if test ! -f center.stl
then
    ../../stl/cylinder.py -n 64 ver.stl
    ../../stl/center.py ver.stl center.stl
fi
zlim=`../../stl/size.py center.stl`
if test ! -f basilisk.$l.$m.dump
then
    ../../dump/stl2dump -v -s 26 -o -w z $((m-1)) $zlim -- \
			-5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.$l.$m.dump
fi
set -- $zlim
zlim=$2
$mpiexec ./cylinder -v -i -Z $zlim \
    	 -r 2000 -l $l -m $m -p 1 -e 200 -f force.dat -d basilisk.$l.$m.dump -o h -b pp
