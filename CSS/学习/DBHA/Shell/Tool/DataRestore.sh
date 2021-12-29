#!/bin/bash
################################################################################################################################################################################
#  使用：sh dataRestore [online_xtrabackup_backup|online_xtrabackup_recover|online_xtrabackup_backup_recover|offline_scp_backup_recover|start_slave|online_mysqldump_database]
#   sh dataRestore.sh online_xtrabackup_backup $masterIP $slaveIP $backupPath:将masterIP上的数据库数据以$xtrabackup_backup_name的形式在线备份到slaveIP的backupPath目录下
#   sh dataRestore.sh online_xtrabackup_recover $backupPath:将指定$xtrabackup_backup_name在backupPath中，将tar文件恢复到slaveIP中
#   sh dataRestore.sh online_xtrabackup_backup_recover $masterIP $slaveIP $backupPath:将masterIP上的数据库数据以$xtrabackup_backup_name的形式在线备份到slaveIP的backupPath目录下
#                                                 并在slaveIP上进行数据库恢复
#   sh dataRestore.sh offline_scp_backup_recover $masterIP $slaveIP:停业务使用scp将主机数据拷贝到备机,并拉起备机数据
#   sh dateRestore.sh start_slave $binlog_name $binlog_pos:拉起DBHA服务，让其从name,pos点开始追同步
#   sh dataRestore.sh online_mysqldump_database $src_host $dst_host $database $remote_backup_dir:将数据库使用mysqldumo从src导入到dst
#   sh dataRestore.sh online_mysqldump_table $src_host $dst_host $database $table $remote_backup_dir:将数据库下的单表使用mysqldumo从src导入到dst
################################################################################################################################################################################
shell_path=$(cd $(dirname "$0");pwd)
shell_parent=$(cd $(dirname "$shell_path");pwd)
source $shell_parent/Common/CommonFunc.sh
DBHA_CFG_FILE=$DBHA_CONFIG/DBHA.conf
self_shell=$0
xtrabackup_backup_name="xtrabackupManual.tar"
TAG=dataRestore

###########################################################################
# 将本机的数据使用xtrabackup备份到slave的指定backup目录中
###########################################################################
function backupWithXtrabackup()
{
	slave_host=$1
	remote_backup_dir=$2
	
    #checkDir $BACKUP_DIR
    getUserCfg
    logInfo "start to back up,slave_host=$slave_host,remote_backup_dir=$remote_backup_dir.";
	
	XTRABACKUP_HOME=/cloud/dahua/DataBaseServer/xtrabackup
	PATH=$PATH:$XTRABACKUP_HOME/bin
	export PATH XTRABACKUP_HOME
	source /etc/profile
	
	rm -rf $remote_backup_dir/backup.log
	/usr/bin/ssh root@$slave_host "rm -rf $remote_backup_dir/$xtrabackup_backup_name"
	innobackupex --user=$mysql_user --password=$mysql_password --throttle=$MYSQL_BACKUP_THROTTLE --no-lock \
		--stream=tar "$remote_backup_dir" 2>$remote_backup_dir/backup.log | ssh root@$slave_host "cat > $remote_backup_dir/$xtrabackup_backup_name" >> "$LOG_FILE" 2>&1
	if [ $? != 0 ];then
		log_info $LOG_FILE "backup mysql failed!"
		return 1
	fi
   
    date_info=$(date "+%Y_%m_%d_%H%M%S")
	new_backup_log=${remote_backup_dir}/backup_${date_info}.log
	mv $remote_backup_dir/backup.log $new_backup_log
	master_log_file_name=$(cat $new_backup_log | grep "binlog position" | awk -F "filename " '{print $2}' | awk -F "," '{print $1}' | tr -d "\'")
	master_log_pos=$(cat $new_backup_log | grep "binlog position" | awk -F "position " '{print $2}' | tr -d "\'")
    logInfo "end to back up, remote_backup_dir:$remote_backup_dir log=$master_log_file_name pos=$master_log_pos."
}

#启动备机模式
function startSlave()
{
	master_ip=$1
	master_file=$2
	master_pos=$3

	getUserCfg
	getSyncUserCfg
    logInfo "begin to start slave, master ip:$master_ip, master_log_file=$master_file, master_log_pos=$master_pos."
    /etc/init.d/mysqld start
    mysql -u$mysql_user -p$mysql_password -hlocalhost -e "set global read_only=0;" 2>/dev/null 

	#关闭事件机制
	#mysql -u$mysql_user -p$mysql_password -e "set global event_scheduler=ON;"
	setEvent disable

    mysql -u$mysql_user -p$mysql_password -hlocalhost -e "stop slave;" 2>/dev/null
	mysql -u$mysql_user -p$mysql_password -hlocalhost -e "reset slave all;" 2>/dev/null

    #判定备机状态是空的，需要主机上的信息来进行配置
    master_host=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Master_Host" | awk -F": " '{print $2}')
    master_user=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Master_User" | awk -F": " '{print $2}')
    master_log_file=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Master_Log_File" | awk -F": " '{print $2}')
    read_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
    relay_log_file=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Relay_Log_File" | awk -F": " '{print $2}')
    relay_log_pos=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Relay_Log_Pos" | awk -F": " '{print $2}')
    if [ "$master_host" = "" ] || [ "$master_log_file" = "" ] || [ "$master_log_file" = "0" ];then
        #DBHA设置的binlog不可信任，以mysql保存的为准，只有在host为空时才下发全新配置，其他情况做局部字段的更新
        mysql -u$mysql_user -p$mysql_password -hlocalhost -e "change master to master_host='$master_ip', master_user='$sync_user', master_password='$sync_password', MASTER_CONNECT_RETRY=5,master_port=3306, master_log_file='$master_file', master_log_pos=$master_pos,MASTER_SSL_VERIFY_SERVER_CERT=1;flush privileges;" 2>/dev/null
        logInfo "start with empty, set master info, master_ip:$master_ip, sync_user:$sync_user, master_file_name:$master_file, master_log_pos:$master_pos."
    elif [ "$master_host" != "$master_ip" ] || [ "$master_user" = "" ] || [ "$master_user" != "$sync_user" ];then
        # 先停掉slave后，再取出当前的master binlog信息，与IP一起设置，避免取出后，实际的Master_Log_File和Read_Master_Log_Pos已经变化了
        mysql -u$mysql_user -p$mysql_password -hlocalhost -e "change master to master_host='$master_ip', master_user='$sync_user', master_password='$sync_password', MASTER_CONNECT_RETRY=5,master_port=3306, master_log_file='$master_log_file', master_log_pos=$read_master_log_pos, relay_log_file='$relay_log_file', relay_log_pos=$relay_log_pos,MASTER_SSL_VERIFY_SERVER_CERT=1;flush privileges;" 2>/dev/null
        logInfo "start with update, set master info, master_ip:$master_ip, sync_user:$sync_user, master_file_name:$master_log_file, master_log_pos:$read_master_log_pos, relay_log_file:$relay_log_file, relay_log_pos:$relay_log_pos."
    else
        #备机非空白模式的时候，直接启动备机
        logInfo "start with self, master_ip:$master_ip, sync_user:$sync_user, file:$master_log_file, pos:$read_master_log_pos, relay_log_file:$relay_log_file, relay_log_pos:$relay_log_pos."
    fi

    logInfo "start slave"
    mysql -u$mysql_user -p$mysql_password -hlocalhost -e "start slave;" 2>/dev/null
    mysql -u$mysql_user -p$mysql_password -hlocalhost -e "set global read_only=1;" 2>/dev/null
    
    #去除恢复标记
	sed -i "s/\"slave_is_recovering\" : 1/\"slave_is_recovering\" : 0/g" $CFG_FILE
	/etc/init.d/DB-Manager start 1>/dev/null 2>&1

    io_status=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    slave_status=$(mysql -u$mysql_user -p$mysql_password -hlocalhost -e "show slave status\G" 2>/dev/null | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    if [[ "$io_status" != "Yes" ]] || [[ "$slave_status" != "Yes" ]];then
        slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" 2>/dev/null)
        logInfo "checkSlaveRunning error:$slave_status"
        return 1
    else
		logInfo "end to start slave success, master ip:$master_ip."
        return 0
    fi
}

function recoverWithXtrabackup()
{	
	file_name=$1
    if [ ! -f "$file_name" ];then
        logInfo "recoverWithXtrabackup: recover file $file_name not exist!"
        return 1
    fi
	
	#从配置文件中读取当前备份路径
	local backup_dir=`cat ${CFG_FILE} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
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
		logInfo "recoverWithXtrabackup: No space left on device,use $used_space , free $avail_space!"
		return 1
	fi
	
	getUserCfg
	logInfo "recoverWithXtrabackup:cleanup $SLAVE_BACKUP_TMP_DIR."
	rm -rf $SLAVE_BACKUP_TMP_DIR
	mkdir -p $SLAVE_BACKUP_TMP_DIR	
	tar -ixvf $file_name -C $SLAVE_BACKUP_TMP_DIR > $RECOVER_LOG 2>&1
	local ret=$?
	if [ $ret != 0 ];then
		logInfo "recoverWithXtrabackup[$ret]:restore_prepare_env tar -ixvf $file_name -C $SLAVE_BACKUP_TMP_DIR failed"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	#停DBHA
	/etc/init.d/DB-Manager stop
	
	#删除数据库目录、进行恢复
	logInfo "recoverWithXtrabackup:cleanup $SLAVE_MYSQL_DATA_DIR."
	rm -rf $SLAVE_MYSQL_DATA_DIR
	mkdir -p $SLAVE_MYSQL_DATA_DIR
	XTRABACKUP_HOME=/cloud/dahua/DataBaseServer/xtrabackup
	PATH=$PATH:$XTRABACKUP_HOME/bin
	export PATH XTRABACKUP_HOME
	source /etc/profile
	
	logInfo "recoverWithXtrabackup:restore_mysql begin"
	innobackupex --user=$mysql_user --password=$mysql_password --apply-log $SLAVE_BACKUP_TMP_DIR >> $RECOVER_LOG 2>&1
	ret=$?
	if [ $ret != 0 ];then
		logInfo "recoverWithXtrabackup[$ret]:innobackupex prepare failed:$SLAVE_BACKUP_TMP_DIR"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	innobackupex --defaults-file=/etc/my.cnf --user=$mysql_user --password=$mysql_password --copy-back $SLAVE_BACKUP_TMP_DIR >> $RECOVER_LOG 2>&1
	ret=$?
	if [ $ret != 0 ];then
		logInfo "recoverWithXtrabackup[$ret]:innobackupex copy-back failed:$SLAVE_BACKUP_TMP_DIR"
		rm -rf $SLAVE_BACKUP_TMP_DIR
		return 1
	fi
	
	#清除recover文件
	rm -rf $SLAVE_BACKUP_TMP_DIR
	chown -hR mysql:mysql $SLAVE_MYSQL_DATA_DIR
	
	#数据库设置成备机
	master_ip=$(cat ${DBHA_CFG_FILE} | grep \"peer_business_IP\" | awk -F '"' '{print $4}')
	xtrabackup_info=$SLAVE_MYSQL_DATA_DIR/xtrabackup_info
	master_file=$(cat $xtrabackup_info | grep binlog_pos | awk -F "filename " '{print $2}' | awk -F "," '{print $1}' | tr -d "\'")
	master_pos=$(cat $xtrabackup_info | grep binlog_pos | awk -F "position " '{print $2}' | tr -d "\'")
	startSlave $master_ip $master_file $master_pos

    return 0
}

function backupWithScp()
{
	master_host=$1
	slave_host=$2
	#开始操作之前需要保证没有数据写入
	
	master_mysqld_status=$(/usr/bin/ssh $master_host "/etc/init.d/mysqld status" | grep -c running)
	if [ $master_mysqld_status -eq 0 ];then
		logInfo "$master_host mysqld is not running,stop scp!"
		return 1
	fi
	
	/usr/bin/ssh $master_host "iptables -A INPUT -p tcp --dport 3306 -j DROP"
	if [ $? -ne 0 ];then
		logInfo "setup iptables in $master_host failed!"
		return 1
	fi
	
	getUserCfg
	
	#登陆主机获取binlog位置,走/tmp/mysql.socket文件，因为3306端口已经被封
	binlog_file=$(/usr/bin/ssh $master_host "mysql -u$mysql_user -p$mysql_password -hlocalhost -e 'show master status\G'" | grep File | awk -F ":" '{print $2}')
	binlog_pos=$(/usr/bin/ssh $master_host "mysql -u$mysql_user -p$mysql_password -hlocalhost -e 'show master status\G'" | grep Position | awk -F ":" '{print $2}')
	if [ -z "$binlog_file" ];then
        /usr/bin/ssh root@$master_host "iptables -D INPUT -p tcp --dport 3306 -j DROP"
		logInfo "$master_host mysqld not running!"
		return 1
	fi
	logInfo "begin scp: $master_host $binlog_file $binlog_pos!"
	
	/usr/bin/ssh $slave_host "/etc/init.d/DB-Manager stop"
	if [ $? -ne 0 ];then
		logInfo "stop $slave_host DB-Manager failed!"
        /usr/bin/ssh root@$master_host "iptables -D INPUT -p tcp --dport 3306 -j DROP"
		return 1
	fi
	
	/usr/bin/ssh $master_host "/etc/init.d/DB-Manager stop"
	if [ $? -ne 0 ];then
		logInfo "stop $master_host DB-Manager failed!"
        /usr/bin/ssh root@$master_host "iptables -D INPUT -p tcp --dport 3306 -j DROP"
		return 1
	fi
	
    # 12.拷贝数据库数据
    mysql_data_dir=$(/usr/bin/ssh root@$slave_host "grep datadir $SERVER_PATH/Config/my.cnf" | awk -F "=" '{print $2}' | sed 's/ //g')
	/usr/bin/ssh root@$slave_host "rm -rf $mysql_data_dir/*"
	if [ $? -ne 0 ];then
		logInfo "rm -rf  $slave_host $mysql_data_dir failed!"
        /usr/bin/ssh root@$master_host "iptables -D INPUT -p tcp --dport 3306 -j DROP"
		return 1
	fi
	
    /usr/bin/scp -r root@$master_host:$mysql_data_dir/* root@$slave_host:$mysql_data_dir/
    if [ $? -ne 0 ];then
		logInfo "scp  $master_host to $slave_host failed."
        /usr/bin/ssh root@$master_host "iptables -D INPUT -p tcp --dport 3306 -j DROP"
		return 1
	fi
	
	#清除3306端口上的防火墙
	/usr/bin/ssh root@$master_host "iptables -D INPUT -p tcp --dport 3306 -j DROP"
	if [ $? -ne 0 ];then
		logInfo "cleanup iptables in $master_host failed!"
		return 1
	fi
	
	/usr/bin/ssh root@$master_host "/etc/init.d/DB-Manager start 1>/dev/null 2>&1"
	/usr/bin/ssh root@$slave_host "chown -R mysql:mysql $mysql_data_dir"
	/usr/bin/ssh root@$slave_host "rm -rf $mysql_data_dir/auto.cnf"
	/usr/bin/ssh root@$slave_host "/bin/bash $shell_path/$self_shell start_slave $binlog_file $binlog_pos"
    return 0
}

function startDBHASlave()
{
	master_file=$1
	master_pos=$2
	
	#数据库设置成备机
	master_ip=$(cat ${DBHA_CFG_FILE} | grep \"peer_business_IP\" | awk -F '"' '{print $4}')
    logInfo "start DBHA slave:$master_ip $master_file $master_pos!"
	startSlave $master_ip $master_file $master_pos
}

function mysqldumpDataBase()
{
	master_host=$1
	slave_host=$2
	database=$3
	file_dir=$4
	
	getUserCfg
	logInfo "mysqldump database:$master_host, $slave_host, $database, $file_dir"
    rm ${file_dir}/${database}_sql.gz
	mysqldump -u$mysql_user -p$mysql_password -h$master_host --databases $database --events --routines --triggers --add-drop-database --log-error=$file_dir/backup.log |gzip > ${file_dir}/${database}_sql.gz
	#获取备份的错误日志
	errorMessage=$(cat $file_dir/backup.log | grep 'Error')
    errorNum=$(cat $file_dir/backup.log | grep 'Error'|wc -l)
    
	#如果日志中有“Error”日志，则认为备份失败，备份文件是不完整的，需要删除
	if [ $errorNum != 0 ];then
        logInfo "backup_mysql failed! mysqldump return:"$errorMessage
        rm ${file_dir}/${database}_sql.gz
        return 1
    fi

	logInfo "mysqldump database recover:$master_host, $slave_host, $database, ${file_dir}/${database}_sql.gz"
	gunzip < ${file_dir}/${database}_sql.gz | mysql -u$mysql_user -p$mysql_password -h$slave_host >> $LOG_FILE 2>&1    
	if [ $? != 0 ];then
		logInfo "mysql recovery return failed!"
		return 1
	fi 
	
	rm ${file_dir}/${database}_sql.gz
    return 0
}

function mysqldumpTable()
{
	master_host=$1
	slave_host=$2
	database=$3
	table=$4
	file_dir=$5
	
	getUserCfg
	logInfo "mysqldump database:$master_host, $slave_host, $database, $table, $file_dir"
    rm ${file_dir}/${table}_sql.gz
	mysqldump -u$mysql_user -p$mysql_password -h$master_host --databases $database --tables $table --log-error=$file_dir/backup.log |gzip > ${file_dir}/${table}_sql.gz
	#获取备份的错误日志
	errorMessage=$(cat $file_dir/backup.log | grep 'Error')
    errorNum=$(cat $file_dir/backup.log | grep 'Error'|wc -l)
    
	#如果日志中有“Error”日志，则认为备份失败，备份文件是不完整的，需要删除
	if [ $errorNum != 0 ];then
        logInfo "backup_mysql failed! mysqldump return:"$errorMessage
        rm ${file_dir}/${table}_sql.gz
        return 1
    fi

	logInfo "mysqldump database recover:$master_host, $slave_host, $database, ${file_dir}/${table}_sql.gz"
	gunzip < ${file_dir}/${table}_sql.gz | mysql -u$mysql_user -p$mysql_password -h$slave_host $database >> $LOG_FILE 2>&1    
	if [ $? != 0 ];then
		logInfo "mysql recovery return failed!"
		return 1
	fi 
    
    rm ${file_dir}/${table}_sql.gz
    return 0
}

#整机备份文件，$1:备份文件的路径
function mysqldumpAllDataBase()
{
    file_dir=$1
    getUserCfg
    logInfo "start to back up,file_dir:$file_dir";	
    #清理历史备份日志
    rm -rf $file_dir/backup.log
    mysqldump -u$mysql_user -p$mysql_password --all-databases --single-transaction --events --routines --triggers --add-drop-database --log-error=$file_dir/backup.log |gzip > ${file_dir}/all_database_tmp

	#获取备份的错误日志
	errorMessage=$(cat $file_dir/backup.log|grep 'Error')
    errorNum=$(cat $file_dir/backup.log|grep 'Error'|wc -l)
    
	#如果日志中有“Error”日志，则认为备份失败，备份文件是不完整的，需要删除
	if [ $errorNum != 0 ];then
             logInfo "backup_mysql failed! mysqldump return:"$errorMessage
             rm ${file_dir}/all_database_tmp
             return 1
        fi
   
    logInfo "end to back up ,file_dir:$file_dir,uuid:$uuid,server_id:$server_id,mode:$mode,auto:$auto."
    new_file_name=${file_dir}/$(date "+%Y_%m_%d_%H%M%S").gz
    mv ${file_dir}/all_database_tmp $new_file_name
    logInfo "end to back up, file_dir:$file_dir"
    return 0
}

function onlineRecovery()
{    
    file_name=$1
    
    getUserCfg
    logInfo "start to recovery."
	#修改成使用mysqldump导出的drop数据库语句
	#purgeDatabases
	#恢复数据
    gunzip < $file_name | mysql -u$mysql_user -p$mysql_password >> $LOG_FILE 2>&1
	if [ $? != 0 ];then
		logInfo "mysql recovery return failed!"
		return 1
	fi
	
    if [ -f "$file_name" ];then
        content=$(cat $$file_name | head -n 1)
        if [ -z "$content" ];then
            logInfo "content empty,checked it!"
            return 1
        fi
    fi
    
    return 0
}

if [ $# -lt 1 ];then
	echo "usage:sh dataRestore.sh [option]"
	echo "sh dataRestore.sh online_xtrabackup_backup_recover masterIP slaveIP backupPath"
	echo "sh dataRestore.sh offline_scp_backup_recover masterIP slaveIP"
	echo "sh dataRestore.sh start_slave binlog_name binlog_pos"
	echo "sh dataRestore.sh online_mysqldump_database src_host dst_host database remote_backup_dir"
	echo "sh dataRestore.sh online_mysqldump_table src_host dst_host database table remote_backup_dir"
    echo "sh dataRestore.sh online_mysqldump_all_database remote_backup_dir"
    echo "sh dataRestore.sh online_mysqldump_recover_all_database remote_backup_dir"
	exit 1
fi

option=$1

case $option in
	"online_xtrabackup_backup")
        master_host=$2
        slave_host=$3
        remote_backup_dir=$4
        is_self=$(ip addr show | grep -c $master_host)
        if [ $is_self -eq 1 ];then
            backupWithXtrabackup $slave_host $remote_backup_dir
            ret=$?
        else
            logInfo "$master_host is not self!"
            /usr/bin/ssh $master_host "/bin/bash $shell_path/$self_shell online_xtrabackup_backup $master_host $slave_host $remote_backup_dir"
            ret=0
        fi
	;;
	"online_xtrabackup_recover")
        backup_file=$2
        recoverWithXtrabackup $backup_file
        ret=$?
	;;
	"online_xtrabackup_backup_recover")
        master_host=$2
        slave_host=$3
        remote_backup_dir=$4
        is_self=$(ip addr show | grep -c $master_host)
        if [ $is_self -eq 1 ];then
            backupWithXtrabackup $slave_host $remote_backup_dir
        else
            logInfo "$master_host is not self!"
            /usr/bin/ssh $master_host "/bin/bash $shell_path/$self_shell online_xtrabackup_backup $master_host $slave_host $remote_backup_dir"
        fi
        
        if [ $? -ne 0 ];then
            logInfo "$master_host xtrabackup backup failed!"
            echo "$master_host xtrabackup backup failed!"
            exit 1
        fi
        
        remote_file="$remote_backup_dir/$xtrabackup_backup_name"
        /usr/bin/ssh $slave_host "/bin/bash $shell_path/$self_shell online_xtrabackup_recover $remote_file"
        ret=0
	;;
	"offline_scp_backup_recover")
        master_host=$2
        slave_host=$3
        backupWithScp $master_host $slave_host
        ret=$?
	;;
	"start_slave")
        file_name=$2
        file_pos=$3
        startDBHASlave $file_name $file_pos
        ret=$?
	;;
	"online_mysqldump_database")
        src_host=$2
        dst_host=$3
        database=$4
        remote_backup_dir=$5
        mysqldumpDataBase $src_host $dst_host $database $remote_backup_dir
        ret=$?
	;;
	"online_mysqldump_table")
        src_host=$2
        dst_host=$3
        database=$4
        table=$5
        remote_backup_dir=$6
        mysqldumpTable $src_host $dst_host $database $table $remote_backup_dir
        ret=$?
	;;
    "online_mysqldump_all_database")
        remote_backup_dir=$2
        mysqldumpAllDataBase $src_host $remote_backup_dir
        ret=$?
	;;
    "online_mysqldump_recover_all_database")
        remote_backup_dir=$2
        onlineRecovery $remote_backup_dir
        ret=$?
	;;
	*)
        echo "usage:sh dataRestore.sh [option]"
        echo "sh dataRestore.sh online_xtrabackup_backup_recover masterIP slaveIP backupPath"
        echo "sh dataRestore.sh offline_scp_backup_recover masterIP slaveIP"
        echo "sh dataRestore.sh start_slave binlog_name binlog_pos"
        echo "sh dataRestore.sh online_mysqldump_database src_host dst_host database remote_backup_dir"
        echo "sh dataRestore.sh online_mysqldump_table src_host dst_host database table remote_backup_dir"
        echo "sh dataRestore.sh online_mysqldump_all_database remote_backup_dir"
        echo "sh dataRestore.sh online_mysqldump_recover_all_database remote_backup_dir"
        exit 1
	;;
esac

if [ $ret -eq 0 ];then
    echo "$option success!"
else
    echo "$option failed!"
fi
exit $ret
	

