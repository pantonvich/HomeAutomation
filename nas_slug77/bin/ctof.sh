#!/bin/bash
tf=${1#-}

if [ ${#tf} -ne ${#1} ]; then 
  posneg=-1
else
  posneg=1
fi

tf=${tf#0}
tf=${tf#0}
tf=${tf#0}
tf=${tf#0}
dot=`expr index $tf .`
len=${#tf}
ff=$((`echo $tf | sed 's/\.//g'`*18))
dot1=$((${#tf}-$dot+1))

s=$((${#ff}-$dot1))


echo $((`expr substr $ff 1 $s`*posneg+32))"."`expr substr $ff $(($s+1)) $(($dot+1))`


