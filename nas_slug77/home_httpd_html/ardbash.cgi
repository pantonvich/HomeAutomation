#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEOUT="/etc/ard-$FILEDATE.log"
FILEOUT2="/etc/spr-$FILEDATE.log"
FILEOUT3="/etc/tst-$FILEDATE.log"
STAMPA="`date +%Y%m%d%H%M%S`"
STAMPB="`date +%Y-%b-%d%_9T`"
D="`date`"

#powerTripLow|powerTripHigh|gasTripOffset|SEND_INTERVAL|RESET_INTERVAL|WATER_DEBOUNCE|
echo "Content-type: text/plain $STAMPA|10|4|8|0|0|0|"

#20131101000001 2013-Nov-01 00:00:01

if [ $QUERY_STRING != "WATER0" -a $QUERY_STRING != "WATER1" ] ; then
  echo "$STAMPA $STAMPB $QUERY_STRING" >> $FILEOUT
fi;

#H5=`echo $QUERY_STRING | sed "s/\[H5\]/|/g" | cut -d"|" -f2 | cut -d"[" -f0`

if [ $QUERY_STRING == "WATER0" -o $QUERY_STRING == "WATER1" ] ; then
  
  DATA=`/opt/bin/curl --silent --request GET "http://$osIp/js?pw=$osHash"`
  
  if [ "$DATA" == "" ] ; then
    DATA=`/opt/bin/curl --silent --request GET "http://$osIp/js?pw=$osHash"`
  fi;  
    
  DATA="`echo $DATA | cut -d "[" -f2 | cut -d"]" -f1 | sed 's/[,]//g'`"
  echo "$STAMPA $STAMPB [sn]$DATA[QS]$QUERY_STRING" >> $FILEOUT
  echo "$STAMPA $STAMPB [sn]$DATA[QS]$QUERY_STRING" >> $FILEOUT2
  echo "$STAMPA $STAMPB [sn]$DATA[QS]$QUERY_STRING" >> $FILEOUT3 
fi;