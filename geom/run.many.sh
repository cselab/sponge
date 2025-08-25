#!/bin/sh

set -xeu
: ${srun=srun}
l=8 m=11
if test -z "$SLURM_ARRAY_TASK_ID"
then printf >&2 'sim.sh: SLURM_ARRAY_TASK_ID is not set\n'
     exit 1
fi

d=`printf '%08d' $SLURM_ARRAY_TASK_ID`
if ! test -d "$d"
then printf >&2 'sim.sh: error: not a directory %s\n' "$d"
     exit 1
fi
PYTHONNOUSERSITE=
export PYTHONNOUSERSITE
cd "$d" &&
    $srun python3 -u ../center.py ver.stl &&
    zlim=`$srun python3 ../../stl/size.py center.stl` &&
    OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE ${srun=srun} \
		   ../stl2dump -o -v -w z $((m-1)) $zlim -- \
		   -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk.dump &&
    ${srun=srun} ../dump2xdmf basilisk.dump basilisk &&
    : > pre.done
