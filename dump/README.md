Generate
```
l=7 m=11
../stl/cylinder.py cylinder.stl
../stl/center.py cylinder.stl
zlim=`../stl/size.py center.stl`
./stl2dump -s 25 -o -v -w z $((m-1)) $zlim -- -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump
./dump2xdmf basilisk.dump basilisk
```
