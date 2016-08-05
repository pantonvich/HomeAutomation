#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEIN="/etc/lgt-$FILEDATE.log"
LINE="`tail -1 $FILEIN`"

z0="";z1="";z2="";z3="";z4="";z5="";z6="";z7="";z8="";z9="";z10="";z11="";
c=0

while [ $c -le 15 ]
do
	HD="`echo $LINE | cut -d "[" -f$c | cut -d"]" -f1`"
	#echo "$HD"

	VD="`echo $LINE | cut -d"]" -f$c | cut -d"[" -f1`"
	#echo "$VD"

	case "$HD" in
      'ea' ) #soil
	    z0="0,$VD\r\n";;
      'fd' ) #pond
        z1="1,$VD\r\n";;
      'R0' ) #wetness
        z2="13,$VD\r\n";;
      'PO' ) #lightness
        z3="14,$VD\r\n";;
      'a5' ) #basement temp
        z4="15,$VD\r\n";;
      '98' ) #hvac out temp
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

 echo "$z0$z1$z2$z3$z4$z5$z6$z7$z8$z9$z10$z11"