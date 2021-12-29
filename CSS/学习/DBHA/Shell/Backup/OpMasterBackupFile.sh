#!/bin/bash
#获取当前dump下的最新dump文件
TAG=opMasterBackupFile
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh

#mysql登录的用户名和密码
getUserCfg

if [[ $# -lt 3 ]];then
    echo -e "USAGE:$0 file_path_dir uuid, server_id"
    logInfo "back failed,$*."
    exit 1;
fi

#检测备份文件的有效性
function checkBackupValid()
{
	backupFileName=$1
	if [[ ! -f $file_dir/$backupFileName ]];then
		logInfo "$file_dir/$backupFileName is not exist."
		return 1
	fi

	backup_file_tmp=`cat $file_dir/$backupFileRecordName |grep "$backupFileName" | grep -v grep |  awk -F " " '{print $1}'`
	backup_file=`echo $backup_file_tmp | grep "$uuid"|grep "$server_id"`
	if [[ "$backup_file" = "" ]];then
		logInfo "$backup_file_tmp is not exist."
		return 1
	fi

	binlog_name=`cat $file_dir/$backupFileRecordName | grep "$backupFileName" | grep -v grep |  awk -F " " '{print $2}'`
	if [[ ! -f $binlog_name ]];then
		logInfo "$binlog_name is not exist."
		return 1
	fi
    
	local change_master_file_tmp=`cat $SQL_FILE | grep -w "master_change_binlog_file" | awk -F": |,|/*" '{print $2}' | sed 's/"//g' | awk -F "mysql-bin." '{print $2}'`
	local binlog_name_format=`echo $binlog_name | awk -F "mysql-bin." '{print $2}'`
	if [[ "$change_master_file_tmp" != "" ]] && [[ "$change_master_file_tmp" > "$binlog_name_format" ]];then
	logInfo "$binlog_name is outdate $change_master_file_tmp."
		return 1
	fi
	
	#如果当前最新备份的记录和数据库相差很大，则重新重生备份文件
	#获取主机当前自己的binlog文件名和位置信息
         master_log_file=$(mysql -u$mysql_user -p$mysql_password  -e "show master status\G" | grep -w "File" | awk -F": " '{print $2}'| awk -F "mysql-bin." '{print $2}'|sed -r 's/0*([0-9])/\1/')
        backup_binlog_file=$(echo $binlog_name_format|sed -r 's/0*([0-9])/\1/')
        local binlog_diff_value=$(($master_log_file-$backup_binlog_file))
        if [[ $binlog_diff_value > 2 ]];then
        logInfo "current binlog-backup binlog more than 2, so backup file outdate"
                return 0
        fi
	
	binlog_pos=`cat $file_dir/$backupFileRecordName | grep "$backupFileName" | grep -v grep |  awk -F " " '{print $3}'`
	md5_sum=`cat $file_dir/$backupFileRecordName | grep "$backupFileName" | grep -v grep |  awk -F " " '{print $4}'`

	return 0
}


#获取最新的备份文件
function getNewBackupFileName()
{
	if [ ! -f $file_dir/$backupFileRecordName ];then
		logInfo "$file_dir/$backupFileRecordName is not exist."
		echo ""
		return 0
	fi
	
	file_name=`sed -n '1p' $file_dir/$backupFileRecordName | awk -F " " '{print $1}'`
	backup_file_tmp=`echo $file_name | grep "$uuid"|grep "$server_id"`
	if [[ "$backup_file_tmp" = "" ]];then
		logInfo "$file_name is not exist."
		echo ""
		return 0
	fi
	
	#如果文件不存在
	if [[ ! -f $backup_file_tmp ]];then
		purgeMasterFileWithName $file_dir $uuid $server_id $backup_file_tmp
		logInfo "$backup_file_tmp is not exist."
		echo ""
		return 0
	fi

	binlog_name=`sed -n '1p' $file_dir/$backupFileRecordName | awk -F " " '{print $2}'`
	if [[ ! -f $binlog_name ]];then
		echo ""
		return 0
	fi
    
	#比较是否过期
	local change_master_file_tmp=`cat $SQL_FILE | grep -w "master_change_binlog_file" | awk -F": |,|/*" '{print $2}' | sed 's/"//g' | awk -F "mysql-bin." '{print $2}'`
	local binlog_name_format=`echo $binlog_name | awk -F "mysql-bin." '{print $2}'`
	if [[ "$change_master_file_tmp" != "" ]] && [[ "$change_master_file_tmp" > "$binlog_name_format" ]];then
	logInfo "$binlog_name is outdate $change_master_file_tmp."
		echo ""
		return 0
	fi
	
	#如果当前最新备份的记录和数据库相差很大，则重新重生备份文件
	#获取主机当前自己的binlog文件名和位置信息
	master_log_file=$(mysql -u$mysql_user -p$mysql_password  -e "show master status\G" | grep -w "File" | awk -F": " '{print $2}'| awk -F "mysql-bin." '{print $2}'|sed -r 's/0*([0-9])/\1/')
        backup_binlog_file=$(echo $binlog_name_format|sed -r 's/0*([0-9])/\1/')
	local binlog_diff_value=$(($master_log_file-$backup_binlog_file))
	if [[ $binlog_diff_value > 2 ]];then
	logInfo "current binlog-backup binlog more than 2, so backup file outdate"
		return 0
	fi
	
	binlog_pos=`sed -n '1p' $file_dir/$backupFileRecordName | awk -F " " '{print $3}'`
	md5_sum=`sed -n '1p' $file_dir/$backupFileRecordName | awk -F " " '{print $4}'`
	echo "$file_name $binlog_name $binlog_pos $md5_sum"
	logInfo "filename:$file_name"
	return 0
}

#1表示文件已经改过了，0表示没有改过
function getDumpFile()
{
	logInfo "$file_dir, $uuid, $server_id, $backup_file"
	if [[ "$backup_file" = "" ]];then
		getNewBackupFileName $file_dir
		return 1
	else
		checkBackupValid $backup_file
		if [ $? -ne 0 ];then
			getNewBackupFileName $file_dir
			return 1
		fi
	fi

	echo "$file_dir/$backupFileName $binlog_name $binlog_pos $md5_sum"
	logInfo "filename:$file_dir/$backupFileName"
	return 0
}

#如果最新的备份文件都没用了，那么当前所有的文件肯定都无用了
function cleanDumpFile()
{
	purgeMasterFileWithName $file_dir $uuid $server_id $backup_file
}

function checkDumpPid()
{
	local backup_pid=`ps -ef|grep "Backup.sh" |grep "start" | grep "$file_dir" |grep "$uuid" |grep "$server_id"|awk '{print $2}'`
	if [ "$backup_pid" != "" ];then
		return 0
	else
		return 1
	fi
}

file_dir=$2
uuid=$3
server_id=$4
#此处为文件名，相对路径，在getDumpFile中表示备机的传入的主机文件名；在cleanDumpFile中表示主机需要从什么文件开始删除备份文件
backup_file=$5

case $1 in
  "getFile")
  getDumpFile
  exit $?
  ;;
  "cleanFile")
  cleanDumpFile
  exit $?
  ;;
  "checkDump")
  checkDumpPid
  exit $?
  ;;
esac
