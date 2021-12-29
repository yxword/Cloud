#!/bin/bash
#参数设置：single backup_path,uuid,server_id
#          cluster slave_ip, slave_heart_ip,backup_path,uuid,server_id
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/DBHAConfig.sh
source $DBHA_SHELL/Common/CommonFunc.sh

TAG=MysqlManualOperater
SHOW_CONSOLE=Y
MANUAL_LOG_FILE=$DBHA_LOG/MysqlManualOper.log

mysql_data_dir=$(cat /etc/my.cnf|grep "datadir"| awk -F "=" '{print $2}' | sed 's/[ ]//g')

# 数据库服务状态配置文件
DBHACONF_PATH=/data/mysql/Config/DBHA.conf
MYSQL_MONIT_SHELL=MysqlKeeper.sh

backup_script=MasterBackup.sh
backup_script_full_path=$parent_dir/Backup/$backup_script
#获取备份路径
backup_dir=`cat ${DBHACONF_PATH} | grep \"mysql_backup_path\" | awk -F '"' '{print $4}'`
backup_path=$(echo $backup_dir | sed 's/\\//g')
uuid=`cat /data/mysql/Config/HAConfig.conf |grep "uid"|awk -F "\"" '{print $4}'`
server_id=$(cat /etc/my.cnf|grep "server-id"| awk -F "=" '{print $2}' | sed 's/[ ]//g')


if [[ $# -lt 2 ]];then
    echo -e "USAGE:$0 [single | cluster ], master_ip, slave_ip"
    manualLogInfo "start Maunal Operate failed,$*."
    exit 1;
fi

mode=$1
if [ "$mode" == "cluster" ];then
    master_ip=$2
    slave_ip=$3
    if [ "$slave_ip" == "" ];then
        echo -e "USAGE:$0 cluster master_ip slave_ip"
        exit 1
    else
        echo -e "\e[31mYou will use the MySQL:$master_ip to recovery MySQL:$slave_ip,Are you sure?\e[m"
        read -p "yes or no:" result
        if [ "$result" != "yes" ];then
            echo "The input is wrong,exit!"
            exit 1
        fi
    fi
elif [ "$mode" == "single" ];then
    master_ip=$2
    if [ "$master_ip" == "" ];then
        echo -e "USAGE:$0 single master_ip"
        exit 1
    else
        echo -e "\e[31mYou will use the MySQL:$master_ip single without slave.Are you sure?\e[m"
        read -p "yes or no:" result
        if [ "$result" != "yes" ];then
            echo "The input is wrong,exit!"
            exit 1
        fi
    fi
else
    echo -e "USAGE:$0 [single | cluster ], master_ip, slave_ip"
    exit 1
fi

#mysql登录的用户名和密码
getUserCfg

#同步信息的用户名和密码
getSyncUserCfg

function manualLogInfo()
{
    echo -e "$(date +"%F %T")|[$TAG] $*" >> $MANUAL_LOG_FILE
    [ $SHOW_CONSOLE == Y ] && echo "$(date +"%F %T")|[$TAG] $*"
    return 0
}

function startMaster()
{
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e "set global read_only=0;"
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e "reset slave all;"
    manualLogInfo "reset slave all, become master, slave_ip:$slave_ip"

    #尝试删除旧的grant用户
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e  "use mysql;delete from user where User='$sync_user';flush privileges;"

    #赋予备机复制用户信息
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e  "grant replication slave,replication client on *.* to '$sync_user'@'$slave_ip' identified by '$sync_password';flush privileges;"

    #启动事件机制
    setEvent enable
    
    #获取主机当前自己的binlog文件名和位置信息
    master_log_file_name=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show master status\G" | grep -w "File" | awk -F": " '{print $2}')
    master_log_file_position=$(mysql -u$mysql_user -p$mysql_password -h$master_ip -e "show master status\G" | grep -w "Position" | awk -F": " '{print $2}')
    if [ "$master_log_file_name" == "" ];then
        manualLogInfo "get master binlog file name failed"
        return 1
    fi
    
    #全量备份
    backupFileCommon $backup_path $uuid $server_id master manual
    if [[ $? -ne 0 ]];then
        manualLogInfo "master backup failed"
        return 1
    fi
    return 0
}

#启动备机模式
function startSlave()
{
    manualLogInfo "begin to start slave, master ip:$master_ip, master_log_file=$master_log_file_name, master_log_pos=$master_log_file_position."
    #尝试去掉备份机制
    sed -i '/masterBackup/d' /var/spool/cron/root
    sed -i '/MasterBackup/d' /var/spool/cron/root

    #关闭事件机制
    setEvent disable

    #恢复数据
    startRecovery
    if [[ $? -ne 0 ]];then
        manualLogInfo "startRecovery slave failed ."
        return 1
    fi
    
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "reset slave all;"
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "change master to master_host='$master_ip', master_user='$sync_user', master_password='$sync_password',master_log_file='$master_log_file_name', master_log_pos=$master_log_file_position,
    MASTER_CONNECT_RETRY=5,master_port=3306,MASTER_SSL_VERIFY_SERVER_CERT=1;flush privileges;"
    manualLogInfo "start with empty, set master info, master_ip:$master_ip, sync_user:$sync_user, master_file_name:$master_log_file_name, master_log_pos:$master_log_file_position."
    manualLogInfo "start slave"
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "start slave;"
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "set global read_only=1;"

    checkSlaveRunning
    if [[ $? -ne 0 ]];then
        manualLogInfo "end to start slave failed, master ip:$master_ip."
        return 1
    fi

    manualLogInfo "end to start slave success, master ip:$master_ip."
    return 0
}

#恢复完成后，机器为只读模式
function startRecovery()
{
    backup_file_temp=`ls $backup_path/ | grep manual_master_${uuid}_${server_id}_$(date "+%Y_%m_%d_%H")| head -n 1`
    backup_file_name="$backup_path/$backup_file_temp"
    echo "backup_file_name:$backup_file_name"
    if [ -z "$backup_file_name" ];then
        manualLogInfo "backup file not exist."
        return 1
    fi
    
    #停止模式
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "reset slave;"
    #变成非只读模式
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "set global read_only=0;"
    #关闭binlog日志
    
    manualLogInfo "start to recovery."
    #修改成使用mysqldump导出的drop数据库语句
    #purgeDatabases
    #恢复数据,session内不产生binlog
    gunzip < $backup_file_name | mysql -u$mysql_user -p$mysql_password  -h$slave_ip --init-command='set sql_log_bin=OFF' >> $LOG_FILE 2>&1
    if [ $? -ne 0 ];then
        manualLogInfo "mysql recovery return failed!"
        #删除掉恢复文件
        rm -rf $backup_file_name
        return 1
    fi
    
    if [ -f "$backup_file_name" ];then
        content=$(gunzip < $backup_file_name | head -n 3)
        if [ -z "$content" ];then
            rm -rf $backup_file_name
            manualLogInfo "$backup_file_name content empty,checked it!"
            return 1
        fi
    fi
    
    #删除掉恢复文件
    rm -rf $backup_file_name

    manualLogInfo "recovery success or killed, backup_file_name:$backup_file_name."
    mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "set global read_only=1;"
    manualLogInfo "success to recovery, write state is no write no read."

    return 0
}

function checkSlaveRunning()
{
    io_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    slave_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    if [[ "$io_status" != "Yes" ]] || [[ "$slave_status" != "Yes" ]];then
        slave_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "show slave status\G")
        manualLogInfo "[checkSlaveRunning] error:$slave_status"
        repl_status=$(mysql -u$mysql_user -p$mysql_password -h$slave_ip -e "select * from performance_schema.replication_applier_status_by_worker\G")
        manualLogInfo "[checkSlaveRunning] error:$repl_status"
        return 1
    else
        return 0
    fi
}

function checkMysqlState()
{
    local mysql_host=$1
    local try_max_count=30
    local try_count=0
    for((try_count=0;try_count<try_max_count;try_count++))
    do
        sleep 1
        mysqladmin -u$mysql_user -p$mysql_password -h$mysql_host --connect-timeout=1 ping 2>&1 | grep -q "mysqld is alive"
        if [ $? -eq 0 ];then
            break;
        fi
    done
    
    if [ $try_count -eq $try_max_count ];then
        manualLogInfo  "MySQL:$mysql_host can not connectd.exit!"
        return 1
    fi
    
    return 0
}

function upVirtualIP()
{
    local DBHA_vif_name=`cat $DBHACONF_PATH |grep "DBHA_vif_name"|awk -F "\"" '{print $4}'`
    local DBHA_if_name=`cat $DBHACONF_PATH |grep "DBHA_if_name"|awk -F "\"" '{print $4}'`
    local DBHA_virtual_IP=`cat $DBHACONF_PATH |grep "DBHA_virtual_IP\""|awk -F "\"" '{print $4}'`
    local DBHA_virtual_IP_netmask=`cat $DBHACONF_PATH |grep "DBHA_virtual_IP_netmask"|awk -F "\"" '{print $4}'`
    local vif_name=`ip addr | grep "$DBHA_virtual_IP\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
    
    if [ -z "$vif_name" ];then
        ifconfig $DBHA_vif_name $DBHA_virtual_IP netmask $DBHA_virtual_IP_netmask up > /dev/null 2>&1
        if [ $? -eq 0 ];then
            manualLogInfo "Virtual IP:$DBHA_virtual_IP up success"
            if [ ! -z "$DBHA_if_name" ];then
                arping -b -U -c 2 -I $DBHA_if_name $DBHA_virtual_IP -w 10
            fi
        else 
            manualLogInfo "Virtual IP:$DBHA_virtual_IP up failed"
        fi
    fi
}

# 从配置文件中获取虚网卡名称，然后down掉
function downVirtualIP()
{
    local DBHA_vif_name=`cat $DBHACONF_PATH |grep "DBHA_vif_name"|awk -F "\"" '{print $4}'`
    local DBHA_virtual_IP=`cat $DBHACONF_PATH |grep "DBHA_virtual_IP\""|awk -F "\"" '{print $4}'`
    local vif_name=`ip addr | grep "$DBHA_virtual_IP\/" | awk -F "global" '{print $2}' | awk -F " " '{print $NF}' | tr -d " "`
    if [ ! -z "$vif_name" ];then
        ifconfig $DBHA_vif_name down > /dev/null 2>&1
    else
        ssh root@$slave_ip ifconfig $DBHA_vif_name down > /dev/null 2>&1
    fi
}


function startMasterSingle()
{
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e "set global read_only=0;"
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e "stop slave;"
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e "reset slave all;"

    #尝试删除旧的grant用户
    mysql -u$mysql_user -p$mysql_password -h$master_ip -e  "use mysql;delete from user where User='$sync_user';flush privileges;"
    
    #起虚IP
    upVirtualIP
    
    #启动事件机制
    setEvent enable
}

function startBackup()
{
    sed -i '/MasterBackup/d' /var/spool/cron/root
    chmod +x $backup_script_full_path
    script_tmp=`echo $backup_script_full_path | sed 's#\/#\\\/#g'`
    echo '0 1 * * * '$script_tmp' '$backup_path' '$uuid' '$server_id'' >>/var/spool/cron/root
    manualLogInfo "add cron task(master_backup)."
    
    local_crond_exist=`ps -ef | grep /usr/sbin/crond | grep start`
    if [ -z "$local_crond_exist" ];then
        logInfo "Master:$master_ip crond not start,begin start"
        /usr/sbin/crond start
        local_crond_exist=`ps -ef | grep /usr/sbin/crond | grep start`
        if [ ! -z "$local_crond_exist" ];then
            logInfo "Master:$master_ip crond start success"
        else 
            logInfo "Master:$master_ip crond start failed"
        fi
    fi
    
    peer_crond_exist=`ssh root@$slave_ip ps -ef | grep /usr/sbin/crond | grep start`
    if [ -z "$peer_crond_exist" ];then
        logInfo "slave:$slave_ip crond not start,begin start"
        ssh root@$slave_ip nohup /usr/sbin/crond start
        peer_crond_exist=`ssh root@$slave_ip ps -ef | grep /usr/sbin/crond | grep start`
        if [ ! -z "$peer_crond_exist" ];then
            logInfo "slave:$slave_ip crond start success"
        else 
            logInfo "slave:$slave_ip crond start failed"
        fi
    fi
}

function startRemoteMysqlKeep()
{
    # 检查守护脚本和主进程
    local keep_exist=`ssh root@$slave_ip ps -ef | grep /bin/sh | grep $MYSQL_MONIT_SHELL | awk '{print $2}'`

    #守护脚本和主进程都不在
    if [ -z "$keep_exist" ];then
        manualLogInfo "try to start MySQL:$slave_ip,$MYSQL_MONIT_SHELL" 
        ssh root@$slave_ip nohup /bin/sh $DBHA_SHELL/Service/$MYSQL_MONIT_SHELL > /dev/null 2>&1 &
        #关闭本机的进程
        #sleep 1
        #pid=`(ps -ef | grep "ssh root@$slave_ip nohup /bin/sh $DBHA_SHELL/Service/$MYSQL_MONIT_SHELL" | awk '{print $2}')`
        #if [ ! -z "$pid" ];then
        #    kill -9 $pid
        #fi
    fi
    
    local keep_exist=`ssh root@$slave_ip ps -ef | grep /bin/sh | grep $MYSQL_MONIT_SHELL | awk '{print $2}'`
    if [ ! -z "$keep_exist" ];then
        echo "MySQL:$slave_ip,$MYSQL_MONIT_SHELL : running" 
    else
        echo "MySQL:$slave_ip,$MYSQL_MONIT_SHELL : stopped" 
    fi
}

function stopRemoteMysqlKeep(){
    local try_count=1
    local try_max_count=15
    for((try_count=1;try_count<=try_max_count;try_count++))
    do
        local keep_exist=`ssh root@$slave_ip ps -ef | grep /bin/sh | grep $MYSQL_MONIT_SHELL | awk '{print $2}'`
        if [ ! -z "$keep_exist" ];then
            ssh root@$slave_ip kill -9 $keep_exist
            manualLogInfo "stop $MYSQL_MONIT_SHELL"
            sleep 1
        else
            break
        fi
    done
}

function startMysqlKeep()
{
    # 检查守护脚本和主进程
    local keep_exist=`ps -ef | grep /bin/sh | grep $MYSQL_MONIT_SHELL | awk '{print $2}'`

    #守护脚本和主进程都不在
    if [ -z "$keep_exist" ];then
        manualLogInfo "try to start MySQL:$master_ip,$MYSQL_MONIT_SHELL" 
        nohup /bin/sh $DBHA_SHELL/Service/$MYSQL_MONIT_SHELL > /dev/null 2>&1 &
    fi
    
    local keep_exist=`ps -ef | grep /bin/sh | grep $MYSQL_MONIT_SHELL | awk '{print $2}'`
    if [ ! -z "$keep_exist" ];then
        echo "MySQL:$master_ip,$MYSQL_MONIT_SHELL : running" 
    fi
}

function stopMysqlKeep(){
    local try_count=1
    local try_max_count=15
    for((try_count=1;try_count<=try_max_count;try_count++))
    do
        local keep_exist=`ps -ef | grep /bin/sh | grep $MYSQL_MONIT_SHELL | awk '{print $2}'`
        if [ ! -z "$keep_exist" ];then
            kill -9 $keep_exist
            manualLogInfo "stop $MYSQL_MONIT_SHELL"
            sleep 1
        else
            break
        fi
    done
}

manualLogInfo "begin to start Maunal Operate:$mode:$master_ip $slave_ip"
if [ "$mode" == "cluster" ];then
    checkMysqlState $master_ip
    if [ $? -ne 0 ];then
        manualLogInfo "MySQL:$master_ip is abnormal."
        exit 1
    fi
    
    checkMysqlState $slave_ip
    if [ $? -ne 0 ];then
        manualLogInfo "MySQL:$master_ip is abnormal."
        exit 1
    fi
    
    #如果虚IP存在，则down掉
    downVirtualIP
    
    #关闭主机守护进程
    stopMysqlKeep
    sleep 1
    
    startMaster
    if [ $? -ne 0 ];then
        exit 1
    fi
    #拉起主机守护
    startMysqlKeep
    
    #关闭备机守护
    stopRemoteMysqlKeep
    sleep 1
    startSlave
    if [ $? -ne 0 ];then
        exit 1
    fi
    #拉起备机守护
    startRemoteMysqlKeep
    
    #主备恢复后，拉起虚IP
    upVirtualIP
    
    #备份
    startBackup
else
    checkMysqlState $master_ip
    if [ $? -ne 0 ];then
        exit 1
    fi
    #关闭守护进程
    stopMysqlKeep
    #单机启动
    startMasterSingle
    #拉起守护进程
    startMysqlKeep
    #备份
    startBackup
fi
manualLogInfo "end to start Maunal Operate sucess."