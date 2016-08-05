#!/bin/sh
minutes="`date +%M`"
if [ "$minutes" -ne "11" ] ; then 
	#/opt/bin/arduino /etc/ard-$(date +%Y%m).log
	/bin/ard_pach.sh
#	/bin/ardtst.sh
fi;

/bin/hvac.sh
exit 0

