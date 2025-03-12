while read p r d
do echo $p $r $d
   mkdir -p $d
   python ascii2bin.py $p $d/ver.stl
   echo $r > $d/re
   echo Geometry_mod/$p > $d/info
   rsync -avzr $d rc:/n/netscratch/koumoutsakos_lab/Lab/slitvinov/sponge_many3/'Create Sponge Geometry STL'/
done <<'!'
327_CFD_NoHelix.stl 2262 00000011
327_CFD_NoPores.stl 2262 00000012
413_CFD_NoHelix.stl 2233 00000013
413_CFD_NoPores.stl 2233 00000014
545_CFD_NoHelix.stl 2221 00000015
545_CFD_NoPores.stl 2221 00000016
!
