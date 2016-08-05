#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEIN2="/etc/tst-$FILEDATE.log"
STAMPA="`date +%Y%m%d%H%M%S`"
STAMPB="`date +%Y-%b-%d%_9T`"
D="`date`"

echo "Content-type: text/plain $STAMPA|0|0|0|0|0|7"
echo ""
echo "Hello World!"
#powerTripLow|powerTripHigh|gasTripOffset|SEND_INTERVAL|RESET_INTERVAL|WATER_DEBOUNCE|
# echo "$STAMPA|0|0|0|0|0|0|"
# echo $QUERY_STRING

#20131101000001 2013-Nov-01 00:00:01
#echo "${1}"
echo "$STAMPA $STAMPB $QUERY_STRING" >> $FILEIN2

