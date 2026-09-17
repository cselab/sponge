# sponge

Flow past a sponge, a porous cylindrical lattice (parameters in
[geom/README.md](geom/README.md)), simulated with
[Basilisk](http://basilisk.fr). The sponge geometry is an STL file
converted to a Basilisk dump, which the solver
[cylinder/cylinder.c](cylinder/cylinder.c) restores with `-d`. The
solver also has analytic shapes (`-S cylinder`, `-S sphere`) used for
validation.

Files:

- [geom/gen.py](geom/gen.py): sponge parameters (`config.py`) to `ver.stl`
- [stl/center.py](stl/center.py): center the STL, scale the diameter to 1, axis along z
- [stl/size.py](stl/size.py): print z extent of an STL
- [dump/stl2dump.c](dump/stl2dump.c): STL to Basilisk dump, `-w z LEVEL Z0 Z1` adds walls at the sponge ends
- [dump/dump2xdmf.c](dump/dump2xdmf.c): dump to XDMF for ParaView
- [cylinder/cylinder.c](cylinder/cylinder.c): solver, `cylinder -h` lists options
- [cylinder/deploy/cylinder.c](cylinder/deploy/cylinder.c): preprocessed solver source, needs only mpicc
- [geom/0pre.sh](geom/0pre.sh), [geom/0run.sh](geom/0run.sh): SLURM scripts used for production runs

## Install

Requires MPI, GNU make, Python 3 with numpy and scipy.

```sh
wget -q http://basilisk.fr/basilisk/basilisk.tar.gz
tar zxf basilisk.tar.gz
cd basilisk/src
cp config.gcc config    # macOS: cp config.osx config
(MAKEFLAGS=-j`nproc`; make ast && make qcc)
mkdir -p "$HOME/.local/bin" && cp qcc "$HOME/.local/bin/"
```

The path to basilisk/src is compiled into qcc, do not move the
directory after building.

```sh
(cd cylinder && make cylinder)
(cd dump && make stl2dump dump2xdmf)
```

On macOS use `make cylinder QCCFLAGS=-D_DARWIN_C_SOURCE` and
`make stl2dump dump2xdmf CC=gcc-15` (Homebrew gcc, for OpenMP).

## Run

```sh
mkdir sponge0 && cd sponge0
cat > config.py <<'EOF'
NV = 20
NC = 30
AA = 0.25
BB = 0.5
LOOP_NO = 1
R_E = 1.2
NumBelix = 1
NumBelix2 = 0
EOF
PYTHONPATH=. python3 ../geom/gen.py
../stl/center.py -v ver.stl center.stl
set -- $(../stl/size.py center.stl)
../dump/stl2dump -v -o -w z 6 $1 $2 -- -5 -6.25 -6.25 12.5  4 7  4 center.stl basilisk.dump
mpiexec -n 4 ../cylinder/cylinder -v -r 100 -p 100 -e 200 -Z $2 \
        -f force.dat -d basilisk.dump -o h -b pp
```

stl2dump arguments: `X0 Y0 Z0 L minlevel maxlevel npe file.stl
out.dump`. Production runs use levels 10 13, see
[geom/0pre.sh](geom/0pre.sh).

Output: `force.dat` (columns: step, t, dt, total force, pressure force,
viscous force, three components each), `h.y.*.xdmf2` and `h.z.*.xdmf2`
slices for ParaView, `h.*.dump` restart files (restart with
`-d h.N.dump -i`).
