#!/bin/bash
#恢复数据，必须是在主备信息完整的情况下进行恢复，否则主单独恢复没有问题，但是备恢复的时候，会没有主的用户信息
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

TAG=recovery
source $parent_dir/Common/CommonFunc.sh

#服务配置文件路径
CONFIG_PATH=$DBHA_CONFIG/DBHA.conf

if [[ $# -lt 2 ]];then
    echo -e "USAGE:$0 file_name(full path)"
    logInfo "back failed,$*."
    exit 1;
fi
logInfo "enter recover.sh $*"

#备份文件的名字
file_name=$2
#主机恢复还是备机恢复，默认备机
flag=0
if [[ $# -ge 3 ]];then
    flag=1
fi

#mysql登录的用户名和密码
getUserCfg

function stopRecovery()
{
    logInfo "stop recovery,filename:$file_name."
    #获取恢复脚本进程号
    recover_pid=`ps -ef|grep "Recovery"|grep "start"|grep "$file_name" | grep -v grep | awk '{print $2}'`
    if [[ "$recover_pid" = "" ]];then
        logInfo "recovery shell pid is not exist."
        return 0
    fi
   
    #获取数据库恢复进程号
	sql_connect_pid=`ps -ef|grep "mysql"|grep "$recover_pid"| grep -v grep | awk '{print $2}'`
    if [[ "$sql_connect_pid" = "" ]];then
        logInfo "recovery sql connect pid is not exist."
        return 0
    fi
   
    kill -9 $recover_pid
    kill -9 $sql_connect_pid
    logInfo "stop recovery $recover_pid, $sql_connect_pid"
    return 0
}

function checkSlaveRunning()
{
    io_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    if [[ "$io_status" != "Yes" ]] || [[ "$slave_status" != "Yes" ]];then
        slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G")
        logInfo "checkSlaveRunning error:$slave_status"
        return 1
    else
        return 0
    fi
}

function startSlave()
{
    mysql -u$mysql_user -p$mysql_password -e "start slave;"
    #关闭事件机制
    setEvent disable
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=1;"

    checkSlaveRunning
    if [[ $? -ne 0 ]];then
        logInfo "start slave failed while recovery success or killed."
        return 1
    fi

    #cd $shell_path
    #./$config_tool $mysql_state_file '{"SQL_write_state" : 1}'
    logInfo "start slave success while recovery success."
    return 0
}

#删除除了基本库和忽略库以外的所有库
function purgeDatabases()
{
	getIgnoreDB
	databases=(`mysql -u$mysql_user -p$mysql_password -e "show databases;" 2>/dev/null |grep -Ewv "$all_ignore_dbs"`)
	num=${#databases[@]}
	for((i=0; i<$num; i++))
	do
		logInfo "drop database ${databases[$i]}"
		mysql -u$mysql_user -p$mysql_password -e "drop database ${databases[$i]};" 2>/dev/null
	done
}

#恢复完成后，机器为只读模式
function startRecovery()
{
    #停止模式
    mysql -u$mysql_user -p$mysql_password -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -e "reset slave;"
    #变成只读模式
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=0;"
    #关闭binlog日志
	
    #获取备份路径
	local backup_dir=`cat ${CONFIG_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
	local backup_path=$(echo $backup_dir | sed 's/\\//g')
   
    logInfo "start to recovery."
	#修改成使用mysqldump导出的drop数据库语句
	#purgeDatabases
	#恢复数据,session内不产生binlog
    gunzip < $file_name | mysql -u$mysql_user -p$mysql_password --init-command='set sql_log_bin=OFF' >> $LOG_FILE 2>&1
	if [ $? != 0 ];then
		logInfo "mysql recovery return failed!"
		#删除掉恢复文件
		rm -rf $backup_path/slaveRecoverFile
		#恢复执行完删除一遍binglog,防止binglog过多占用太多磁盘空间
		sh $DBHA_SHELL/Binlog/PurgeBinlog.sh
		return 1
	fi
	
    if [ -f "$file_name" ];then
        content=$(gunzip < $file_name | head -n 3)
        if [ -z "$content" ];then
            rm -rf $backup_path/slaveRecoverFile
            logInfo "$file_name content empty,checked it!"
            return 1
        fi
    fi
    
	#删除掉恢复文件
	rm -rf $backup_path/slaveRecoverFile
	#恢复执行完删除一遍binglog,防止binglog过多占用太多磁盘空间
	sh $DBHA_SHELL/Binlog/PurgeBinlog.sh
	
    logInfo "recovery success or killed, file_name:$file_name."
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=1;"
    logInfo "success to recovery, write state is no write no read."
    #cd $shell_path
    #./$config_tool $mysql_state_file '{"SQL_write_state" : 0}'

    if [[ $flag -eq 0 ]];then
        startSlave;
        return $?
    fi
    return 0
}

#备份文件，$1:备份文件的路径，$2:UUID，$3:server_id，$4:主备模式master slave，$5:自动还是手动auto,manual
function startRecoveryWithXtrabackup()
{	
	#从配置文件中读取当前备份路径
	local backup_dir=`cat ${CONFIG_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
	local backup_path=$(echo $backup_dir | sed 's/\\//g')
	
	FULL_BACKUP_PATH=" ${backup_path}/xtrabackup/backup"
	SLAVE_BACKUP_TMP_DIR="${backup_path}/xtrabackup/recover"
	RECOVER_LOG="${backup_path}/xtrabackup/recover.log"
	SLAVE_MYSQL_DATA_DIR=`cat /etc/my.cnf | grep datadir | awk -F "=" '{print $2}' | sed 's/[ ]//g' | sed 's/\/$//g'`
	SLAVE_MYSQL_DATA_DIR_PARENT=`dirname $SLAVE_MYSQL_DATA_DIR`
	
	#恢复前空间检查
	need_space=`du -m $SLAVE_MYSQL_DATA_DIR_PARENT --max-depth=1 |grep ${SLAVE_MYSQL_DATA_DIR}$ | awk '{print $1}'`
	avail_space=`df -m $backup_path | awk 'END{print}' | awk -F ' ' '{print $(NF-2)}'`
	used_space=$((need_space + 1024))
	if [ $used_space -gt $avail_space ];then
		logInfo "startRecoveryWithXtrabackup: No space left on device,use $used_space , free $avail_space!"
		return 1
	fi
	
	#FULL_BACKUP_PATH="/data/mysql_backup/xtrabackup/backup"
    #SLAVE_BACKUP_TMP_DIR="/data/mysql_backup/xtrabackup/recover"
	#RECOVER_LOG="/data/mysql_backup/xtrabackup/recover.log"
	
	logInfo "startRecoveryWithXtrabackup:cleanup $SLAVE_BACKUP_TMP_DIR."
	rm -rf $SLAVE_BACKUP_TMP_DIR
	mkdir -p $SLAVE_BACKUP_TMP_DIR	
	tar -izxvf $file_name -C $SLAVE_BACKUP_TMP_DIR > $RECOVER_LOG 2>&1
	local ret=$?
	if [ $ret != 0 ];then
		logInfo "startRecoveryWithXtrabackup[$ret]:restore_prepare_env tar -ixvf $file_name -C $SLAVE_BACKUP_TMP_DIR failed"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	#停止模式
    mysql -u$mysql_user -p$mysql_password -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -e "reset slave all;"
    #变成只读模式
    mysql -u$mysql_user -p$mysql_password -e "set global read_only=0;"
	
	logInfo "startRecoveryWithXtrabackup:stop mysql."
	service mysqld stop
	if [ $? != 0 ];then
		logInfo "startRecoveryWithXtrabackup:mysql stop failed!"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	#删除数据库目录、进行恢复
	logInfo "startRecoveryWithXtrabackup:cleanup $SLAVE_MYSQL_DATA_DIR."
	rm -rf $SLAVE_MYSQL_DATA_DIR
	mkdir -p $SLAVE_MYSQL_DATA_DIR
	XTRABACKUP_HOME=/cloud/dahua/DataBaseServer/xtrabackup
	PATH=$PATH:$XTRABACKUP_HOME/bin
	export PATH XTRABACKUP_HOME
	source /etc/profile
	
	logInfo "startRecoveryWithXtrabackup:restore_mysql begin"
	innobackupex --user=$mysql_user --password=$mysql_password --apply-log $SLAVE_BACKUP_TMP_DIR >> $RECOVER_LOG 2>&1
	ret=$?
	if [ $ret != 0 ];then
		logInfo "startRecoveryWithXtrabackup[$ret]:innobackupex prepare failed:$SLAVE_BACKUP_TMP_DIR"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	innobackupex --defaults-file=/etc/my.cnf --user=$mysql_user --password=$mysql_password --copy-back $SLAVE_BACKUP_TMP_DIR >> $RECOVER_LOG 2>&1
	ret=$?
	if [ $ret != 0 ];then
		logInfo "startRecoveryWithXtrabackup[$ret]:innobackupex copy-back failed:$SLAVE_BACKUP_TMP_DIR"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	chown -hR mysql:mysql $SLAVE_MYSQL_DATA_DIR
	service mysqld start
	ret=$?
	if [ $ret != 0 ];then
		logInfo "startRecoveryWithXtrabackup[$ret]:mysql start failed!"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi

    logInfo "recovery success or killed, file_name:$file_name."
	if [[ $flag -eq 0 ]];then
		mysql -u$mysql_user -p$mysql_password -e "set global read_only=1;"
	fi
    logInfo "success to recovery, write state is no write no read."
	
	#清除recover文件
	rm -rf $SLAVE_BACKUP_TMP_DIR
    #cd $shell_path
    #./$config_tool $mysql_state_file '{"SQL_write_state" : 0}'
    return 0
}

case $1 in
  "start")
  startRecovery
  exit $?
  ;;
  "stop")
  stopRecovery
  exit $?
  ;;
  *)
  echo "Usage: $0 {start|stop}"
  exit 1
  ;;
esac
