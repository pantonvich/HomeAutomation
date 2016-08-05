#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEIN2="/etc/wd-$FILEDATE.log"
STAMPA="`date +%Y%m%d%H%M%S`"
STAMPB="`date +%Y-%b-%d%_9T`"
D="`date`"

echo "Content-type: text/plain $STAMPA|0|0|0|0|0|0|0|"
#powerTripLow|powerTripHigh|gasTripOffset|SEND_INTERVAL|RESET_INTERVAL|WATER_DEBOUNCE|
#echo ""
#echo "Hello World!"
#echo "$STAMPA"
# echo $QUERY_STRING

#20131101000001 2013-Nov-01 00:00:01
#echo "${1}"
echo "$STAMPA $STAMPB $QUERY_STRING" >> $FILEIN2

if [ $QUERY_STRING == "BELL" ] ; then

	/opt/bin/curl --silent -u $cam33login http://$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480 > /etc/bellimage.jpg

	HD="$duckdns"
	
	echo "http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi?resolution=640x480
			http://$cam33login@$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480" | /opt/bin/nail -s "$QUERY_STRING $D" -a /etc/bellimage.jpg $hotmail  

	echo "http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi
			http://$cam33login@$$cam33LocalIp/cgi/jpg/image.cgi" | /opt/bin/nail -s "$QUERY_STRING $D" $boxio 
fi;

if [ `echo $QUERY_STRING | cut -c-4` == "RAIN" ] ; then
	
	/opt/bin/curl "http://$osIp/cv?pw=$osHash&rd=24"

	/opt/bin/curl -u $cam33login http://$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480 > /etc/bellimage.jpg
	
	HD="$duckdns"
	
	echo "http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi?resolution=640x480
			http://$cam33login@$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480" | /opt/bin/nail -s "$QUERY_STRING $D" -a /etc/bellimage.jpg $hotmail  

	echo "http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi
			http://$cam33login@$$cam33LocalIp/cgi/jpg/image.cgi" | /opt/bin/nail -s "$QUERY_STRING $D" $boxio 		
	

fi;

