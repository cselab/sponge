#!/bin/sh

set -xeu
: ${srun=srun}
l=9 m=12 r=2120
OMP_NUM_THREADS=1
export OMP_NUM_THREADS
cd "$1" &&
    $srun -u --cpus-per-task 1 --mpi=pmix $HOME/.local/bin/cylinder -v -r $r -l $l -m $m -p 100 -e 200 -f force.dat -d basilisk.dump -o h -b pp &&
    : > sim.done
