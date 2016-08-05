#!/bin/sh
#sed '$!N; /^\(.*\)\n\1$/!P; D'
#awk 'a != $0; { a = $0 }' input.txt
badseconds=3600;

echo "Content-type: text/html"
echo ""
echo "<pre>"
s='s/^.*'${1}',\([^&]*\).*$/\1/p'
echo $QUERY_STRING | sed -n $s | sed "s/%20/ /g"
start=`echo $QUERY_STRING | cut -d',' -f1`
end=`echo $QUERY_STRING | cut -d',' -f2`


if [ "$start" == "" ] || [ "$end" == 0 ]  ; then
start="0";
end="0";
fi;

IP="<a href='http://$duckdns:$slug77port80/ADMIN%202"

LAN="<a href='http://$slug77/ADMIN%202"

if [ ${#end} == 8 ] ; then
  FILEDATE=`expr substr $end 1 6`;
else
  FILEDATE="`date +%Y%m`"
fi;
s="`date +%s`"

log="/etc/arm-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 echo "<b style='color:red'>$log $diff check</b>"
fi;

echo " Lan:$LAN$log'>$log</a>     Wan:$IP$log'>$log</a>" 
echo " "`/opt/bin/armParse $log | tail -25`

log="/etc/wu-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 echo "<b style='color:red'>$log $diff check</b>"
fi;

echo " Lan:$LAN$log'>$log</a>     Wan:$IP$log'>$log</a>" 
echo " "`tail -10 $log | sed 's/.*$/&<br\/>/'`

log="/etc/wd-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 echo "<b style='color:red'>$log $diff check</b>"
fi;
echo " Lan:$LAN$log'>$log</a>     Wan:$IP$log'>$log</a>" 
echo " "`tail -10 $log | sed 's/.*$/&<br\/>/'`

#log="/etc/out-$FILEDATE.log"
#f="`date -r $log +%s`"
#diff=$((s - f))
#if [ "$diff" -gt "$badseconds" ] ; then
# echo "<b style='color:red'>$log $diff check</b>"
#fi;
#echo " Lan:$LAN$log'>$log</a>     Wan:$IP$log'>$log</a>" 
#echo " "`tail -10 $log | sed 's/.*$/&<br\/>/'`

log="/etc/ac-$FILEDATE.log"
f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 echo "<b style='color:red'>$log $diff check</b>"
fi;
echo " Lan:$LAN$log'>$log</a>     Wan:$IP$log'>$log</a>" 
echo " "`tail -10 $log | sed 's/.*$/&<br\/>/'`

log="/etc/ard-$FILEDATE.log"
logWu1="/etc/wu-$FILEDATE.log"

if [ ${#start} == 8 ] ; then
  FILEDATE=`expr substr $start 1 6`;
else
  FILEDATE="`date --date "last month" +%Y%m`"
fi;

FILEIN1="/etc/ard-$FILEDATE.log"
logWu0="/etc/wu-$FILEDATE.log"

f="`date -r $log +%s`"
diff=$((s - f))
if [ "$diff" -gt "$badseconds" ] ; then
 echo "<b style='color:red'>$log $diff check</b>"
fi;

echo " Lan:$LAN$log'>$log</a>     Wan:$IP$log'>$log</a>" 
echo " Lan:$LAN$FILEIN1'>$FILEIN1</a>     Wan:$IP$FILEIN1'>$FILEIN1</a>" 
echo " "`tail -50 $log | sed 's/.*$/&<br\/>/'`

echo " "`/opt/bin/parseme K0 3.0f 4.0f $start $end 4.2f $FILEIN1 $log ` 
echo " "`/opt/bin/parseme H0 02.0f 03.0f $start $end 4.2f $FILEIN1 $log `
echo " "`/opt/bin/parseme G0 02.0f 03.0f $start $end 4.2f $FILEIN1 $log `
echo " "`/opt/bin/wuParse $logWu0 $logWu1 `
echo "<\pre>"
