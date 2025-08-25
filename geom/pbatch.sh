#!/bin/sh

. /etc/profile
module purge
module load python
exec sbatch -J post --mem 0 --exclusive -N 1 -p seas_compute -t 60 --array 256-465 post.sh
