#!/bin/sh

c=0
for i in ~/[0-9]_*/
do  for s in ${i}*_binary.stl; do : ; done
    for re in `awk 'sub(/^AVG[ ]*-[ ]*Re=/, "") {
     print
   }' ${i}Re_overview_[0-9]*.txt`
    do d=`printf '%08d' $c`
       mkdir -p "$d"
       cp "$s" "$d/ver.stl"
       echo $re > "$d/re"
       echo "$s"  > "$d/info"
       c=$((c + 1))
    done
done
