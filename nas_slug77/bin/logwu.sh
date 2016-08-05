#!/bin/sh
WUPID=`pidof "log2300"`
echo $WUPID
if [ "$WUPID" -ne "" ]; then
kill `pidof log2300`
fi
/opt/bin/log2300 /etc/wu-$(date +%Y%m).log /etc/open2300.conf
/bin/wu_pach.sh

#WUPID=`pidof "wu2300"`
#if [ "$WUPID" -ne "" ]; then
#kill `pidof wu2300`
#fi
#/opt/bin/wu2300 /etc/open2300.conf

WUPID=`pidof "wu_curl.sh"`
if [ "$WUPID" -ne "" ]; then
kill `pidof "curl"`
fi

/bin/wu_curl.sh
exit 0
