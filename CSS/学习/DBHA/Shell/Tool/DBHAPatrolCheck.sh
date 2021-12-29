#[!/bin/bash]
# 用于市场对DBHA服务进行巡检
#########################
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(cd $(dirname "$cur_dir");pwd)
source $parent_dir/Common/CommonFunc.sh
getUserCfg

LOG_FILE="$cur_dir/dbha_patrol_$(date +"%F_%T").log"
SHOW_CONSOLE='Y'
TAG=DBHAPatrolCheck

DBHA_INSTALL_PATH=/cloud/dahua/DataBaseServer/DBHA
DBHA_CONF=$DBHA_INSTALL_PATH/Config/DBHA.conf
DBHA_STATE=$DBHA_INSTALL_PATH/Config/HAState.stat
DBHA_ELECT_LOG=$DBHA_INSTALL_PATH/Log/DBHA_election_log_current.log
MYSQL_CNF=/etc/my.cnf 

#参数1 模块名
#参数2..n message
function logInfo()
{
    echo -e "$(date +"%F %T")|[$TAG] $*" >> $LOG_FILE
    [ $SHOW_CONSOLE == Y ] && echo "$(date +"%F %T")|[$TAG] $*"
	return 0
}

function check_value()
{
    check_param=$1
    real_value=$2
    check_value=$3
    if [ "$real_value" == "$check_value" ];then
        logInfo "$check_param[check pass]:$real_value "
    else
        logInfo "$check_param[check failed]:$real_value "
    fi
    return 0
}

function check_value_opposite()
{
    check_param=$1
    real_value=$2
    check_value=$3
    if [ "$real_value" != "$check_value" ];then
        logInfo "$check_param[check pass]:$real_value "
    else
        logInfo "$check_param[check failed]:$real_value "
    fi
    return 0
}

function check_mysql()
{
    #获取系统环境是CentOS6还是CentOS7的环境
    OS_VERSION=$(rpm -q centos-release | cut -d- -f3)
    logInfo "Centos Version:$OS_VERSION"
    
    #检查enforce
    EN_FORCE=$(getenforce)
    check_value_opposite "Centos Enforce" $EN_FORCE "Enforcing"
    
    #检查数据库版本信息
    mysql_rpm=$(rpm -qa | grep mysql-community-server)
    if [ -z "$mysql_rpm" ];then
        logInfo "MySQL Server not install,check it!"
        #rpm -qa | grep mariadb
        #libaio
        return 1
    fi
    mysql_version=$(echo $mysql_rpm | cut -d- -f4)
    logInfo "MySQL Server Version:$mysql_version"
    
    #检查MySQL软连接是否正常
    mysql_lib_link=$(ls -l /usr/lib64 | grep "libmysqlclient.so.18" | wc -l)
    if [ $mysql_lib_link -gt 0 ];then
        logInfo "MySQL libmysqlclient.so Link[check success]:"
        logInfo "$(ls -l /usr/lib64 | grep libmysqlclient | awk -F " " '{print $(NF-2),$(NF-1),$NF}')"
    else
        logInfo "MySQL libmysqlclient.so Link[check failed]:"
        logInfo "$(ls -l /usr/lib64 | grep libmysqlclient | awk -F " " '{print $(NF-2),$(NF-1),$NF}')"
    fi
    
    #检查数据库数据目录
    if [ ! -f "$MYSQL_CNF" ];then
        logInfo "$MYSQL_CNF missing,check it!"
        return 1
    fi
    
    if [ ! -s $MYSQL_CNF ];then
        logInfo "$MYSQL_CNF is empty,check it!"
        return 1
    fi
    
    #检查my.cnf是否存在语法错误
    my_cnf_error=$(cat $MYSQL_CNF | grep -v "#" | grep "=" | sed -n "/=[ ]*$/p") 
    if [ ! -z $my_cnf_error ];then
        logInfo "$MYSQL_CNF has syntax errors,check it!"
    else
        logInfo "$MYSQL_CNF check success!"
    fi
    
    MYSQL_DATA_DIR=$(cat $MYSQL_CNF | grep datadir | awk -F "=" '{print $2}' | sed 's/ //g')
    if [ ! -d "$MYSQL_DATA_DIR" ];then
        logInfo "$MYSQL_DATA_DIR[check failed]: not exist!"
        return 1
    fi
    
   #检查目录权限
   level_1_dir=$(echo $MYSQL_DATA_DIR | awk -F "/" '{print $2}')
   level_2_dir=$(echo $MYSQL_DATA_DIR | awk -F "/" '{print $3}')
   level_2_dir_mod=$(ls -l "/"$level_1_dir | grep  $level_2_dir | awk -F " " '{print $1}')
   logInfo "/$level_1_dir/$level_2_dir file mode is $level_2_dir_mod shoud be drwxr-xr-x,check it!"
   data_mod=$(ls -l $MYSQL_DATA_DIR | grep -v mysql | grep -v total | wc -l)
   if [ $data_mod -gt 0 ];then
        logInfo "$MYSQL_DATA_DIR not all file belong to mysql:mysql,check it!"
   fi
   
   #检查数据库目录完整性，是否为已经使用过的还是新安装的
   base_database=$(du -h $MYSQL_DATA_DIR --max-depth=1 | awk -F " " '{print $2}' | grep "sys$")
   has_databases=$(du -h $MYSQL_DATA_DIR --max-depth=1 | awk -F " " '{print $2}' | grep -v "mysql$" | grep -v "sys$" | grep -v "performance_schema$" | grep -v "/$" | awk -F "/" '{print $NF}')
   if [ -z "$base_database" ];then
        logInfo "$MYSQL_DATA_DIR database sys not exist,check it!"
   else
        logInfo "$MYSQL_DATA_DIR has database:\"$has_databases\""
   fi
   
   #检查my.cnf和数据目录是否匹配
   undo_log_count=$(cat $MYSQL_CNF | grep innodb_undo_tablespaces | awk -F "=" '{print $2}' | sed 's/ //g')
   if [ -z "$undo_log_count" ];then
        undo_log_count=0
   fi

   real_undo_log_count=$(ls -l $MYSQL_DATA_DIR | grep undo | wc -l)
   if [ $undo_log_count -ne $real_undo_log_count ];then
        logInfo "MySQL Version match check failed,need $undo_log_count undo log but real has $real_undo_log_count!"
   fi
   
   #检查磁盘空间
   root_space_ratio=$(df -h / | tail -n 1 | awk -F " " '{print $(NF-1)}' | awk -F "%" '{print $1}')
   if [ $root_space_ratio -eq 100 ];then
        logInfo "the root space is full,check it!"
   else
        logInfo "the root space has used $root_space_ratio%"
   fi
   
   data_space_ratio=$(df -h $MYSQL_DATA_DIR | tail -n 1 | awk -F " " '{print $(NF-1)}' | awk -F "%" '{print $1}')
   if [ $data_space_ratio -eq 100 ];then
        logInfo "the $MYSQL_DATA_DIR space is full,check it!"
   else
        logInfo "the $MYSQL_DATA_DIR space has used $data_space_ratio%"
   fi
   
   mysql_pid=$(pidof mysqld)
    if [ -z "$mysql_pid" ];then
        logInfo "mysqld not running,check it!"
    else
        mysqld_runtime=$(ps -p $mysql_pid -o pid,start_time,etime,comm | awk -F " " '{print $3}' | tail -n 1)
        logInfo "mysqld runtime:$mysqld_runtime"
    fi
    
    #检查mysql登陆是否成功
	mysql_running=$(mysql -u$mysql_user -p$mysql_password -e "show databases" 2>/dev/null | grep sys )
    if [ -z "$mysql_running" ];then
        logInfo "mysql login failed,check it!"
        return 1
    else
        logInfo "mysql can login success!"
    fi
    
    #统计活跃连接数
    connection_count=$(mysql -u$mysql_user -p$mysql_password -e "select id from information_schema.processlist;" 2>/dev/null | wc -l)
    connection_active=$(mysql -u$mysql_user -p$mysql_password -e "select id from information_schema.processlist where command !="Sleep";" 2>/dev/null | wc -l)
    if [ $connection_count -eq 0 ];then
        logInfo "mysql has zero connection,check it!"
    else
        logInfo "mysql has $connection_count connection,active connection $connection_active!"
    fi
    
    #QPS统计
    mysql_queries=$(mysql -u$mysql_user -p$mysql_password -e "show status like 'Queries';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    sleep 1
    mysql_incr_queries=$(mysql -u$mysql_user -p$mysql_password -e "show status like 'Queries';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    ((QPS=$mysql_incr_queries - $mysql_queries))
    
    #TPS统计
    mysql_com_commit=$(mysql -u$mysql_user -p$mysql_password -e "show status like 'Com_commit';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    mysql_com_rollback=$(mysql -u$mysql_user -p$mysql_password -e "show status like 'Com_rollback';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    sleep 1
    mysql_incr_com_commit=$(mysql -u$mysql_user -p$mysql_password -e "show status like 'Com_commit';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    mysql_incr_com_rollback=$(mysql -u$mysql_user -p$mysql_password -e "show status like 'Com_rollback';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    ((TPS=$mysql_incr_com_rollback + $mysql_incr_com_commit - $mysql_com_rollback - $mysql_com_commit))
    logInfo "mysql QPS:$QPS, TPS:$TPS"
    
    #最大表展示
    big_table=$(mysql -u$mysql_user -p$mysql_password -e "SELECT table_schema as 'DB',table_name as 'TABLE',CONCAT(ROUND(( data_length + index_length ) / ( 1024 * 1024 * 1024 ), 2), 'GB') 'TOTAL' FROM information_schema.tables ORDER BY data_length + index_length DESC  limit 1;" 2>/dev/null | tail -n 1)
    logInfo "mysql biggest table:$big_table"
    
    #数据库错误日志
    log_error_name=$(mysql -u$mysql_user -p$mysql_password -e "show global variables like 'log_error';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    mysql_log_errors=$(cat $MYSQL_DATA_DIR/$log_error_name | grep ERROR)
    if [ ! -z "$mysql_log_errors" ];then
        logInfo "mysql log $MYSQL_DATA_DIR/$log_error_name has errors"
    else
        logInfo "mysql log $MYSQL_DATA_DIR/$log_error_name has no error!"
    fi
    
    #慢查询日志slow_query_log_file，最近一周是否存在慢查询日志
    today=$(date +"%FT%T")
    seven_days_ago=$(date -d '-7 day' +"%FT%T")
    slow_log_name=$(mysql -u$mysql_user -p$mysql_password -e "show global variables like 'slow_query_log_file';" 2>/dev/null | tail -n 1 | awk -F " " '{print $2}')
    slow_log=`sed "/${seven_days_ago}/,/${today}/"p $slow_log_name`
    if [ ! -z "$slow_log" ];then
        logInfo "has slow log in this week,check it!"
        #logInfo "$slow_log"
    else
        logInfo "has no slow log this week!"
    fi
   return 0
}

function check_dbha()
{
	#是否安装DBHA
    if [ ! -d "$DBHA_INSTALL_PATH" ];then
        logInfo "DBHA service not install,check it!"
        return 1
    fi
    
    #获取版本号
    dbha_version=$($DBHA_INSTALL_PATH/Bin/DBHAServer64 -v | grep version | awk -F ":" '{print $2}' | sed 's/ //g')
    dbha_inner_version=$($DBHA_INSTALL_PATH/Bin/DBHAServer64 -v | grep "svn revision" | awk -F ":" '{print $2}' | sed 's/ //g')
    logInfo "DBHA version:$dbha_version_$dbha_inner_version"
    
    #检查DBHA和mysql运行时长
    dbha_pid=$(pidof DBHAServer64)
    if [ -z "$dbha_pid" ];then
        logInfo "DBHAServer64 not running,check it!"
    else
        dbha_runtime=$(ps -p $dbha_pid -o pid,start_time,etime,comm | awk -F " " '{print $3}' | tail -n 1)
        logInfo "DBHAServer64 runtime:$dbha_runtime"
    fi
    
    #获取是否有部署
    local_business_ip=$(/etc/init.d/DB-Manager detail | grep localIP | awk -F ":" '{print $2}')
    peer_business_ip=$(cat $DBHA_CONF | grep peer_business_IP  | awk -F ":" '{print $2}' |awk -F "\"" '{print $2}')
    local_vip=$(/etc/init.d/DB-Manager detail | grep virtualIP | awk -F ":" '{print $2}')
    cluster_id=$(/etc/init.d/DB-Manager detail | grep clusterID | awk -F ":" '{print $2}')
    local_vipNetmask=$(cat $DBHA_CONF | grep DBHA_virtual_IP_netmask | awk -F ":" '{print $2}' |awk -F "\"" '{print $2}')
    if [ -z "$local_business_ip" ] || [ -z "$local_vip" ] || [ -z "$cluster_id" ] || [ -z "$local_vipNetmask" ];then
        logInfo "DBHA.conf has empty configuration or not deploy,check it!"
        return 1
    fi
    
    #检查备份文件是否正常
    mysql_backup_file_path=$(cat $DBHA_CONF  | grep mysql_backup_path | awk -F ":" '{print $2}' | awk -F "\"" '{print $2}' | sed 's/\\//g')
    if [ ! -d "$mysql_backup_file_path" ];then
        logInfo "$local_business_ip backup:$mysql_backup_file_path not exist!"
    else
        backup_info=$(ls -l $mysql_backup_file_path | grep gz)
        logInfo "$local_business_ip backup:$backup_info"
    fi
    
    if [ -z "$peer_business_ip" ];then
        logInfo "$local_business_ip run in single mode!"
        return 0
    fi

    #检查主从状态
    local_dbha_state=$(/etc/init.d/DB-Manager detail | grep SQLMode | awk -F ":" '{print $2}')
    peer_dbha_state=$(ssh -o LogLevel=QUIET root@$peer_business_ip /etc/init.d/DB-Manager detail | grep SQLMode | awk -F ":" '{print $2}' )
    ((result_state=$local_dbha_state ^ $peer_dbha_state))
    if [ -z "$peer_dbha_state" ];then
	logInfo "dbha $peer_business_ip not exist,check it!"
    elif [ $result_state -eq 0 ];then
        logInfo "dbha master-slave state check failed:$local_business_ip[$local_dbha_state],$peer_business_ip[$peer_dbha_state]"
    else
        logInfo "dbha master-slave state:$local_business_ip[$local_dbha_state],$peer_business_ip[$peer_dbha_state](1,master;0,slave;-1,init)"
        if [ $local_dbha_state -eq 1 ];then
            master_ip=$local_business_ip
            slave_ip=$peer_business_ip
        else
            master_ip=$peer_business_ip
            slave_ip=$local_business_ip
        fi
		
		#检测是否和云存储双写存在冲突
		dbha_cluster_id=$(/etc/init.d/DB-Manager detail | grep clusterID | awk -F ":" '{print $2}')
		has_conflict=$(mysql -u$mysql_user -p$mysql_password catalogdb -e "show events" | grep EVT_autoClean | wc -l)
		if [ -z "$dbha_cluster_id" ] && [ $has_conflict -gt 0 ];then
			logInfo "dbha has conflict with catalogdb,check it!"
		fi
        
        #检查主从同步状态
        slave_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status" 2>/dev/null)
        io_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
        if [ "$io_status" = "Yes" ];then
            slave_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
            if [[ "$slave_status" != "Yes" ]];then
                logInfo "dbha replicate failed io thread or sql thread has error,check it!"
                return 1
            fi
        elif [ "$io_status" = "Connecting" ];then
            retry_cnt=$( mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep Connect_Retry | awk -F ": " '{print $2}')
            has_retry_cnt=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep Last_IO_Error | grep retries | awk -F "retries: " '{print $2}')
            if [ $has_retry_cnt -gt $retry_cnt ];then
                logInfo "dbha replicate failed io thread connect failed,check it!"
                return 1
            fi
        else
            logInfo "dbha replicate failed io status check failed,check it!"
            return 1
        fi
        
        slave_master_log_file=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep -w "Master_Log_File" | awk -F": " '{print $2}')
        relay_master_log_file=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep -w "Relay_Master_Log_File" | awk -F": " '{print $2}')
        slave_read_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep -w "Read_Master_Log_Pos" | awk -F": " '{print $2}')
        exec_master_log_pos=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" 2>/dev/null | grep -w "Exec_Master_Log_Pos" | awk -F": " '{print $2}')
        if [ "$slave_master_log_file" != "$relay_master_log_file" ] || [ "$slave_read_master_log_pos" != "$exec_master_log_pos" ];then
            logInfo  "[replicate check failed]slave_master_log_file:$slave_master_log_file, relay_log_file:$relay_master_log_file, slave_read_master_log_pos: $slave_read_master_log_pos, exec_master_log_pos:$exec_master_log_pos"
        fi
        
        master_log_file=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show master status\G" 2>/dev/null | grep -w "File" | awk -F": " '{print $2}')
        master_log_pos=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show master status\G" 2>/dev/null | grep -w "Position" | awk -F": " '{print $2}')
        if [ "$master_log_file" != "$slave_master_log_file" ] || [ "$master_log_pos" != "$slave_read_master_log_pos" ];then
            logInfo  "[replicate check failed]slave_master_log_file:$slave_master_log_file, relay_log_file:$relay_master_log_file, slave_read_master_log_pos:$slave_read_master_log_pos, exec_master_log_pos:$exec_master_log_pos, master_log_file:$master_log_file, master_log_pos:$master_log_pos"
        else
            logInfo "replicate check success, master and slave replicate has no delay!"
        fi
        
        #主备切换次数
        dbha_switch_over=$(cat $DBHA_ELECT_LOG | grep "slave -> master")
        if [ ! -z "$dbha_switch_over" ];then
            logInfo "slave switch to master happened:"
            logInfo "$dbha_switch_over"
        else
            logInfo "slave switch to master not happened!"
        fi
        
        #检查DBHA和云存储双写是否冲突
        has_catalogdb=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show databases" 2>/dev/null | grep catalogdb | wc -l)
        if [ $has_catalogdb -eq 1 ];then
            has_event=$(mysql -u$mysql_user -p$mysql_password -h$master_ip catalogdb -e "show events" 2>/dev/null | grep EVT_autoClean | wc -l)
            if [ $has_event -eq 1 ];then
                logInfo "DBHA and CatalogServer has conflict,check it!"
            else 
                logInfo "DBHA and CatalogServer check success!"
            fi
        fi
    fi
 
    return 0
}

check_mysql
check_dbha

