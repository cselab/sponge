set -e
p=data/NextSamples.zip
zipinfo -1 $p | grep '/config.py$' | while IFS='
' read -r i
do d=${i%/config.py}
   d=${d#*/}
   case "$d" in
       [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9])
	   unzip -q -o -j $p "$i" -d "$d"
	   ;;
       *) printf "unzip.sh: error: wrong directory '%s'\n" "$d"
	  exit 1
   esac
done
