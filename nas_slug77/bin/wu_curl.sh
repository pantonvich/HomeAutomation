#!/bin/sh
FILEDATE=`date +%Y%m`;
FILEIN=/etc/wu-$FILEDATE.log;
LINE="`tail -1 $FILEIN`";
urlOut="&dateutc=`date -u +%Y-%m-%d+%H%%3A%M%%3A%S`";
dtNow=`date +%Y%m%d%H%M%S`

l2="-1 -1 2 4 5 3 6 7 8 -1 9 -1 10 11 -1 12 -1 -1"
Q="-datetime -date -time -indoortempf tempf dewptf -indoorhumidity humidity -windspeedmph -winddir -winddirstring -windgustmph -windchill rainin dailyrainin -totalrain baromin -TENDENCY -Forcast"
i=1;
tmp="";
pach="";
dtData=0;

for c in $LINE
 do
  HD="`echo $Q | cut -d " " -f$i | cut -d " " -f0`"
  if [ $i -eq 1 ] ; then dtData=$c ; fi;
  skip="0";
  if [ `expr substr $HD 1 1` == "-" ] ; then skip="1" ; fi;
 # if [ "$HD" == "tempf" -a "$c" == "177.1" ] ; then skip="1" ; fi;
 # if [ "$HD" == "tempf" -a "$c" == "176.5" ] ; then skip="1" ; fi;
 # if [ "$HD" == "tempf" -a "$c" == "176.9" ] ; then skip="1" ; fi;
 # if [ "$HD" == "tempf" -a "$c" == "178.0" ] ; then skip="1" ; fi;
  if [ "$HD" == "tempf" -a $c -gt 110 ] ; then skip="1" ; fi;
  if [ "$HD" == "humidity" -a "$c" == "110" ] ; then skip="1" ; fi;
  if [ "$HD" == "humidity" -a "$c" == "99.9" ] ; then skip="1" ; fi;
  if [ "$HD" == "dewptf" -a "$c" == "99.9" ] ; then skip="1" ; fi;
  
  if [ "$skip" == "0" ] ; then
	tmp="$tmp&$HD=$c" ;
  fi ;
  
 # HD="`echo $l2 | cut -d " " -f$i | cut -d " " -f0`"
 # if [ "$HD" != "-1" ] ; then
 #	pach="$pach$HD,$c\r\n" ;
 # fi ;
	
  let $((i++));
 done;

dtDiff=`expr $dtNow - $dtData`;
#echo $dtDiff;
if [ $dtDiff -lt 18000 ] ; then urlOut="$urlOut$tmp" ; fi ;

FILEIN=/etc/wd-$FILEDATE.log;
LINE="`tail -1 $FILEIN`";
dtData="`echo $LINE | cut -d" " -f1 `"
tmp="";
pachTmp="";

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
			tmp="$tmp&windspeedmph=$V1";
			pachTmp=$pachTmp"7,$V1\r\n";;
		'W2' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			#tmp="$tmp&windspdmph_avg2m=$V1"
			;;
		'WD' )
			V1=`addZero.sh $VD`;
			tmp="$tmp&winddir=$V1";
			pachTmp=$pachTmp"8,$V1\r\n";;
		'D2' )
			V1=`addZero.sh $VD`;
			#tmp="$tmp&winddir_avg2m=$V1"
			;;
		'WG' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			tmp="$tmp&windgustmph=$V1";
			pachTmp=$pachTmp"9,$V1\r\n";;
		'GA' )
			V1=`addZero.sh $VD`; h=$((V1 / 100)); d=$((V1 - h * 100)); if [ $d -gt 10 ] ; then V1=$h.$d ; else V1=$h.0$d ; fi ;
			tmp="$tmp&windgustmph_10m=$V1";;
		#soil ea
		'd2' ) 
			V1=`crof.sh $VD`;
			tmp="$tmp&soiltempf=$V1";;
		#wetness
		'R0' ) ;;
		#lightness
		'P0' ) 	;;	
		#V1=`addZero.sh $VD`;
		#V1=$((V1 / 100));
		#tmp=$tmp&solarradiation=$V1;;
	esac
	let $((c++))
done 

dtDiff=`expr $dtNow - $dtData`;
#echo $dtDiff;
if [ $dtDiff -lt 10800 ] ; then urlOut="$urlOut$tmp" ; fi ;


#TOUT=`echo -e "$z0$z1$z2$z3$z4$z5$z6$z7$z8$z9$z10$z11$z12$z13"`

BASEURL="weatherstation.wunderground.com"
PATH="/weatherstation/updateweatherstation.php"
ID="?ID=KGAATLAN40"  # ID received from Weather Underground
PW="&PASSWORD=$wuPassword"    # Password for Weather Underground
ACT="&softwaretype=ardunio&action=updateraw"

urlOut="$BASEURL$PATH$ID$PW$urlOut$ACT"
#echo $urlOut
 /opt/bin/curl -i "$urlOut" 
#curl --request PUT --header "X-PachubeApiKey: $PachubeApiKey" --data "$TOUT" "api.pachube.com/v2/feeds/28786.csv" >/dev/null 2>&1 
echo send!

TOUT=`echo -e "$pachTmp"`
#echo $TOUT
/opt/bin/curl --request PUT --header "X-PachubeApiKey: $PachubeApiKey" --data "$TOUT" "api.pachube.com/v2/feeds/6449.csv" >/dev/null 2>&1 

exit 0
