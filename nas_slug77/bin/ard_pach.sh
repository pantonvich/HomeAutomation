#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEIN="/etc/ard-$FILEDATE.log"
LINE="`tail -1 $FILEIN`"
DATE="`date +%Y%m%d%H%M%S' '%d-%b-%Y' '%T`"

#echo $LINE

z0="";z1="";z2="";z3="";z4="";z5="";z6="";z7="";z8="";z9="";z10="";z11="";
z12="";z13="";
c=0;H=-1;G=-1;

while [ $c -le 35 ]
do
	HD="`echo $LINE | cut -d "[" -f$c | cut -d"]" -f1`"
#	echo "$HD"

        if [ "$HD" == "IP" ] ; then break ; fi


	VD="`echo $LINE | cut -d"]" -f$c | cut -d"[" -f1`"

#	echo "$HD $VD"

	case "$HD" in
          'd2' ) #soil ea
            V1=`ctof.sh $VD`;
            z0="0,$V1\r\n";;
          '50' ) #pond fd
            V1=`ctof.sh $VD`;
            z1="1,$V1\r\n";;
 #         'R0' ) #wetness
 #           V1=`addZero.sh $VD`;
 #           z2="13,$V1\r\n";;
 #         'P0' ) #lightness
 #           V1=`addZero.sh $VD`;
 #           z3="14,$V1\r\n";;
          '20' ) #basement temp
            V1=`ctof.sh $VD`;
            z4="15,$V1\r\n";;
          '26' ) #hvac out temp 98
            V1=`ctof.sh $VD`;
            z5="16,$V1\r\n";;
	  '5e' ) #hvac return temp e4 5a
            V1=`ctof.sh $VD`;
            z6="17,$V1\r\n";;
	  '6a' ) #attic temp
            V1=`ctof.sh $VD`;
            z7="18,$V1\r\n";;
          'K5' ) #5min watts
            V1=`addZero.sh $VD`;
            z8="19,$V1\r\n";;
          'G5' ) #Gas
            V1=`addZero.sh $VD`;
			G=$V1;
            z9="20,$V1\r\n";;
          'K0' ) #Total Watts
            V1=`addZero.sh $VD`;
            z10="21,$V1\r\n";;
	  'G0' ) #Total Gas
            V1=`addZero.sh $VD`;
            z11="22,$V1\r\n";;
          'H0' ) #Water
            V1=`addZero.sh $VD`;
            z12="26,$V1\r\n";;
          'H5' ) #Water 5
            V1=`addZero.sh $VD`;
            H=$V1;
            z13="27,$V1\r\n";;
	esac
	let $((c++))
done 

FILEIN="/etc/out-$FILEDATE.log"
LINE="`tail -1 $FILEIN`"
c=0;
while [ $c -le 3 ]
do
	HD="`echo $LINE | cut -d "[" -f$c | cut -d"]" -f1`"
	VD="`echo $LINE | cut -d"]" -f$c | cut -d"[" -f1`"
	case "$HD" in
	      'Ra' ) #wetness
            V1=`addZero.sh $VD`;
            z2="13,$V1\r\n";;
          'Pr' ) #lightness
            V1=`addZero.sh $VD`;
            z3="14,$V1\r\n";;
	esac
	let $((c++))
done 

TOUT=`echo -e "$z0$z1$z2$z3$z4$z5$z6$z7$z8$z9$z10$z11$z12$z13"`

/opt/bin/curl --request PUT --header "X-PachubeApiKey: $PachubeApiKey" --data "$TOUT" "api.pachube.com/v2/feeds/6449.csv" >/dev/null 2>&1 

if [ "$H" -gt "4" ] ; then

  if [ "$G" == "0" ] ; then
	out="579:H0:$H:G5:$G - Water Alert"
	env MAILRC=/dev/null from=$senderEmail smtp=$smtp \
		smtp-auth=login smtp-auth-user=pantonvich \
		smtp-auth-password=$senderPassword nail -s "$out" $hotmail < /dev/null

		env MAILRC=/dev/null from=$senderEmail smtp=$smtp \
		smtp-auth=login smtp-auth-user=pantonvich \
		smtp-auth-password=$senderPassword nail -s "$out" $boxio < /dev/null
  fi;
 
fi;
#if [ "$H" -gt "0" ] ; then
#  FILEIN="/etc/spr-$FILEDATE.log"
#  DATA=`curl --silent --request GET "http://$osIp/js?pw=$osHash"`
#  
#  if [ "$DATA" == "" ] ; then
#    DATA=`curl --silent --request GET "http://$osIp/js?pw=$osHash"`
#    #echo $DATE "[*]"$DATA >> $FILEIN
#  fi;  
#    
#  DATA="`echo $DATA | cut -d "[" -f2 | cut -d"]" -f1 | sed 's/[,]//g'`"
#  echo $DATE "[sn]"$DATA >> $FILEIN
#fi;
  
exit 0







