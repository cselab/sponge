#!/bin/sh
module load gcc openmpi
set -x
r=220
d=$SCRATCH/koumoutsakos_lab/slitvinov/$r
make MPICC=mpicc 'MPICCFLAGS = -O2 -g' && {
    mkdir -p $d
    exec srun -n $SLURM_NTASKS --mpi=pmix ./cylinder -v -r $r -l 8 -m 11 -p 10 -e 3000 -f $d/force.dat -z 50 -S cylinder -o $d/h -b t -d $r/h.000001000.dump
}
