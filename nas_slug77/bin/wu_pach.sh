#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEOUT="/etc/wu-$FILEDATE.log"
LINE="`tail -1 $FILEOUT`"
#l2="0  1 4 5 6 7 8 9 10 11 12 13 14 15 16 17 "
#l2="-1 -1 2 4 5 3 6 7 8 -1 9 -1 10 11 -1 12 -1 -1"
l2="-1 -1 2 4 5 3 6 -1 -1 -1 -1 -1 10 11 -1 12 -1 -1"

#echo "$LINE"

i=0
TOUT=""

for c in $LINE
 do
  #echo "$c"
  HD="`echo $l2 | cut -d " " -f$i | cut -d " " -f0`"
  if [ "$HD" != "-1" ] ; then
#    echo "Temp: $TOUT" ;
	tmp="$tmp$HD,$c\r\n" ;
#	echo "tmp: $tmp" ;
#    TOUT=`echo -e "$tmp"`

#        echo "tout: $TOUT"  ;
  fi
  let $((i++))
 done
TOUT=`echo -e "$tmp"`
curl --request PUT --header "X-PachubeApiKey: $PachubeApiKey" --data "$TOUT" "api.pachube.com/v2/feeds/6449.csv" >/dev/null 2>&1 
#>/dev/null 2>&1
echo "$TOUT"
exit 0

