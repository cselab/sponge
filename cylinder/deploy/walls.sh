#!/bin/sh
set -eu

#l=9 m=12
: ${l=7} ${m=10} ${s=26} ${n=4} ${k=16}
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

awk 'BEGIN {
    n = '$n'
    k = '$k'
    for (i = 0; i < n; i++) {
        printf "" > "rank." i
        for (j = 0; j < k; j++) {
            printf "rank %d=localhost slot=%d\n", j, k * i + j >> "rank." i
        }
    }
}'

xargs --process-slot-var I -n 1 -P $n sh -xc '
re=$0
mkdir -p $re
mpiexec --oversubscribe -n '$k' ./cylinder -v \
	 -Z '$z' \
    	 -r $re -p 100 -e 200 -f $re/force.dat -d basilisk.'$l'.'$m'.dump -o $re/h -b sp >$re/stdout 2>$re/stderr
echo $? > $re/status
' <<'!'
0025
0050
0100
0200
0400
0800
1600
2000
3200
!
