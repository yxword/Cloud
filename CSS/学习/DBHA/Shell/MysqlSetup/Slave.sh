#!/bin/bash
#mysql主备模式下启动备机模式
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

TAG=slave

source $parent_dir/Common/CommonFunc.sh

slave_path=$parent_dir/Service/CheckSlave.sh
mysql_state_file=$DBHA_CONFIG/Sql.stat

#服务配置文件路径
CONFIG_PATH=$DBHA_CONFIG/DBHA.conf
HACONFIG_PATH=$DBHA_CONFIG/HAConfig.conf

config_tool=ConfigModifier

#主备ip
master_ip=$2
master_file=$3
master_pos=$4
server_id=$5
master_back_up_dir=$6

#mysql登录的用户名和密码
getUserCfg

#同步信息的用户名和密码
getSyncUserCfg

function checkSlaveRunning()
{
    io_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    if [[ "$io_status" != "Yes" ]] || [[ "$slave_status" != "Yes" ]];then
        slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G")
        logInfo "checkSlaveRunning error:$slave_status"
        repl_status=$(mysql -u$mysql_user -p$mysql_password -e "select * from performance_schema.replication_applier_status_by_worker\G")
        logInfo "checkSlaveRunning error:$repl_status"
        return 1
    else
        return 0
    fi
}

#备份失败之后，用于清除在恢复之前就设置的master binglog文件和位置，保证故障恢复可以持续进行
function cleanSlave()
{
    mysql -u$mysql_user -p$mysql_password -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -e "reset slave;"
    mysql -u$mysql_user -p$mysql_password -e "start slave;"
    
}

#清理备机上的主机备份文件
function cleanMasterBackupFiles(){
	
	#获取备份路径
	local backup_dir=`cat ${CONFIG_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
	local backup_path=$(echo $backup_dir | sed 's/\\//g')
	
	#当变成备机后，主动将当前备机上所有作为主的备份文件全部删除
	rm -rf $backup_path/*master*$server_id*
	rm -rf $backup_path/$backupFileRecordName
}

#启动备机模式
function slaveStart()
{
    logInfo "begin to start slave, master ip:$master_ip, master_log_file=$master_file, master_log_pos=$master_pos."
    #尝试去掉备份机制
    sed -i '/masterBackup/d' /var/spool/cron/root
    sed -i '/MasterBackup/d' /var/spool/cron/root

    mysql -u$mysql_user -p$mysql_password -e "set global read_only=0;"

	#关闭事件机制
	#mysql -u$mysql_user -p$mysql_password -e "set global event_scheduler=ON;"
	setEvent disable

    mysql -u$mysql_user -p$mysql_password -e "stop slave;"
	
	#根据master_ip获取本机的master_bond信息,为空时走默认值
	master_business_ip=`cat ${CONFIG_PATH} | grep \"peer_business_IP\" | awk -F '"' '{print $4}'`
	business_ip=`cat ${CONFIG_PATH} | grep \"local_business_IP\" | awk -F '"' '{print $4}'`
	master_heart_ip=`cat ${CONFIG_PATH} | grep \"peer_heartbeat_IP\" | awk -F '"' '{print $4}'`
	heart_ip=`cat ${CONFIG_PATH} | grep \"local_heartbeat_IP\" | awk -F '"' '{print $4}'`
	master_bind=""
	if [ "$master_ip"X == "$master_business_ip"X ];then
		master_bind=$business_ip
	fi
	
	if [ "$master_ip"X == "$master_heart_ip"X  ];then
		master_bind=$heart_ip
	fi

    #判定备机状态是空的，需要主机上的信息来进行配置
    master_host=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Host" | awk -F": " '{print $2}')
    master_user=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_User" | awk -F": " '{print $2}')
    master_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Log_File" | awk -F": " '{print $2}')
    read_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
    relay_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Log_File" | awk -F": " '{print $2}')
    relay_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Log_Pos" | awk -F": " '{print $2}')
    if [ "$master_host" = "" ] || [ "$master_log_file" = "" ] || [ "$master_log_file" = "0" ];then
        #DBHA设置的binlog不可信任，以mysql保存的为准，只有在host为空时才下发全新配置，其他情况做局部字段的更新
        mysql -u$mysql_user -p$mysql_password -e "change master to master_host='$master_ip', master_user='$sync_user', master_password='$sync_password', master_bind='$master_bind', MASTER_CONNECT_RETRY=5,master_port=3306, master_log_file='$master_file', master_log_pos=$master_pos,MASTER_SSL_VERIFY_SERVER_CERT=1;flush privileges;"
        logInfo "start with empty, set master info, master_ip:$master_ip, sync_user:$sync_user, master_file_name:$master_file, master_log_pos:$master_pos, master_bind:$master_bind."
    elif [ "$master_host" != "$master_ip" ] || [ "$master_user" = "" ] || [ "$master_user" != "$sync_user" ];then
        # 先停掉slave后，再取出当前的master binlog信息，与IP一起设置，避免取出后，实际的Master_Log_File和Read_Master_Log_Pos已经变化了
        mysql -u$mysql_user -p$mysql_password -e "change master to master_host='$master_ip', master_user='$sync_user', master_password='$sync_password', master_bind='$master_bind', MASTER_CONNECT_RETRY=5,master_port=3306, master_log_file='$master_log_file', master_log_pos=$read_master_log_pos, relay_log_file='$relay_log_file', relay_log_pos=$relay_log_pos,MASTER_SSL_VERIFY_SERVER_CERT=1;flush privileges;"
        logInfo "start with update, set master info, master_ip:$master_ip, sync_user:$sync_user, master_file_name:$master_log_file, master_log_pos:$read_master_log_pos, relay_log_file:$relay_log_file, relay_log_pos:$relay_log_pos, master_bind:$master_bind."
    else
        #备机非空白模式的时候，直接启动备机
        logInfo "start with self, master_ip:$master_ip, sync_user:$sync_user, file:$master_log_file, pos:$read_master_log_pos, relay_log_file:$relay_log_file, relay_log_pos:$relay_log_pos."
    fi

    logInfo "start slave"
    mysql -u$mysql_user -p$mysql_password -e "start slave;"
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=1;"

    checkSlaveRunning
    if [[ $? -ne 0 ]];then
        logInfo "end to start slave failed, master ip:$master_ip."
        return 1
    fi

    #cd $shell_path
    #./$config_tool $mysql_state_file '{"SQL_write_state" : 1}'
    logInfo "end to start slave success, master ip:$master_ip."
    return 0
}

function changeSlave()
{
	# 检查指定ip的mysql是否可用
	mysql -u$mysql_user -p$mysql_password -h$master_ip --connect_timeout=2 -e "show master status" >/dev/null
	if [ $? -ne 0 ];then
		logInfo "master ip ${master_ip} is unavailable"
		return 1
	fi
	#根据master_ip获取本机的master_bond信息,为空时走默认值
	master_business_ip=`cat ${CONFIG_PATH} | grep \"peer_business_IP\" | awk -F '"' '{print $4}'`
	business_ip=`cat ${CONFIG_PATH} | grep \"local_business_IP\" | awk -F '"' '{print $4}'`
	master_heart_ip=`cat ${CONFIG_PATH} | grep \"peer_heartbeat_IP\" | awk -F '"' '{print $4}'`
	heart_ip=`cat ${CONFIG_PATH} | grep \"local_heartbeat_IP\" | awk -F '"' '{print $4}'`
	master_bind=""
	if [ "$master_ip"X == "$master_business_ip"X ];then
		master_bind=$business_ip
	fi
	
	if [ "$master_ip"X == "$master_heart_ip"X  ];then
		master_bind=$heart_ip
	fi
	
	mysql -u$mysql_user -p$mysql_password -e "stop slave;"
	# 先停掉slave后，再取出当前的master binlog信息，与IP一起设置，避免取出后，实际的Master_Log_File和Read_Master_Log_Pos已经变化了
	master_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Log_File" | awk -F": " '{print $2}')
	read_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
	relay_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Log_File" | awk -F": " '{print $2}')
    relay_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Log_Pos" | awk -F": " '{print $2}')
    mysql -u$mysql_user -p$mysql_password -e "change master to master_host='$master_ip', master_log_file='$master_log_file', master_log_pos=$read_master_log_pos, relay_log_file='$relay_log_file', relay_log_pos=$relay_log_pos, master_bind='$master_bind', MASTER_SSL_VERIFY_SERVER_CERT=1;"
	mysql -u$mysql_user -p$mysql_password -e "start slave;"
	logInfo "change slave sync ip is $master_ip"
	return 0
}

function createMasterBackupFile(){
	local master_ip=`cat ${CONFIG_PATH} | grep \"peer_business_IP\" | awk -F '"' '{print $4}'`
	local backup_dir_path=`cat ${CONFIG_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
	local backup_dir=$(echo $backup_dir_path | sed 's/\\//g')
	local uid=`cat ${HACONFIG_PATH} | grep \"uid\" | awk -F '"' '{print $4}'`
	local serverId=`ssh $master_ip cat /etc/my.cnf | grep 'server-id' | awk -F '=' '{print $2}'|head -1`
	
	ssh $master_ip $shell_path/Backup.sh start $backup_dir $uid $serverId master
}

case $1 in
  "startSlave")
  slaveStart
  exit $?
  ;;
 "changeSlaveInfo")
  changeSlave
  exit $?
  ;;
 "cleanSlave")
	cleanSlave
    exit $?
  ;;
 "cleanMasterBackup")
	cleanMasterBackupFiles
    exit $?
	;;
 "createMasterBackupFile")
	createMasterBackupFile
    exit $?
	;;
  *)
    echo "error format."
    exit 1
  ;;
esac
