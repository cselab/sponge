```
make
../stl/cylinder.py /u/t
./stl2dump -v -- -1 -1 -1 2   3 6      2 center.stl basilisk.dump
./dump2xdmf basilisk.dump basilisk
mpiexec -n 2 ../cylinder -v -r 100 -l 3 -m 6 -p 1 -e 3000 -f force.dat -z 2 -S cylinder -o h -d basilisk.dump
```

Production
```
l=7 m=11
../stl/cylinder.py
../stl/center.py cylinder.stl
zlim=`../stl/size.py center.stl`
./stl2dump -s 25 -o -v -w z $((m-1)) $zlim -- -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump
./dump2xdmf basilisk.dump basilisk
```
