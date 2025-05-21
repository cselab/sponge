#!/bin/sh
xargs --process-slot-var I -n 1 -P 4 sh -xc '
n=128 l=7 m=11 k=16
re=$0
mkdir -p $re
cd $0 &&
   mpiexec -n $k --bind-to core --map-by core:PE=2 \
   --report-bindings ../cylinder \
   -v -r $re -l $l -m $m -p 100 -e 10 -f force.dat -o h -b pp -S sphere -z 12.5 >stdout 2>stderr
   echo $? > status
' <<'!'
0025
0050
0100
0200
0400
0800
1600
3200
!
