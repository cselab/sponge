#!/bin/sh
# sbatch -p seas_compute --array 0-8 -t 3- --mem 80Gb --wrap 'sh -x mv.sh' -J move
d=`printf %08d $SLURM_ARRAY_TASK_ID`
cd "$d" &&
    for i in h.*.dump; do : ; done &&
    ~/.local/bin/dump_move $i basilisk2.dump basilisk3.dump &&
    mv $i $i.bak &&
    mv basilisk3.dump $i
