#!/bin/sh
echo "Content-type: text/plain"
 echo ""
 echo "Hello World!"

 echo $QUERY_STRING

#s='s/^.*'${1}'=\([^&]*\).*$/\1/p'
#echo $QUERY_STRING | sed -n $s | sed "s/%20/ /g"

#id=`./getvar.sh id`


FILEDATE="`date +%Y%m`"
FILEIN2="/etc/arm-$FILEDATE.log"
STAMPA="`date +%Y%m%d%H%M%S`"
STAMPB="`date +%Y-%b-%d%_9T`"
D="`date`"
HVAC=""

#20131101000001 2013-Nov-01 00:00:01
#echo "${1}"
echo "$STAMPA $STAMPB $QUERY_STRING" >> $FILEIN2
MSG=`echo $QUERY_STRING | cut -d'|' -f2`;

FLAG=0;
if [ $MSG == "2" -o $MSG == "3" -o $MSG == "4" -o $MSG == "7" -o $MSG == "disarm" ] ; then
	FLAG="1";
fi;

if [ $MSG == "entry" ] ; then
	FLAG="2";
fi;

if [ $MSG == "alarm" ] ; then
 # want :|alarm and not :nr-fc|alarm
 if [ `echo $QUERY_STRING | cut -d'|' -f1 | sed -e 's/^.*\(.\)$/\1/'` == ":" ] ; then
	FLAG="2";
 fi;	
fi;

//if [ $MSG == "?" ] ; then
  if [ "`echo $QUERY_STRING | cut -d: -f7`" == "" ] ; then
	MATCH=`echo $QUERY_STRING | cut -d: -f3,4,5,6`
  else
    MATCH=`echo $QUERY_STRING | cut -d: -f4,5,6,7`
  fi;
  
MSG=""
 
  case "$MATCH" in
	"fe:9c:fe:|?" )
	   FLAG="2"; MSG="2|entry";;
	"fc:9c:fe:2|alarm" )
	   FLAG="0"; MSG="fc|arm";;
	"9c:3c:fe:|?" )
	   FLAG="0"; MSG="b|nr";;   
	"1c:78:cf:|?" )
	   FLAG="0"; MSG="b|ok";;
	"ce:1c:fc:|?" )
	   FLAG="1"; MSG="b|2";;
	"9c:1c:fc:|?" )
	   FLAG="0"; MSG="b|2|arm";;	   
  esac
 
if [ $MSG != "" ] ; then 
  echo "$STAMPA $STAMPB $QUERY_STRING $MATCH - $MSG -" >> $FILEIN2
fi;

if [ "$FLAG" == "1" ] ; then
	
	DATA=`/opt/bin/curl --silent http://192.168.0.76/tstat | sed 's/[a-z{}_":]//g'`
	v6=`echo $DATA | cut -d "," -f 6 | sed 's/\.//'`
	v5=`echo $DATA | cut -d "," -f 5 | sed 's/\.//'`
	
	/opt/bin/nail -s "$v6 $v5 $QUERY_STRING $D" $hotmail < /dev/null
	/opt/bin/nail -s "$v6 $v5 $QUERY_STRING $D" $boxio < /dev/null
fi;

if [ "$FLAG" == "2" ] ; then
	/opt/bin/curl -u $cam33login http://$cam33LocalIp/cgi/jpg/image.cgi?resolution=640x480 > /etc/bellimage.jpg
	
	HD="$duckdns"
	
	echo "http://$cam33login@$HD:3380/cgi/jpg/image.cgi?resolution=640x480
			http://$cam33login@$$cam33LocalIp/cgi/jpg/image.cgi" | /opt/bin/nail -s "$QUERY_STRING $D" -a /etc/bellimage.jpg $hotmail  

	echo "http://$cam33login@$HD:3380/cgi/jpg/image.cgi
			http://$cam33login@$$cam33LocalIp/cgi/jpg/image.cgi" | /opt/bin/nail -s "$QUERY_STRING $D" $boxio 

fi;

