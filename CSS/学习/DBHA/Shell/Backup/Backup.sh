#!/bin/bash
#定时备份
TAG=backup
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)
source $parent_dir/Common/CommonFunc.sh
#文件名的命名规则uid_serverID_time

if [[ $# -lt 5 ]];then
    logInfo "back failed,$*."
    exit 1;
fi

#备份文件的名字，为了表示文件的唯一性，建议文件名带上时间
file_dir=$2
uuid=$3
server_id=$4
#模式，master或者slave
mode=$5

#mysql登录的用户名和密码
getUserCfg

function stopBackup()
{
    pids=`ps -ef|grep "Backup.sh" |grep "start" | grep "$file_dir" |grep "$uuid" |grep "$server_id"|awk '{print $2}'`
	logInfo "$pids"
    for pid in $pids;
    do
        #if [[ "$pid" = "" ]];then
        kill -9 $pid
        logInfo "kill -9 $pid"
    done
    #tmp文件的删除统一由start和stop服务的时候删除
    return 0
}

#${file_dir}/manual_master_${uuid}_${server_id}_tmp ${file_dir}/master_${uuid}_${server_id}_$(date "+%Y_%m_%d_%H%M%S")
function startBackup()
{
    logInfo "start to back up for self, file_dir:$file_dir, mode:$mode.";
    if [ ! -d $file_dir ];then
        mkdir -p $file_dir
        #安全基线要求
        chmod 0700 $file_dir
    fi

    #backupFileWithXtrabackup $file_dir $uuid $server_id $mode manual
	backupFileCommon $file_dir $uuid $server_id $mode manual
    if [[ $? -ne 0 ]];then
	logInfo "failed to backup for self"
    return 1
    fi
	
    logInfo "end to backup for self, file_dir:$file_dir, mode:$mode."
    return 0
}

case $1 in
  "start")
  startBackup
  exit $?
  ;;
  "stop")
  stopBackup
  exit $?
  ;;
  *)
  echo "Usage: $0 {start|stop}"
  exit 1
  ;;
esac
