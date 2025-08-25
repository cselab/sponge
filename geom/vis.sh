python3 run.py
sh unzip.sh

for i in `seq 0 305`
do i=`printf %08d $i`
   (
       cd $i
       PYTHONPATH=. python3 ../gen.py
       pvbatch ../vis.py
       convert ver.png -gravity NorthWest -pointsize 90 -fill blue -annotate +0+100 $i ver0.png
       scp ver0.png eth:homepage/sponge/ver.$i.png
   )
done
