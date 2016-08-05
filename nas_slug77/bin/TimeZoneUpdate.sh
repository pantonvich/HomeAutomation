#!/bin/sh
#
# Get the latest rules
#
ipkg update
ipkg install tz
#
# Backup linksys TZ directory & create a new one with links to tz package's rules
#
if [ ! -e /usr/zoneinfo.linksys ]; then
    mv /usr/zoneinfo /usr/zoneinfo.linksys
fi
if [ -d /usr/zoneinfo ]; then
    rm -rf /usr/zoneinfo/*
else
    mkdir /usr/zoneinfo
fi
#
# The left side of this map was created from Set_TimeZone's list of timezones
# The right size was created automagically by searching the tz package for the
# corresponding rule.  A few manual edits removed duplicate names.
#
MAP=`cat <<EOF
Kwajalein|Pacific/Kwajalein
Midway|Pacific/Midway
Honolulu|Pacific/Honolulu
Anchorage|America/Anchorage
Tijuana|America/Tijuana
Phoenix|America/Phoenix
Mountain|US/Mountain
Central|US/Central
Mexico_City|America/Mexico_City
Regina|America/Regina
Lima|America/Lima
Eastern|US/Eastern
East-Indiana|US/East-Indiana
Atlantic|Atlantic
Caracas|America/Caracas
Santiago|America/Santiago
St_Johns|America/St_Johns
Buenos_Aires|America/Argentina/Buenos_Aires
South_Georgia|Atlantic/South_Georgia
Azores|Atlantic/Azores
Casablanca|Africa/Casablanca
Dublin|Europe/Dublin
Berlin|Europe/Berlin
Belgrade|Europe/Belgrade
Brussels|Europe/Brussels
Sarajevo|Europe/Sarajevo
Athens|Europe/Athens
Bucharest|Europe/Bucharest
Cairo|Africa/Cairo
Harare|Africa/Harare
Helsinki|Europe/Helsinki
Baghdad|Asia/Baghdad
Moscow|Europe/Moscow
Tehran|Asia/Tehran
Muscat|Asia/Muscat
Baku|Asia/Baku
Kabul|Asia/Kabul
Yekaterinburg|Asia/Yekaterinburg
Karachi|Asia/Karachi
Calcutta|Asia/Calcutta
Almaty|Asia/Almaty
Colombo|Asia/Colombo
Bangkok|Asia/Bangkok
Hong_Kong|Asia/Hong_Kong
Perth|Australia/Perth
Singapore|Asia/Singapore
Taipei|Asia/Taipei
Tokyo|Asia/Tokyo
Seoul|Asia/Seoul
Yakutsk|Asia/Yakutsk
Adelaide|Australia/Adelaide
Darwin|Australia/Darwin
Brisbane|Australia/Brisbane
Sydney|Australia/Sydney
Guam|Pacific/Guam
Hobart|Australia/Hobart
Vladivostok|Asia/Vladivostok
Magadan|Asia/Magadan
Auckland|Pacific/Auckland
Fiji|Pacific/Fiji
EOF
`
#
# For each linksys-supported TZ, create a softlink to the corresponding ipk rule
#
for z in $MAP ; do
    LN=`echo "$z" | sed -e's+^\(.*\)|\(.*\)$+/opt/share/zoneinfo/\2 /usr/zoneinfo/\1+'`
    ln -sf $LN
done
#
# Find current timezone per linksys GUI
#
tzone=`grep 'time_zone=' /etc/CGI_ds.conf | sed -e's|^.*=\(.*\)$|\1|'`
#
# Convert to its proper name
#
tzone=`expr $tzone + 1`
#
tzone=`sed -n -e"${tzone}p" <<EOF
Kwajalein
Midway
Honolulu
Anchorage
Tijuana
Phoenix
Mountain
Central
Mexico_City
Regina
Lima
Eastern
East-Indiana
Atlantic
Caracas
Santiago
St_Johns
Buenos_Aires
South_Georgia
Azores
Casablanca
Dublin
Berlin
Belgrade
Brussels
Sarajevo
Athens
Bucharest
Cairo
Harare
Helsinki
Baghdad
Moscow
Tehran
Muscat
Baku
Kabul
Yekaterinburg
Karachi
Calcutta
Almaty
Colombo
Bangkok
Hong_Kong
Perth
Singapore
Taipei
Tokyo
Seoul
Yakutsk
Adelaide
Darwin
Brisbane
Sydney
Guam
Hobart
Vladivostok
Magadan
Auckland
Fiji
EOF
`
#
# Copy the new rule over localtime (just what Set_TimeZone would do)
#
cp -f /usr/zoneinfo/$tzone /usr/local/localtime 2>/dev/null
#
# Create the conventional links in /usr/share
#
if [ ! -e /usr/share ]; then
    mkdir /usr/share
fi
ln -sf /opt/share/zoneinfo /usr/share/zoneinfo
ln -sf /opt/share/zoneinfo-leaps /usr/share/zoneinfo-leaps
ln -sf /opt/share/zoneinfo-posix /usr/share/zoneinfo-posix

# ******************************
exit
# ******************************
#
# To create the MAP, we ran strings on Set_TimeZone and extracted
# this list
#
LIST=`cat <<EOF
Kwajalein
Midway
Honolulu
Anchorage
Tijuana
Phoenix
Mountain
Central
Mexico_City
Regina
Lima
Eastern
East-Indiana
Atlantic
Caracas
Santiago
St_Johns
Buenos_Aires
South_Georgia
Azores
Casablanca
Dublin
Berlin
Belgrade
Brussels
Sarajevo
Athens
Bucharest
Cairo
Harare
Helsinki
Baghdad
Moscow
Tehran
Muscat
Baku
Kabul
Yekaterinburg
Karachi
Calcutta
Almaty
Colombo
Bangkok
Hong_Kong
Perth
Singapore
Taipei
Tokyo
Seoul
Yakutsk
Adelaide
Darwin
Brisbane
Sydney
Guam
Hobart
Vladivostok
Magadan
Auckland
Fiji
EOF
`
#
# Here, we look for the matching zone in the real database
#
for z in $LIST ; do
    echo "$z|`find /opt/share/zoneinfo/ -name "$z" | sed -e's|^/opt/share/zoneinfo/||'`"
done
#
# Manual edits are required to remove a few alises.
#

