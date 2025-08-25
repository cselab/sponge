#!/bin/sh

set -xeu
: ${srun=srun}
l=9 m=12
PYTHONNOUSERSITE=
export PYTHONNOUSERSITE
cd "$1" &&
    PYTHONPATH=. $srun python3 -u ../gen.py &&
    $srun python3 -u ../center.py ver.stl &&
    zlim=`$srun python3 ../../stl/size.py center.stl` &&
    OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE ${srun=srun} \
		   ../stl2dump -o -v -w z $((m-1)) $zlim -- \
		   -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump &&
    ${srun=srun} ../dump2xdmf basilisk.dump basilisk &&
    : > pre.done
