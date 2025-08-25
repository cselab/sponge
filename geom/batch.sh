#!/bin/sh

. /etc/profile
if ! test -f sponge-optimization-input/clone.done
then git clone https://github.com/timon-meier/sponge-optimization-input &&
	: > sponge-optimization-input/clone.done
else (cd sponge-optimization-input
      git pull)
fi

first= last=
for i in sponge-optimization-input/[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]/config.py
do d=${i%%/config.py}
   d=${d##*/}
   if test ! -f $d/pre.done
   then if test -z $first; then first=$d; fi
	last=$d
	mkdir -p $d
	cp $i $d/
   fi
done

if ! test -z $first
then
	module purge
	module load python gcc openmpi
	make 'CFLAGS = -Ofast -march=sapphirerapids' &&
		i=`sbatch --parsable -J pre --mem 0 --exclusive -N 1 -p sapphire -t 1-00:00:00 --array $first-$last run.sh`
	exec sbatch -J sim --mem 0 --exclusive -N 2 -p sapphire -t 3-00:00:00 --array $first-$last --dependency afterok:$i sim.sh
else exit 1
fi
