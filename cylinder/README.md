# Install basilisk

<pre>
$ wget -q http://basilisk.fr/basilisk/basilisk.tar.gz
$ tar zxf basilisk.tar.gz
$ cd basilisk/src
</pre>

On Linux:
<pre>
$ cp config.gcc config
</pre>

On macOS
<pre>
$ cp config.osx config
</pre>

<pre>
$ (MAKEFLAGS=-j`nproc`; make ast && make qcc)
$ cp qcc "$HOME/.local/bin/"
</pre>

# Build

<pre>
$ make
CC99=mpicc qcc -disable-dimensions -source  -D_MPI=1 cylinder.c
./output_xdmf.h:9: warning: Basilisk C parse error near `MPI_File mpi_file'
mv _cylinder.c deploy/cylinder.c
</pre>

<pre>
$ make install
mpicc cylinder.c -o cylinder -O2 -g  -I/home/lisergey/basilisk/src -lm
mkdir -p -- '/home/lisergey/.local/bin' && \
cp -- cylinder '/home/lisergey/.local/bin/'
<pre>

# Examples

<pre>
$ mpiexec cylinder -v -r 2120 -l 8 -m 11 -p 100 -e 200 -f force.dat -S cylinder -o h -b pn -z 5
$ mpiexec cylinder -v -r 2120 -l 5 -m 8 -p 100 -e 200 -f force.dat -S cylinder -o h -b pn -z 5
</pre>


right, top, front
