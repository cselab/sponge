#!/bin/sh
xargs --process-slot-var I -n 1 -P 4 sh -xc '
n=128 l=7 m=11
i=$((n*I/4)) j=$((n*(I+1)/4-1)) k=$((n/4))
mkdir -p $0
cd $0 &&
   taskset --cpu-list $i-$j \
   mpiexec -n $k --bind-to core --map-by core --report-bindings ../cylinder \
   -v -r 100 -l $l -m $m -p 100 -e 10 -f force.dat -o h -b pp -S sphere -z 12.5 >stdout 2>stderr
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
