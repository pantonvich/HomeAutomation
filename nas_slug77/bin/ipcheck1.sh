#!/bin/sh
IPFILE=/etc/ipaddress
#CURRENT_IP=$(wget -q -O - http://automation.whatismyip.com/n09230945.asp)
#if [ -f $IPFILE ]; then
#        KNOWN_IP=$(cat $IPFILE)
#else
#        KNOWN_IP=
#fi
# 
#if [ "$CURRENT_IP" != "$KNOWN_IP" ]; then
#        echo $CURRENT_IP > $IPFILE
#        wget -q --keep-session-cookies --http-user=pantonvich1 --http-password=ssom30092 --post-data="status=IP is $CURRENT_IP" http://twitter.com:80/statuses/update.xml
#fi
#echo "`date`: `/sbin/ifconfig ixp0 | grep 'RX bytes:'`" >> /etc/ixp0_traffic.log
#exit 0
