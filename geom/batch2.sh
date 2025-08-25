#!/bin/sh

. /etc/profile
list='545 413 327 509'
if ! test -f sponge-optimization-input/clone.done
then git clone https://github.com/timon-meier/sponge-optimization-input &&
	: > sponge-optimization-input/clone.done
else (cd sponge-optimization-input
      git pull)
fi

module purge
module load python gcc openmpi
PYTHONNOUSERSITE= python3 -m pip install scipy --user
make 'CFLAGS = -Ofast -march=sapphirerapids'
for i in $list
do d=`printf %08d $i`
   if test ! -f $d/pre.done
   then mkdir -p $d
	cp sponge-optimization-input/$d/config.py $d/
	i=`sbatch --parsable -J pre --mem 0 --exclusive -N 1 -p sapphire -t 3-00:00:00 run2.sh $d`
	sbatch -J sim --mem 0 --exclusive -N 8 -p sapphire -t 3-00:00:00 --dependency afterok:$i sim2.sh $d
   else sbatch -J sim --mem 0 --exclusive -N 8 -p sapphire -t 3-00:00:00 sim2.sh $d
   fi
done
exit
