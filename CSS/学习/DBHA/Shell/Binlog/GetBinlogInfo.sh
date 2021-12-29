#!/bin/bash
#记录了当前机器作为主机的文件名和位置偏移
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

TAG=putBinlogInfo
source $parent_dir/Common/CommonFunc.sh

#mysql登录的用户名和密码
getUserCfg

function getInfo()
{
	master_ip=$1
	if [ -n "${master_ip}" ];then
		master_status=$(mysql -u$mysql_user -p$mysql_password  -h$master_ip --connect_timeout=2 -e "show master status\G"|grep -E "File|Position")
		# master_status格式如"File: mysql-bin.000022 Position: 1483049"
		master_log_file_name=$(echo ${master_status} | awk '{print $2}')
		master_log_file_position=$(echo ${master_status} | awk '{print $4}')
	else
		#获取主机当前自己的binlog文件名和位置信息
		master_log_file_name=$(mysql -u$mysql_user -p$mysql_password  -e "show master status\G" | grep -w "File" | awk -F": " '{print $2}')
		master_log_file_position=$(mysql -u$mysql_user -p$mysql_password  -e "show master status\G" | grep -w "Position" | awk -F": " '{print $2}')
	fi
	if [ "$master_log_file_name" = "" ];then
		echo ""
		logInfo "master log file is empty"
		return 1
	fi
	#logInfo "master log file is $master_log_file_name $master_log_file_position"
	echo "$master_log_file_name $master_log_file_position"
	return 0
}

if [ $# -eq 1 ];then
	getInfo  $1
else
	getInfo
fi

exit $?