#!/bin/sh
set -eu

#l=9 m=12
: ${l=7} ${m=10} ${s=26} ${re=2000}
: ${mpiexec=mpiexec}

center=~/00000017/center.stl
make cylinder
(cd ../../dump && make stl2dump)
if test ! -f $center
then
    ../../stl/cylinder.py -n 64 ver.stl
    ../../stl/center.py ver.stl $center
fi
zlim=`../../stl/size.py $center`
if test ! -f basilisk.$l.$m.dump
then
    ../../dump/stl2dump -v -s $s -o  -w z $((m-1)) $zlim -- \
			-5 -6.25 -6.25 12.5  $l $m  64 $center basilisk.$l.$m.dump
fi
set -- $zlim
z=$2
mkdir -p $re
$mpiexec --map-by slot ./cylinder -v \
	 -Z $z \
    	 -r $re -p 1 -e 200 -f $re/force.dat -d basilisk.$l.$m.dump -o $re/h -b sp
