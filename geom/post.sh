#!/bin/sh
set -e

if test -z "$SLURM_ARRAY_TASK_ID"
then printf >&2 'sim.sh: SLURM_ARRAY_TASK_ID is not set\n'
     exit 1
fi
d=`printf '%08d' $SLURM_ARRAY_TASK_ID`
srun -n 1 -u --gres-flags disable-binding python3 -u post.py $d $d/h.slice.*.attr.raw
exec co.ffmpeg -o ~/$d.mp4 $d/*[05]00.png
