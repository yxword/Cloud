#!/bin/sh
#description: MySQL monitor
# made by mjj 24182 2016-12-8
this=${BASH_SOURCE-$0}
cur_dir=$(cd "$(dirname "$this")"; pwd)
parent_dir=$(dirname $cur_dir)

source $parent_dir/Common/CommonFunc.sh

# 独立服务文件夹
TAG=MysqlKeeper

####################################需根据服务配置#########################################
#mysql服务名
MYSQL_SERVICE=${DBHA_SHELL}/Service/MysqlServer.sh


#####################################################################
mysql_cfg=/etc/my.cnf
#mysql登录的用户名和密码
getUserCfg

function checkSlaveRunning()
{
    io_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_IO_Running" | awk -F": " '{print $2}' )
    slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G" | grep -w "Slave_SQL_Running" | awk -F": " '{print $2}' )
    if [ "$io_status" == "" ];then
        return 0
    elif [[ "$io_status" != "Yes" ]] || [[ "$slave_status" != "Yes" ]];then
        slave_status=$(mysql -u$mysql_user -p$mysql_password -e "show slave status\G")
        logInfo "checkSlaveRunning error:$slave_status"
        mysql -u$mysql_user -p$mysql_password -e "start slave;"
        logInfo "checkSlaveRunning : start slave"
        return 1
    else
        return 0
    fi
}

function checkServer()
{
    #拉起mysql
    sh $MYSQL_SERVICE status
    if [ $? -ne 0 ];then
        sh $MYSQL_SERVICE start
    fi
    
    checkSlaveRunning
}

function check_mysql_bin_index()
{
    sh $MYSQL_SERVICE status
    if [[ $? -eq  0 ]];then
        return 0
    fi
    
    local mysql_path=$(grep datadir /etc/my.cnf | awk -F= '{print $2}' | awk '{print $1}')
    if [ -f $mysql_path/mysql-bin.index ];then
        local binlog_name=`ls -tr $mysql_path | grep "mysql-bin.[0-9]*" | grep -v "mysql-bin.index"`
        local binlog_num=`cat $mysql_path/mysql-bin.index | wc -l `
        
        while [ $binlog_num -gt 0 ] ;do
            local binlog_file=`tail -n 1 $mysql_path/mysql-bin.index | head -n 1 | awk -F '/' '{print $2}'`
            local need_remove=`echo $binlog_name | grep $binlog_file | wc -l`
            sh $MYSQL_SERVICE status
            mysql_dead=$?
            if [ $need_remove -eq 0 ] && [ $mysql_dead -ne 0 ];then
                sed -i "/$binlog_file/d" /data/mysql/data/mysql-bin.index
                try_restart_mysql=1
                redirect "csdriver.sh : remove $binlog_file from mysql-bin.index!"
            else
                break
            fi
            
            binlog_num=`cat $mysql_path/mysql-bin.index | wc -l `
        done
    fi
    
    sh $MYSQL_SERVICE status
    mysql_dead=$?
    if [ $try_restart_mysql == 1 ] && [ $mysql_dead -ne 0 ];then
        sh $MYSQL_SERVICE start
    fi
    return 0
}

#0无需检测，1需要检测
function need_do_check()
{
    #DBHA在恢复和备份时无需进行数据库检测
    #获取恢复脚本进程号
    recover_pid=`ps -ef | grep "Recovery"| grep "start"|grep -v grep | awk '{print $2}'`
    backup_pid=`ps -ef | grep "mysqldump"|grep -v grep | awk '{print $2}'`
    if [[ "$recover_pid" != "" ]] || [[ "$backup_pid" != "" ]];then
        redirect "recover:$recover_pid, backup:$backup_pid"
        return 0
    fi
    
    return 1
}

function check_mysql_health_status()
{
    sh $MYSQL_SERVICE status
    if [[ $? -ne  0 ]];then
        return 0
    fi
    
    need_do_check
    if [[ $? != 0 ]];then
        # 大于1200秒的事物会被kill掉
        python $DBHA_SHELL/Service/MysqlHealthCheck.py -u$mysql_user -p$mysql_password -H localhost -w 1200 -f $DBHA_LOG/mysql_health_check.log
        if [ $? != 0 ];then
            redirect "no mysqldb module installed, install it"
        fi
    fi
}

function monitor()
{
    logInfo "start monitor"
    # 监控主进程
    #cd $DBHA_BIN
    while true
    do
        checkServer
        check_mysql_bin_index
        check_mysql_health_status
       
        sleep 2;
    done
    logInfo "stop monitor"
}

monitor
exit 0

