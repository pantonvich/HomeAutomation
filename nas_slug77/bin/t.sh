#!/bin/bash
LINE=`echo $(/opt/bin/curl -s http://192.168.0.1/)`
HD="`echo $LINE | cut -d";" -f87 | cut -d"'" -f6`"
echo $HD
