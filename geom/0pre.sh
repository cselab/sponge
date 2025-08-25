#!/bin/sh

. /etc/profile
module purge
module load python gcc openmpi

sbatch -J pre --mem 0 --exclusive -N 1 -p sapphire -t 3-00:00:00 --array 17-17 --wrap '
l=10 m=13
d=`printf %08d $SLURM_ARRAY_TASK_ID`
if ! test -d "$d"
then printf >&2 "0pre.sh: error: not a directory %s\n" "$d"
     exit 1
fi
PYTHONNOUSERSITE=
export PYTHONNOUSERSITE
cd "$d" &&
    srun python3 -u ../../stl/center.py ver.stl &&
    zlim=`srun python3 ../../stl/size.py center.stl` &&
    OMP_NUM_THREADS=$SLURM_CPUS_ON_NODE srun \
		   $HOME/.local/bin/stl2dump -o -v -w z $((m-1)) $zlim -- \
		   -5 -6.25 -6.25 12.5  $l $m  64 center.stl basilisk2.dump &&
    srun $HOME/.local/bin/dump2xdmf -s y basilisk2.dump basilisk2 &&
    : > pre2.done
'
