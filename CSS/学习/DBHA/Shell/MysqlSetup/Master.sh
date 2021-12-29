#!/bin/bash
#参数设置：single backup_path,uuid,serverID
#          cluster slave_ip, slave_heart_ip,backup_path,uuid,serverID
#配置mysql主备中的主机模式
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)
. $parent_dir/Common/CommonFunc.sh

#注意，此处修改后，下面cron语句也要修改
mysql_state_file=$DBHA_CONFIG/Sql.stat
backup_script=MasterBackup.sh
backup_full_path=$parent_dir/Backup/$backup_script

TAG=master
config_tool=ConfigModifier
if [[ $# -lt 1 ]];then
	echo -e "USAGE:$0 [single | cluster ] slave_ip, slave_heart_ip,backup_path,uuid,serverID"
	logInfo "start master failed,$*."
	exit 1;
fi

mode=$1
if [ "$mode" == "single" ];then
	para_num=4
else
	para_num=6
fi

if [[ $# -lt $para_num ]];then
	echo -e "USAGE:$0 [single | cluster ] slave_ip, slave_heart_ip,backup_path,uuid,serverID"
	logInfo "start master failed,$*."
	exit 1;
fi

#备机情况
backup_path=$2
uuid=$3
serverID=$4
if [ "$mode" == "cluster" ];then
	slave_ip=$5
	slave_heartbeat_ip=$6
fi

#mysql登录的用户名和密码
getUserCfg

#同步信息的用户名和密码
getSyncUserCfg

function startMaster()
{
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=0;"
    mysql -u$mysql_user -p$mysql_password -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -e "reset slave all;"
    logInfo "reset slave all, become master, slave_ip:$slave_ip, slave_heartbeat_ip:$slave_heartbeat_ip."

    #获取主机的位置
    #mysql -u$mysql_user -p$mysql_password -e "set global read_only=1;"
    #sh putMasterInfo.sh

    #查找是否存在同步用户，存在则不创建，避免异常断电导致user表损坏 #redmime 67585
    slave_user_count=$(mysql -u$mysql_user -p$mysql_password -e  "select * from mysql.user where Host='%' and User='$sync_user';" |wc -l)
    if [ $slave_user_count -eq 0 ];then
         mysql -u$mysql_user -p$mysql_password -e  "grant replication slave,replication client on *.* to '$sync_user'@'%' identified by '$sync_password';flush privileges;"
    fi
       
    #启动事件机制
    #mysql -u$mysql_user -p$mysql_password -e "set global event_scheduler=ON;"
    setEvent enable
}

function startMasterSingle()
{
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=0;"
    mysql -u$mysql_user -p$mysql_password -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -e "reset slave all;"

    #尝试删除旧的grant用户
    mysql -u$mysql_user -p$mysql_password -e  "use mysql;delete from user where User='$sync_user';flush privileges;"
	
	#启动事件机制
	#mysql -u$mysql_user -p$mysql_password -e "set global event_scheduler=ON;"
	setEvent enable
}

function startBackup()
{
    #获取masterback的路径
    sed -i '/MasterBackup/d' /var/spool/cron/root
    chmod +x $backup_full_path
    script_tmp=`echo $backup_full_path | sed 's#\/#\\\/#g'`
    #sed -i '$a0 1 * * * '$script_tmp' '$backup_path' '$uuid' '$serverID'' /var/spool/cron/root
	#注:sed -i '$a '匹配行结束，然后添加,/var/spool/cron/root 为空时，则无匹配项，不会添加
	echo '0 1 * * * '$script_tmp' '$backup_path' '$uuid' '$serverID'' >>/var/spool/cron/root
    logInfo "add cron task(master_backup)."
}

logInfo "begin to start master."
if [ "$mode" == "cluster" ];then
	startMaster
else
	startMasterSingle
fi
startBackup
logInfo "end to start master."

exit 0