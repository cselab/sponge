#!/bin/sh
xargs --process-slot-var I -n 1 -P 4 sh -xc '
n=128 l=7 m=11 k=16
re=$0
set 0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30 \
    32,34,36,38,40,42,44,46,48,50,52,54,56,58,60,62 \
    64,66,68,70,72,74,76,78,80,82,84,86,88,90,92,94 \
    96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126
shift $I
mkdir -p $re
cd $0 &&
   mpiexec -n $k --bind-to core --map-by core --cpu-list $1 \
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
