#!/bin/bash
#提供三个状态值slave_connect_master、slave_running_state、slave_sql_thread
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

TAG=checkSyncState
source $parent_dir/Common/CommonFunc.sh

#mysql登录的用户名和密码
getUserCfg

#检测同步状态，
#"slave_running_state" : 0,         /* 备机特有的，备机上IO thread,0表示初始值，1表示同步中,2表示同步失败 */
function checkSyncState()
{
    slave_master_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Master_Log_File" | awk -F": " '{print $2}')
    relay_master_log_file=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Relay_Master_Log_File" | awk -F": " '{print $2}')
    slave_read_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
    exec_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Exec_Master_Log_Pos" | awk -F": " '{print $2}')
	
	if [ "$slave_master_log_file" != "$relay_master_log_file" ] || [ "$slave_read_master_log_pos" != "$exec_master_log_pos" ];then
		logInfo  "slave_master_log_file:$slave_master_log_file, relay_log_file:$relay_master_log_file, slave_read_master_log_pos: $slave_read_master_log_pos, exec_master_log_pos:$exec_master_log_pos"
		return 1
	fi
	
	echo "$slave_master_log_file $slave_read_master_log_pos"
	return 0	
}

checkSyncState
exit $?