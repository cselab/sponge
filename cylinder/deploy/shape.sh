#!/bin/sh
: ${l=7} ${m=11} ${n=4} ${k=18} ${shape=sphere}

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
cd $0 &&
   mpiexec -n '$k' --bind-to core --rankfile ../rank.$I --report-bindings \
   ../cylinder -v -r $re -l '$l' -m '$m' -p 100 -e 100 -f force.dat -o h -b sp -S '$shape' -z 12.5 >stdout 2>stderr
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
