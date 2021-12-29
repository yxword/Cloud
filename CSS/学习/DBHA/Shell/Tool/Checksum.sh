#!/bin/bash
shell_path=$(cd $(dirname "$0");pwd)
shell_parent=$(cd $(dirname "$shell_path");pwd)
#DBHA_CONFIG=$shell_parent/Config/DBHA.conf
TAG=checksum

source $shell_parent/Common/CommonFunc.sh
getUserCfg

DBHA_CONFIG=$DBHA_CONFIG/DBHA.conf

MASTER_HOST=$(cat $DBHA_CONFIG | grep 'DBHA_virtual_IP\"' | awk -F ":" '{print $2}' | awk -F "\"" '{print $2}')

SLAVE_HOST=$(mysql -u$mysql_user -p$mysql_password -h$MASTER_HOST -e "select HOST from information_schema.PROCESSLIST where USER='slave' limit 1;" 2>/dev/null | grep -v 'HOST' | awk -F ":" '{print $1}')
if [ -z "$SLAVE_HOST" ];then
	echo "[error]get slave host failed!"
	exit 1
fi
PTABLE=ptonlinecheck
need_check=0

#检查结果存储在ptonlinecheck数据库中，先对数据库进行一次清理
function cleanup()
{
	local dropSql="drop database $PTABLE"
	mysql -u$mysql_user -p$mysql_password -h$MASTER_HOST -e "$dropSql" 2>/dev/null;
}

#检测主备数据库是否存在单侧表
function check_meta_tables()
{
	local master=$1
	local slave=$2
	local database=$3
	tables=$(mysql -u$mysql_user -p$mysql_password -h$master $database -e "show tables" 2>/dev/null)
	slave_tables=$(mysql -u$mysql_user -p$mysql_password -h$slave $database -e "show tables" 2>/dev/null)
	local ret=0
	for table in ${tables[@]};do	
		check_slave_exist=$(echo $slave_tables | grep -c "$table")
		if [ $check_slave_exist -eq 0 ];then
			logInfo "[error][table missed]$database.$table [+$master-$slave]"
            echo "[error][table missed]$database.$table [+$master-$slave]"
			need_check=1
            ret=1
		fi
	done
	
	return $ret
}

#检测主备数据库是否存在单侧数据库
function check_meta_data()
{
	local master=$1
	local slave=$2
	
	getIgnoreDB
	#检查数据库是否一致
	databases=$(mysql -u$mysql_user -p$mysql_password -h$master -e "show databases" 2>/dev/null)
	for database in ${databases[@]}
	do
		need_ignore=$(echo $all_ignore_dbs | grep -c $database)
		if [ $need_ignore -gt 0 ];then
			#该数据库需要被忽略
			continue
		fi
		
		check_slave_exist=$(mysql -u$mysql_user -p$mysql_password -h$slave -e "show databases" 2>/dev/null | grep -c "$database")
		if [ $check_slave_exist -eq 0 ];then
			logInfo "[error][database missed]$database [+$master-$slave]"
            echo "[error][database missed]$database [+$master-$slave]"
            need_check=1
			continue
		fi
		
		#检测主备数据库是否存在单侧表
		check_meta_tables $master $slave $database
		local retm=$?
		check_meta_tables $slave $master $database
		local rets=$?
		if [ $retm -ne 0 ] || [ $rets -ne 0 ];then
			#内部含有打印信息
			continue
		fi
		
		#检查数据库的数据一致性
		local ptablename="$PTABLE.$database"
		local checkSql="select group_concat(tbl) from $database where this_crc <> master_crc or master_cnt <> this_cnt or ISNULL(master_crc) <> ISNULL(this_crc);"
		pt-table-checksum --nocheck-replication-filters --no-check-binlog-format --replicate=$ptablename --empty-replicate-table  --recursion-method=processlist  --databases $database -h $master -u $mysql_user -p $mysql_password -P 3306  --replicate-check 1>>$LOG_FILE 2>&1
		
		#强制等待180秒，保证结果的正确性
        result=$(mysql -u$mysql_user -p$mysql_password -h$slave $PTABLE -e "$checkSql" 2>/dev/null | grep -v 'tbl' )
        if [ ! -z "$result" ] && [ "$result"X != "NULL"X ];then
            logInfo "[error][ record missed ]$database.[$result]!"
            logInfo "[USAGE][pt-table-sync --replicate=$PTABLE.$database h=$master,u=$mysql_user,p=$mysql_password,P=3306 -h=$slave --print]"
            echo "[error][ record missed ]$database.[$result]!"
            echo "[USAGE][pt-table-sync --replicate=$PTABLE.$database h=$master,u=$mysql_user,p=$mysql_password,P=3306 -h=$slave --print]"
            need_check=1
        else
            logInfo "[info][ record consistent ]$database!"
            echo "[info][ record consistent ]$database"
        fi
	done
	
	return 0
}

#检测是否存在单侧数据库
function check_database_only()
{
	local master=$1
	local slave=$2
	
	getIgnoreDB
	databases=$(mysql -u$mysql_user -p$mysql_password -h$master -e "show databases" 2>/dev/null)
	for database in ${databases[@]}
	do
		need_ignore=$(echo $all_ignore_dbs | grep -c $database)
		if [ $need_ignore -gt 0 ];then
			#该数据库需要被忽略
			continue
		fi
		
		check_slave_exist=$(mysql -u$mysql_user -p$mysql_password -h$slave -e "show databases" 2>/dev/null | grep -c "$database")
		if [ $check_slave_exist -eq 0 ];then
			logInfo "[database missed]$database [+$master-$slave]"
            echo "[database missed]$database [+$master-$slave]"
            need_check=1
			continue
		fi
	done
	
	return 0
}

function main()
{
	#环境清理
	cleanup
	check_meta_data $MASTER_HOST $SLAVE_HOST
	check_database_only $SLAVE_HOST $MASTER_HOST
	cleanup
    
    if [ $need_check -eq 1 ];then
        echo "[finished]$MASTER_HOST and $SLAVE_HOST databases are inconsistent! "
    else
        echo "[finished]$MASTER_HOST and $SLAVE_HOST database are consistent! "
    fi
    return 0
}

main
