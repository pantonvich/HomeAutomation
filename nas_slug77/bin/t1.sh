#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEIN="/etc/ard-$FILEDATE.log"
LINE="`tail -1 $FILEIN`"

#echo $LINE

z0="";z1="";z2="";z3="";z4="";z5="";z6="";z7="";z8="";z9="";z10="";z11="";
c=0

while [ $c -le 30 ]
do
	HD="`echo $LINE | cut -d "[" -f$c | cut -d"]" -f1`"
#	echo "$HD"

        if [ "$HD" == "IP" ] ; then break ; fi


	VD="`echo $LINE | cut -d"]" -f$c | cut -d"[" -f1`"

	echo "$HD $VD"

	case "$HD" in
          'a2' ) #soil ea
     v1=`ctof.sh $VD`;
     echo $v1;       
     z0="0,$VD\r\n";;
          'e6' ) #pond fd
            z1="1,$VD\r\n";;
          'RO' ) #wetness
            z2="13,$VD\r\n";;
          'PO' ) #lightness
            z3="14,$VD\r\n";;
          'a5' ) #basement temp
            z4="15,$VD\r\n";;
          '26' ) #hvac out temp 98
            z5="16,$VD\r\n";;
	  'e4' ) #hvac return temp
            z6="17,$VD\r\n";;
	  '6a' ) #attic temp
            z7="18,$VD\r\n";;
          'K5' ) #5min watts
            z8="19,$VD\r\n";;
          'G5' ) #Gas
            z9="20,$VD\r\n";;
          'K0' ) #Total Watts
            z10="21,$VD\r\n";;
	  'G0' ) #Total Gas
            z11="22,$VD\r\n";;
	esac
	let $((c++))
done 

TOUT=`echo -e "$z0$z1$z2$z3$z4$z5$z6$z7$z8$z9$z10$z11"`

echo "$TOUT"

#curl --request PUT --header "X-PachubeApiKey: $PachubeApiKey" --data "$TOUT" "api.pachube.com/v2/feeds/28786.csv" >/dev/null 2>&1 

exit 0







"