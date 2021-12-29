#!/bin/bash
#定时备份
TAG=masterBackup
#文件名的命名规则master端：master_uid_serverID_time，备机端：slave_uid_serverID_time
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh

if [[ $# -lt 3 ]];then
    echo -e "USAGE:$0 full path, uuid, serverID)"
    logInfo "back failed,$*."
    exit 1;
fi

#备份文件的名字，为了表示文件的唯一性，建议文件名带上时间
file_dir=$1
uuid=$2
server_id=$3

#mysql登录的用户名和密码
getUserCfg

function checkMode()
{
    local retry=15
    for((ping_times=0;ping_times<$retry;ping_times++));
    do
        mysqladmin -u$mysql_user -p$mysql_password --connect-timeout=2 ping 2>&1 | grep -q "mysqld is alive"
        if [ $? -eq 0 ];then
            break
        else
            logInfo "[checkServer]mysql ping timeout:$ping_times!"
            sleep 1
        fi
    done
    
    #ping 15均不通，则直接返回。
    [ $ping_times -eq $retry ] && return 0

    #检测是主机还是备机
    sql_running_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Host" | awk -F": " '{print $2}' )

    #主机
    if [[ "$sql_running_status" = "" ]];then
        return 1
    else #备机
        return 2
    fi
}

#检测是否手动已经触发了
function check()
{
    #中间状态文件获取成功
    file=`ls -alfrt $file_dir |grep "master" | grep "$uuid" |grep "$server_id"| grep "tmp" |sed -n '1p'`
    if [[ "$file" != "" ]];then
        logInfo "backup is running, file name:$file_dir/$file"
        return 1
    fi
    return 0
}

function backupMaster()
{
    check
    if [[ $? -ne 0 ]];then
        return
    fi

    if [ ! -d $file_dir ];then
        mkdir -p $file_dir
        #安全基线要求
        chmod 0700 $file_dir
    fi

	backupFileCommon $file_dir $uuid $server_id master auto
    if [[ $? -ne 0 ]];then
	logInfo "backup is failed"
 	#保存及清理auto下的备份文件
	purgeAutobackupFile $file_dir $uuid $server_id xtrabackup
	
    return 1
    fi
	#保存及清理auto下的备份文件
	purgeAutobackupFile $file_dir $uuid $server_id dump
	
	logInfo "backup success"
	return 0
}

checkMode
#非主机不备份
if [[ $? -ne 1 ]];then
    logInfo "this is not master mode."
    exit 1
else
    backupMaster
    exit $?
fi

exit 0
