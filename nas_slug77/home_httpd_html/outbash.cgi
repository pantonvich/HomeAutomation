#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEIN2="/etc/out-$FILEDATE.log"
STAMPA="`date +%Y%m%d%H%M%S`"
STAMPB="`date +%Y-%b-%d%_9T`"
D="`date`"

echo "Content-type: text/plain $STAMPA|0|0|0|0|0|0|0|"
#echo ""
#echo "Hello World!"
#powerTripLow|powerTripHigh|gasTripOffset|SEND_INTERVAL|RESET_INTERVAL|WATER_DEBOUNCE|
#echo "$STAMPA|0|0|0|0|0|0|0|"
# echo $QUERY_STRING

#20131101000001 2013-Nov-01 00:00:01
#echo "${1}"
echo "$STAMPA $STAMPB $QUERY_STRING" >> $FILEIN2

if [ `echo $QUERY_STRING | cut -c-4` == "RAIN" ] ; then
	
	/opt/bin/curl "http://192.168.0.22/cv?pw=a6d82bced638de3def1e9bbb4983225c&rd=24"

	/opt/bin/curl -u $cam33login http://$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480 > /etc/bellimage.jpg

	HD="$duckdns"
	
	echo "http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi?resolution=640x480
			http://$cam33login@$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480" | /opt/bin/nail -s "$QUERY_STRING $D" -a /etc/bellimage.jpg $hotmail  

	echo "http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi
			http://$cam33login@$$cam33LocalIp/cgi/jpg/image.cgi" | /opt/bin/nail -s "$QUERY_STRING $D" $boxio 		
	

fi;
