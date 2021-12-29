#!/bin/bash
#记录了当前机器作为主机的文件名和位置偏移
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)
bin_dir=$(dirname $parent_dir)/Bin
source $parent_dir/Common/CommonFunc.sh
sql_stat=$DBHA_CONFIG/Sql.stat

config_tool=$bin_dir/ConfigModifier
#mysql登录的用户名和密码
getUserCfg

reserver_1=information_schema
reserver_2=mysql
reserver_3=performance_schema
reserver_4=sys

function cleanupSQL()
{
	mysql -u$mysql_user -p$mysql_password -e "stop slave;"
	mysql -u$mysql_user -p$mysql_password -e "reset slave all;"
	mysql -u$mysql_user -p$mysql_password -e "reset master;"
	
	cd $shell_path
	$CONFIG_TOOL $sql_stat  '{"SQL_mode" : -1, "SQL_write_state": 0,"SQL_init_binlog_file" : "","SQL_init_binlog_position" : 0,"master_change_flag" : -1,
	"master_change_binlog_file" : "","master_change_binlog_postion" : 0, "slave_connect_master" : "","slave_sync_info" : ""}'
}

function cleanupDatabases()
{
	databases=`mysql -u$mysql_user -p$mysql_password -e "show databases\G" |grep Database |awk -F ": " '{print $2}'`
	for database in $databases;
	do
		if [[ "$database" != "$reserver_1" ]] && [[ "$database" != "$reserver_2" ]] && [[ "$database" != "$reserver_3" ]] && [[ "$database" != "$reserver_4" ]];then
			logInfo "attention:drop $database"
			mysql -u$mysql_user -p$mysql_password -e "drop database $database;"
		fi
	done
}