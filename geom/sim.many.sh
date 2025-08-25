#!/bin/sh

set -xeu
: ${srun=srun}
l=8 m=11 r=`cat re`
if test -z "$SLURM_ARRAY_TASK_ID"
then printf >&2 'sim.many.sh: SLURM_ARRAY_TASK_ID is not set\n'
     exit 1
fi

d=`printf '%08d' $SLURM_ARRAY_TASK_ID`
if ! test -d "$d"
then printf >&2 'sim.many.sh: error: not a directory %s\n' "$d"
     exit 1
fi
OMP_NUM_THREADS=1
export OMP_NUM_THREADS
cd "$d" &&
    $srun -u --cpus-per-task 1 --mpi=pmix $HOME/.local/bin/cylinder -v -r $r -l $l -m $m -p 100 -e 200 -f force.dat -d basilisk.dump -o h -b pp &&
    : > sim.done
