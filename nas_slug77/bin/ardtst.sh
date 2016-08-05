#!/bin/sh
FILEDATE=`date +%Y%m`;
FILEIN=/etc/wu-$FILEDATE.log;
LINE="`tail -1 $FILEIN`";
urlOut="&dateutc=`date -u +%Y-%m-%d+%H%%3A%M%%3A%S`";
dtNow=`date +%Y%m%d%H%M%S`

l2="-1 -1 -1 2 4 5 3 6 -7 -8 -1 -9 -1 10 11 -1 12 -1 -1"
Q="-datetime -date -time -indoortempf tempf dewptf -indoorhumidity humidity -windspeedmph -winddir -winddirstring -windgustmph -windchill rainin dailyrainin -totalrain baromin -TENDENCY -Forcast"
i=1;
wuQuery="";
paQuery="";
dtData=0;

for c in $LINE
 do
  HD="`echo $Q | cut -d " " -f$i | cut -d " " -f0`"
  if [ $i -eq 1 ] ; then dtData=$c ; fi;
  if [ `expr substr $HD 1 1` != "-" ] ; then
	wuQuery="$wuQuery&$HD=$c" ;
  fi ;
  
  HD="`echo $l2 | cut -d " " -f$i | cut -d " " -f0`"
  if [ `expr substr $HD 1 1` != "-" ] ; then
 	paQuery="$paQuery$HD,$c\r\n" ;
  fi ;
 
  let $((i++));
 done;

dtDiff=`expr $dtNow - $dtData`;
#echo $dtDiff;
if [ $dtDiff -lt 18000 ] ; then urlOut="$urlOut$wuQuery" ; fi ;

FILEIN=/etc/wd-$FILEDATE.log;
LINE="`tail -1 $FILEIN`";
dtData="`echo $LINE | cut -d" " -f1 `"
wuQuery="";

c=2;
while [ $c -le 35 ]
do
	HD="`echo $LINE | cut -d "[" -f$c | cut -d"]" -f1`"
    if [ "$HD" == "bH" ] ; then break ; fi
	VD="`echo $LINE | cut -d"]" -f$c | cut -d"[" -f1`"
	#echo $HD $VD
	case "$HD" in
		'WS' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			wuQuery="$wuQuery&windspeedmph=$V1";
			paQuery=$paQuery"7,$V1\r\n" ;;
		'W2' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			#wuQuery="$wuQuery&windspdmph_avg2m=$V1"
			;;
		'WD' )
			V1=`addZero.sh $VD`;
			wuQuery="$wuQuery&winddir=$V1";
			paQuery=$paQuery"8,$V1\r\n" ;;
		'D2' )
			V1=`addZero.sh $VD`;
			#wuQuery="$wuQuery&winddir_avg2m=$V1"
			;;
		'WG' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			wuQuery="$wuQuery&windgustmph=$V1";
			paQuery=$paQuery"9,$V1\r\n" ;;
		'GA' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			wuQuery="$wuQuery&windgustmph_10m=$V1";;
		#soil ea
		'd2' ) 
			V1=`crof.sh $VD`;
			wuQuery="$wuQuery&soiltempf=$V1";;
		#wetness
		'R0' ) ;;
		#lightness
		'P0' ) 	;;	
		#V1=`addZero.sh $VD`;
		#V1=$((V1 / 100));
		#wuQuery=$wuQuery&solarradiation=$V1;;
	esac
	let $((c++))
done 

dtDiff=`expr $dtNow - $dtData`;
#echo $dtDiff;
if [ $dtDiff -lt 10800 ] ; then urlOut="$urlOut$wuQuery" ; fi ;


#TOUT=`echo -e "$z0$z1$z2$z3$z4$z5$z6$z7$z8$z9$z10$z11$z12$z13"`

BASEURL="weatherstation.wunderground.com"
PATH="/weatherstation/updateweatherstation.php"
ID="?ID=KGAATLAN40"  # ID received from Weather Underground
PW="&PASSWORD=$wuPassword"    # Password for Weather Underground
ACT="&softwaretype=ardunio&action=updateraw"

urlOut="$BASEURL$PATH$ID$PW$urlOut$ACT"
echo $urlOut

#/opt/bin/curl -i "$urlOut" 
#curl --request PUT --header "X-PachubeApiKey: $PachubeApiKey" --data "$TOUT" "api.pachube.com/v2/feeds/28786.csv" >/dev/null 2>&1 

echo $paQuery

echo send!

exit 0
