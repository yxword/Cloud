#!/bin/sh
#chkconfig: 2345 60 40
#description: DBHA server driver

this=$(readlink -f "$0")
cur_dir=$(cd "$(dirname "$this")"; pwd)
SERVER_NAME=DB-Manager
LOG_FILE=/cloud/dahua/DataBaseServer/DBHA/Log/Shell.log

echo $@ | egrep "start|stop|restart" > /dev/null 
if [ $? -eq 0 ];then
	pid_name=`cat /proc/$PPID/status | grep Name | awk -F ":" '{print $2}'`
	echo -e "$(date +"%F %T")|[VCS-DBHA] $@ is by $pid_name $PPID" >> $LOG_FILE
fi

/bin/bash $cur_dir/Service/VCS-DBHA.sh $*
exit $?