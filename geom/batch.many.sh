#!/bin/sh

. /etc/profile

first= last=
for d in [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]/
do 
   d=${d%/}
   if test ! -f $d/pre.done
   then if test -z $first; then first=$d; fi
	last=$d
   fi
done

if ! test -z $first
then
   module purge
   module load python gcc openmpi
   make 'CFLAGS = -Ofast -march=sapphirerapids' &&
       i=`sbatch --parsable -J pre --mem 0 --exclusive -N 1 -p sapphire -t 1-00:00:00 --array $first-$last run.many.sh`
   sbatch -J sim --mem 0 --exclusive -N 2 -p sapphire -t 3-00:00:00 --array $first-$last --dependency afterok:$i sim.many.sh
   else exit 1
fi
