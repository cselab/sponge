| Parameter Name | Description                   | Range in mm |
|----------------|-------------------------------|-------------|
| R              | Sponge radius                 | 11          |
| H              | Sponge height                 | 100         |
| D_SHELIX       | Spacing of small double helixes | 1.3       |
| NV             | # vertical beams              | (20,50)     |
| NC             | # circumferential beams       | (30,74)     |
| AA             | Beam cross section height | (0.15,0.65)  |
| BB             | Beam cross section width/thickness  | (0.15,0.65)  |
| LOOP_NO        | # of big helix loops          | (1,6)       |
| R_E            | Radius of big helix major axis cross section | (0.6, 1.6)  |
| NumBelix     | # CW big helix                  | (0,3)       |
| NumBelix2     | # CCW big helix                 | (0,3)       |

Definition of cross section:

<img src="DefinitionParameter.png" alt="Cefinition of cross section:" width="600" >


To generate STL files
```
$ python run.py
$ for i in */; do (cd $i; PYTHONPATH=. python ../gen.py); done
```

For paraview
```
$ for i in */ver.stl; do echo cp $i a.`dirname $i`.stl; done | sh
```

To generate dump
```
$ make
$ python ../stl/cylinder.py
$ ./stl2dump -o -- -5 -6.25 -6.25 12.5  6 9  64 center.stl basilisk.dump
$ ./dump2xdmf basilisk.dump basilisk
```

To generate dump with z-wall
```
$ ./stl2dump -v -w z 8 -2 2 -o -- -5 -6.25 -6.25 12.5  6 9  64 center.stl basilisk.dump
$ ./dump2xdmf basilisk.dump basilisk
$ mpiexec -n 4 cylinder -v -r 2120 -l 6 -m 9 -p 10 -e 200 -f force -d basilisk.dump -o h -b pp
```

l=10 m=13
00000022 cyliner
00000023 A
00000024 I
00000025 D
00000026 H

l=9 m=12
x00000022 cyliner
x00000023 A
x00000024 I
x00000025 D
x00000026 H

l=10 m=13 nowall
n.00000022 cyliner
n.00000023 A
n.00000024 I
n.00000025 D
n.00000026 H

l=9 m=12 nowall
n.x00000022 cyliner
n.x00000023 A
n.x00000024 I
n.x00000025 D
n.x00000026 H
