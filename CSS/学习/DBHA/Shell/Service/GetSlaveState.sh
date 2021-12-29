#!/bin/bash
#提供三个状态值slave_connect_master、slave_running_state、slave_sql_thread
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh
server_path=$DBHA_SHELL/Service/MysqlServer.sh

TAG=getSlaveState
#mysql登录的用户名和密码
getUserCfg

#主机ip
master_ip=$1

#检测是主机还是备机,1表示主机，0表示备机
function checkMode()
{
    sql_running_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Host" | awk -F": " '{print $2}' )
    #主机则自动退出
    if [ "$sql_running_status" = "" ]; then
        return 1;
    fi

    return 0;
}

function printSlaveStatusInfo()
{
    Master_Host=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Master_Host" | awk -F": " '{print $2}')
    Master_Log_File=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Log_File" | awk -F": " '{print $2}')
    Relay_Master_Log_File=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Master_Log_File" | awk -F": " '{print $2}')
    Read_Master_Log_Pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
    Exec_Master_Log_Pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Exec_Master_Log_Pos" | awk -F": " '{print $2}')
    Relay_Log_File=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Log_File" | awk -F": " '{print $2}')
    Relay_Log_Pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Log_Pos" | awk -F": " '{print $2}')
    Slave_IO_State=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Slave_IO_State" | awk -F": " '{print $2}')
    Slave_SQL_Running_State=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Slave_SQL_Running_State" | awk -F": " '{print $2}')
    Last_IO_Errno=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Last_IO_Errno" | awk -F": " '{print $2}')
    Last_IO_Error=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Last_IO_Error:" | awk -F "Last_IO_Error:" '{print $2}')
    Last_SQL_Errno=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Last_SQL_Errno" | awk -F": " '{print $2}')
    Last_SQL_Error=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep "Last_SQL_Error" | awk -F": " '{print $2}')
    Slave_IO_Running=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    Slave_SQL_Running=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    REPL_STATUS=$(mysql -u$mysql_user -p$mysql_password -e "select * from performance_schema.replication_applier_status_by_worker\G")
    logInfo "slave sql running io info begin"
    logInfo "Master_Host=$Master_Host,Slave_IO_Running=$Slave_IO_Running,Slave_SQL_Running=$Slave_SQL_Running;"
    logInfo "Master_Log_File=$Master_Log_File,Read_Master_Log_Pos=$Read_Master_Log_Pos,Relay_Master_Log_File=$Relay_Master_Log_File,Exec_Master_Log_Pos=$Exec_Master_Log_Pos"
    logInfo "Relay_Log_File=$Relay_Log_File,Relay_Log_Pos=$Relay_Log_Pos"
    logInfo "Slave_IO_State=$Slave_IO_State,Last_IO_Errno=$Last_IO_Errno,Last_IO_Error=$Last_IO_Error"
    logInfo "Slave_SQL_Running_State=$Slave_SQL_Running_State,Last_SQL_Errno=$Last_SQL_Errno,Last_SQL_Error=$Last_SQL_Error"
    logInfo "REPL_APPLIER_STATUS:$REPL_STATUS"
    logInfo "slave sql running io info end."
}

#检测主备是否同步，0表示同步，1表示不同步
function getRunningState()
{
    #备机的IO线程有问题,0:No,1:connection,2:Yes
    io_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    if [ "$io_status" = "Yes" ];then
        slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
		if [[ "$slave_status" != "Yes" ]];then
            printSlaveStatusInfo
			return 1
		fi
    elif [ "$io_status" = "Connecting" ];then
        retry_cnt=$( mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep Connect_Retry | awk -F ": " '{print $2}')
        has_retry_cnt=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep Last_IO_Error | grep retries | awk -F "retries: " '{print $2}')
        if [ $has_retry_cnt -gt $retry_cnt ];then
            printSlaveStatusInfo
            return 1
        fi
    else
        printSlaveStatusInfo
		return 1
	fi

	return 0
}

#检测同步状态，
#"slave_running_state" : 0,         /* 备机特有的，备机上IO thread,0表示初始值，1表示同步中,2表示同步失败 */
function getSyncState()
{
	if [[ "$master_ip" == "" ]];then
		return 0
	fi

    slave_master_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Log_File" | awk -F": " '{print $2}')
    relay_master_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Master_Log_File" | awk -F": " '{print $2}')
    slave_read_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
    exec_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Exec_Master_Log_Pos" | awk -F": " '{print $2}')
	
	if [ "$slave_master_log_file" != "$relay_master_log_file" ] || [ "$slave_read_master_log_pos" != "$exec_master_log_pos" ];then
		logInfo  "slave_master_log_file:$slave_master_log_file, relay_log_file:$relay_master_log_file, slave_read_master_log_pos: $slave_read_master_log_pos, exec_master_log_pos:$exec_master_log_pos"
		return 1
	fi
	
	master_log_file=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show master status\G" | grep -w "File" | awk -F": " '{print $2}')
	master_log_pos=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show master status\G" | grep -w "Position" | awk -F": " '{print $2}')
	if [ "$master_log_file" != "$slave_master_log_file" ] || [ "$master_log_pos" != "$slave_read_master_log_pos" ];then
		logInfo  "slave_master_log_file:$slave_master_log_file, relay_log_file:$relay_master_log_file, slave_read_master_log_pos:$slave_read_master_log_pos, exec_master_log_pos:$exec_master_log_pos, master_log_file:$master_log_file, master_log_pos:$master_log_pos"
		return 1
	fi

	return 0	
}

function getSlaveState()
{
    sh $server_path status
    if [[ $? -ne 0 ]];then
        return 1
    fi

	#此步骤可以省略
    checkMode
    if [[ $? -ne 0 ]];then
        return 1
    fi

    getRunningState
	if [[ $? -ne 0 ]];then
        return 1
    fi

    getSyncState
	return $?
}

getSlaveState
exit $?