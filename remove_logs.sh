#!/bin/sh
. /etc/profile
LOGDIRS="/opt/lserv/logs"
for LOGDIR in $LOGDIRS
do 
	echo remove logs dir:$LOGDIR
	rm -f $LOGDIR/*
done

LOGFILES="/opt/lserv/lserv.log /opt/lserv/std.log /opt/lserv/err.log"
for LOGFILE in $LOGFILES
do 
	echo remove log:$LOGFILE
	cat /dev/null > $LOGFILE
done
COREDIRS="/opt/lserv"
for COREDIR in $COREDIRS
do 
	echo remove coredir:$COREDIR
	rm -f $COREDIR/core*
done
