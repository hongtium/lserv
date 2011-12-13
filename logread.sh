#!/bin/sh
. /etc/profile
if [ $# -lt 2 ]; then
    echo usage:$0 who ext datedir
	    exit
fi
HOMEDIR=/opt/lserv
DATAFILE=`$HOMEDIR/homedir $1 .$2`
if test -n "$3"
then
	DATAFILE=$3`echo $DATAFILE |sed 's/\/opt\/usr//g'`
fi
echo $DATAFILE
#----------------------------------
if test -f $DATAFILE
then
	grep $1 $DATAFILE
else
	echo "No Result"
fi
