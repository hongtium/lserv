#!/bin/sh
. /etc/profile
SERV=$1
if [ $# -lt 3 ]; then
    echo usage:$0 servname port act
	    exit
fi
TOKILLS="$1"
HOMEDIR=/opt/$SERV
#----------------------------------
case "$3" in
	stop)
		for TOKILL in $TOKILLS
		do
			killall $TOKILL
			killall $TOKILL
			killall $TOKILL
		done
	;;
	start)
		cd $HOMEDIR
		cp $SERV.log logs/"$SERV"_`date +"%y%m%d%H%M%S"`.log
		cat /dev/null > $SERV.log
		mv err.log logs/err_`date +"%y%m%d%H%M%S"`.log
		mv std.log logs/std_`date +"%y%m%d%H%M%S"`.log
		ulimit -S -c unlimited
		sleep 2
		./$SERV $2 >> $HOMEDIR/std.log 2>>$HOMEDIR/err.log &
	;;
	restart)
		$0 $1 $2 stop
		sleep 2
		$0 $1 $2 start
	;;
	monitor)
		PID=`ps -eo pid,comm |grep $SERV|awk '{print $1}'`
		if test -z $PID
		then
			echo $SERV progress is not running ! restart@`date`
			$0 $1 $2 restart
			exit
		fi
		V1=`cat /tmp/$SERV.dog.lock`
		V2=`tail -n1 $HOMEDIR/$SERV.log`
		if test $HOMEDIR/$SERV.log -ot /tmp/$SERV.dog.lock 
			then
			if [ "$V1" = "$V2" ]
			then
				echo "$SERV log die ! restart@`date`"
			$0 $1 $2 restart
			fi
		fi
	;;

esac
tail -n1 $HOMEDIR/$SERV.log > /tmp/$SERV.dog.lock
