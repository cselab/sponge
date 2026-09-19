: ${l=6} ${m=10}
./stl2dump -s 25 -o -v -- -5 -6.25 -6.25 12.5  $l $m  64 ../real.stl basilisk.dump

make tree_check
mpiexec ./tree_check -v basilisk.dump
