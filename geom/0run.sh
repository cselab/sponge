#!/bin/sh
. /etc/profile
module purge
module load python gcc/13 openmpi
sbatch -J sim --mem 0 --exclusive -N 16 -p sapphire -t 3- --array 0,2,4,6 --wrap '
set -xeu
l=10 m=13
d=`printf %08d $SLURM_ARRAY_TASK_ID`
if ! test -d "$d"
then printf >&2 "0run.sh: error: not a directory %s\n" "$d"
     exit 1
fi
OMP_NUM_THREADS=1
export OMP_NUM_THREADS
PYTHONNOUSERSITE=
export PYTHONNOUSERSITE
cd "$d" &&
    r=`cat re`
    set -- `srun python3 ../../stl/size.py center.stl` &&
    zlim=$2
    for i in h.[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].dump; do : ; done &&
    for j in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25; do if ! test -f force.$j.dat; then break; fi; done &&
    srun -u --cpus-per-task 1 --mpi=pmix $HOME/.local/bin/cylinder -v -i -Z $zlim \
    	 -r $r -l $l -m $m -p 100 -e 200 -f force.$j.dat -d $i -o h -b pp &&
    : > sim.done
'
