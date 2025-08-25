#!/bin/sh
set -e

for i in `seq 255 -1 140`
do d=`printf %08d $i`
   echo $d
   srun -u --gres-flags disable-binding python3 -u post.py $d $d/h.slice.*.attr.raw
   co.ffmpeg -o $d.mp4 $d/*.png
   #cp $d/force.dat $d.dat
   for f in $d/*.png; do :; done
   cp $f $d.png
done
