#!/bin/sh
FILEDATE="`date +%Y%m`"
#FILEIN2="/etc/ard-$FILEDATE.log"
#FILEDATE="`date --date "last month" +%Y%m`"
#FILEIN1="/etc/ard-$FILEDATE.log"
badseconds=3600;
s="`date +%s`"
out=""
err=""

#FILEIN78="/mnt/slug78/etc/lgt-$FILEDATE.log"

HD="$duckdns"

log="/etc/arm-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 out="$log $diff check
 $out"
 err="Error "
fi;

log="/etc/wu-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 out="$log $diff check
 $out"
 err="Error "
fi;

log="/etc/wd-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 out="$log $diff check
 $out"
 err="Error "
fi;

log="/etc/ac-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 out="$log $diff check
 $out"
 err="Error "
fi;

log="/etc/ard-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 out="$log $diff check
 $out"
 err="Error "
fi;

echo "http://$HD:$slug77port80/stats.cgi 
http://$cam33login@$HD:$cam33port80/cgi/jpg/image.cgi

http://$slug77/stats.cgi
http://$cam33login@$cam33LocalIp/cgi/jpg/image.cgi
$out" | \
/opt/bin/nail -s "$err 77-STATUS $HD" $hotmail 

exit 0

echo "start $FILEIN1 $FILEIN2"
 /opt/bin/parseme [K0] 3.0f 4.0f 0 0 4.2f $FILEIN1 $FILEIN2 | \
 /opt/bin/nail -s "77-K0 $HD" $hotmail

/opt/bin/parseme [G0] 3.0f 4.0f 0 0 4.2f $FILEIN1 $FILEIN2 | \
  /opt/bin/nail -s 77-G0 $hotmail 

parseme [H0] 3.0f 4.0f 0 0 4.2f $FILEIN1 $FILEIN2| \
 /opt/bin/nail -s 77-H0 $hotmail 


exit 0
