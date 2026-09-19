#!/bin/sh
set -eu

#l=9 m=12
: ${l=7} ${m=10} ${s=26}
: ${mpiexec=mpiexec}

center=../../real.stl
make cylinder
(cd ../../dump && make dump2xdmf stl2dump)
if test ! -f basilisk.$l.$m.dump
then
    ../../dump/stl2dump -v -s $s -o  -- \
			-5 -6.25 -6.25 12.5  $l $m  64 $center basilisk.$l.$m.dump
fi
$mpiexec ./cylinder -v \
    	 -r 2000 -l $l -m $m -p 1 -e 200 -f force.dat -d basilisk.$l.$m.dump -o h -b sp
