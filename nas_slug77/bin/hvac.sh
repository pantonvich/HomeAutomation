#!/bin/sh
FILEDATE="`date +%Y%m`"
FILEOUT="/etc/ac-$FILEDATE.log"
DATE="`date +%Y%m%d%H%M%S' '%d-%b-%Y' '%T`"

#{"today":{"heat_runtime":{"hour":0,"minute":0},
#"cool_runtime":{"hour":0,"minute":0}},
#"yesterday":{"heat_runtime":{"hour":0,"minute":0},
#"cool_runtime":{"hour":0,"minute":0}}}

DATA=`curl --silent http://$tstatIp/tstat/datalog | sed 's/[a-z{}_":]//g'`

v1=`echo $DATA | cut -d "," -f 1`
v2=`echo $DATA | cut -d "," -f 2`
v3=`echo $DATA | cut -d "," -f 3`
v4=`echo $DATA | cut -d "," -f 4`
v5=`echo $DATA | cut -d "," -f 5`
v6=`echo $DATA | cut -d "," -f 6`
v7=`echo $DATA | cut -d "," -f 7`
v8=`echo $DATA | cut -d "," -f 8`
DATALOG="$(($v1*60+$v2)) $(($v3*60+$v4)) $(($v5*60+$v6)) $(($v7*60+$v8))"

# temp - Current temperature in degrees farenheit
# tmode - Current thermostat mode
# fmode - Current fan mode
# override - Current temperature override mode(?)
# hold - Current hold state
# t_heat - Current set point for heat * in tmode 1 only (heat)
# t_cool - Current set point for cool * in tmode 2 only (cool)
# tstate - Current thermostat state - Values are 0 (Off), 1 (Heating), 2 (Cooling?)
# fstate - Current fan state
# time - Current time

DATA=`curl --silent http://$tstatIp/tstat | sed 's/[a-z{}_":]//g'`

v1=`echo $DATA | cut -d "," -f 1`
v2=`echo $DATA | cut -d "," -f 2`
v3=`echo $DATA | cut -d "," -f 3`
v4=`echo $DATA | cut -d "," -f 4`
v5=`echo $DATA | cut -d "," -f 5`
v6=`echo $DATA | cut -d "," -f 6`
v7=`echo $DATA | cut -d "," -f 7`
v8=`echo $DATA | cut -d "," -f 8`
SETTING="$v1 $v2 $v3 $v4 $v5 $v6 $v7 $v8"

echo $DATE $DATALOG $SETTING >> $FILEOUT

exit 0
