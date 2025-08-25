cd ..
CC99=mpicc $HOME/.local/bin/qcc -disable-dimensions -debug -events -D_MPI=1 -DTRASH=1 cylinder.c -o deploy/cylinder -lm -O0 -g3
