```
make
../stl/cylinder.py /u/t
./stl2dump -v -- -1 -1 -1 2   3 6      2 center.stl basilisk.dump
./dump2xdmf basilisk.dump basilisk
mpiexec -n 2 ../cylinder -v -r 100 -l 3 -m 6 -p 1 -e 3000 -f force.dat -z 2 -S cylinder -o h -d basilisk.dump
```

Production
```
../stl/center.py '/u/tsponge/geometry/No helix.stl'
./stl2dump -v -- -20 -25 -25 50   8 11      2 center.stl basilisk.dump
d=.
./cylinder -v -r 220 -l 8 -m 11 -p 10 -e 3000 -f $d/force.dat -z 50 -S cylinder -o $d/h -b t -d basilisk.dump
```
